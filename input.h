//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_INPUT_H
#define SCRATCH_FOP_INPUT_H

#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include "structs.h"
#include "globals.h"


bool point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

bool point_in_block(int px, int py, Block* b) {
    if (!b) return false;
    return point_in_rect(px, py, b->x, b->y, b->w, b->h);
}

// ─── Scroll ──────────────────────────────────────────────
void handle_scroll_value(SDL_Event& e, int& scrollOffset,
                         int scrollSpeed = 20)
{
    if (e.type == SDL_MOUSEWHEEL) {
        scrollOffset += (e.wheel.y > 0) ? scrollSpeed : -scrollSpeed;
        if (scrollOffset > 0) scrollOffset = 0;
    }
}


void handle_snap(Block* dragged, std::vector<Block*>& blocks) {
    if (!dragged) return;


    for (Block* b : blocks) {
        if (b->next == dragged) {
            b->next = nullptr;
        }
    }
    if (dragged->prev) {
        dragged->prev->next = nullptr;
        dragged->prev = nullptr;
    }

    for (Block* target : blocks) {
        if (target == dragged) continue;

        Block* check = target;
        bool circular = false;
        while (check) {
            if (check == dragged) { circular = true; break; }
            check = check->next;
        }
        if (circular) continue;


        int targetBottomX = target->x + 12;
        int targetBottomY = target->y + target->h;
        int draggedTopX  = dragged->x + 12;
        int draggedTopY  = dragged->y;

        double dist = std::sqrt(
            std::pow(targetBottomX - draggedTopX, 2) +
            std::pow(targetBottomY - draggedTopY, 2));

        if (dist < SNAP_DISTANCE) {
            dragged->x = target->x;
            dragged->y = target->y + target->h;
            target->next = dragged;
            dragged->prev = target;


            Block* cur = dragged;
            while (cur->next) {
                cur->next->x = cur->x;
                cur->next->y = cur->y + cur->h;
                cur = cur->next;
            }
            return;
        }
    }
}


void handle_mouse_down(SDL_Event& e, std::vector<Block*>& blocks,
                       Block** draggedBlock)
{
    int mx = e.button.x;
    int my = e.button.y;

    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        if (point_in_block(mx, my, blocks[i])) {
            *draggedBlock = blocks[i];
            (*draggedBlock)->isDragging = true;
            (*draggedBlock)->dragOffsetX = mx - blocks[i]->x;
            (*draggedBlock)->dragOffsetY = my - blocks[i]->y;


            blocks.erase(blocks.begin() + i);
            blocks.push_back(*draggedBlock);
            break;
        }
    }
}

void handle_mouse_up(Block** draggedBlock, std::vector<Block*>& blocks,
                     Workspace& workspace)
{
    if (!*draggedBlock) return;

    (*draggedBlock)->isDragging = false;


    Block* b = *draggedBlock;
    bool inWorkspace = point_in_rect(b->x, b->y,
        workspace.x, workspace.y, workspace.w, workspace.h);

    if (!inWorkspace) {

        for (int i = 0; i < (int)blocks.size(); i++) {
            if (blocks[i] == b) {
                blocks.erase(blocks.begin() + i);
                break;
            }
        }
        delete b;
        *draggedBlock = nullptr;
        return;
    }

    handle_snap(*draggedBlock, blocks);
    *draggedBlock = nullptr;
}


void handle_mouse_motion(SDL_Event& e, Block** draggedBlock,
                         Workspace& workspace)
{
    if (!*draggedBlock) return;

    Block* b = *draggedBlock;
    b->x = e.motion.x - b->dragOffsetX;
    b->y = e.motion.y - b->dragOffsetY;


    if (b->x < workspace.x)
        b->x = workspace.x;
    if (b->x + b->w > workspace.x + workspace.w)
        b->x = workspace.x + workspace.w - b->w;
    if (b->y < workspace.y)
        b->y = workspace.y;
    if (b->y + b->h > workspace.y + workspace.h)
        b->y = workspace.y + workspace.h - b->h;


    Block* cur = b;
    while (cur->next) {
        cur->next->x = cur->x;
        cur->next->y = cur->y + cur->h;
        cur = cur->next;
    }
}


Block* check_palette_click(int mx, int my,
                           std::vector<Block*>& paletteBlocks,
                           Palette& palette)
{
    for (Block* b : paletteBlocks) {

        if (mx >= b->x && mx <= b->x + b->w &&
            my >= b->y && my <= b->y + b->h) {
            return b;
        }
    }
    return nullptr;
}


bool handle_category_click(int mx, int my, Palette& palette) {
    for (auto& cat : palette.categories) {
        if (mx >= cat.rect.x && mx <= cat.rect.x + cat.rect.w &&
            my >= cat.rect.y && my <= cat.rect.y + cat.rect.h) {
            palette.activeCategory = cat.type;
            palette.scrollOffset   = 0;
            return true;
        }
    }
    return false;
}

#endif

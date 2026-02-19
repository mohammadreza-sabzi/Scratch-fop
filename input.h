//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_INPUT_H
#define SCRATCH_FOP_INPUT_H

#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include "structs.h"
#include "globals.h"
#include "utils.h"

void handle_snap(Block* dragged, std::vector<Block*>& blocks) {
    if (!dragged) return;


    if (dragged->prev) {
        dragged->prev->next = nullptr;
        dragged->prev = nullptr;
    }
    if (dragged->next) {
        dragged->next->prev = nullptr;
        dragged->next = nullptr;
    }

    for (Block* b : blocks) {
        if (b == dragged) continue;
        if (b->isDragging)  continue;

        int snapX = b->x;
        int snapY = b->y + b->h;
        int dx = std::abs(dragged->x - snapX);
        int dy = std::abs(dragged->y - snapY);

        if (dx < SNAP_DISTANCE && dy < SNAP_DISTANCE && b->next == nullptr) {
            dragged->x    = b->x;
            dragged->y    = b->y + b->h;
            b->next       = dragged;
            dragged->prev = b;
            return;
        }

        snapY = b->y - dragged->h;
        dy    = std::abs(dragged->y - snapY);
        if (dx < SNAP_DISTANCE && dy < SNAP_DISTANCE && b->prev == nullptr) {
            dragged->x    = b->x;
            dragged->y    = b->y - dragged->h;
            dragged->next = b;
            b->prev       = dragged;
            return;
        }
    }
}

void handle_mouse_down(SDL_Event& e,
                        std::vector<Block*>& blocks,
                        Block** draggedBlock)
{
    int mx = e.button.x;
    int my = e.button.y;

    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        Block* b = blocks[i];
        if (point_in_rect(mx, my, b->x, b->y, b->w, b->h)) {

            if (b->prev) {
                b->prev->next = nullptr;
                b->prev = nullptr;
            }
            b->isDragging  = true;
            b->dragOffsetX = mx - b->x;
            b->dragOffsetY = my - b->y;
            *draggedBlock  = b;

            blocks.erase(blocks.begin() + i);
            blocks.push_back(b);
            return;
        }
    }
}

void handle_mouse_motion(SDL_Event& e,
                          Block** draggedBlock,
                          Workspace& workspace)
{
    if (!*draggedBlock) return;
    Block* b = *draggedBlock;
    b->x = e.motion.x - b->dragOffsetX;
    b->y = e.motion.y - b->dragOffsetY;
}


void handle_mouse_up(Block** draggedBlock,
                      std::vector<Block*>& blocks,
                      Workspace& workspace)
{
    if (!*draggedBlock) return;
    Block* b = *draggedBlock;
    b->isDragging = false;


    bool inWS = point_in_rect(b->x + b->w / 2, b->y + b->h / 2,
        workspace.x, workspace.y,
        workspace.w, workspace.h);

    if (!inWS) {
        blocks.erase(std::remove(blocks.begin(), blocks.end(), b),
                     blocks.end());

        if (b->prev) { b->prev->next = nullptr; }
        if (b->next) { b->next->prev = nullptr; }
        delete b;
        *draggedBlock = nullptr;
        return;
    }

    handle_snap(b, blocks);
    *draggedBlock = nullptr;
}

#endif

//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_UTILS_H
#define SCRATCH_FOP_UTILS_H

#include <vector>
#include <string>
#include <algorithm>
#include "structs.h"
#include "globals.h"

bool point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

Block* clone_block(Block* src) {
    Block* nb = new Block(*src);
    nb->next = nullptr; nb->prev = nullptr; nb->isDragging = false;
    return nb;
}

bool block_matches_category(Block* b, CategoryType cat) {
    switch (cat) {
        case CAT_MOTION:    return b->type == BLOCK_MOTION;
        case CAT_LOOKS:     return b->type == BLOCK_LOOKS;
        case CAT_SOUND:     return b->type == BLOCK_SOUND;
        case CAT_EVENTS:    return b->type == BLOCK_EVENT;
        case CAT_CONTROL:   return b->type == BLOCK_CONTROL;
        case CAT_SENSING:   return b->type == BLOCK_SENSING;
        case CAT_OPERATORS: return b->type == BLOCK_OPERATORS;
        case CAT_VARIABLES: return b->type == BLOCK_VARIABLES;
        case CAT_MYBLOCKS:  return b->type == BLOCK_MYBLOCKS;
        default: return false;
    }
}

void layout_palette_blocks(std::vector<Block*>& blocks, Palette& palette) {
    int y = palette.blockListY + 52 + palette.scrollOffset;
    int x = palette.blockListX + 12;
    for (Block* b : blocks) {
        if (!block_matches_category(b, palette.activeCategory)) continue;
        b->x = x; b->y = y; b->w = BLOCK_W; b->h = BLOCK_H;
        y += BLOCK_H + BLOCK_PADDING;
    }
}

Block* check_palette_click(int mx, int my, std::vector<Block*>& blocks, Palette& palette) {
    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        Block* b = blocks[i];
        if (!block_matches_category(b, palette.activeCategory)) continue;
        if (point_in_rect(mx, my, b->x, b->y, b->w, b->h)) return b;
    }
    return nullptr;
}

void handle_category_click(int mx, int my, Palette& palette) {
    for (auto& cat : palette.categories) {
        if (point_in_rect(mx, my, cat.iconRect.x, cat.iconRect.y, cat.iconRect.w, cat.iconRect.h)) {
            palette.activeCategory = cat.type;
            palette.scrollOffset   = 0;
            break;
        }
    }
}

void handle_scroll_value(SDL_Event& e, int& scrollOffset) {
    scrollOffset += e.wheel.y * 20;
    if (scrollOffset > 0) scrollOffset = 0;
}

#endif
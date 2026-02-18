//
// Created by Domim on 2/18/2026.
//
#ifndef SCRATCH_FOP_UTILS_H
#define SCRATCH_FOP_UTILS_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "globals.h"
#include "structs.h"


bool block_matches_category(Block* b, CategoryType cat) {
    switch (cat) {
        case CAT_MOTION:    return b->type == BLOCK_MOTION;
        case CAT_LOOKS:     return b->type == BLOCK_LOOKS;
        case CAT_EVENTS:    return b->type == BLOCK_EVENT;
        case CAT_CONTROL:   return b->type == BLOCK_CONTROL;
        case CAT_SOUND:     return b->type == BLOCK_SOUND;
        case CAT_SENSING:   return b->type == BLOCK_SENSING;
        case CAT_OPERATORS: return b->type == BLOCK_OPERATORS;
        case CAT_VARIABLES: return b->type == BLOCK_VARIABLES;
        default:            return false;
    }
}

static int NEXT_ID = 1000;

Block* clone_block(Block* src) {
    if (!src) return nullptr;
    return new Block{
        NEXT_ID++,
        src->type,
        src->text,
        src->x, src->y,
        src->w, src->h,
        false,
        0, 0,
        nullptr, // next
        nullptr  // prev
    };
}


void layout_palette_blocks(std::vector<Block*>& blocks,
                           Palette& palette)
{

    int catBottom = 0;
    for (auto& cat : palette.categories) {
        int bottom = cat.rect.y + cat.rect.h;
        if (bottom > catBottom) catBottom = bottom;
    }

    int y = catBottom + 12 + palette.scrollOffset;
    const int spacing = BLOCK_PADDING;

    for (Block* b : blocks) {
        if (!block_matches_category(b, palette.activeCategory)) continue;

        b->x = palette.x + 16;
        b->y = y;
        b->w = palette.w - 32;
        b->h = BLOCK_H;

        y += b->h + spacing;
    }
}

Block* find_script_start(std::vector<Block*>& blocks) {
    for (Block* b : blocks) {
        if (b->type == BLOCK_EVENT && b->prev == nullptr) {
            return b;
        }
    }
    return nullptr;
}

#endif

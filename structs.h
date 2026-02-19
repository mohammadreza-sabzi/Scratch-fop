//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_STRUCTS_H
#define SCRATCH_FOP_STRUCTS_H

#include <string>
#include <vector>
#include <SDL2/SDL.h>

enum BlockType {
    BLOCK_EVENT,
    BLOCK_MOTION,
    BLOCK_LOOKS,
    BLOCK_CONTROL,
    BLOCK_SOUND,
    BLOCK_SENSING,
    BLOCK_OPERATORS,
    BLOCK_VARIABLES,
    BLOCK_MYBLOCKS
};

enum CategoryType {
    CAT_MOTION,
    CAT_LOOKS,
    CAT_SOUND,
    CAT_EVENTS,
    CAT_CONTROL,
    CAT_SENSING,
    CAT_OPERATORS,
    CAT_VARIABLES,
    CAT_MYBLOCKS
};

struct Block {
    int id;
    BlockType type;
    std::string text;
    int x, y, w, h;
    bool isDragging;
    int dragOffsetX, dragOffsetY;
    Block* next;
    Block* prev;
};

struct Stage {
    int x, y, w, h;
    SDL_Color color;
};

struct Sprite {
    float x, y;
    int w, h;
    SDL_Texture* texture;
    bool visible;
    std::string sayText;
    int sayTimer;
};

struct Workspace {
    int x, y, w, h;
};


struct Category {
    CategoryType type;
    std::string  name;
    SDL_Color    color;
    SDL_Rect     iconRect;
};

struct Palette {
    std::vector<Category> categories;
    CategoryType activeCategory;
    int scrollOffset;
    // ابعاد دو پانل
    int catBarX, catBarY, catBarW, catBarH;
    int blockListX, blockListY, blockListW, blockListH;
};

#endif

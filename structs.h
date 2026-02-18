//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_STRUCTS_H
#define SCRATCH_FOP_STRUCTS_H

#include <string>
#include <vector>
#include <SDL2/SDL.h>

// انواع بلوک‌ها برای رنگ‌بندی مختلف
enum BlockType {
    BLOCK_EVENT,   // زرد (شروع)
    BLOCK_MOTION,  // آبی (حرکت)
    BLOCK_LOOKS,   // بنفش (ظاهر)
    BLOCK_CONTROL  // نارنجی (حلقه‌ها)
    BLOCK_LOOKS,
    BLOCK_CONTROL,
    BLOCK_SOUND,
    BLOCK_SENSING,
    BLOCK_OPERATORS,
    BLOCK_VARIABLES
};

enum CategoryType {
    CAT_MOTION,
    CAT_LOOKS,
    CAT_SOUND,
    CAT_EVENTS,
    CAT_CONTROL,
    CAT_SENSING,
    CAT_OPERATORS,
    CAT_VARIABLES
};

struct Block {
    int id;
    BlockType type;
    std::string text;

    int x, y;
    int w, h;

    bool isDragging;
    int dragOffsetX, dragOffsetY;

    Block* next;
    Block* prev;
};

struct Stage {
    int x, y;
    int w, h;
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
    std::string name;
    SDL_Rect rect;
    SDL_Color color;
};

struct Palette {
    std::vector<Category> categories;
    CategoryType activeCategory;
    int scrollOffset;
    int x, y, w, h;
};

#endif

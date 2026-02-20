//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_STRUCTS_H
#define SCRATCH_FOP_STRUCTS_H

#include <string>
#include <vector>
#include <map>
#include <SDL2/SDL.h>

enum BlockType {
    BLOCK_EVENT, BLOCK_MOTION, BLOCK_LOOKS, BLOCK_CONTROL,
    BLOCK_SOUND, BLOCK_SENSING, BLOCK_OPERATORS, BLOCK_VARIABLES, BLOCK_MYBLOCKS
};

enum CategoryType {
    CAT_MOTION, CAT_LOOKS, CAT_SOUND, CAT_EVENTS, CAT_CONTROL,
    CAT_SENSING, CAT_OPERATORS, CAT_VARIABLES, CAT_MYBLOCKS
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

struct Costume {
    std::string  name;
    SDL_Texture* texture = nullptr;
    int w = 48, h = 48;
};

struct Sprite {
    float x = 0, y = 0;
    float direction = 90.0f;
    float scale = 1.0f;
    int   w = 96, h = 96;
    SDL_Texture* texture = nullptr;
    bool  visible = true;
    std::string sayText;
    int   sayTimer = 0;
    int   currentCostume = 0;
    std::vector<Costume> costumes;
};

struct Workspace { int x, y, w, h; };

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
    int catBarX, catBarY, catBarW, catBarH;
    int blockListX, blockListY, blockListW, blockListH;
};

struct CostumePanel {
    int x, y, w, h;
    int scrollOffset = 0;
    int selectedIndex = 0;
    bool visible = true;
};

// Variable definition
struct Variable {
    std::string name;
    float       value = 0.0f;
    bool        showOnStage = true;
};

// Variables panel
struct VariablesPanel {
    int x, y, w, h;
    int scrollOffset = 0;
    bool visible = true;
    std::vector<Variable> variables;
    bool creating = false;      // Are we in name-entry mode?
    std::string newVarName;     // Buffer for new variable name
};

#endif
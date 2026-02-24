#ifndef SCRATCH_FOP_STRUCTS_H
#define SCRATCH_FOP_STRUCTS_H
using namespace std;
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

enum BlockType {
    BLOCK_EVENT, BLOCK_MOTION, BLOCK_LOOKS, BLOCK_CONTROL,
    BLOCK_SOUND, BLOCK_SENSING, BLOCK_OPERATORS, BLOCK_VARIABLES, BLOCK_MYBLOCKS
};

enum CategoryType {
    CAT_MOTION, CAT_LOOKS, CAT_SOUND, CAT_EVENTS, CAT_CONTROL,
    CAT_SENSING, CAT_OPERATORS, CAT_VARIABLES, CAT_MYBLOCKS
};

struct Block; // forward declaration

// نوع slot: numeric () یا boolean <>
enum SlotType { SLOT_NUMERIC, SLOT_BOOLEAN };

// ورودی قابل ویرایش داخل بلوک
struct BlockInput {
    string value = "0";
    bool editing = false;
    SDL_Rect rect = {0, 0, 0, 0};
    int index = 0;
    SlotType slotType = SLOT_NUMERIC;   // نوع slot
    Block* embeddedBlock = nullptr;      // بلاک اپراتور جاسازی‌شده
};

struct Block {
    int id;
    BlockType type;
    string text;
    int x, y, w, h;
    bool isDragging;
    int dragOffsetX, dragOffsetY;
    Block* next;      // بلاک بعدی در زنجیر اصلی
    Block* prev;      // بلاک قبلی در زنجیر اصلی
    std::vector<BlockInput> inputs;

    // ── C-shaped block support ───────────────────────────────────────────────
    bool   isCShaped    = false;   // آیا این بلاک C-شکل است؟
    Block* innerFirst   = nullptr; // اولین بلاک داخل C
    Block* innerLast    = nullptr; // آخرین بلاک داخل C  (برای snap)
    int    innerH       = 36;      // ارتفاع فضای داخلی C (حداقل 36)
    // برای بلاک if/else دو بخش داریم
    Block* elseFirst    = nullptr;
    int    elseH        = 36;
    bool   hasElse      = false;
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
    std::string name = "Sprite1";   // نام کاراکتر
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

struct Variable {
    std::string name;
    float       value = 0.0f;
    bool        showOnStage = true;
};

struct VariablesPanel {
    int x, y, w, h;
    int scrollOffset = 0;
    bool visible = true;
    std::vector<Variable> variables;
    bool creating = false;
    std::string newVarName;
};

struct BlockToken {
    string text;
    bool   editable;
    int    x, w;
};

struct SoundClip {
    std::string name;
    std::string filePath;
    Mix_Chunk*  chunk       = nullptr;
    float       durationSecs = 0.0f;
    int         channel     = -1;
    bool        isPlaying   = false;
    float       volume      = 100.0f; // 0-100
};

struct SoundsPanel {
    int  x = 0, y = 0, w = 0, h = 0;
    bool visible = true;
    int  scrollOffset  = 0;
    int  selectedIndex = -1;
    std::vector<SoundClip> sounds;

    // upload dialog state
    bool        uploadDialogOpen = false;
    std::string uploadPathInput;
    bool        uploadEditing    = false;
};

#endif

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
};

struct Block {
    int id;
    BlockType type;
    std::string text; // متنی که روی بلوک نوشته می‌شود

    // مختصات و ابعاد
    int x, y;
    int w, h;

    // وضعیت درگ شدن
    bool isDragging;
    int dragOffsetX, dragOffsetY; // فاصله موس تا گوشه بلوک هنگام درگ کردن

    // اتصال به بلوک بعدی
    Block* next;
};

// یک لیست جهانی از تمام بلوک‌های موجود در صفحه

#endif //SCRATCH_FOP_STRUCTS_H
//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_INPUT_H
#define SCRATCH_FOP_INPUT_H

#include <complex>
#include <SDL2/SDL.h>
#include <string>
#include "structs.h"
const int Snap_distance = 30;

// مدیریت تایپ کردن متن (اضافه کردن حروف و Backspace)
void handle_typing(SDL_Event& e, std::string& targetString) {
    if (e.type == SDL_TEXTINPUT) {
        // جلوگیری از کاراکترهای خاص اگر نیاز بود
        targetString += e.text.text;
    }
    else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && targetString.length() > 0) {
            targetString.pop_back();
        }
    }
}

// مدیریت اسکرول موس
void handle_scroll_value(SDL_Event& e, int& scrollOffset, int scrollSpeed = 20) {
    if (e.type == SDL_MOUSEWHEEL) {
        if (e.wheel.y > 0) { // Scroll Up
            scrollOffset += scrollSpeed;
        } else if (e.wheel.y < 0) { // Scroll Down
            scrollOffset -= scrollSpeed;
        }
    }
}


// بررسی برخورد نقطه (موس) با مستطیل (بلوک)
bool point_in_rect(int px, int py, Block* b) {
    return (px >= b->x && px <= b->x + b->w && py >= b->y && py <= b->y + b->h);
}

// مدیریت کلیک موس
void handle_mouse_down(SDL_Event& e, std::vector<Block*>& blocks, Block** draggedBlock) {
    int mx = e.button.x;
    int my = e.button.y;

    // حلقه برعکس: از آخرین بلوک (روترین) شروع می‌کنیم
    // تا اگر دو بلوک روی هم بودند، اونی که بالاست انتخاب شود
    for (int i = blocks.size() - 1; i >= 0; i--) {
        if (point_in_rect(mx, my, blocks[i])) {
            *draggedBlock = blocks[i];
            (*draggedBlock)->isDragging = true;

            // محاسبه فاصله موس تا گوشه چپ-بالای بلوک
            (*draggedBlock)->dragOffsetX = mx - blocks[i]->x;
            (*draggedBlock)->dragOffsetY = my - blocks[i]->y;
            break; // فقط یک بلوک را بردار
        }
    }
}
void handle_snap(Block* dragged, std::vector<Block*>& blocks) {
    if (!dragged){return;}
    for (Block* target: blocks) {
        if (target==dragged) {
            continue;
        }
        int targetBottomX=target->x;
        int targetBottomY=target->y + target->h;
        int draggedTopX=dragged->x;
        int draggedTopY=dragged->y;
        double distance= sqrt(pow(targetBottomX- draggedTopX, 2) + pow(targetBottomY- draggedTopY, 2));
        if (distance < Snap_distance) {
            dragged->x = targetBottomX;
            dragged->y = targetBottomY;
            target->next=dragged;
            return;
        }
    }
}

// مدیریت رها کردن موس
void handle_mouse_up(Block** draggedBlock, std::vector<Block*>& blocks) {
    if (*draggedBlock) {
        (*draggedBlock)->isDragging = false;
        handle_snap(*draggedBlock, blocks);
        *draggedBlock = nullptr; // هیچ بلوکی دیگر در حال درگ نیست
    }
}

// مدیریت حرکت موس
void handle_mouse_motion(SDL_Event& e, Block** draggedBlock) {
    if (*draggedBlock) {
        // تغییر مکان بلوک بر اساس مکان جدید موس منهای آفست اولیه
        (*draggedBlock)->x = e.motion.x - (*draggedBlock)->dragOffsetX;
        (*draggedBlock)->y = e.motion.y - (*draggedBlock)->dragOffsetY;
        Block* current=*draggedBlock;
        while (current->next!=nullptr) {
            current->next->x=current->x;
            current->next->y=current->y + current->h;
            current=current->next;
        }
    }
}

#endif //SCRATCH_FOP_INPUT_H
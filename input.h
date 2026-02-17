//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_INPUT_H
#define SCRATCH_FOP_INPUT_H

#include <SDL2/SDL.h>
#include <string>

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

#endif //SCRATCH_FOP_INPUT_H
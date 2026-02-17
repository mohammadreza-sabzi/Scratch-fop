//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_RENDER_H
#define SCRATCH_FOP_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

// رسم متن روی صفحه با فونت مشخص
void render_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty() || font == nullptr) return;

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!textSurface) return;

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// رسم یک دکمه یا بلوک ساده (مستطیل توپر + حاشیه)
void render_rect_with_border(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color fillParams, SDL_Color borderParams) {
    SDL_Rect rect = {x, y, w, h};

    // توپر
    SDL_SetRenderDrawColor(renderer, fillParams.r, fillParams.g, fillParams.b, fillParams.a);
    SDL_RenderFillRect(renderer, &rect);

    // حاشیه
    SDL_SetRenderDrawColor(renderer, borderParams.r, borderParams.g, borderParams.b, borderParams.a);
    SDL_RenderDrawRect(renderer, &rect);
}


// تابع کمکی برای تنظیم رنگ بر اساس نوع بلوک
void set_block_color(SDL_Renderer* renderer, BlockType type) {
    switch (type) {
        case BLOCK_MOTION:  SDL_SetRenderDrawColor(renderer, 76, 151, 255, 255); break; // آبی
        case BLOCK_EVENT:   SDL_SetRenderDrawColor(renderer, 255, 191, 0, 255); break;  // زرد
        case BLOCK_LOOKS:   SDL_SetRenderDrawColor(renderer, 153, 102, 255, 255); break; // بنفش
        case BLOCK_CONTROL: SDL_SetRenderDrawColor(renderer, 255, 171, 25, 255); break; // نارنجی
        default:            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); break; // خاکستری
    }
}

// رسم متن (از کدهای قبلی شما با کمی اصلاح)
void draw_text(SDL_Renderer* renderer, TTF_Font* font, std::string text, int x, int y) {
    if (text.empty() || !font) return;
    SDL_Color color = {255, 255, 255, 255}; // متن سفید
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

// تابع اصلی رسم بلوک
void draw_block(SDL_Renderer* renderer, TTF_Font* font, Block* block) {
    if (!block) return;

    // 1. تنظیم رنگ و رسم بدنه اصلی
    set_block_color(renderer, block->type);
    SDL_Rect rect = {block->x, block->y, block->w, block->h};
    SDL_RenderFillRect(renderer, &rect);

    // 2. رسم حاشیه (Border)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50); // مشکی کمرنگ
    SDL_RenderDrawRect(renderer, &rect);

    // 3. نوشتن متن وسط بلوک
    draw_text(renderer, font, block->text, block->x + 10, block->y + 10);
}

#endif //SCRATCH_FOP_RENDER_H
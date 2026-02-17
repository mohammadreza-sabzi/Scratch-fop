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

#endif //SCRATCH_FOP_RENDER_H
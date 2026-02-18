//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_RENDER_H
#define SCRATCH_FOP_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfx.h>
#include <string>
#include "structs.h"
#include "globals.h"


void draw_text(SDL_Renderer* renderer, TTF_Font* font,
               const std::string& text, int x, int y,
               SDL_Color color = {255, 255, 255, 255})
{
    if (text.empty() || !font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        SDL_Rect r = {x, y, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, nullptr, &r);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}


SDL_Color get_block_color(BlockType type) {
    switch (type) {
        case BLOCK_MOTION:    return COLOR_MOTION;
        case BLOCK_LOOKS:     return COLOR_LOOKS;
        case BLOCK_SOUND:     return COLOR_SOUND;
        case BLOCK_EVENT:     return COLOR_EVENTS;
        case BLOCK_CONTROL:   return COLOR_CONTROL;
        case BLOCK_SENSING:   return COLOR_SENSING;
        case BLOCK_OPERATORS: return COLOR_OPERATORS;
        case BLOCK_VARIABLES: return COLOR_VARIABLES;
        default:              return {100, 100, 100, 255};
    }
}


SDL_Color darken(SDL_Color c, int amount = 40) {
    return {
        (Uint8)std::max(0, (int)c.r - amount),
        (Uint8)std::max(0, (int)c.g - amount),
        (Uint8)std::max(0, (int)c.b - amount),
        255
    };
}

void draw_block(SDL_Renderer* renderer, TTF_Font* font, Block* block,
                bool isGhost = false)
{
    if (!block) return;

    SDL_Color col = get_block_color(block->type);
    SDL_Color shadow = darken(col, 50);

    int x = block->x, y = block->y, w = block->w, h = block->h;

    if (isGhost) {
        col.a = 120;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    // سایه پایین
    SDL_SetRenderDrawColor(renderer, shadow.r, shadow.g, shadow.b, col.a);
    SDL_Rect shadowRect = {x + 2, y + 3, w, h};
    SDL_RenderFillRect(renderer, &shadowRect);

    // بدنه اصلی بلوک
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_Rect mainRect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &mainRect);


    if (block->type != BLOCK_EVENT) {
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_Rect notchTop = {x + 12, y - 4, 20, 5};
        SDL_RenderFillRect(renderer, &notchTop);
    }


    {
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_Rect notchBot = {x + 12, y + h, 20, 5};
        SDL_RenderFillRect(renderer, &notchBot);
    }

    // حاشیه تیره‌تر
    SDL_SetRenderDrawColor(renderer, shadow.r, shadow.g, shadow.b, 255);
    SDL_RenderDrawRect(renderer, &mainRect);


    if (block->type == BLOCK_EVENT) {
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);

        SDL_Rect hatTop = {x, y - 10, w, 14};
        SDL_RenderFillRect(renderer, &hatTop);

        for (int i = 0; i < 6; i++) {
            SDL_Rect r = {x + i, y - 14 + i, w - i * 2, 2};
            SDL_RenderFillRect(renderer, &r);
        }
    }


    if (font) {
        int textY = y + (h - 16) / 2;
        draw_text(renderer, font, block->text, x + 10, textY, COLOR_TEXT_WHITE);
    }

    if (isGhost) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}


void draw_stage(SDL_Renderer* renderer, Stage* stage) {
    if (!stage) return;


    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect r = {stage->x, stage->y, stage->w, stage->h};
    SDL_RenderFillRect(renderer, &r);


    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &r);


    SDL_SetRenderDrawColor(renderer, 224, 224, 224, 255);
    SDL_Rect statusBar = {stage->x, stage->y + stage->h, stage->w, 40};
    SDL_RenderFillRect(renderer, &statusBar);
}


void draw_sprite(SDL_Renderer* renderer, Sprite* sprite, Stage* stage) {
    if (!sprite || !sprite->texture || !sprite->visible) return;


    int sx = stage->x + (int)sprite->x;
    int sy = stage->y + (int)sprite->y;

    SDL_Rect r = {sx, sy, sprite->w, sprite->h};
    SDL_RenderCopy(renderer, sprite->texture, nullptr, &r);


    if (!sprite->sayText.empty() && sprite->sayTimer > 0) {
        int bx = sx + sprite->w;
        int by = sy - 40;
        int bw = (int)sprite->sayText.size() * 9 + 20;
        int bh = 34;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect bubble = {bx, by, bw, bh};
        SDL_RenderFillRect(renderer, &bubble);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &bubble);
    }
}


void draw_categories(SDL_Renderer* renderer, TTF_Font* font,
                     Palette& palette)
{
    for (auto& cat : palette.categories) {
        bool active = (cat.type == palette.activeCategory);

        SDL_Color bg = active ? cat.color : SDL_Color{220, 220, 220, 255};
        SDL_Color textCol = active ? COLOR_TEXT_WHITE : COLOR_TEXT_DARK;


        SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, 255);
        SDL_RenderFillRect(renderer, &cat.rect);

        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        SDL_RenderDrawRect(renderer, &cat.rect);

        if (!active) {
            SDL_SetRenderDrawColor(renderer, cat.color.r, cat.color.g,
                                   cat.color.b, 255);
            SDL_Rect dot = {cat.rect.x + 8, cat.rect.y + 10, 14, 14};
            SDL_RenderFillRect(renderer, &dot);
        }

        draw_text(renderer, font, cat.name,
                  cat.rect.x + (active ? 12 : 28),
                  cat.rect.y + (cat.rect.h - 16) / 2,
                  textCol);
    }
}


void draw_toolbar(SDL_Renderer* renderer, TTF_Font* font,
                  SDL_Rect& playBtn, SDL_Rect& stopBtn)
{

    SDL_SetRenderDrawColor(renderer, COLOR_BG_TOOLBAR.r,
                           COLOR_BG_TOOLBAR.g,
                           COLOR_BG_TOOLBAR.b, 255);
    SDL_Rect bar = {0, 0, SCREEN_WIDTH, TOOLBAR_HEIGHT};
    SDL_RenderFillRect(renderer, &bar);


    draw_text(renderer, font, "Scratch C++ Engine",
              10, 12, COLOR_TEXT_WHITE);

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    SDL_RenderFillRect(renderer, &playBtn);
    draw_text(renderer, font, "▶", playBtn.x + 8, playBtn.y + 8);


    SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
    SDL_RenderFillRect(renderer, &stopBtn);
    draw_text(renderer, font, "■", stopBtn.x + 8, stopBtn.y + 8);
}


void draw_workspace_bg(SDL_Renderer* renderer, Workspace& ws) {
    // پس‌زمینه
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect r = {ws.x, ws.y, ws.w, ws.h};
    SDL_RenderFillRect(renderer, &r);

    // نقاط Grid
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    for (int x = ws.x; x < ws.x + ws.w; x += 24) {
        for (int y = ws.y; y < ws.y + ws.h; y += 24) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    // حاشیه چپ
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawLine(renderer, ws.x, ws.y, ws.x, ws.y + ws.h);
}


void draw_palette_bg(SDL_Renderer* renderer, Palette& palette) {
    SDL_SetRenderDrawColor(renderer, COLOR_BG_PALETTE.r,
                           COLOR_BG_PALETTE.g,
                           COLOR_BG_PALETTE.b, 255);
    SDL_Rect r = {palette.x, palette.y, palette.w, palette.h};
    SDL_RenderFillRect(renderer, &r);


    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawLine(renderer,
        palette.x + palette.w, palette.y,
        palette.x + palette.w, palette.y + palette.h);
}

#endif

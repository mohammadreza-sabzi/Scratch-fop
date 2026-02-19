//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_RENDER_H
#define SCRATCH_FOP_RENDER_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <string>
#include <algorithm>
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

void draw_text_centered(SDL_Renderer* renderer, TTF_Font* font,
                        const std::string& text, SDL_Rect rect,
                        SDL_Color color = {255,255,255,255})
{
    if (text.empty() || !font) return;
    int tw, th;
    TTF_SizeUTF8(font, text.c_str(), &tw, &th);
    int tx = rect.x + (rect.w - tw) / 2;
    int ty = rect.y + (rect.h - th) / 2;
    draw_text(renderer, font, text, tx, ty, color);
}


void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int r,
                        SDL_Color col)
{
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)std::sqrt((double)(r*r - dy*dy));
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int r,
                         SDL_Color col)
{
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int angle = 0; angle < 360; angle++) {
        double rad = angle * M_PI / 180.0;
        int px = cx + (int)(r * std::cos(rad));
        int py = cy + (int)(r * std::sin(rad));
        SDL_RenderDrawPoint(renderer, px, py);
    }
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
        case BLOCK_MYBLOCKS:  return COLOR_MYBLOCKS;
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


void draw_notch(SDL_Renderer* renderer, int bx, int by, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    // ذوزنقه کوچک: 4px ارتفاع
    for (int i = 0; i < 4; i++) {
        SDL_RenderDrawLine(renderer,
            bx + 10 + i, by - i,
            bx + 30 - i, by - i);
    }
}

void draw_block(SDL_Renderer* renderer, TTF_Font* font, Block* block,
                bool isGhost = false)
{
    if (!block) return;

    SDL_Color col    = get_block_color(block->type);
    SDL_Color shadow = darken(col, 50);

    if (isGhost) {
        col.a = 140;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    int x = block->x, y = block->y, w = block->w, h = block->h;

    SDL_SetRenderDrawColor(renderer,
        shadow.r, shadow.g, shadow.b, isGhost ? 80 : 255);
    SDL_Rect shadowR = {x + 2, y + 3, w, h};
    SDL_RenderFillRect(renderer, &shadowR);

    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_Rect mainR = {x, y, w, h};
    SDL_RenderFillRect(renderer, &mainR);

    if (block->type != BLOCK_EVENT) {
        draw_notch(renderer, x, y, col);
    }

    {
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        for (int i = 0; i < 4; i++) {
            SDL_RenderDrawLine(renderer,
                x + 10 + i, y + h + i,
                x + 30 - i, y + h + i);
        }
    }


    if (block->type == BLOCK_EVENT) {
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

        for (int i = 0; i < 12; i++) {
            SDL_Rect hatR = {x + i/2, y - 12 + i, w - i, 4};
            SDL_RenderFillRect(renderer, &hatR);
        }
    }

    SDL_SetRenderDrawColor(renderer, shadow.r, shadow.g, shadow.b,
                           isGhost ? 80 : 255);
    SDL_RenderDrawRect(renderer, &mainR);

    if (font) {
        int textY = y + (h - 14) / 2;
        draw_text(renderer, font, block->text, x + 10, textY,
                  COLOR_TEXT_WHITE);
    }

    if (isGhost)
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}


void draw_category_bar(SDL_Renderer* renderer, TTF_Font* font,
                       Palette& palette)
{
    SDL_SetRenderDrawColor(renderer,
        COLOR_BG_CATBAR.r, COLOR_BG_CATBAR.g, COLOR_BG_CATBAR.b, 255);
    SDL_Rect barRect = {
        palette.catBarX, palette.catBarY,
        palette.catBarW, palette.catBarH
    };
    SDL_RenderFillRect(renderer, &barRect);


    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawLine(renderer,
        palette.catBarX + palette.catBarW, palette.catBarY,
        palette.catBarX + palette.catBarW,
        palette.catBarY + palette.catBarH);

    for (auto& cat : palette.categories) {
        bool active = (cat.type == palette.activeCategory);

        int cx = cat.iconRect.x + cat.iconRect.w / 2;
        int cy = cat.iconRect.y + cat.iconRect.h / 2 - 8;


        if (active) {
            SDL_SetRenderDrawColor(renderer, 230, 230, 240, 255);
            SDL_Rect hl = {
                palette.catBarX, cat.iconRect.y - 5,
                palette.catBarW, CAT_ITEM_H
            };
            SDL_RenderFillRect(renderer, &hl);


            SDL_SetRenderDrawColor(renderer,
                cat.color.r, cat.color.g, cat.color.b, 255);
            SDL_Rect line = {palette.catBarX, cat.iconRect.y - 5, 3, CAT_ITEM_H};
            SDL_RenderFillRect(renderer, &line);
        }


        draw_filled_circle(renderer, cx, cy, CAT_CIRCLE_R, cat.color);


        if (active) {
            draw_circle_outline(renderer, cx, cy, CAT_CIRCLE_R + 2,
                                {255,255,255,255});
            draw_circle_outline(renderer, cx, cy, CAT_CIRCLE_R + 3,
                                {180,180,180,255});
        }


        if (font) {
            int tw, th;
            TTF_SizeUTF8(font, cat.name.c_str(), &tw, &th);
            int tx = cx - tw / 2;
            int ty = cy + CAT_CIRCLE_R + 4;
            draw_text(renderer, font, cat.name, tx, ty, COLOR_TEXT_DARK);
        }
    }
}


void draw_block_list_header(SDL_Renderer* renderer, TTF_Font* fontBig,
                            Palette& palette,
                            const std::string& activeCatName,
                            SDL_Color activeCatColor)
{

    SDL_SetRenderDrawColor(renderer,
        COLOR_BG_BLOCKLIST.r, COLOR_BG_BLOCKLIST.g,
        COLOR_BG_BLOCKLIST.b, 255);
    SDL_Rect listRect = {
        palette.blockListX, palette.blockListY,
        palette.blockListW, palette.blockListH
    };
    SDL_RenderFillRect(renderer, &listRect);


    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawLine(renderer,
        palette.blockListX + palette.blockListW, palette.blockListY,
        palette.blockListX + palette.blockListW,
        palette.blockListY + palette.blockListH);


    if (fontBig) {
        draw_text(renderer, fontBig, activeCatName,
                  palette.blockListX + 12,
                  palette.blockListY + 12,
                  activeCatColor);
    }
}

void draw_stage(SDL_Renderer* renderer, Stage* stage) {
    if (!stage) return;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect r = {stage->x, stage->y, stage->w, stage->h};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
    SDL_RenderDrawRect(renderer, &r);


    SDL_SetRenderDrawColor(renderer, 224, 224, 224, 255);
    SDL_Rect statusBar = {stage->x, stage->y + stage->h,
                          stage->w, SCREEN_HEIGHT - stage->h};
    SDL_RenderFillRect(renderer, &statusBar);
}


void draw_sprite(SDL_Renderer* renderer, Sprite* sprite,
                 Stage* stage, TTF_Font* font)
{
    if (!sprite || !sprite->visible) return;

    int sx = stage->x + (int)sprite->x;
    int sy = stage->y + (int)sprite->y;

    if (sprite->texture) {
        SDL_Rect r = {sx, sy, sprite->w, sprite->h};
        SDL_RenderCopy(renderer, sprite->texture, nullptr, &r);
    } else {

        SDL_SetRenderDrawColor(renderer, 74, 144, 226, 255);
        SDL_Rect r = {sx, sy, 48, 48};
        SDL_RenderFillRect(renderer, &r);
    }


    if (!sprite->sayText.empty() && sprite->sayTimer > 0 && font) {
        int bx = sx + sprite->w + 6;
        int by = sy - 44;
        int tw, th;
        TTF_SizeUTF8(font, sprite->sayText.c_str(), &tw, &th);
        int bw = tw + 20;
        int bh = th + 14;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect bubble = {bx, by, bw, bh};
        SDL_RenderFillRect(renderer, &bubble);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(renderer, &bubble);
        draw_text(renderer, font, sprite->sayText,
                  bx + 10, by + 7, COLOR_TEXT_DARK);
    }
}


void draw_workspace_bg(SDL_Renderer* renderer, Workspace& ws) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect r = {ws.x, ws.y, ws.w, ws.h};
    SDL_RenderFillRect(renderer, &r);

    // نقاط Grid
    SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
    for (int x = ws.x + 12; x < ws.x + ws.w; x += 24)
        for (int y = ws.y + 12; y < ws.y + ws.h; y += 24)
            SDL_RenderDrawPoint(renderer, x, y);
}


void draw_play_stop_buttons(SDL_Renderer* renderer, TTF_Font* font,
                             SDL_Rect& playBtn, SDL_Rect& stopBtn,
                             SDL_Texture* playTex, SDL_Texture* stopTex,
                             bool isRunning)
{
    // Play
    if (playTex) {
        SDL_RenderCopy(renderer, playTex, nullptr, &playBtn);
    } else {
        // Fallback: مثلث سبز
        SDL_SetRenderDrawColor(renderer, isRunning ? 0 : 80,
                               isRunning ? 200 : 220,
                               isRunning ? 0 : 80, 255);
        SDL_RenderFillRect(renderer, &playBtn);
        // مثلث
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < playBtn.h / 2 - 4; i++) {
            SDL_RenderDrawLine(renderer,
                playBtn.x + 8 + i,
                playBtn.y + 4 + i,
                playBtn.x + 8 + i,
                playBtn.y + playBtn.h - 4 - i);
        }
    }

    // Stop
    if (stopTex) {
        SDL_RenderCopy(renderer, stopTex, nullptr, &stopBtn);
    } else {

        SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
        SDL_RenderFillRect(renderer, &stopBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect inner = {
            stopBtn.x + 7, stopBtn.y + 7,
            stopBtn.w - 14, stopBtn.h - 14
        };
        SDL_RenderFillRect(renderer, &inner);
    }
}

#endif

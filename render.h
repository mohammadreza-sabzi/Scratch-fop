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

void draw_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
               int x, int y, SDL_Color color = {255,255,255,255})
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

void draw_text_centered(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                        SDL_Rect rect, SDL_Color color = {255,255,255,255})
{
    if (text.empty() || !font) return;
    int tw, th;
    TTF_SizeUTF8(font, text.c_str(), &tw, &th);
    draw_text(renderer, font, text, rect.x + (rect.w - tw)/2, rect.y + (rect.h - th)/2, color);
}

void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int r, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)std::sqrt((double)(r*r - dy*dy));
        SDL_RenderDrawLine(renderer, cx-dx, cy+dy, cx+dx, cy+dy);
    }
}

void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int r, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int a = 0; a < 360; a++) {
        double rad = a * M_PI / 180.0;
        SDL_RenderDrawPoint(renderer, cx+(int)(r*std::cos(rad)), cy+(int)(r*std::sin(rad)));
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
        default: return {100,100,100,255};
    }
}

SDL_Color darken(SDL_Color c, int amt = 40) {
    return {(Uint8)std::max(0,(int)c.r-amt),(Uint8)std::max(0,(int)c.g-amt),
            (Uint8)std::max(0,(int)c.b-amt),255};
}

void draw_rounded_rect(SDL_Renderer* renderer, SDL_Rect r, int radius, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_Rect inner_h = {r.x + radius, r.y, r.w - 2*radius, r.h};
    SDL_Rect inner_v = {r.x, r.y + radius, r.w, r.h - 2*radius};
    SDL_RenderFillRect(renderer, &inner_h);
    SDL_RenderFillRect(renderer, &inner_v);
    int cx[4] = {r.x+radius, r.x+r.w-radius-1, r.x+radius, r.x+r.w-radius-1};
    int cy[4] = {r.y+radius, r.y+radius, r.y+r.h-radius-1, r.y+r.h-radius-1};
    for (int i = 0; i < 4; i++)
        for (int dy = -radius; dy <= radius; dy++) {
            int dx = (int)std::sqrt((double)(radius*radius - dy*dy));
            SDL_RenderDrawLine(renderer, cx[i]-dx, cy[i]+dy, cx[i]+dx, cy[i]+dy);
        }
}

void draw_notch(SDL_Renderer* renderer, int bx, int by, SDL_Color col) {
    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int i = 0; i < 4; i++)
        SDL_RenderDrawLine(renderer, bx+10+i, by-i, bx+30-i, by-i);
}

void draw_block(SDL_Renderer* renderer, TTF_Font* font, Block* block, bool isGhost = false) {
    if (!block) return;
    SDL_Color col    = get_block_color(block->type);
    SDL_Color shadow = darken(col, 50);
    if (isGhost) { col.a = 140; SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); }

    int x = block->x, y = block->y, w = block->w, h = block->h;

    SDL_SetRenderDrawColor(renderer, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 255);
    SDL_Rect shadowR = {x+2, y+3, w, h};
    SDL_RenderFillRect(renderer, &shadowR);

    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    SDL_Rect mainR = {x, y, w, h};
    SDL_RenderFillRect(renderer, &mainR);

    if (block->type != BLOCK_EVENT) draw_notch(renderer, x, y, col);

    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int i = 0; i < 4; i++)
        SDL_RenderDrawLine(renderer, x+10+i, y+h+i, x+30-i, y+h+i);

    if (block->type == BLOCK_EVENT) {
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        for (int i = 0; i < 12; i++) {
            SDL_Rect hatR = {x+i/2, y-12+i, w-i, 4};
            SDL_RenderFillRect(renderer, &hatR);
        }
    }

    SDL_SetRenderDrawColor(renderer, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 255);
    SDL_RenderDrawRect(renderer, &mainR);

    if (font) draw_text(renderer, font, block->text, x+10, y+(h-14)/2, COLOR_TEXT_WHITE);
    if (isGhost) SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void draw_category_bar(SDL_Renderer* renderer, TTF_Font* font, Palette& palette) {
    SDL_SetRenderDrawColor(renderer, COLOR_BG_CATBAR.r, COLOR_BG_CATBAR.g, COLOR_BG_CATBAR.b, 255);
    SDL_Rect barRect = {palette.catBarX, palette.catBarY, palette.catBarW, palette.catBarH};
    SDL_RenderFillRect(renderer, &barRect);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawLine(renderer, palette.catBarX+palette.catBarW, palette.catBarY,
                                 palette.catBarX+palette.catBarW, palette.catBarY+palette.catBarH);

    for (auto& cat : palette.categories) {
        bool active = (cat.type == palette.activeCategory);
        int cx = cat.iconRect.x + cat.iconRect.w / 2;
        int cy = cat.iconRect.y + cat.iconRect.h / 2 - 8;

        if (active) {
            SDL_SetRenderDrawColor(renderer, 230, 230, 240, 255);
            SDL_Rect hl = {palette.catBarX, cat.iconRect.y-5, palette.catBarW, CAT_ITEM_H};
            SDL_RenderFillRect(renderer, &hl);
            SDL_SetRenderDrawColor(renderer, cat.color.r, cat.color.g, cat.color.b, 255);
            SDL_Rect line = {palette.catBarX, cat.iconRect.y-5, 3, CAT_ITEM_H};
            SDL_RenderFillRect(renderer, &line);
        }

        draw_filled_circle(renderer, cx, cy, CAT_CIRCLE_R, cat.color);

        if (active) {
            draw_circle_outline(renderer, cx, cy, CAT_CIRCLE_R+2, {255,255,255,255});
            draw_circle_outline(renderer, cx, cy, CAT_CIRCLE_R+3, {180,180,180,255});
        }

        if (font) {
            int tw, th;
            TTF_SizeUTF8(font, cat.name.c_str(), &tw, &th);
            draw_text(renderer, font, cat.name, cx-tw/2, cy+CAT_CIRCLE_R+4, COLOR_TEXT_DARK);
        }
    }
}

void draw_block_list_header(SDL_Renderer* renderer, TTF_Font* fontBig,
                            Palette& palette, const std::string& activeCatName,
                            SDL_Color activeCatColor)
{
    SDL_SetRenderDrawColor(renderer, COLOR_BG_BLOCKLIST.r, COLOR_BG_BLOCKLIST.g,
                           COLOR_BG_BLOCKLIST.b, 255);
    SDL_Rect listRect = {palette.blockListX, palette.blockListY,
                         palette.blockListW, palette.blockListH};
    SDL_RenderFillRect(renderer, &listRect);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawLine(renderer,
        palette.blockListX+palette.blockListW, palette.blockListY,
        palette.blockListX+palette.blockListW, palette.blockListY+palette.blockListH);

    if (fontBig)
        draw_text(renderer, fontBig, activeCatName,
                  palette.blockListX+12, palette.blockListY+12, activeCatColor);
}

void draw_stage(SDL_Renderer* renderer, Stage* stage) {
    if (!stage) return;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect r = {stage->x, stage->y, stage->w, stage->h};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
    SDL_RenderDrawRect(renderer, &r);
}

void draw_sprite(SDL_Renderer* renderer, Sprite* sprite, Stage* stage, TTF_Font* font) {
    if (!sprite || !sprite->visible) return;

    int sw = (int)(sprite->w * sprite->scale);
    int sh = (int)(sprite->h * sprite->scale);
    int sx = stage->x + (int)sprite->x;
    int sy = stage->y + (int)sprite->y;

    SDL_Rect stageClip = {stage->x, stage->y, stage->w, stage->h};
    SDL_RenderSetClipRect(renderer, &stageClip);

    if (sprite->texture) {
        SDL_Rect r = {sx, sy, sw, sh};
        SDL_RenderCopyEx(renderer, sprite->texture, nullptr, &r,
                         sprite->direction - 90.0, nullptr, SDL_FLIP_NONE);
    } else {
        SDL_SetRenderDrawColor(renderer, 74, 144, 226, 255);
        SDL_Rect r = {sx, sy, sw, sh};
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect e1 = {sx+sw/4-4, sy+sh/3-4, 8, 8};
        SDL_Rect e2 = {sx+3*sw/4-4, sy+sh/3-4, 8, 8};
        SDL_RenderFillRect(renderer, &e1);
        SDL_RenderFillRect(renderer, &e2);
    }
    SDL_RenderSetClipRect(renderer, nullptr);

    if (!sprite->sayText.empty() && sprite->sayTimer > 0 && font) {
        int tw, th;
        TTF_SizeUTF8(font, sprite->sayText.c_str(), &tw, &th);
        int bw = tw+24, bh = th+16;
        int bx = sx+sw+8, by = sy-bh-8;
        if (bx+bw > stage->x+stage->w) bx = sx-bw-8;
        if (by < stage->y) by = sy+sh+8;
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 160);
        SDL_Rect shad = {bx+2, by+2, bw, bh};
        SDL_RenderFillRect(renderer, &shad);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect bub = {bx, by, bw, bh};
        SDL_RenderFillRect(renderer, &bub);
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderDrawRect(renderer, &bub);
        draw_text(renderer, font, sprite->sayText, bx+12, by+(bh-th)/2, COLOR_TEXT_DARK);
    }
}

void draw_workspace_bg(SDL_Renderer* renderer, Workspace& ws) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect r = {ws.x, ws.y, ws.w, ws.h};
    SDL_RenderFillRect(renderer, &r);
    SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
    for (int x = ws.x+12; x < ws.x+ws.w; x += 24)
        for (int y = ws.y+12; y < ws.y+ws.h; y += 24)
            SDL_RenderDrawPoint(renderer, x, y);
}

void draw_play_stop_buttons(SDL_Renderer* renderer, TTF_Font* font,
                             SDL_Rect& playBtn, SDL_Rect& stopBtn,
                             SDL_Texture* playTex, SDL_Texture* stopTex,
                             bool isRunning)
{
    if (playTex) {
        SDL_RenderCopy(renderer, playTex, nullptr, &playBtn);
    } else {
        SDL_SetRenderDrawColor(renderer, isRunning ? 0:80, isRunning ? 200:220, isRunning ? 0:80, 255);
        SDL_RenderFillRect(renderer, &playBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < playBtn.h/2-4; i++)
            SDL_RenderDrawLine(renderer, playBtn.x+8+i, playBtn.y+4+i,
                               playBtn.x+8+i, playBtn.y+playBtn.h-4-i);
    }
    if (stopTex) {
        SDL_RenderCopy(renderer, stopTex, nullptr, &stopBtn);
    } else {
        SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
        SDL_RenderFillRect(renderer, &stopBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect inner = {stopBtn.x+7, stopBtn.y+7, stopBtn.w-14, stopBtn.h-14};
        SDL_RenderFillRect(renderer, &inner);
    }
}

void draw_costume_panel(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontBig,
                        CostumePanel& panel, Sprite* sprite)
{
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_Rect bg = {panel.x, panel.y, panel.w, panel.h};
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawLine(renderer, panel.x, panel.y, panel.x+panel.w, panel.y);

    SDL_SetRenderDrawColor(renderer, COLOR_LOOKS.r, COLOR_LOOKS.g, COLOR_LOOKS.b, 255);
    SDL_Rect header = {panel.x, panel.y, panel.w, 30};
    SDL_RenderFillRect(renderer, &header);
    if (fontBig) draw_text_centered(renderer, fontBig, "Costumes", header, COLOR_TEXT_WHITE);

    if (!sprite || sprite->costumes.empty()) {
        if (font) draw_text(renderer, font, "No costumes loaded",
                            panel.x+10, panel.y+40, COLOR_TEXT_DARK);
        return;
    }

    int startY = panel.y + 36 + panel.scrollOffset;
    int itemH  = COSTUME_THUMB + 28;
    int itemW  = panel.w - 16;
    int ix     = panel.x + 8;

    for (int i = 0; i < (int)sprite->costumes.size(); i++) {
        int iy = startY + i * (itemH + 6);
        if (iy + itemH < panel.y || iy > panel.y + panel.h) continue;

        bool selected = (i == sprite->currentCostume);

        if (selected) {
            SDL_SetRenderDrawColor(renderer, 200, 210, 255, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        SDL_Rect card = {ix, iy, itemW, itemH};
        SDL_RenderFillRect(renderer, &card);

        SDL_SetRenderDrawColor(renderer, selected ? 100 : 200,
                               selected ? 140 : 200,
                               selected ? 255 : 200, 255);
        SDL_RenderDrawRect(renderer, &card);

        SDL_Rect thumbArea = {ix+4, iy+4, COSTUME_THUMB, COSTUME_THUMB};
        if (sprite->costumes[i].texture) {
            SDL_RenderCopy(renderer, sprite->costumes[i].texture, nullptr, &thumbArea);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
            SDL_RenderFillRect(renderer, &thumbArea);
            if (font) draw_text_centered(renderer, font, "?", thumbArea, COLOR_TEXT_DARK);
        }

        if (font) {
            std::string label = std::to_string(i+1) + ". " + sprite->costumes[i].name;
            draw_text(renderer, font, label, ix+COSTUME_THUMB+10, iy+itemH/2-7, COLOR_TEXT_DARK);
        }
    }
}

#endif

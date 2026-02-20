//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_RENDER_H
#define SCRATCH_FOP_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "structs.h"
#include "globals.h"


void draw_text(SDL_Renderer* r, TTF_Font* font, const std::string& text,
               int x, int y, SDL_Color color = {255,255,255,255})
{
    if (text.empty() || !font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect rc = {x, y, surf->w, surf->h};
        SDL_RenderCopy(r, tex, nullptr, &rc);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void draw_text_centered(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                        SDL_Rect rect, SDL_Color color = {255,255,255,255})
{
    if (text.empty() || !font) return;
    int tw, th;
    TTF_SizeUTF8(font, text.c_str(), &tw, &th);
    draw_text(r, font, text, rect.x + (rect.w - tw)/2, rect.y + (rect.h - th)/2, color);
}

void draw_filled_circle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)std::sqrt((double)(radius*radius - dy*dy));
        SDL_RenderDrawLine(r, cx-dx, cy+dy, cx+dx, cy+dy);
    }
}

void draw_circle_outline(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    for (int a = 0; a < 360; a++) {
        double rad = a * M_PI / 180.0;
        SDL_RenderDrawPoint(r, cx+(int)(radius*std::cos(rad)), cy+(int)(radius*std::sin(rad)));
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
    return {(Uint8)std::max(0,(int)c.r-amt),
            (Uint8)std::max(0,(int)c.g-amt),
            (Uint8)std::max(0,(int)c.b-amt),255};
}

void draw_rounded_rect(SDL_Renderer* r, SDL_Rect rc, int radius, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_Rect h = {rc.x+radius, rc.y, rc.w-2*radius, rc.h};
    SDL_Rect v = {rc.x, rc.y+radius, rc.w, rc.h-2*radius};
    SDL_RenderFillRect(r, &h);
    SDL_RenderFillRect(r, &v);
    int cx[4] = {rc.x+radius, rc.x+rc.w-radius-1, rc.x+radius, rc.x+rc.w-radius-1};
    int cy[4] = {rc.y+radius, rc.y+radius, rc.y+rc.h-radius-1, rc.y+rc.h-radius-1};
    for (int i = 0; i < 4; i++)
        for (int dy = -radius; dy <= radius; dy++) {
            int dx = (int)std::sqrt((double)(radius*radius - dy*dy));
            SDL_RenderDrawLine(r, cx[i]-dx, cy[i]+dy, cx[i]+dx, cy[i]+dy);
        }
}


void draw_block(SDL_Renderer* r, TTF_Font* font, Block* block, bool isGhost = false) {
    if (!block) return;
    SDL_Color col    = get_block_color(block->type);
    SDL_Color shadow = darken(col, 50);
    if (isGhost) { col.a = 140; SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND); }

    int x = block->x, y = block->y, w = block->w, h = block->h;

    SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 220);
    SDL_Rect shadowR = {x+2, y+3, w, h};
    SDL_RenderFillRect(r, &shadowR);

    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_Rect mainR = {x, y, w, h};
    SDL_RenderFillRect(r, &mainR);

    if (block->type != BLOCK_EVENT) {
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        for (int i = 0; i < 4; i++)
            SDL_RenderDrawLine(r, x+10+i, y-i, x+30-i, y-i);
    }

    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    for (int i = 0; i < 4; i++)
        SDL_RenderDrawLine(r, x+10+i, y+h+i, x+30-i, y+h+i);

    if (block->type == BLOCK_EVENT) {
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        for (int i = 0; i < 14; i++) {
            SDL_Rect hatR = {x+i/2, y-14+i, w-i, 4};
            SDL_RenderFillRect(r, &hatR);
        }
    }

    SDL_Color hl = {(Uint8)std::min(255,(int)col.r+50),
                    (Uint8)std::min(255,(int)col.g+50),
                    (Uint8)std::min(255,(int)col.b+50), (Uint8)(isGhost?80:255)};
    SDL_SetRenderDrawColor(r, hl.r, hl.g, hl.b, hl.a);
    SDL_RenderDrawLine(r, x, y, x+w, y);

    SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 200);
    SDL_RenderDrawRect(r, &mainR);

    if (font) draw_text(r, font, block->text, x+10, y+(h-14)/2, COLOR_TEXT_WHITE);
    if (isGhost) SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}


void draw_category_bar(SDL_Renderer* r, TTF_Font* font, Palette& palette) {
    SDL_SetRenderDrawColor(r, COLOR_BG_CATBAR.r, COLOR_BG_CATBAR.g, COLOR_BG_CATBAR.b, 255);
    SDL_Rect barRect = {palette.catBarX, palette.catBarY, palette.catBarW, palette.catBarH};
    SDL_RenderFillRect(r, &barRect);

    SDL_SetRenderDrawColor(r, 210, 210, 210, 255);
    SDL_RenderDrawLine(r, palette.catBarX+palette.catBarW-1, 0,
                          palette.catBarX+palette.catBarW-1, SCREEN_HEIGHT);

    for (auto& cat : palette.categories) {
        bool active = (cat.type == palette.activeCategory);
        int cx = palette.catBarX + palette.catBarW / 2;
        int cy = cat.iconRect.y + cat.iconRect.h / 2 - 10;

        if (active) {
            SDL_SetRenderDrawColor(r, 235, 235, 248, 255);
            SDL_Rect hl = {palette.catBarX, cat.iconRect.y-2, palette.catBarW, CAT_ITEM_H};
            SDL_RenderFillRect(r, &hl);
            SDL_SetRenderDrawColor(r, cat.color.r, cat.color.g, cat.color.b, 255);
            SDL_Rect line = {palette.catBarX, cat.iconRect.y-2, 4, CAT_ITEM_H};
            SDL_RenderFillRect(r, &line);
        }

        draw_filled_circle(r, cx, cy, CAT_CIRCLE_R + 2, {40, 40, 40, 255});
        draw_filled_circle(r, cx, cy, CAT_CIRCLE_R, cat.color);

        if (active) {
            draw_circle_outline(r, cx, cy, CAT_CIRCLE_R+3, {255,255,255,255});
            draw_circle_outline(r, cx, cy, CAT_CIRCLE_R+4, {200,200,220,255});
        }

        if (font) {
            int tw, th;
            TTF_SizeUTF8(font, cat.name.c_str(), &tw, &th);
            SDL_Color textCol = active ? COLOR_TEXT_DARK : SDL_Color{100,100,100,255};
            draw_text(r, font, cat.name, cx-tw/2, cy+CAT_CIRCLE_R+5, textCol);
        }
    }
}


void draw_block_list_header(SDL_Renderer* r, TTF_Font* fontBig,
                            Palette& palette, const std::string& activeCatName,
                            SDL_Color activeCatColor)
{
    SDL_SetRenderDrawColor(r, COLOR_BG_BLOCKLIST.r, COLOR_BG_BLOCKLIST.g,
                           COLOR_BG_BLOCKLIST.b, 255);
    SDL_Rect listRect = {palette.blockListX, palette.blockListY,
                         palette.blockListW, palette.blockListH};
    SDL_RenderFillRect(r, &listRect);

    SDL_SetRenderDrawColor(r, activeCatColor.r, activeCatColor.g, activeCatColor.b, 40);
    SDL_Rect hdr = {palette.blockListX, 0, palette.blockListW, 44};
    SDL_RenderFillRect(r, &hdr);

    SDL_SetRenderDrawColor(r, activeCatColor.r, activeCatColor.g, activeCatColor.b, 255);
    SDL_Rect accentLine = {palette.blockListX, 42, palette.blockListW, 3};
    SDL_RenderFillRect(r, &accentLine);

    SDL_SetRenderDrawColor(r, 210, 210, 210, 255);
    SDL_RenderDrawLine(r, palette.blockListX+palette.blockListW-1, 0,
                          palette.blockListX+palette.blockListW-1, SCREEN_HEIGHT);

    if (fontBig)
        draw_text(r, fontBig, activeCatName,
                  palette.blockListX+12, 12, activeCatColor);
}


void draw_stage(SDL_Renderer* r, Stage* stage) {
    if (!stage) return;
    // subtle checkerboard background
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_Rect rc = {stage->x, stage->y, stage->w, stage->h};
    SDL_RenderFillRect(r, &rc);

    SDL_SetRenderDrawColor(r, 230, 230, 230, 255);
    for (int x = stage->x+20; x < stage->x+stage->w; x+=40)
        for (int y = stage->y+20; y < stage->y+stage->h; y+=40)
            SDL_RenderDrawPoint(r, x, y);

    SDL_SetRenderDrawColor(r, 160, 160, 160, 255);
    SDL_RenderDrawRect(r, &rc);
}


void draw_sprite(SDL_Renderer* r, Sprite* sprite, Stage* stage, TTF_Font* font) {
    if (!sprite || !sprite->visible) return;

    int sw = (int)(sprite->w * sprite->scale);
    int sh = (int)(sprite->h * sprite->scale);
    int sx = stage->x + (int)sprite->x;
    int sy = stage->y + (int)sprite->y;

    SDL_Rect stageClip = {stage->x, stage->y, stage->w, stage->h};
    SDL_RenderSetClipRect(r, &stageClip);

    if (sprite->texture) {
        SDL_Rect rc = {sx, sy, sw, sh};
        SDL_RenderCopyEx(r, sprite->texture, nullptr, &rc,
                         sprite->direction - 90.0, nullptr, SDL_FLIP_NONE);
    } else {
        SDL_SetRenderDrawColor(r, 74, 144, 226, 255);
        SDL_Rect body = {sx+4, sy+sh/3, sw-8, sh*2/3-2};
        SDL_RenderFillRect(r, &body);
        SDL_SetRenderDrawColor(r, 50, 120, 200, 255);
        SDL_Rect head = {sx+sw/4, sy, sw/2, sh/3+4};
        SDL_RenderFillRect(r, &head);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_Rect e1 = {sx+sw/4+3, sy+6, 6, 6};
        SDL_Rect e2 = {sx+sw*3/4-9, sy+6, 6, 6};
        SDL_RenderFillRect(r, &e1);
        SDL_RenderFillRect(r, &e2);
    }
    SDL_RenderSetClipRect(r, nullptr);

    if (!sprite->sayText.empty() && sprite->sayTimer > 0 && font) {
        int tw, th;
        TTF_SizeUTF8(font, sprite->sayText.c_str(), &tw, &th);
        int bw = tw+24, bh = th+16;
        int bx = sx+sw+8, by = sy-bh-8;
        if (bx+bw > stage->x+stage->w) bx = sx-bw-8;
        if (by < stage->y) by = sy+sh+8;
        SDL_SetRenderDrawColor(r, 180, 180, 180, 160);
        SDL_Rect shad = {bx+2, by+2, bw, bh};
        SDL_RenderFillRect(r, &shad);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_Rect bub = {bx, by, bw, bh};
        SDL_RenderFillRect(r, &bub);
        SDL_SetRenderDrawColor(r, 150, 150, 150, 255);
        SDL_RenderDrawRect(r, &bub);
        draw_text(r, font, sprite->sayText, bx+12, by+(bh-th)/2, COLOR_TEXT_DARK);
    }
}


void draw_workspace_bg(SDL_Renderer* r, Workspace& ws) {
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_Rect rc = {ws.x, ws.y, ws.w, ws.h};
    SDL_RenderFillRect(r, &rc);
    SDL_SetRenderDrawColor(r, 215, 215, 215, 255);
    for (int x = ws.x+12; x < ws.x+ws.w; x += 24)
        for (int y = ws.y+12; y < ws.y+ws.h; y += 24)
            SDL_RenderDrawPoint(r, x, y);
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_Rect border = {ws.x, ws.y, ws.w, ws.h};
    SDL_RenderDrawRect(r, &border);
}



void draw_play_stop_buttons(SDL_Renderer* r, TTF_Font* font,
                             SDL_Rect& playBtn, SDL_Rect& stopBtn,
                             SDL_Texture* playTex, SDL_Texture* stopTex,
                             bool isRunning)
{
    if (playTex) {
        SDL_RenderCopy(r, playTex, nullptr, &playBtn);
    } else {
        SDL_Color gc = isRunning ? SDL_Color{30,180,30,255} : SDL_Color{60,200,60,255};
        SDL_SetRenderDrawColor(r, gc.r, gc.g, gc.b, 255);
        SDL_RenderFillRect(r, &playBtn);
        SDL_SetRenderDrawColor(r, 255,255,255,255);
        int px = playBtn.x+10, py = playBtn.y+8, ph = playBtn.h-16;
        for (int i = 0; i < ph/2; i++)
            SDL_RenderDrawLine(r, px+i, py+i, px+i, py+ph-i);
        SDL_SetRenderDrawColor(r, 30,150,30,255);
        SDL_RenderDrawRect(r, &playBtn);
    }
    if (stopTex) {
        SDL_RenderCopy(r, stopTex, nullptr, &stopBtn);
    } else {
        SDL_SetRenderDrawColor(r, 220, 50, 50, 255);
        SDL_RenderFillRect(r, &stopBtn);
        SDL_SetRenderDrawColor(r, 255,255,255,255);
        SDL_Rect inner = {stopBtn.x+7, stopBtn.y+7, stopBtn.w-14, stopBtn.h-14};
        SDL_RenderFillRect(r, &inner);
        SDL_SetRenderDrawColor(r, 180,30,30,255);
        SDL_RenderDrawRect(r, &stopBtn);
    }
}


void draw_costume_panel(SDL_Renderer* r, TTF_Font* font, TTF_Font* fontBig,
                        CostumePanel& panel, Sprite* sprite)
{
    SDL_SetRenderDrawColor(r, 245, 245, 248, 255);
    SDL_Rect bg = {panel.x, panel.y, panel.w, panel.h};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 180, 180, 200, 255);
    SDL_RenderDrawLine(r, panel.x, panel.y, panel.x+panel.w, panel.y);

    SDL_SetRenderDrawColor(r, COLOR_LOOKS.r, COLOR_LOOKS.g, COLOR_LOOKS.b, 255);
    SDL_Rect header = {panel.x, panel.y, panel.w, 30};
    SDL_RenderFillRect(r, &header);
    if (fontBig) draw_text_centered(r, fontBig, "Costumes", header, COLOR_TEXT_WHITE);

    if (!sprite || sprite->costumes.empty()) {
        if (font) draw_text(r, font, "No costumes", panel.x+10, panel.y+40, COLOR_TEXT_DARK);
        return;
    }

    SDL_Rect clip = {panel.x, panel.y+30, panel.w, panel.h-30};
    SDL_RenderSetClipRect(r, &clip);

    int startY = panel.y + 36 + panel.scrollOffset;
    int thumbW = COSTUME_THUMB;
    int itemH  = thumbW + 24;
    int itemW  = panel.w - 12;
    int ix     = panel.x + 6;

    for (int i = 0; i < (int)sprite->costumes.size(); i++) {
        int iy = startY + i * (itemH + 4);
        if (iy + itemH < panel.y+30 || iy > panel.y + panel.h) continue;

        bool selected = (i == sprite->currentCostume);
        SDL_Color bg2 = selected ? SDL_Color{200,210,255,255} : SDL_Color{255,255,255,255};
        SDL_SetRenderDrawColor(r, bg2.r, bg2.g, bg2.b, 255);
        SDL_Rect card = {ix, iy, itemW, itemH};
        SDL_RenderFillRect(r, &card);

        SDL_SetRenderDrawColor(r, selected ? 80:200, selected ? 100:200, selected ? 220:200, 255);
        SDL_RenderDrawRect(r, &card);

        SDL_Rect thumbArea = {ix+4, iy+4, thumbW, thumbW};
        if (sprite->costumes[i].texture) {
            SDL_RenderCopy(r, sprite->costumes[i].texture, nullptr, &thumbArea);
        } else {
            SDL_SetRenderDrawColor(r, 200, 200, 220, 255);
            SDL_RenderFillRect(r, &thumbArea);
            if (font) draw_text_centered(r, font, "?", thumbArea, COLOR_TEXT_DARK);
        }

        if (font) {
            std::string label = std::to_string(i+1) + ". " + sprite->costumes[i].name;
            draw_text(r, font, label, ix+thumbW+10, iy+itemH/2-7, COLOR_TEXT_DARK);
        }
    }

    SDL_RenderSetClipRect(r, nullptr);
}


void draw_sprite_info_panel(SDL_Renderer* r, TTF_Font* font, TTF_Font* fontBig,
                             Sprite* sprite)
{
    int px = SPRITE_INFO_X, py = SPRITE_INFO_Y;
    int pw = SPRITE_INFO_W, ph = SPRITE_INFO_H;

    SDL_SetRenderDrawColor(r, 245, 245, 248, 255);
    SDL_Rect bg = {px, py, pw, ph};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 180, 180, 200, 255);
    SDL_RenderDrawLine(r, px, py, px+pw, py);

    SDL_SetRenderDrawColor(r, COLOR_SCRATCH_PURPLE.r, COLOR_SCRATCH_PURPLE.g,
                           COLOR_SCRATCH_PURPLE.b, 255);
    SDL_Rect header = {px, py, pw, 30};
    SDL_RenderFillRect(r, &header);
    if (fontBig) draw_text_centered(r, fontBig, "Sprite Info", header, COLOR_TEXT_WHITE);

    if (!sprite || !font) return;

    auto fmt = [](float v) -> std::string {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << v;
        return ss.str();
    };

    int tx = px + 10, ty = py + 38, lineH = 20;

    float scrX = sprite->x - STAGE_WIDTH/2.0f + sprite->w/2.0f;
    float scrY = -(sprite->y - STAGE_HEIGHT/2.0f + sprite->h/2.0f);

    auto drawRow = [&](const std::string& label, const std::string& val) {
        draw_text(r, font, label, tx, ty, {120,120,120,255});
        draw_text(r, font, val,   tx+60, ty, COLOR_TEXT_DARK);
        ty += lineH;
    };

    drawRow("x:",     fmt(scrX));
    drawRow("y:",     fmt(scrY));
    drawRow("dir:",   fmt(sprite->direction) + "°");
    drawRow("size:",  fmt(sprite->scale * 100.0f) + "%");
    drawRow("shown:", sprite->visible ? "yes" : "no");
    drawRow("costume:", std::to_string(sprite->currentCostume + 1));
}


void draw_variables_panel(SDL_Renderer* r, TTF_Font* font, TTF_Font* fontBig,
                          VariablesPanel& vp, Workspace& ws)
{
    if (!vp.visible) return;

    int px = ws.x + ws.w - VAR_PANEL_W - 10;
    int py = ws.y + 10;
    int pw = VAR_PANEL_W;

    int rowH  = 22;
    int ph    = 34 + (int)vp.variables.size() * rowH + 32 + 4;
    if (ph < 70) ph = 70;
    if (ph > VAR_PANEL_H) ph = VAR_PANEL_H;

    SDL_SetRenderDrawColor(r, 0, 0, 0, 40);
    SDL_Rect shadow = {px+3, py+3, pw, ph};
    SDL_RenderFillRect(r, &shadow);

    SDL_SetRenderDrawColor(r, 250, 250, 255, 255);
    SDL_Rect bg = {px, py, pw, ph};
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 200, 200, 220, 255);
    SDL_RenderDrawRect(r, &bg);

    SDL_SetRenderDrawColor(r, COLOR_VARIABLES.r, COLOR_VARIABLES.g, COLOR_VARIABLES.b, 255);
    SDL_Rect hdr = {px, py, pw, 28};
    SDL_RenderFillRect(r, &hdr);
    if (fontBig) draw_text(r, fontBig, "Variables", px+8, py+6, COLOR_TEXT_WHITE);

    int iy = py + 34;
    for (auto& v : vp.variables) {
        SDL_SetRenderDrawColor(r, COLOR_VARIABLES.r, COLOR_VARIABLES.g, COLOR_VARIABLES.b, 200);
        SDL_Rect chip = {px+6, iy+2, pw-12, rowH-4};
        SDL_RenderFillRect(r, &chip);
        if (font) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << v.value;
            std::string display = v.name + " = " + ss.str();
            draw_text(r, font, display, px+10, iy+4, COLOR_TEXT_WHITE);
        }
        iy += rowH;
    }

    int btnY = py + ph - 28;
    SDL_SetRenderDrawColor(r, 220, 100, 40, 255);
    SDL_Rect btn = {px+6, btnY, pw-12, 22};
    SDL_RenderFillRect(r, &btn);
    SDL_SetRenderDrawColor(r, 180, 70, 20, 255);
    SDL_RenderDrawRect(r, &btn);
    if (font) draw_text_centered(r, font, "+ Make a Variable", btn, COLOR_TEXT_WHITE);

    vp.x = px; vp.y = py; vp.w = pw; vp.h = ph;
}


void draw_variable_monitors(SDL_Renderer* r, TTF_Font* font,
                             VariablesPanel& vp, Stage* stage)
{
    if (!stage || !font) return;
    int mx = stage->x + 4;
    int my = stage->y + 4;
    for (auto& v : vp.variables) {
        if (!v.showOnStage) continue;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << v.value;
        std::string display = v.name + " = " + ss.str();
        int tw, th;
        TTF_SizeUTF8(font, display.c_str(), &tw, &th);

        SDL_SetRenderDrawColor(r, 200, 100, 40, 200);
        SDL_Rect bg2 = {mx, my, tw+12, th+6};
        SDL_RenderFillRect(r, &bg2);
        SDL_SetRenderDrawColor(r, 150, 70, 20, 255);
        SDL_RenderDrawRect(r, &bg2);
        draw_text(r, font, display, mx+6, my+3, COLOR_TEXT_WHITE);

        my += th + 10;
    }
}

#endif
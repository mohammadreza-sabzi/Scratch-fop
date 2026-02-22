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

void draw_tab_bar(SDL_Renderer* r, TTF_Font* font,
                  AppTab activeTab,
                  SDL_Rect tabRects[TAB_COUNT])   // OUT – filled by this fn
{
    SDL_SetRenderDrawColor(r, COLOR_TAB_BG.r, COLOR_TAB_BG.g, COLOR_TAB_BG.b, 255);
    SDL_Rect strip = {0, 0, PALETTE_WIDTH, TAB_BAR_H};
    SDL_RenderFillRect(r, &strip);

    const char* labels[TAB_COUNT] = { "Code", "Costumes", "Sounds" };

    const int PADDING_X = 16;
    int totalNeeded = 0;
    int textWidths[TAB_COUNT];
    for (int i = 0; i < TAB_COUNT; i++) {
        int tw = 60, th = 0;
        if (font) TTF_SizeUTF8(font, labels[i], &tw, &th);
        textWidths[i] = tw;
        totalNeeded += tw + PADDING_X * 2;
    }

    int tabW[TAB_COUNT];
    int remaining = PALETTE_WIDTH;
    for (int i = 0; i < TAB_COUNT; i++) {
        tabW[i] = (i < TAB_COUNT - 1)
                  ? (textWidths[i] + PADDING_X * 2)
                  : remaining;
        remaining -= tabW[i];
    }
    if (totalNeeded <= PALETTE_WIDTH) {
        int evenW = PALETTE_WIDTH / TAB_COUNT;
        for (int i = 0; i < TAB_COUNT; i++) tabW[i] = evenW;
        tabW[TAB_COUNT-1] = PALETTE_WIDTH - evenW * (TAB_COUNT-1);
    }

    int xCursor = 0;
    for (int i = 0; i < TAB_COUNT; i++) {
        bool active = (i == (int)activeTab);
        int  tw     = tabW[i];
        int  tabTop = active ? 2 : 5;

        SDL_Rect tab = {xCursor, tabTop, tw, TAB_BAR_H - tabTop};
        tabRects[i]  = {xCursor, 0, tw, TAB_BAR_H};


        SDL_Color bgCol = active ? COLOR_TAB_ACTIVE : COLOR_TAB_INACTIVE;
        SDL_SetRenderDrawColor(r, bgCol.r, bgCol.g, bgCol.b, 255);
        SDL_RenderFillRect(r, &tab);

        const int R = 6;
        SDL_Color tl = {
            (Uint8)std::min(255, (int)bgCol.r + 20),
            (Uint8)std::min(255, (int)bgCol.g + 20),
            (Uint8)std::min(255, (int)bgCol.b + 20), 255
        };

        for (int dy = 0; dy <= R; dy++) {
            int dx = (int)std::sqrt((double)(R*R - dy*dy));
            SDL_SetRenderDrawColor(r, bgCol.r, bgCol.g, bgCol.b, 255);
            SDL_RenderDrawLine(r,
                tab.x, tab.y + R - dy,
                tab.x + R - dx, tab.y + R - dy);

            SDL_SetRenderDrawColor(r, COLOR_TAB_BG.r, COLOR_TAB_BG.g, COLOR_TAB_BG.b, 255);
            SDL_RenderDrawLine(r,
                tab.x, tab.y + R - dy,
                tab.x + R - dx - 1, tab.y + R - dy);
        }

        SDL_SetRenderDrawColor(r, bgCol.r, bgCol.g, bgCol.b, 255);
        SDL_Rect body1 = {tab.x + R, tab.y,     tab.w - 2*R, tab.h};
        SDL_Rect body2 = {tab.x,     tab.y + R, tab.w,        tab.h - R};
        SDL_RenderFillRect(r, &body1);
        SDL_RenderFillRect(r, &body2);


        if (!active) {
            SDL_SetRenderDrawColor(r, 160, 130, 200, 255);
            SDL_RenderDrawLine(r, tab.x, tab.y, tab.x + tab.w - 1, tab.y);      // top
            SDL_RenderDrawLine(r, tab.x, tab.y, tab.x,              tab.y + tab.h); // left
            SDL_RenderDrawLine(r, tab.x + tab.w - 1, tab.y,
                                  tab.x + tab.w - 1, tab.y + tab.h); // right
        } else {

            SDL_SetRenderDrawColor(r, COLOR_SCRATCH_PURPLE.r,
                                      COLOR_SCRATCH_PURPLE.g,
                                      COLOR_SCRATCH_PURPLE.b, 255);
            SDL_RenderDrawLine(r, tab.x, tab.y, tab.x + tab.w - 1, tab.y);
        }


        SDL_Color textCol = active ? COLOR_TAB_TEXT_ACT : COLOR_TAB_TEXT_INA;
        if (font) {
            int txtW, txtH;
            TTF_SizeUTF8(font, labels[i], &txtW, &txtH);
            int tx = tab.x + (tab.w - txtW) / 2;
            int ty = tab.y + (tab.h - txtH) / 2;
            draw_text(r, font, labels[i], tx, ty, textCol);
        }

        xCursor += tw;
    }

    SDL_SetRenderDrawColor(r, COLOR_SCRATCH_PURPLE.r,
                              COLOR_SCRATCH_PURPLE.g,
                              COLOR_SCRATCH_PURPLE.b, 255);
    SDL_RenderDrawLine(r, 0, TAB_BAR_H - 1, PALETTE_WIDTH - 1, TAB_BAR_H - 1);
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

    if (!block->inputs.empty() && font) {
        const std::string& txt = block->text;
        int inputIdx = 0;
        int charW    = 7;
        int cx       = x + 10;
        int cy       = y + (h - 14) / 2;
        std::string segment;

        for (size_t i = 0; i <= txt.size(); i++) {
            bool isInput = (i < txt.size() && txt[i] == '(' &&
                            i+1 < txt.size() && txt[i+1] == ')');
            bool isEnd   = (i == txt.size());

            if (isInput || isEnd) {
                if (!segment.empty()) {
                    draw_text(r, font, segment, cx, cy, COLOR_TEXT_WHITE);
                    int tw, th;
                    TTF_SizeUTF8(font, segment.c_str(), &tw, &th);
                    cx += tw;
                    segment.clear();
                }
                if (isInput && inputIdx < (int)block->inputs.size()) {
                    BlockInput& inp = block->inputs[inputIdx];
                    int valW = std::max(20, (int)inp.value.size() * charW + 6);
                    SDL_Rect field = {cx, cy - 1, valW, 18};
                    inp.rect = field;

                    SDL_SetRenderDrawColor(r, 255, 255, 255, isGhost ? 100 : 220);
                    SDL_RenderFillRect(r, &field);
                    SDL_Color borderCol = inp.editing ?
                        SDL_Color{80, 80, 255, 255} : SDL_Color{180, 180, 180, 255};
                    SDL_SetRenderDrawColor(r, borderCol.r, borderCol.g, borderCol.b, 255);
                    SDL_RenderDrawRect(r, &field);

                    std::string display = inp.value + (inp.editing ? "|" : "");
                    draw_text(r, font, display, cx + 3, cy + 1, COLOR_TEXT_DARK);
                    cx += valW + 4;
                    inputIdx++;
                    i++;
                }
            } else {
                segment += txt[i];
            }
        }
    } else if (font) {
        draw_text(r, font, block->text, x+10, y+(h-14)/2, COLOR_TEXT_WHITE);
    }

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
                            SDL_Color activeCatColor,
                            bool showMakeVarBtn = false,
                            SDL_Rect* makeVarBtnOut = nullptr)
{
    SDL_SetRenderDrawColor(r, COLOR_BG_BLOCKLIST.r, COLOR_BG_BLOCKLIST.g,
                           COLOR_BG_BLOCKLIST.b, 255);
    SDL_Rect listRect = {palette.blockListX, palette.blockListY,
                         palette.blockListW, palette.blockListH};
    SDL_RenderFillRect(r, &listRect);

    SDL_SetRenderDrawColor(r, activeCatColor.r, activeCatColor.g, activeCatColor.b, 40);
    SDL_Rect hdr = {palette.blockListX, palette.blockListY, palette.blockListW, 44};
    SDL_RenderFillRect(r, &hdr);

    SDL_SetRenderDrawColor(r, activeCatColor.r, activeCatColor.g, activeCatColor.b, 255);
    SDL_Rect accentLine = {palette.blockListX, palette.blockListY + 42, palette.blockListW, 3};
    SDL_RenderFillRect(r, &accentLine);

    SDL_SetRenderDrawColor(r, 210, 210, 210, 255);
    SDL_RenderDrawLine(r, palette.blockListX+palette.blockListW-1, 0,
                          palette.blockListX+palette.blockListW-1, SCREEN_HEIGHT);

    if (fontBig)
        draw_text(r, fontBig, activeCatName,
                  palette.blockListX+12, palette.blockListY + 12, activeCatColor);

    if (showMakeVarBtn) {
        SDL_Rect btn = {palette.blockListX + 8, palette.blockListY + 52,
                        palette.blockListW - 16, 26};
        SDL_SetRenderDrawColor(r, COLOR_VARIABLES.r, COLOR_VARIABLES.g, COLOR_VARIABLES.b, 255);
        SDL_RenderFillRect(r, &btn);
        SDL_SetRenderDrawColor(r, 180, 60, 10, 255);
        SDL_RenderDrawRect(r, &btn);
        if (fontBig) draw_text_centered(r, fontBig, "+ Make a Variable", btn, COLOR_TEXT_WHITE);
        if (makeVarBtnOut) *makeVarBtnOut = btn;
    }
}

void draw_stage(SDL_Renderer* r, Stage* stage) {
    if (!stage) return;
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
    SDL_RenderDrawRect(r, &rc);
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

    drawRow("x:",      fmt(scrX));
    drawRow("y:",      fmt(scrY));
    drawRow("dir:",    fmt(sprite->direction) + "°");
    drawRow("size:",   fmt(sprite->scale * 100.0f) + "%");
    drawRow("shown:",  sprite->visible ? "yes" : "no");
    drawRow("costume:", std::to_string(sprite->currentCostume + 1));
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

void draw_sounds_tab(SDL_Renderer* r, TTF_Font* font, TTF_Font* fontBig,
                     SoundsPanel& panel,
                     SDL_Rect addBtnOut[1])          // OUT: the "+ Add Sound" button
{
    const int HEADER_H  = 40;
    const int ITEM_H    = 54;
    const int ITEM_PAD  = 6;
    const int PLAY_BTN_W = 34;
    const int PLAY_BTN_H = 34;

    SDL_SetRenderDrawColor(r, 248, 244, 255, 255);
    SDL_Rect bg = {panel.x, panel.y, panel.w, panel.h};
    SDL_RenderFillRect(r, &bg);

    // ── Header strip ──────────────────────────────────────────────────────
    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
    SDL_Rect hdr = {panel.x, panel.y, panel.w, HEADER_H};
    SDL_RenderFillRect(r, &hdr);
    if (fontBig)
        draw_text_centered(r, fontBig, "Sounds", hdr, COLOR_TEXT_WHITE);

    SDL_Rect addBtn = {panel.x + 6, panel.y + HEADER_H + 6,
                       panel.w - 12, 28};
    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 200);
    SDL_RenderFillRect(r, &addBtn);
    SDL_SetRenderDrawColor(r, 160, 50, 180, 255);
    SDL_RenderDrawRect(r, &addBtn);
    if (font) draw_text_centered(r, font, "+ Add Sound", addBtn, COLOR_TEXT_WHITE);
    if (addBtnOut) addBtnOut[0] = addBtn;

    const int LIST_Y = panel.y + HEADER_H + 40;

    if (panel.sounds.empty()) {
        if (font) {
            draw_text(r, font, "No sounds yet.", panel.x + 14, LIST_Y + 10, {150,120,170,255});
            draw_text(r, font, "Click '+ Add Sound' above.", panel.x + 14, LIST_Y + 30, {180,160,200,255});
        }
        return;
    }

    SDL_Rect clip = {panel.x, LIST_Y, panel.w, panel.h - (LIST_Y - panel.y)};
    SDL_RenderSetClipRect(r, &clip);

    int yy = LIST_Y + panel.scrollOffset;

    for (int i = 0; i < (int)panel.sounds.size(); i++) {
        SoundClip& s = panel.sounds[i];
        int iy = yy + i * (ITEM_H + ITEM_PAD);
        if (iy + ITEM_H < clip.y || iy > clip.y + clip.h) continue;

        bool selected = (i == panel.selectedIndex);

        SDL_Color cardBg = selected
            ? SDL_Color{220, 200, 255, 255}
            : SDL_Color{255, 255, 255, 255};
        SDL_SetRenderDrawColor(r, cardBg.r, cardBg.g, cardBg.b, 255);
        SDL_Rect card = {panel.x + 6, iy, panel.w - 12, ITEM_H};
        SDL_RenderFillRect(r, &card);

        SDL_SetRenderDrawColor(r,
            selected ? 160 : 210,
            selected ? 80  : 190,
            selected ? 220 : 220, 255);
        SDL_RenderDrawRect(r, &card);

        SDL_Rect waveArea = {card.x + 6, card.y + 8, 60, ITEM_H - 16};
        SDL_SetRenderDrawColor(r, 240, 235, 250, 255);
        SDL_RenderFillRect(r, &waveArea);
        SDL_SetRenderDrawColor(r, selected ? 160 : 190, 90, selected ? 220 : 200, 180);

        int heights[] = {8, 20, 14, 26, 10, 22, 16, 12, 24, 8};
        int barW = 5, barGap = 1;
        int bx = waveArea.x + 2;
        for (int b = 0; b < 10 && bx + barW <= waveArea.x + waveArea.w; b++) {
            int bh = heights[b % 10];
            int by = waveArea.y + (waveArea.h - bh) / 2;
            SDL_Rect bar = {bx, by, barW, bh};
            SDL_RenderFillRect(r, &bar);
            bx += barW + barGap;
        }
        SDL_Rect playBtnR = {card.x + 74, card.y + (ITEM_H - PLAY_BTN_H) / 2,
                             PLAY_BTN_W, PLAY_BTN_H};
        SDL_Color pbCol = s.isPlaying
            ? SDL_Color{220, 50, 60, 255}
            : SDL_Color{100, 200, 120, 255};
        SDL_SetRenderDrawColor(r, pbCol.r, pbCol.g, pbCol.b, 255);
        SDL_RenderFillRect(r, &playBtnR);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);

        if (s.isPlaying) {
            SDL_Rect sq1 = {playBtnR.x + 6, playBtnR.y + 8, 8, 18};
            SDL_Rect sq2 = {playBtnR.x + 18, playBtnR.y + 8, 8, 18};
            SDL_RenderFillRect(r, &sq1);
            SDL_RenderFillRect(r, &sq2);
        } else {
            int px = playBtnR.x + 10, py = playBtnR.y + 8, ph = 18;
            for (int row = 0; row < ph / 2; row++)
                SDL_RenderDrawLine(r, px + row, py + row, px + row, py + ph - row);
        }


        if (font) {
            draw_text(r, font, s.name,
                      card.x + 116, card.y + 10, COLOR_TEXT_DARK);
            if (s.durationSecs > 0.0f) {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(1) << s.durationSecs << "s";
                draw_text(r, font, ss.str(),
                          card.x + 116, card.y + 30, {140, 100, 160, 255});
            } else {
                draw_text(r, font, "unknown length",
                          card.x + 116, card.y + 30, {180, 160, 200, 255});
            }
        }

        if (font) {
            std::string idx = std::to_string(i + 1);
            draw_text(r, font, idx, card.x + card.w - 18, card.y + 6, {180, 150, 200, 255});
        }
    }

    SDL_RenderSetClipRect(r, nullptr);
}

#endif // SCRATCH_FOP_RENDER_H

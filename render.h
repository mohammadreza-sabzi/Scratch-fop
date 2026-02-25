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
#include "utils.h"

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

SDL_Color lighten(SDL_Color c, int amt = 50) {
    return {(Uint8)std::min(255,(int)c.r+amt),
            (Uint8)std::min(255,(int)c.g+amt),
            (Uint8)std::min(255,(int)c.b+amt),255};
}

static void draw_embedded_block(SDL_Renderer* r, TTF_Font* font, Block* block, bool asBoolean);

static void draw_block_content(SDL_Renderer* r, TTF_Font* font, Block* block,
                                bool isGhost = false)
{
    const std::string& txt = block->text;
    SDL_Color tCol = isGhost ? SDL_Color{255,255,255,140} : SDL_Color{255,255,255,255};

    if (block->inputs.empty()) {
        if (font) draw_text(r, font, txt, block->x+10, block->y+(block->h-14)/2, tCol);
        return;
    }

    int inputIdx = 0;
    int cx       = block->x + 10;
    int cy       = block->y + (block->h - 14) / 2;
    std::string segment;

    for (size_t i = 0; i <= txt.size(); i++) {
        bool isNumInput = (i < txt.size() && txt[i] == '(' &&
                           i+1 < txt.size() && txt[i+1] == ')');
        bool isBoolInput = (i < txt.size() && txt[i] == '<' &&
                            i+1 < txt.size() && txt[i+1] == '>');
        bool isEnd   = (i == txt.size());

        if (isNumInput || isBoolInput || isEnd) {
            if (!segment.empty() && font) {
                draw_text(r, font, segment, cx, cy, tCol);
                int tw, th;
                TTF_SizeUTF8(font, segment.c_str(), &tw, &th);
                cx += tw;
                segment.clear();
            }
            if ((isNumInput || isBoolInput) && inputIdx < (int)block->inputs.size()) {
                BlockInput& inp = block->inputs[inputIdx];

                if (isBoolInput) {
                    int slotW = inp.rect.w > 0 ? inp.rect.w : 40;
                    int slotH = 20;
                    int sx = cx, sy = block->y + (block->h - slotH) / 2;
                    inp.rect = {sx, sy, slotW, slotH};

                    if (inp.embeddedBlock) {
                        inp.embeddedBlock->x = sx + 2;
                        inp.embeddedBlock->y = sy;
                        inp.embeddedBlock->h = slotH;
                        inp.embeddedBlock->w = slotW - 4;
                        draw_embedded_block(r, font, inp.embeddedBlock, true);
                    } else {
                        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                        SDL_SetRenderDrawColor(r, 0, 0, 0, (Uint8)(isGhost ? 40 : 80));
                        for (int dy = 0; dy < slotH; dy++) {
                            float t = (float)dy / (slotH > 1 ? slotH-1 : 1);
                            float edge = (t < 0.5f) ? t*2*6 : (1.0f-t)*2*6;
                            SDL_RenderDrawLine(r, sx+(int)edge, sy+dy, sx+slotW-(int)edge, sy+dy);
                        }
                        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
                        SDL_SetRenderDrawColor(r, 255, 255, 255, (Uint8)(isGhost ? 100 : 180));
                        int pts[][2] = {
                            {sx+6, sy}, {sx+slotW-6, sy},
                            {sx+slotW, sy+slotH/2}, {sx+slotW-6, sy+slotH},
                            {sx+6, sy+slotH}, {sx, sy+slotH/2}
                        };
                        for (int p = 0; p < 6; p++) {
                            int nx = pts[(p+1)%6][0], ny = pts[(p+1)%6][1];
                            SDL_RenderDrawLine(r, pts[p][0], pts[p][1], nx, ny);
                        }
                    }
                    cx += slotW + 4;
                } else {
                    int valW = inp.rect.w > 0 ? inp.rect.w :
                               std::max(30, (int)inp.value.size() * 8 + 8);
                    int slotH = 18;
                    int sx = cx, sy = block->y + (block->h - slotH) / 2 - 1;
                    inp.rect = {sx, sy, valW, slotH};

                    if (inp.embeddedBlock) {
                        inp.embeddedBlock->x = sx + 2;
                        inp.embeddedBlock->y = sy;
                        inp.embeddedBlock->h = slotH;
                        inp.embeddedBlock->w = valW - 4;
                        draw_embedded_block(r, font, inp.embeddedBlock, false);
                    } else {
                        SDL_Color fieldBg = {255,255,255, (Uint8)(isGhost ? 100 : 230)};
                        SDL_SetRenderDrawColor(r, fieldBg.r, fieldBg.g, fieldBg.b, fieldBg.a);
                        SDL_Rect field = {sx, sy, valW, slotH};
                        SDL_RenderFillRect(r, &field);

                        SDL_Color borderCol = inp.editing
                            ? SDL_Color{60,100,255,255}
                            : SDL_Color{170,170,170,255};
                        SDL_SetRenderDrawColor(r, borderCol.r, borderCol.g, borderCol.b, 255);
                        SDL_RenderDrawRect(r, &field);

                        std::string display = inp.value + (inp.editing ? "|" : "");
                        if (font)
                            draw_text(r, font, display, sx+3, sy+1, SDL_Color{30,30,30,255});
                    }
                    cx += valW + 4;
                }
                inputIdx++;
                i++;
            }
        } else {
            segment += txt[i];
        }
    }
}

static void draw_embedded_block(SDL_Renderer* r, TTF_Font* font, Block* block, bool asBoolean) {
    if (!block) return;

    SDL_Color col = get_block_color(block->type);
    SDL_Color dark = darken(col, 30);
    int x = block->x, y = block->y, w = block->w, h = block->h;

    if (asBoolean) {
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, 255);
        int indent = h/2;
        for (int dy = 0; dy < h; dy++) {
            float t = (float)dy / (h > 1 ? h-1 : 1);
            float edge = (t < 0.5f) ? t * 2 * indent : (1.0f - t) * 2 * indent;
            int lx = x + (int)edge;
            int rx = x + w - (int)edge;
            SDL_RenderDrawLine(r, lx, y + dy, rx, y + dy);
        }
        SDL_SetRenderDrawColor(r, dark.r, dark.g, dark.b, 255);
        int pts[][2] = {
            {x+indent, y}, {x+w-indent, y},
            {x+w, y+h/2}, {x+w-indent, y+h},
            {x+indent, y+h}, {x, y+h/2}
        };
        for (int p = 0; p < 6; p++) {
            int nx = pts[(p+1)%6][0], ny = pts[(p+1)%6][1];
            SDL_RenderDrawLine(r, pts[p][0], pts[p][1], nx, ny);
        }
    } else {
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, 255);
        int r2 = h / 2;
        for (int dy = 0; dy < h; dy++) {
            float t = (float)(dy - r2) / r2;
            int cap = (int)(r2 * std::sqrt(1.0f - std::min(1.0f, t*t)));
            SDL_RenderDrawLine(r, x + r2 - cap, y + dy, x + w - r2 + cap, y + dy);
        }
        SDL_SetRenderDrawColor(r, dark.r, dark.g, dark.b, 255);
        SDL_Rect border = {x, y, w, h};
        SDL_RenderDrawRect(r, &border);
    }

    if (font) {
        draw_block_content(r, font, block, false);
    }
}

static void draw_notch_top(SDL_Renderer* r, int x, int y, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    for (int i = 0; i < 4; i++)
        SDL_RenderDrawLine(r, x+10+i, y-i, x+30-i, y-i);
}
static void draw_notch_bottom(SDL_Renderer* r, int x, int y, int bh, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    for (int i = 0; i < 4; i++)
        SDL_RenderDrawLine(r, x+10+i, y+bh+i, x+30-i, y+bh+i);
}

static void draw_normal_block(SDL_Renderer* r, TTF_Font* font,
                               Block* block, bool isGhost = false)
{
    SDL_Color col    = get_block_color(block->type);
    SDL_Color shadow = darken(col, 50);
    SDL_Color hl     = lighten(col, 50);
    if (isGhost) { SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND); col.a = 140; }

    int x = block->x, y = block->y, w = block->w, h = block->h;

    if (block->type == BLOCK_OPERATORS) {
        bool isBool = is_boolean_operator(block->text);

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 60 : 180);
        if (isBool) {
            int pts[][2] = {
                {x+h/2+2, y+3}, {x+w-h/2+2, y+3},
                {x+w+2, y+h/2+3}, {x+w-h/2+2, y+h+3},
                {x+h/2+2, y+h+3}, {x+2, y+h/2+3}
            };
            for (int dy2 = 0; dy2 < h; dy2++) {
                float t = (float)dy2 / (h > 1 ? h-1 : 1);
                float edge = (t < 0.5f) ? t * 2 * (h/2) : (1.0f-t) * 2 * (h/2);
                SDL_RenderDrawLine(r, x+(int)edge+2, y+dy2+3, x+w-(int)edge+2, y+dy2+3);
            }
        } else {
            for (int dy2 = 0; dy2 < h; dy2++) {
                float t = (float)(dy2 - h/2) / (h/2 > 0 ? h/2 : 1);
                int cap = (int)((h/2) * std::sqrt(1.0f - std::min(1.0f, t*t)));
                SDL_RenderDrawLine(r, x+(h/2)-cap+2, y+dy2+3, x+w-(h/2)+cap+2, y+dy2+3);
            }
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        if (isBool) {
            int indent = h/2;
            for (int dy2 = 0; dy2 < h; dy2++) {
                float t = (float)dy2 / (h > 1 ? h-1 : 1);
                float edge = (t < 0.5f) ? t * 2 * indent : (1.0f-t) * 2 * indent;
                SDL_RenderDrawLine(r, x+(int)edge, y+dy2, x+w-(int)edge, y+dy2);
            }
            SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 220);
            int pts2[][2] = {
                {x+indent, y}, {x+w-indent, y},
                {x+w, y+h/2}, {x+w-indent, y+h},
                {x+indent, y+h}, {x, y+h/2}
            };
            for (int p = 0; p < 6; p++) {
                int nx = pts2[(p+1)%6][0], ny = pts2[(p+1)%6][1];
                SDL_RenderDrawLine(r, pts2[p][0], pts2[p][1], nx, ny);
            }
        } else {
            int r2 = h/2;
            for (int dy2 = 0; dy2 < h; dy2++) {
                float t = (float)(dy2 - r2) / (r2 > 0 ? r2 : 1);
                int cap = (int)(r2 * std::sqrt(1.0f - std::min(1.0f, t*t)));
                SDL_RenderDrawLine(r, x+r2-cap, y+dy2, x+w-r2+cap, y+dy2);
            }
            SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 220);
            SDL_Rect border = {x, y, w, h};
            SDL_RenderDrawRect(r, &border);
        }

        SDL_SetRenderDrawColor(r, hl.r, hl.g, hl.b, isGhost ? 60 : 180);
        SDL_RenderDrawLine(r, x+4, y+1, x+w-4, y+1);

        draw_block_content(r, font, block, isGhost);

        if (isGhost) SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        return;
    }

    SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 200);
    SDL_Rect shadowR = {x+2, y+3, w, h};
    SDL_RenderFillRect(r, &shadowR);

    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_Rect mainR = {x, y, w, h};
    SDL_RenderFillRect(r, &mainR);

    if (block->type != BLOCK_EVENT)
        draw_notch_top(r, x, y, col);

    draw_notch_bottom(r, x, y, h, col);

    if (block->type == BLOCK_EVENT) {
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        for (int i = 0; i < 14; i++) {
            SDL_Rect hatR = {x+i/2, y-14+i, w-i, 4};
            SDL_RenderFillRect(r, &hatR);
        }
    }

    SDL_SetRenderDrawColor(r, hl.r, hl.g, hl.b, isGhost ? 80 : 255);
    SDL_RenderDrawLine(r, x, y, x+w, y);

    SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 200);
    SDL_RenderDrawRect(r, &mainR);

    draw_block_content(r, font, block, isGhost);

    if (isGhost) SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

static void draw_c_block(SDL_Renderer* r, TTF_Font* font,
                          Block* block, bool isGhost = false)
{
    SDL_Color col    = get_block_color(block->type);
    SDL_Color shadow = darken(col, 50);
    SDL_Color hl     = lighten(col, 50);
    if (isGhost) { SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND); col.a = 140; }

    int x = block->x, y = block->y, w = block->w;
    int headerH = block->h;
    int indent   = 16;
    int capH     = 8;

    int innerH = block->innerH;
    int elseBarH = block->hasElse ? 20 : 0;
    int elseH    = block->hasElse ? block->elseH : 0;

    SDL_SetRenderDrawColor(r, shadow.r, shadow.g, shadow.b, isGhost ? 80 : 200);
    SDL_Rect headerShadow = {x+2, y+3, w, headerH};
    SDL_RenderFillRect(r, &headerShadow);

    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_Rect headerR = {x, y, w, headerH};
    SDL_RenderFillRect(r, &headerR);

    draw_notch_top(r, x, y, col);

    int innerY = y + headerH;
    int totalInnerSectionH = innerH + capH;
    if (block->hasElse) totalInnerSectionH += elseBarH + elseH + capH;

    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_Rect leftBar = {x, innerY, indent, totalInnerSectionH};
    SDL_RenderFillRect(r, &leftBar);

    int cap1Y = innerY + innerH;
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_Rect cap1 = {x, cap1Y, w, capH};
    SDL_RenderFillRect(r, &cap1);
    draw_notch_top(r, x+indent, cap1Y, col);

    if (block->hasElse) {
        int elseBarY = cap1Y + capH;
        SDL_SetRenderDrawColor(r, darken(col, 20).r, darken(col, 20).g,
                               darken(col, 20).b, col.a);
        SDL_Rect elseBar = {x, elseBarY, w, elseBarH};
        SDL_RenderFillRect(r, &elseBar);
        if (font)
            draw_text(r, font, "else",
                      x + w/2 - 12, elseBarY + (elseBarH-14)/2,
                      {255,255,255,255});

        int elseSectionY = elseBarY + elseBarH;
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        SDL_Rect leftBar2 = {x, elseSectionY, indent, elseH + capH};
        SDL_RenderFillRect(r, &leftBar2);

        int cap2Y = elseSectionY + elseH;
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        SDL_Rect cap2 = {x, cap2Y, w, capH};
        SDL_RenderFillRect(r, &cap2);
        draw_notch_top(r, x+indent, cap2Y, col);

        draw_notch_bottom(r, x, cap2Y, capH, col);
    } else {
        draw_notch_bottom(r, x, cap1Y, capH, col);
    }

    SDL_SetRenderDrawColor(r, hl.r, hl.g, hl.b, isGhost ? 80 : 200);
    SDL_RenderDrawLine(r, x, y, x+w, y);

    draw_block_content(r, font, block, isGhost);

    if (isGhost) SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

void draw_block(SDL_Renderer* r, TTF_Font* font,
                Block* block, bool isGhost = false)
{
    if (!block) return;
    block->w = std::max(BLOCK_W, compute_block_width(block));
    update_block_input_rects(block);

    if (block->isCShaped)
        draw_c_block(r, font, block, isGhost);
    else
        draw_normal_block(r, font, block, isGhost);
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

        draw_filled_circle(r, cx, cy, CAT_CIRCLE_R + 2, {40,40,40,255});
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
                             const Palette& palette,
                             const std::string& catName,
                             SDL_Color catColor,
                             bool showMakeBtn,
                             SDL_Rect* makeBtnOut)
{
    SDL_SetRenderDrawColor(r, 245, 245, 250, 255);
    SDL_Rect listRect = {palette.blockListX, palette.blockListY,
                         palette.blockListW, palette.blockListH};
    SDL_RenderFillRect(r, &listRect);

    int hdrY = palette.blockListY;
    int hdrH = (palette.activeCategory == CAT_VARIABLES ||
                palette.activeCategory == CAT_MYBLOCKS) ? 88 : 44;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, catColor.r, catColor.g, catColor.b, 40);
    SDL_Rect hdr = {palette.blockListX, hdrY, palette.blockListW, hdrH};
    SDL_RenderFillRect(r, &hdr);

    SDL_SetRenderDrawColor(r, catColor.r, catColor.g, catColor.b, 255);
    SDL_Rect accentLine = {palette.blockListX, hdrY+hdrH-3, palette.blockListW, 3};
    SDL_RenderFillRect(r, &accentLine);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    if (fontBig) {
        SDL_Rect textRect = {palette.blockListX+10, hdrY+10, palette.blockListW-20, 24};
        draw_text_centered(r, fontBig, catName, textRect, catColor);
    }

    if (showMakeBtn && makeBtnOut) {
        std::string btnLabel = (palette.activeCategory == CAT_VARIABLES)
                               ? "Make a Variable" : "Make a Block";
        SDL_Rect btn = {palette.blockListX+10, hdrY+50, palette.blockListW-20, 28};
        SDL_SetRenderDrawColor(r, catColor.r, catColor.g, catColor.b, 200);
        SDL_RenderFillRect(r, &btn);
        SDL_SetRenderDrawColor(r, catColor.r, catColor.g, catColor.b, 255);
        SDL_RenderDrawRect(r, &btn);
        if (fontBig)
            draw_text_centered(r, fontBig, btnLabel, btn, {255,255,255,255});
        *makeBtnOut = btn;
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

void draw_ask_input(SDL_Renderer* r, TTF_Font* font, Stage* stage,
                    const std::string& question, const std::string& currentInput)
{
    if (!stage || !font) return;
    int bx = stage->x + 10;
    int by = stage->y + stage->h - 44;
    int bw = stage->w - 20;
    int bh = 36;

    SDL_SetRenderDrawColor(r, 255, 255, 255, 245);
    SDL_Rect bg = {bx, by, bw, bh};
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 100, 150, 220, 255);
    SDL_RenderDrawRect(r, &bg);

    std::string display = currentInput + "|";
    draw_text(r, font, display, bx+8, by+(bh-14)/2, COLOR_TEXT_DARK);

    SDL_Rect okBtn = {bx+bw-50, by+4, 44, 28};
    SDL_SetRenderDrawColor(r, 60,140,200,255);
    SDL_RenderFillRect(r, &okBtn);
    draw_text_centered(r, font, "OK", okBtn, {255,255,255,255});
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
    if (playTex) SDL_RenderCopy(r, playTex, nullptr, &playBtn);
    else {
        SDL_Color gc = isRunning ? SDL_Color{30,180,30,255} : SDL_Color{60,200,60,255};
        SDL_SetRenderDrawColor(r, gc.r, gc.g, gc.b, 255);
        SDL_RenderFillRect(r, &playBtn);
        SDL_SetRenderDrawColor(r, 30,150,30,255);
        SDL_RenderDrawRect(r, &playBtn);
    }
    if (stopTex) SDL_RenderCopy(r, stopTex, nullptr, &stopBtn);
    else {
        SDL_SetRenderDrawColor(r, 220, 50, 50, 255);
        SDL_RenderFillRect(r, &stopBtn);
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
        if (iy + itemH < panel.y+30 || iy > panel.y+panel.h) continue;
        bool selected = (i == sprite->currentCostume);
        SDL_Color bg2 = selected ? SDL_Color{200,210,255,255} : SDL_Color{255,255,255,255};
        SDL_SetRenderDrawColor(r, bg2.r, bg2.g, bg2.b, 255);
        SDL_Rect card = {ix, iy, itemW, itemH};
        SDL_RenderFillRect(r, &card);
        SDL_SetRenderDrawColor(r, selected?80:200, selected?100:200, selected?220:200, 255);
        SDL_RenderDrawRect(r, &card);

        SDL_Rect thumbArea = {ix+4, iy+4, thumbW, thumbW};
        if (sprite->costumes[i].texture)
            SDL_RenderCopy(r, sprite->costumes[i].texture, nullptr, &thumbArea);
        else {
            SDL_SetRenderDrawColor(r, 200,200,220,255);
            SDL_RenderFillRect(r, &thumbArea);
            if (font) draw_text_centered(r, font, "?", thumbArea, COLOR_TEXT_DARK);
        }

        if (font) {
            std::string label = std::to_string(i+1) + ". " + sprite->costumes[i].name;
            draw_text(r, font, label, ix+thumbW+10, iy+itemH/2-7, COLOR_TEXT_DARK);
        }
    }
    SDL_RenderSetClipRect(r, nullptr);


    {
        int btnY = panel.y + panel.h - 38;
        SDL_Rect uploadBtn = {panel.x + 4, btnY, (panel.w - 12) / 2, 30};
        SDL_Rect deleteBtn = {panel.x + 4 + (panel.w - 12) / 2 + 4, btnY,
                              (panel.w - 12) / 2, 30};

        SDL_SetRenderDrawColor(r, 80, 160, 80, 255);
        SDL_RenderFillRect(r, &uploadBtn);
        SDL_SetRenderDrawColor(r, 50, 110, 50, 255);
        SDL_RenderDrawRect(r, &uploadBtn);
        if (font) draw_text_centered(r, font, "Upload", uploadBtn, COLOR_TEXT_WHITE);

        bool hasSelected = (sprite && sprite->currentCostume >= 0 &&
                            sprite->currentCostume < (int)sprite->costumes.size());
        SDL_SetRenderDrawColor(r, hasSelected ? 200 : 160,
                                  hasSelected ? 60  : 60,
                                  hasSelected ? 60  : 60, 255);
        SDL_RenderFillRect(r, &deleteBtn);
        SDL_SetRenderDrawColor(r, 140, 40, 40, 255);
        SDL_RenderDrawRect(r, &deleteBtn);
        if (font) draw_text_centered(r, font, "Delete", deleteBtn, COLOR_TEXT_WHITE);
    }
}

void draw_sprite_info_panel(SDL_Renderer* r, TTF_Font* font, TTF_Font* fontBig,
                             Sprite* sprite, int activeField = -1,
                             const std::string& editText = "")
{
    int px = SPRITE_INFO_X, py = SPRITE_INFO_Y;
    int pw = SPRITE_INFO_W, ph = SPRITE_INFO_H;

    SDL_SetRenderDrawColor(r, 245, 245, 248, 255);
    SDL_Rect bg = {px, py, pw, ph};
    SDL_RenderFillRect(r, &bg);

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

    int tx = px+10, ty = py+38, lineH = 24;
    float scrX = sprite->x - STAGE_WIDTH/2.0f + sprite->w/2.0f;
    float scrY = -(sprite->y - STAGE_HEIGHT/2.0f + sprite->h/2.0f);

    draw_text(r, font, "name:", tx, ty, {120,120,120,255});
    SDL_Rect nameBg = {tx+50, ty-2, pw-60, 18};
    bool nameActive = (activeField == 4);
    SDL_SetRenderDrawColor(r, nameActive?220:235, nameActive?240:235, nameActive?255:240, 255);
    SDL_RenderFillRect(r, &nameBg);
    SDL_SetRenderDrawColor(r, nameActive?80:180, nameActive?140:180, nameActive?220:200, 255);
    SDL_RenderDrawRect(r, &nameBg);
    std::string nameDisplay = nameActive ? (editText + "|") : sprite->name;
    draw_text(r, font, nameDisplay, tx+54, ty, COLOR_TEXT_DARK);
    ty += lineH;

    int fieldIdx = 0;
    auto drawRow = [&](const std::string& label, const std::string& val, bool editable) {
        draw_text(r, font, label, tx, ty, {120,120,120,255});

        SDL_Rect fieldBg = {tx+50, ty-2, pw-60, 18};
        if (editable) {
            bool isActive = (activeField == fieldIdx);
            if (isActive) {
                SDL_SetRenderDrawColor(r, 220, 240, 255, 255);
                SDL_RenderFillRect(r, &fieldBg);
                SDL_SetRenderDrawColor(r, 80, 140, 220, 255);
                SDL_RenderDrawRect(r, &fieldBg);
                std::string display = editText + "|";
                draw_text(r, font, display, tx+54, ty, COLOR_TEXT_DARK);
            } else {
                SDL_SetRenderDrawColor(r, 235, 235, 240, 255);
                SDL_RenderFillRect(r, &fieldBg);
                SDL_SetRenderDrawColor(r, 180, 180, 200, 255);
                SDL_RenderDrawRect(r, &fieldBg);
                draw_text(r, font, val, tx+54, ty, COLOR_TEXT_DARK);
            }
        } else {
            draw_text(r, font, val, tx+54, ty, COLOR_TEXT_DARK);
        }
        ty += lineH;
        if (editable) fieldIdx++;
    };
    drawRow("x:",      fmt(scrX),                   true);
    drawRow("y:",      fmt(scrY),                   true);
    drawRow("dir:",    fmt(sprite->direction)+"°",   true);
    drawRow("size:",   fmt(sprite->scale*100.0f)+"%",true);
    drawRow("shown:",  sprite->visible ? "yes" : "no", false);
    drawRow("costume:", std::to_string(sprite->currentCostume+1), false);
}

inline int sprite_info_field_at(int mx, int my) {
    int px = SPRITE_INFO_X, py = SPRITE_INFO_Y;
    int pw = SPRITE_INFO_W;
    int tx = px+10, ty = py+38, lineH = 24;
    SDL_Rect nameBg = {tx+50, ty-2, pw-60, 18};
    if (mx >= nameBg.x && mx < nameBg.x+nameBg.w &&
        my >= nameBg.y && my < nameBg.y+nameBg.h)
        return 4;
    ty += lineH;
    for (int i = 0; i < 4; i++) {
        SDL_Rect fieldBg = {tx+50, ty-2, pw-60, 18};
        if (mx >= fieldBg.x && mx < fieldBg.x+fieldBg.w &&
            my >= fieldBg.y && my < fieldBg.y+fieldBg.h)
            return i;
        ty += lineH;
    }
    return -1;
}

void draw_variable_monitors(SDL_Renderer* r, TTF_Font* font,
                             VariablesPanel& vp, Stage* stage)
{
    if (!stage || !font) return;
    int mx = stage->x + 4, my = stage->y + 4;
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

void draw_variables_panel(SDL_Renderer* r, TTF_Font* font, TTF_Font* fontBig,
                          VariablesPanel& vp, Workspace& ws)
{
    (void)r; (void)font; (void)fontBig; (void)vp; (void)ws;
}

#endif
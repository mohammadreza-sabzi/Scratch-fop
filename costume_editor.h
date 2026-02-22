//
// Created by Domim on 2/21/2026.
//

#ifndef COSTUME_EDITOR_H
#define COSTUME_EDITOR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include "structs.h"
#include "globals.h"
#include "render.h"

static const int CE_W          = 900;
static const int CE_H          = 620;
static const int CE_CANVAS_W   = 480;
static const int CE_CANVAS_H   = 480;
static const int CE_TOOLBAR_H  = 56;
static const int CE_SIDEBAR_W  = 180;
static const int CE_MAX_UNDO   = 30;

enum CETool {
    CE_TOOL_PEN = 0,
    CE_TOOL_ERASER,
    CE_TOOL_LINE,
    CE_TOOL_RECT,
    CE_TOOL_ELLIPSE,
    CE_TOOL_FILL,
    CE_TOOL_COUNT
};

struct UndoFrame {
    std::vector<Uint32> pixels;
    int w, h;
};

struct CostumeEditor {
    bool      isOpen        = false;
    int       winX          = 0;
    int       winY          = 0;

    SDL_Texture* canvasTex  = nullptr;
    SDL_Surface* canvasSurf = nullptr;
    int          canvasW    = CE_CANVAS_W;
    int          canvasH    = CE_CANVAS_H;

    CETool    tool          = CE_TOOL_PEN;
    SDL_Color drawColor     = {0, 0, 0, 255};
    SDL_Color bgColor       = {255, 255, 255, 255};
    int       penSize       = 4;

    bool      dragging      = false;
    int       lastX         = 0;
    int       lastY         = 0;
    int       startX        = 0;
    int       startY        = 0;

    std::vector<UndoFrame> undoStack;
    std::vector<UndoFrame> redoStack;

    int       costumeIndex  = -1;
};

static void ce_push_undo(CostumeEditor& ce);
static void ce_draw_line_on_surf(CostumeEditor& ce, int x0, int y0, int x1, int y1);
static void ce_fill(CostumeEditor& ce, int x, int y, SDL_Color target, SDL_Color fill);
static void ce_apply_to_texture(CostumeEditor& ce, SDL_Renderer* r);
static void ce_draw_shape_preview(SDL_Renderer* r, CostumeEditor& ce, SDL_Rect canvasRect);


inline void ce_init(CostumeEditor& ce) {
    ce.isOpen       = false;
    ce.canvasTex    = nullptr;
    ce.canvasSurf   = nullptr;
    ce.tool         = CE_TOOL_PEN;
    ce.drawColor    = {0, 0, 0, 255};
    ce.bgColor      = {255, 255, 255, 255};
    ce.penSize      = 4;
    ce.dragging     = false;
    ce.costumeIndex = -1;
    ce.undoStack.clear();
    ce.redoStack.clear();
}
inline void ce_open(CostumeEditor& ce, SDL_Renderer* r, Sprite* sprite, int costumeIdx) {
    ce.isOpen       = true;
    ce.costumeIndex = costumeIdx;
    ce.winX = (SCREEN_WIDTH  - CE_W) / 2;
    ce.winY = (SCREEN_HEIGHT - CE_H) / 2;
    ce.dragging = false;
    ce.undoStack.clear();
    ce.redoStack.clear();

    // destroy old
    if (ce.canvasTex)  { SDL_DestroyTexture(ce.canvasTex); ce.canvasTex = nullptr; }
    if (ce.canvasSurf) { SDL_FreeSurface(ce.canvasSurf); ce.canvasSurf = nullptr; }

    ce.canvasSurf = SDL_CreateRGBSurface(0, CE_CANVAS_W, CE_CANVAS_H, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(ce.canvasSurf, nullptr,
        SDL_MapRGBA(ce.canvasSurf->format, 255, 255, 255, 255));


    ce.canvasTex = SDL_CreateTextureFromSurface(r, ce.canvasSurf);
    SDL_SetTextureBlendMode(ce.canvasTex, SDL_BLENDMODE_NONE);
}

inline void ce_close(CostumeEditor& ce) {
    ce.isOpen = false;

}

inline bool ce_handle_event(CostumeEditor& ce, SDL_Event& e, SDL_Renderer* r, Sprite* sprite) {
    if (!ce.isOpen) return false;

    int wx = ce.winX, wy = ce.winY;
    SDL_Rect canvasRect = {
        wx + CE_SIDEBAR_W + 12,
        wy + CE_TOOLBAR_H + 12,
        CE_CANVAS_W,
        CE_CANVAS_H
    };

    auto toCanvasXY = [&](int sx, int sy, int& cx, int& cy) {
        cx = sx - canvasRect.x;
        cy = sy - canvasRect.y;
    };

    if (e.type == SDL_KEYDOWN) {
        SDL_Keymod mod = SDL_GetModState();
        if (e.key.keysym.sym == SDLK_ESCAPE) { ce_close(ce); return true; }
        if ((mod & KMOD_CTRL) && e.key.keysym.sym == SDLK_z) {
            // undo
            if (!ce.undoStack.empty()) {
                UndoFrame cur;
                cur.w = ce.canvasW; cur.h = ce.canvasH;
                int sz = cur.w * cur.h;
                cur.pixels.resize(sz);
                SDL_LockSurface(ce.canvasSurf);
                memcpy(cur.pixels.data(), ce.canvasSurf->pixels, sz * sizeof(Uint32));
                SDL_UnlockSurface(ce.canvasSurf);
                ce.redoStack.push_back(cur);

                UndoFrame& prev = ce.undoStack.back();
                SDL_LockSurface(ce.canvasSurf);
                memcpy(ce.canvasSurf->pixels, prev.pixels.data(), sz * sizeof(Uint32));
                SDL_UnlockSurface(ce.canvasSurf);
                ce.undoStack.pop_back();
                ce_apply_to_texture(ce, r);
            }
            return true;
        }
        if ((mod & KMOD_CTRL) && e.key.keysym.sym == SDLK_y) {

            if (!ce.redoStack.empty()) {
                ce_push_undo(ce);
                UndoFrame& next = ce.redoStack.back();
                int sz = next.w * next.h;
                SDL_LockSurface(ce.canvasSurf);
                memcpy(ce.canvasSurf->pixels, next.pixels.data(), sz * sizeof(Uint32));
                SDL_UnlockSurface(ce.canvasSurf);
                ce.redoStack.pop_back();
                ce_apply_to_texture(ce, r);
            }
            return true;
        }
        return true;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;

        SDL_Rect closeBtn = {wx + CE_W - 36, wy + 8, 28, 28};
        if (mx >= closeBtn.x && mx <= closeBtn.x+closeBtn.w &&
            my >= closeBtn.y && my <= closeBtn.y+closeBtn.h) {
            ce_close(ce); return true;
        }

        const char* toolNames[] = {"Pen","Eraser","Line","Rect","Ellipse","Fill"};
        for (int i = 0; i < CE_TOOL_COUNT; i++) {
            SDL_Rect tb = {wx + 8, wy + CE_TOOLBAR_H + 12 + i * 42, CE_SIDEBAR_W - 16, 36};
            if (mx >= tb.x && mx <= tb.x+tb.w && my >= tb.y && my <= tb.y+tb.h) {
                ce.tool = (CETool)i; return true;
            }
        }

        SDL_Color palette8[] = {
            {0,0,0,255},{255,255,255,255},{220,50,50,255},{50,180,50,255},
            {50,50,220,255},{230,170,30,255},{150,60,200,255},{60,200,200,255}
        };
        int swX = wx + 8, swY = wy + CE_TOOLBAR_H + CE_TOOL_COUNT*42 + 24;
        for (int i = 0; i < 8; i++) {
            int col = i % 4, row = i / 4;
            SDL_Rect sr = {swX + col*34, swY + row*34, 30, 30};
            if (mx >= sr.x && mx <= sr.x+sr.w && my >= sr.y && my <= sr.y+sr.h) {
                ce.drawColor = palette8[i]; return true;
            }
        }

        int szY = swY + 80;
        for (int i = 0; i < 4; i++) {
            int sz = (i+1)*3;
            SDL_Rect sr = {swX + i*40, szY, 36, 36};
            if (mx >= sr.x && mx <= sr.x+sr.w && my >= sr.y && my <= sr.y+sr.h) {
                ce.penSize = sz; return true;
            }
        }

        SDL_Rect clearBtn = {wx + 8, wy + CE_H - 48, CE_SIDEBAR_W - 16, 32};
        if (mx >= clearBtn.x && mx <= clearBtn.x+clearBtn.w &&
            my >= clearBtn.y && my <= clearBtn.y+clearBtn.h) {
            ce_push_undo(ce);
            SDL_FillRect(ce.canvasSurf, nullptr,
                SDL_MapRGBA(ce.canvasSurf->format, 255,255,255,255));
            ce_apply_to_texture(ce, r);
            return true;
        }

        SDL_Rect saveBtn = {wx + CE_SIDEBAR_W + 12 + CE_CANVAS_W - 100, wy + CE_H - 48, 96, 32};
        if (mx >= saveBtn.x && mx <= saveBtn.x+saveBtn.w &&
            my >= saveBtn.y && my <= saveBtn.y+saveBtn.h) {
            // Apply canvas to sprite costume texture
            if (sprite && ce.costumeIndex >= 0 &&
                ce.costumeIndex < (int)sprite->costumes.size()) {
                SDL_Texture* newTex = SDL_CreateTextureFromSurface(r, ce.canvasSurf);
                if (sprite->costumes[ce.costumeIndex].texture)
                    SDL_DestroyTexture(sprite->costumes[ce.costumeIndex].texture);
                sprite->costumes[ce.costumeIndex].texture = newTex;
                sprite->costumes[ce.costumeIndex].w = CE_CANVAS_W;
                sprite->costumes[ce.costumeIndex].h = CE_CANVAS_H;
                if (sprite->currentCostume == ce.costumeIndex)
                    sprite->texture = newTex;
            }
            ce_close(ce);
            return true;
        }

        if (mx >= canvasRect.x && mx <= canvasRect.x + canvasRect.w &&
            my >= canvasRect.y && my <= canvasRect.y + canvasRect.h) {
            int cx, cy; toCanvasXY(mx, my, cx, cy);
            ce_push_undo(ce);
            ce.dragging = true;
            ce.startX = cx; ce.startY = cy;
            ce.lastX  = cx; ce.lastY  = cy;

            if (ce.tool == CE_TOOL_PEN || ce.tool == CE_TOOL_ERASER) {
                ce_draw_line_on_surf(ce, cx, cy, cx, cy);
                ce_apply_to_texture(ce, r);
            } else if (ce.tool == CE_TOOL_FILL) {
                // read target color
                SDL_LockSurface(ce.canvasSurf);
                Uint32* px = (Uint32*)ce.canvasSurf->pixels;
                int idx = cy * ce.canvasW + cx;
                Uint8 tr, tg, tb, ta;
                SDL_GetRGBA(px[idx], ce.canvasSurf->format, &tr, &tg, &tb, &ta);
                SDL_UnlockSurface(ce.canvasSurf);
                SDL_Color target = {tr, tg, tb, ta};
                ce_fill(ce, cx, cy, target, ce.drawColor);
                ce.dragging = false;
                ce_apply_to_texture(ce, r);
            }
            return true;
        }

        // click anywhere outside the editor = ignore (still consume)
        return true;
    }

    if (e.type == SDL_MOUSEMOTION && ce.dragging) {
        int mx = e.motion.x, my = e.motion.y;
        int cx, cy; toCanvasXY(mx, my, cx, cy);
        cx = std::max(0, std::min(cx, ce.canvasW-1));
        cy = std::max(0, std::min(cy, ce.canvasH-1));

        if (ce.tool == CE_TOOL_PEN || ce.tool == CE_TOOL_ERASER) {
            ce_draw_line_on_surf(ce, ce.lastX, ce.lastY, cx, cy);
            ce_apply_to_texture(ce, r);
        }
        ce.lastX = cx; ce.lastY = cy;
        return true;
    }

    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && ce.dragging) {
        int mx = e.button.x, my = e.button.y;
        int cx, cy; toCanvasXY(mx, my, cx, cy);
        cx = std::max(0, std::min(cx, ce.canvasW-1));
        cy = std::max(0, std::min(cy, ce.canvasH-1));

        if (ce.tool == CE_TOOL_LINE) {
            ce_draw_line_on_surf(ce, ce.startX, ce.startY, cx, cy);
            ce_apply_to_texture(ce, r);
        } else if (ce.tool == CE_TOOL_RECT) {
            SDL_LockSurface(ce.canvasSurf);
            Uint32 col = SDL_MapRGBA(ce.canvasSurf->format,
                ce.drawColor.r, ce.drawColor.g, ce.drawColor.b, ce.drawColor.a);
            int x0 = std::min(ce.startX, cx), x1 = std::max(ce.startX, cx);
            int y0 = std::min(ce.startY, cy), y1 = std::max(ce.startY, cy);
            Uint32* pixels = (Uint32*)ce.canvasSurf->pixels;
            for (int xx = x0; xx <= x1; xx++) {
                if (y0 >= 0 && y0 < ce.canvasH && xx >= 0 && xx < ce.canvasW)
                    pixels[y0*ce.canvasW+xx] = col;
                if (y1 >= 0 && y1 < ce.canvasH && xx >= 0 && xx < ce.canvasW)
                    pixels[y1*ce.canvasW+xx] = col;
            }
            for (int yy = y0; yy <= y1; yy++) {
                if (x0 >= 0 && x0 < ce.canvasW && yy >= 0 && yy < ce.canvasH)
                    pixels[yy*ce.canvasW+x0] = col;
                if (x1 >= 0 && x1 < ce.canvasW && yy >= 0 && yy < ce.canvasH)
                    pixels[yy*ce.canvasW+x1] = col;
            }
            SDL_UnlockSurface(ce.canvasSurf);
            ce_apply_to_texture(ce, r);
        } else if (ce.tool == CE_TOOL_ELLIPSE) {
            int rx = std::abs(cx - ce.startX) / 2;
            int ry = std::abs(cy - ce.startY) / 2;
            int ocx = (ce.startX + cx) / 2;
            int ocy = (ce.startY + cy) / 2;
            SDL_LockSurface(ce.canvasSurf);
            Uint32 col = SDL_MapRGBA(ce.canvasSurf->format,
                ce.drawColor.r, ce.drawColor.g, ce.drawColor.b, ce.drawColor.a);
            Uint32* pixels = (Uint32*)ce.canvasSurf->pixels;
            for (int a = 0; a < 360; a++) {
                double rad = a * M_PI / 180.0;
                int px2 = ocx + (int)(rx * std::cos(rad));
                int py2 = ocy + (int)(ry * std::sin(rad));
                if (px2 >= 0 && px2 < ce.canvasW && py2 >= 0 && py2 < ce.canvasH)
                    pixels[py2*ce.canvasW+px2] = col;
            }
            SDL_UnlockSurface(ce.canvasSurf);
            ce_apply_to_texture(ce, r);
        }
        ce.dragging = false;
        return true;
    }

    return true;
}

inline void ce_render(CostumeEditor& ce, SDL_Renderer* r, TTF_Font* font, TTF_Font* fontBig) {
    if (!ce.isOpen) return;

    int wx = ce.winX, wy = ce.winY;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 140);
    SDL_Rect full = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(r, &full);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_SetRenderDrawColor(r, 245, 245, 248, 255);
    SDL_Rect win = {wx, wy, CE_W, CE_H};
    SDL_RenderFillRect(r, &win);
    SDL_SetRenderDrawColor(r, 180, 180, 200, 255);
    SDL_RenderDrawRect(r, &win);

    SDL_SetRenderDrawColor(r, COLOR_LOOKS.r, COLOR_LOOKS.g, COLOR_LOOKS.b, 255);
    SDL_Rect toolbar = {wx, wy, CE_W, CE_TOOLBAR_H};
    SDL_RenderFillRect(r, &toolbar);
    if (fontBig) draw_text(r, fontBig, "Costume Editor", wx+16, wy+16, COLOR_TEXT_WHITE);

    SDL_Rect closeBtn = {wx + CE_W - 36, wy + 8, 28, 28};
    SDL_SetRenderDrawColor(r, 200, 60, 60, 255);
    SDL_RenderFillRect(r, &closeBtn);
    if (font) draw_text_centered(r, font, "X", closeBtn, COLOR_TEXT_WHITE);

    SDL_SetRenderDrawColor(r, 230, 230, 235, 255);
    SDL_Rect sidebar = {wx, wy + CE_TOOLBAR_H, CE_SIDEBAR_W, CE_H - CE_TOOLBAR_H};
    SDL_RenderFillRect(r, &sidebar);
    SDL_SetRenderDrawColor(r, 200, 200, 210, 255);
    SDL_RenderDrawLine(r, wx + CE_SIDEBAR_W, wy + CE_TOOLBAR_H,
                           wx + CE_SIDEBAR_W, wy + CE_H);
    const char* toolNames[] = {"Pen","Eraser","Line","Rect","Ellipse","Fill"};
    SDL_Color toolColors[] = {
        {60,60,60,255},{200,200,200,255},{80,120,200,255},
        {200,120,80,255},{80,180,120,255},{220,160,40,255}
    };
    for (int i = 0; i < CE_TOOL_COUNT; i++) {
        SDL_Rect tb = {wx + 8, wy + CE_TOOLBAR_H + 12 + i * 42, CE_SIDEBAR_W - 16, 36};
        bool active = (ce.tool == (CETool)i);
        SDL_Color bc = active ? toolColors[i] : SDL_Color{210,210,215,255};
        SDL_SetRenderDrawColor(r, bc.r, bc.g, bc.b, 255);
        SDL_RenderFillRect(r, &tb);
        if (active) {
            SDL_SetRenderDrawColor(r, 80, 80, 200, 255);
            SDL_RenderDrawRect(r, &tb);
        }
        if (font) draw_text_centered(r, font, toolNames[i], tb,
            active ? COLOR_TEXT_WHITE : COLOR_TEXT_DARK);
    }

    SDL_Color palette8[] = {
        {0,0,0,255},{255,255,255,255},{220,50,50,255},{50,180,50,255},
        {50,50,220,255},{230,170,30,255},{150,60,200,255},{60,200,200,255}
    };
    int swX = wx + 8, swY = wy + CE_TOOLBAR_H + CE_TOOL_COUNT*42 + 24;
    if (font) draw_text(r, font, "Color:", swX, swY - 16, COLOR_TEXT_DARK);
    for (int i = 0; i < 8; i++) {
        int col = i % 4, row = i / 4;
        SDL_Rect sr = {swX + col*34, swY + row*34, 30, 30};
        SDL_SetRenderDrawColor(r, palette8[i].r, palette8[i].g, palette8[i].b, 255);
        SDL_RenderFillRect(r, &sr);
        SDL_SetRenderDrawColor(r, 100, 100, 100, 255);
        SDL_RenderDrawRect(r, &sr);
        if (palette8[i].r == ce.drawColor.r &&
            palette8[i].g == ce.drawColor.g &&
            palette8[i].b == ce.drawColor.b) {
            SDL_SetRenderDrawColor(r, 255, 255, 0, 255);
            SDL_Rect sel = {sr.x-2, sr.y-2, sr.w+4, sr.h+4};
            SDL_RenderDrawRect(r, &sel);
        }
    }

    // Current color preview
    SDL_Rect curCol = {swX + 4*34 + 4, swY, 20, 64};
    SDL_SetRenderDrawColor(r, ce.drawColor.r, ce.drawColor.g, ce.drawColor.b, 255);
    SDL_RenderFillRect(r, &curCol);
    SDL_SetRenderDrawColor(r, 100, 100, 100, 255);
    SDL_RenderDrawRect(r, &curCol);

    // Pen size
    int szY = swY + 80;
    if (font) draw_text(r, font, "Size:", swX, szY - 14, COLOR_TEXT_DARK);
    for (int i = 0; i < 4; i++) {
        int sz = (i+1)*3;
        SDL_Rect sr = {swX + i*40, szY, 36, 36};
        bool active = (ce.penSize == sz);
        SDL_SetRenderDrawColor(r, active ? 80:220, active ? 80:220, active ? 200:220, 255);
        SDL_RenderFillRect(r, &sr);
        SDL_SetRenderDrawColor(r, 100, 100, 100, 255);
        SDL_RenderDrawRect(r, &sr);
        // draw dot in center proportional to size
        SDL_SetRenderDrawColor(r, 30, 30, 30, 255);
        int dc = sz/2+1;
        for (int dy = -dc; dy <= dc; dy++) {
            int dx2 = (int)std::sqrt((double)(dc*dc - dy*dy));
            SDL_RenderDrawLine(r,
                sr.x+18-dx2, sr.y+18+dy,
                sr.x+18+dx2, sr.y+18+dy);
        }
    }

    // Clear button
    SDL_Rect clearBtn = {wx + 8, wy + CE_H - 48, CE_SIDEBAR_W - 16, 32};
    SDL_SetRenderDrawColor(r, 200, 80, 80, 255);
    SDL_RenderFillRect(r, &clearBtn);
    if (font) draw_text_centered(r, font, "Clear", clearBtn, COLOR_TEXT_WHITE);

    // ── Canvas area ────────────────────────────────────────────────────────────
    SDL_Rect canvasRect = {
        wx + CE_SIDEBAR_W + 12,
        wy + CE_TOOLBAR_H + 12,
        CE_CANVAS_W,
        CE_CANVAS_H
    };

    // checkerboard bg (transparent indicator)
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_RenderFillRect(r, &canvasRect);
    SDL_SetRenderDrawColor(r, 170, 170, 170, 255);
    for (int x = canvasRect.x; x < canvasRect.x+canvasRect.w; x+=16)
        for (int y = canvasRect.y; y < canvasRect.y+canvasRect.h; y+=16)
            if (((x/16)+(y/16)) % 2 == 0) {
                SDL_Rect cb = {x, y, 16, 16};
                SDL_RenderFillRect(r, &cb);
            }

    if (ce.canvasTex)
        SDL_RenderCopy(r, ce.canvasTex, nullptr, &canvasRect);

    // Shape preview while dragging
    if (ce.dragging &&
        (ce.tool == CE_TOOL_LINE || ce.tool == CE_TOOL_RECT || ce.tool == CE_TOOL_ELLIPSE)) {
        ce_draw_shape_preview(r, ce, canvasRect);
    }

    // Canvas border
    SDL_SetRenderDrawColor(r, 100, 100, 120, 255);
    SDL_RenderDrawRect(r, &canvasRect);

    // ── Save button ────────────────────────────────────────────────────────────
    SDL_Rect saveBtn = {canvasRect.x + canvasRect.w - 100, wy + CE_H - 48, 96, 32};
    SDL_SetRenderDrawColor(r, COLOR_LOOKS.r, COLOR_LOOKS.g, COLOR_LOOKS.b, 255);
    SDL_RenderFillRect(r, &saveBtn);
    if (font) draw_text_centered(r, font, "Save", saveBtn, COLOR_TEXT_WHITE);

    // ── Info bar ───────────────────────────────────────────────────────────────
    if (font) {
        std::string info = "Tool: ";
        info += toolNames[ce.tool];
        info += "  |  Ctrl+Z: Undo  |  Ctrl+Y: Redo";
        draw_text(r, font, info, canvasRect.x + 4, wy + CE_H - 42, {120,120,130,255});
    }
}

// ─── Internal helpers ─────────────────────────────────────────────────────────

static void ce_push_undo(CostumeEditor& ce) {
    if (!ce.canvasSurf) return;
    UndoFrame f;
    f.w = ce.canvasW; f.h = ce.canvasH;
    int sz = f.w * f.h;
    f.pixels.resize(sz);
    SDL_LockSurface(ce.canvasSurf);
    memcpy(f.pixels.data(), ce.canvasSurf->pixels, sz * sizeof(Uint32));
    SDL_UnlockSurface(ce.canvasSurf);
    ce.undoStack.push_back(f);
    if ((int)ce.undoStack.size() > CE_MAX_UNDO)
        ce.undoStack.erase(ce.undoStack.begin());
    ce.redoStack.clear();
}

static void ce_set_pixel(CostumeEditor& ce, int x, int y, SDL_Color col) {
    if (x < 0 || x >= ce.canvasW || y < 0 || y >= ce.canvasH) return;
    Uint32* pixels = (Uint32*)ce.canvasSurf->pixels;
    pixels[y * ce.canvasW + x] = SDL_MapRGBA(ce.canvasSurf->format,
        col.r, col.g, col.b, col.a);
}

static void ce_draw_line_on_surf(CostumeEditor& ce, int x0, int y0, int x1, int y1) {
    if (!ce.canvasSurf) return;
    SDL_Color col = (ce.tool == CE_TOOL_ERASER) ?
        SDL_Color{255,255,255,255} : ce.drawColor;

    SDL_LockSurface(ce.canvasSurf);
    int dx = std::abs(x1-x0), sx = x0<x1?1:-1;
    int dy = -std::abs(y1-y0), sy = y0<y1?1:-1;
    int err = dx+dy;
    int half = ce.penSize / 2;
    while (true) {
        for (int by = -half; by <= half; by++)
            for (int bx = -half; bx <= half; bx++)
                if (bx*bx+by*by <= half*half+1)
                    ce_set_pixel(ce, x0+bx, y0+by, col);
        if (x0==x1 && y0==y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    SDL_UnlockSurface(ce.canvasSurf);
}

static void ce_fill(CostumeEditor& ce, int x, int y, SDL_Color target, SDL_Color fill) {
    if (!ce.canvasSurf) return;
    // avoid infinite loop if same color
    if (target.r==fill.r && target.g==fill.g && target.b==fill.b && target.a==fill.a) return;

    Uint32 targetPx = SDL_MapRGBA(ce.canvasSurf->format,
        target.r, target.g, target.b, target.a);
    Uint32 fillPx   = SDL_MapRGBA(ce.canvasSurf->format,
        fill.r, fill.g, fill.b, fill.a);

    SDL_LockSurface(ce.canvasSurf);
    Uint32* pixels = (Uint32*)ce.canvasSurf->pixels;

    // iterative flood fill with a stack
    std::vector<std::pair<int,int>> stack;
    stack.push_back(std::make_pair(x, y));
    while (!stack.empty()) {
        std::pair<int,int> p = stack.back(); stack.pop_back();
        int cx = p.first;
        int cy = p.second;
        if (cx < 0 || cx >= ce.canvasW || cy < 0 || cy >= ce.canvasH) continue;
        if (pixels[cy*ce.canvasW+cx] != targetPx) continue;
        pixels[cy*ce.canvasW+cx] = fillPx;
        stack.push_back(std::make_pair(cx+1, cy));
        stack.push_back(std::make_pair(cx-1, cy));
        stack.push_back(std::make_pair(cx, cy+1));
        stack.push_back(std::make_pair(cx, cy-1));
    }

    SDL_UnlockSurface(ce.canvasSurf);
}

static void ce_apply_to_texture(CostumeEditor& ce, SDL_Renderer* r) {
    if (!ce.canvasSurf) return;
    if (ce.canvasTex) SDL_DestroyTexture(ce.canvasTex);
    ce.canvasTex = SDL_CreateTextureFromSurface(r, ce.canvasSurf);
}

static void ce_draw_shape_preview(SDL_Renderer* r, CostumeEditor& ce, SDL_Rect canvasRect) {
    int sx = canvasRect.x + ce.startX;
    int sy = canvasRect.y + ce.startY;
    int ex = canvasRect.x + ce.lastX;
    int ey = canvasRect.y + ce.lastY;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, ce.drawColor.r, ce.drawColor.g, ce.drawColor.b, 180);

    if (ce.tool == CE_TOOL_LINE) {
        SDL_RenderDrawLine(r, sx, sy, ex, ey);
    } else if (ce.tool == CE_TOOL_RECT) {
        SDL_Rect rv = {std::min(sx,ex), std::min(sy,ey),
                       std::abs(ex-sx), std::abs(ey-sy)};
        SDL_RenderDrawRect(r, &rv);
    } else if (ce.tool == CE_TOOL_ELLIPSE) {
        int ocx = (sx+ex)/2, ocy = (sy+ey)/2;
        int rx = std::abs(ex-sx)/2, ry2 = std::abs(ey-sy)/2;
        for (int a = 0; a < 360; a++) {
            double rad = a * M_PI / 180.0;
            int px2 = ocx + (int)(rx * std::cos(rad));
            int py2 = ocy + (int)(ry2 * std::sin(rad));
            SDL_RenderDrawPoint(r, px2, py2);
        }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

#endif // COSTUME_EDITOR_H



#ifndef SCRATCH_FOP_OPERATORMANAGER_H
#define SCRATCH_FOP_OPERATORMANAGER_H
#include <SDL2/SDL.h>
#include "structs.h"
#include "globals.h"

static SDL_Texture* g_penTrailTex = nullptr;

inline void pen_trail_init(SDL_Renderer* r) {
    g_penTrailTex = SDL_CreateTexture(r,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        STAGE_WIDTH, STAGE_HEIGHT);
    SDL_SetTextureBlendMode(g_penTrailTex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(r, g_penTrailTex);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderTarget(r, nullptr);
}

inline void pen_trail_clear(SDL_Renderer* r) {
    if (!g_penTrailTex) return;
    SDL_SetRenderTarget(r, g_penTrailTex);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderTarget(r, nullptr);
}

inline void pen_draw_line(SDL_Renderer* r,
                           int x0, int y0, int x1, int y1,
                           SDL_Color col, int size)
{
    if (!g_penTrailTex) return;
    SDL_SetRenderTarget(r, g_penTrailTex);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);

    int half = size / 2;
    if (half < 1) half = 1;
    int dx = abs(x1-x0), sx = x0<x1?1:-1;
    int dy = -abs(y1-y0), sy = y0<y1?1:-1;
    int err = dx+dy;
    while (true) {
        for (int by = -half; by <= half; by++)
            for (int bx = -half; bx <= half; bx++)
                if (bx*bx+by*by <= half*half+1)
                    SDL_RenderDrawPoint(r, x0+bx, y0+by);
        if (x0==x1 && y0==y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(r, nullptr);
}

inline void pen_stamp(SDL_Renderer* r, Sprite* sprite) {
    if (!g_penTrailTex || !sprite || !sprite->texture) return;
    SDL_SetRenderTarget(r, g_penTrailTex);
    int sw = (int)(sprite->w * sprite->scale);
    int sh = (int)(sprite->h * sprite->scale);
    SDL_Rect dst = {(int)sprite->x, (int)sprite->y, sw, sh};
    SDL_RenderCopy(r, sprite->texture, nullptr, &dst);
    SDL_SetRenderTarget(r, nullptr);
}

inline void pen_trail_render(SDL_Renderer* r, Stage* stage) {
    if (!g_penTrailTex) return;
    SDL_Rect dst = {stage->x, stage->y, stage->w, stage->h};
    SDL_RenderCopy(r, g_penTrailTex, nullptr, &dst);
}

inline void pen_trail_update(SDL_Renderer* r, Sprite* sprite) {
    if (!sprite || !sprite->penDown) {
        sprite->lastPenX = sprite->x;
        sprite->lastPenY = sprite->y;
        return;
    }
    float cx = sprite->x + sprite->w * sprite->scale / 2.0f;
    float cy = sprite->y + sprite->h * sprite->scale / 2.0f;

    if (sprite->lastPenX > -9998) {
        float lcx = sprite->lastPenX + sprite->w * sprite->scale / 2.0f;
        float lcy = sprite->lastPenY + sprite->h * sprite->scale / 2.0f;
        pen_draw_line(r,
            (int)lcx, (int)lcy,
            (int)cx,  (int)cy,
            sprite->penColor, sprite->penSize);
    }
    sprite->lastPenX = sprite->x;
    sprite->lastPenY = sprite->y;
}

#endif
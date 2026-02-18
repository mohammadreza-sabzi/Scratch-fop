//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_GLOBALS_H
#define SCRATCH_FOP_GLOBALS_H

#include <SDL2/SDL_pixels.h>

const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;

const int PALETTE_WIDTH     = 200;
const int CATEGORY_WIDTH    = 200;
const int CATEGORY_HEIGHT   = 35;
const int WORKSPACE_X       = PALETTE_WIDTH;
const int STAGE_WIDTH       = 480;
const int STAGE_HEIGHT      = 360;
const int STAGE_X           = SCREEN_WIDTH - STAGE_WIDTH;
const int STAGE_Y           = 0;
const int TOOLBAR_HEIGHT    = 40;
const int BLOCK_W           = 170;
const int BLOCK_H           = 36;
const int BLOCK_PADDING     = 6;
const int SNAP_DISTANCE     = 25;

// رنگ‌های دقیق Scratch MIT
const SDL_Color COLOR_MOTION    = {74,  144, 226, 255}; // آبی
const SDL_Color COLOR_LOOKS     = {155, 89,  182, 255}; // بنفش
const SDL_Color COLOR_SOUND     = {207, 74,  217, 255}; // صورتی
const SDL_Color COLOR_EVENTS    = {230, 168, 34,  255}; // زرد-نارنجی
const SDL_Color COLOR_CONTROL   = {229, 148, 0,   255}; // نارنجی
const SDL_Color COLOR_SENSING   = {92,  177, 214, 255}; // آبی روشن
const SDL_Color COLOR_OPERATORS = {89,  192, 89,  255}; // سبز
const SDL_Color COLOR_VARIABLES = {242, 100, 47,  255}; // نارنجی-قرمز

const SDL_Color COLOR_BG_PALETTE   = {243, 244, 245, 255};
const SDL_Color COLOR_BG_WORKSPACE = {255, 255, 255, 255};
const SDL_Color COLOR_BG_TOOLBAR   = {30,  30,  30,  255};
const SDL_Color COLOR_TEXT_DARK    = {30,  30,  30,  255};
const SDL_Color COLOR_TEXT_WHITE   = {255, 255, 255, 255};

#endif

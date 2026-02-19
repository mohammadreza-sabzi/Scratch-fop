//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_GLOBALS_H
#define SCRATCH_FOP_GLOBALS_H

#include <SDL2/SDL_pixels.h>

const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;

const int CAT_ICON_W    = 80;
const int BLOCK_LIST_W  = 180;
const int PALETTE_WIDTH = CAT_ICON_W + BLOCK_LIST_W;
const int STAGE_WIDTH   = 480;
const int STAGE_HEIGHT  = 360;
const int STAGE_X       = SCREEN_WIDTH - STAGE_WIDTH;
const int STAGE_Y       = 66;
const int WORKSPACE_X   = PALETTE_WIDTH;
const int WORKSPACE_W   = SCREEN_WIDTH - PALETTE_WIDTH - STAGE_WIDTH;

const int BLOCK_W       = 155;
const int BLOCK_H       = 36;
const int BLOCK_PADDING = 8;
const int SNAP_DISTANCE = 25;
const int CAT_CIRCLE_R  = 20;
const int CAT_ITEM_H    = 60;

const int COSTUME_PANEL_X = STAGE_X;
const int COSTUME_PANEL_Y = STAGE_Y + STAGE_HEIGHT;
const int COSTUME_PANEL_W = STAGE_WIDTH;
const int COSTUME_PANEL_H = SCREEN_HEIGHT - STAGE_Y - STAGE_HEIGHT;
const int COSTUME_THUMB   = 64;

const SDL_Color COLOR_MOTION    = {74,  144, 226, 255};
const SDL_Color COLOR_LOOKS     = {155, 89,  182, 255};
const SDL_Color COLOR_SOUND     = {207, 74,  217, 255};
const SDL_Color COLOR_EVENTS    = {230, 168, 34,  255};
const SDL_Color COLOR_CONTROL   = {229, 148, 0,   255};
const SDL_Color COLOR_SENSING   = {92,  177, 214, 255};
const SDL_Color COLOR_OPERATORS = {89,  192, 89,  255};
const SDL_Color COLOR_VARIABLES = {242, 100, 47,  255};
const SDL_Color COLOR_MYBLOCKS  = {194, 68,  68,  255};

const SDL_Color COLOR_BG_CATBAR    = {255, 255, 255, 255};
const SDL_Color COLOR_BG_BLOCKLIST = {250, 250, 250, 255};
const SDL_Color COLOR_BG_WORKSPACE = {255, 255, 255, 255};
const SDL_Color COLOR_TEXT_DARK    = {30,  30,  30,  255};
const SDL_Color COLOR_TEXT_WHITE   = {255, 255, 255, 255};
const SDL_Color COLOR_SCRATCH_PURPLE = {106, 74, 167, 255};
const SDL_Color COLOR_HEADER_BAR   = {100, 65,  165, 255};

#endif

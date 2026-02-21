//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_GLOBALS_H
#define SCRATCH_FOP_GLOBALS_H

#include <SDL2/SDL_pixels.h>

const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;

const int CAT_ICON_W    = 80;
const int BLOCK_LIST_W  = 185;
const int PALETTE_WIDTH = CAT_ICON_W + BLOCK_LIST_W;
const int STAGE_WIDTH   = 480;
const int STAGE_HEIGHT  = 360;
const int STAGE_X       = SCREEN_WIDTH - STAGE_WIDTH;

// ─── Tab Bar ───────────────────────────────────────────────────────────────
const int TAB_BAR_H     = 38;   // height of the tab strip at the very top
const int STAGE_Y       = 60;   // header bar height (unchanged)
// ──────────────────────────────────────────────────────────────────────────

const int WORKSPACE_X   = PALETTE_WIDTH;
const int WORKSPACE_W   = SCREEN_WIDTH - PALETTE_WIDTH - STAGE_WIDTH;

const int BLOCK_W       = 160;
const int BLOCK_H       = 36;
const int BLOCK_PADDING = 8;
const int SNAP_DISTANCE = 25;
const int CAT_CIRCLE_R  = 10;
const int CAT_ITEM_H    = 56;

const int COSTUME_PANEL_X = STAGE_X;
const int COSTUME_PANEL_Y = STAGE_Y + STAGE_HEIGHT;
const int COSTUME_PANEL_W = STAGE_WIDTH / 2;
const int COSTUME_PANEL_H = SCREEN_HEIGHT - STAGE_Y - STAGE_HEIGHT;
const int COSTUME_THUMB   = 52;
const int SPRITE_INFO_X = STAGE_X + STAGE_WIDTH / 2;
const int SPRITE_INFO_Y = STAGE_Y + STAGE_HEIGHT;
const int SPRITE_INFO_W = STAGE_WIDTH / 2;
const int SPRITE_INFO_H = SCREEN_HEIGHT - STAGE_Y - STAGE_HEIGHT;
const int VAR_PANEL_W   = 170;
const int VAR_PANEL_H   = 200;

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

// Tab bar colours  (matching Scratch UI)
const SDL_Color COLOR_TAB_ACTIVE   = {255, 255, 255, 255};   // white tab
const SDL_Color COLOR_TAB_INACTIVE = {210, 200, 230, 255};   // muted purple-grey
const SDL_Color COLOR_TAB_BG       = {130,  90, 175, 255};   // purple strip
const SDL_Color COLOR_TAB_TEXT_ACT = {106,  74, 167, 255};   // purple text on active
const SDL_Color COLOR_TAB_TEXT_INA = {255, 255, 255, 255};   // white text on inactive

#endif

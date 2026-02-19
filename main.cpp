#include <iostream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "globals.h"
#include "structs.h"
#include "utils.h"
#include "input.h"
#include "render.h"
#include "engine.h"

using namespace std;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    SDL_Window*   window   = SDL_CreateWindow("Scratch", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

#ifdef _WIN32
    const char* fontPathR = "C:\\Windows\\Fonts\\arial.ttf";
    const char* fontPathB = "C:\\Windows\\Fonts\\arialbd.ttf";
#else
    const char* fontPathR = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    const char* fontPathB = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
#endif
    TTF_Font* fontSmall = TTF_OpenFont(fontPathR, 12);
    TTF_Font* fontBig   = TTF_OpenFont(fontPathB, 16);
    if (!fontSmall) cerr << "Font error: " << TTF_GetError() << endl;

    SDL_Texture* playTex = nullptr, *stopTex = nullptr;
    {
        SDL_Surface* ps = IMG_Load("play1.png");
        if (ps) { playTex = SDL_CreateTextureFromSurface(renderer, ps); SDL_FreeSurface(ps); }
        SDL_Surface* ss = IMG_Load("stop1.png");
        if (ss) { stopTex = SDL_CreateTextureFromSurface(renderer, ss); SDL_FreeSurface(ss); }
    }

    Sprite sprite;
    sprite.x = STAGE_WIDTH/2.0f - 24;
    sprite.y = STAGE_HEIGHT/2.0f - 24;
    sprite.w = 48; sprite.h = 48;

    auto load_costume = [&](const char* path, const char* name) {
        SDL_Surface* s = IMG_Load(path);
        if (s) {
            Costume c;
            c.name    = name;
            c.texture = SDL_CreateTextureFromSurface(renderer, s);
            c.w = s->w; c.h = s->h;
            SDL_FreeSurface(s);
            sprite.costumes.push_back(c);
        } else {
            Costume c; c.name = name; c.texture = nullptr;
            c.w = 48; c.h = 48;
            sprite.costumes.push_back(c);
        }
    };
    load_costume("sprite.png",   "costume1");
    load_costume("sprite2.png",  "costume2");

    if (!sprite.costumes.empty() && sprite.costumes[0].texture)
        sprite.texture = sprite.costumes[0].texture;

    Stage stage = {STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT, {255,255,255,255}};
    Workspace workspace = {WORKSPACE_X, 0, WORKSPACE_W, SCREEN_HEIGHT};

    Palette palette;
    palette.activeCategory = CAT_MOTION;
    palette.scrollOffset   = 0;
    palette.catBarX = 0; palette.catBarY = 0;
    palette.catBarW = CAT_ICON_W; palette.catBarH = SCREEN_HEIGHT;
    palette.blockListX = CAT_ICON_W; palette.blockListY = 0;
    palette.blockListW = BLOCK_LIST_W; palette.blockListH = SCREEN_HEIGHT;

    int iy = 16;
    auto addCat = [&](CategoryType t, const string& n, SDL_Color col) {
        Category c; c.type = t; c.name = n; c.color = col;
        c.iconRect = {0, iy, CAT_ICON_W, CAT_ITEM_H};
        palette.categories.push_back(c);
        iy += CAT_ITEM_H + 4;
    };
    addCat(CAT_MOTION,    "Motion",    COLOR_MOTION);
    addCat(CAT_LOOKS,     "Looks",     COLOR_LOOKS);
    addCat(CAT_SOUND,     "Sound",     COLOR_SOUND);
    addCat(CAT_EVENTS,    "Events",    COLOR_EVENTS);
    addCat(CAT_CONTROL,   "Control",   COLOR_CONTROL);
    addCat(CAT_SENSING,   "Sensing",   COLOR_SENSING);
    addCat(CAT_OPERATORS, "Operators", COLOR_OPERATORS);
    addCat(CAT_VARIABLES, "Variables", COLOR_VARIABLES);
    addCat(CAT_MYBLOCKS,  "My Blocks", COLOR_MYBLOCKS);

    vector<Block*> paletteBlocks;
    int bid = 1;
    auto addPB = [&](BlockType t, const string& txt) {
        paletteBlocks.push_back(new Block{bid++, t, txt, 0,0,BLOCK_W,BLOCK_H,false,0,0,nullptr,nullptr});
    };

    addPB(BLOCK_MOTION, "move 10 steps");
    addPB(BLOCK_MOTION, "turn 15 degrees");
    addPB(BLOCK_MOTION, "go to x:0 y:0");
    addPB(BLOCK_MOTION, "point in direction 90");
    addPB(BLOCK_MOTION, "change x by 10");
    addPB(BLOCK_MOTION, "set x to 0");
    addPB(BLOCK_MOTION, "change y by 10");
    addPB(BLOCK_MOTION, "set y to 0");
    addPB(BLOCK_MOTION, "if on edge, bounce");

    addPB(BLOCK_LOOKS, "say Hello!");
    addPB(BLOCK_LOOKS, "say Hi for 2 secs");
    addPB(BLOCK_LOOKS, "think Hmm...");
    addPB(BLOCK_LOOKS, "show");
    addPB(BLOCK_LOOKS, "hide");
    addPB(BLOCK_LOOKS, "next costume");
    addPB(BLOCK_LOOKS, "switch costume to 0");
    addPB(BLOCK_LOOKS, "set size to 100%");
    addPB(BLOCK_LOOKS, "change size by 10");

    addPB(BLOCK_EVENT, "when flag clicked");
    addPB(BLOCK_EVENT, "when key pressed");
    addPB(BLOCK_EVENT, "when sprite clicked");

    addPB(BLOCK_CONTROL, "wait 1 secs");
    addPB(BLOCK_CONTROL, "repeat 10");
    addPB(BLOCK_CONTROL, "repeat 3");
    addPB(BLOCK_CONTROL, "forever");
    addPB(BLOCK_CONTROL, "if <> then");
    addPB(BLOCK_CONTROL, "stop all");

    addPB(BLOCK_SOUND, "play sound");
    addPB(BLOCK_SOUND, "stop all sounds");

    addPB(BLOCK_SENSING, "touching mouse-pointer?");
    addPB(BLOCK_SENSING, "touching edge?");
    addPB(BLOCK_SENSING, "key space pressed?");
    addPB(BLOCK_SENSING, "mouse x");
    addPB(BLOCK_SENSING, "mouse y");

    addPB(BLOCK_OPERATORS, "pick random 1 to 10");
    addPB(BLOCK_OPERATORS, "pick random 1 to 360");
    addPB(BLOCK_OPERATORS, "join hello world");

    SDL_Rect playBtn = {STAGE_X + 10, STAGE_Y - 56, 50, 50};
    SDL_Rect stopBtn = {STAGE_X + 66, STAGE_Y - 50, 38, 38};

    CostumePanel costumePanel;
    costumePanel.x = COSTUME_PANEL_X;
    costumePanel.y = COSTUME_PANEL_Y;
    costumePanel.w = COSTUME_PANEL_W;
    costumePanel.h = COSTUME_PANEL_H;
    costumePanel.scrollOffset  = 0;
    costumePanel.selectedIndex = 0;
    costumePanel.visible       = true;

    vector<Block*> workspaceBlocks;
    Block* draggedBlock = nullptr;
    bool   quit         = false;
    SDL_Event e;
    ScriptRunner scriptRunner;

    auto getActiveCatInfo = [&](string& name, SDL_Color& col) {
        for (auto& c : palette.categories) {
            if (c.type == palette.activeCategory) { name = c.name; col = c.color; return; }
        }
        name = ""; col = {100,100,100,255};
    };

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { quit = true; break; }

            if (e.type == SDL_MOUSEWHEEL) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (point_in_rect(mx, my, palette.blockListX, palette.blockListY,
                                  palette.blockListW, palette.blockListH))
                    handle_scroll_value(e, palette.scrollOffset);
                if (costumePanel.visible &&
                    point_in_rect(mx, my, costumePanel.x, costumePanel.y,
                                  costumePanel.w, costumePanel.h)) {
                    costumePanel.scrollOffset += e.wheel.y * 20;
                    if (costumePanel.scrollOffset > 0) costumePanel.scrollOffset = 0;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x, my = e.button.y;

                if (point_in_rect(mx, my, playBtn.x, playBtn.y, playBtn.w, playBtn.h)) {
                    Block* s = find_script_start(workspaceBlocks);
                    if (s) scriptRunner.start(s);
                }
                else if (point_in_rect(mx, my, stopBtn.x, stopBtn.y, stopBtn.w, stopBtn.h)) {
                    scriptRunner.stop();
                }
                else if (costumePanel.visible &&
                         point_in_rect(mx, my, costumePanel.x, costumePanel.y,
                                       costumePanel.w, costumePanel.h)) {
                    int itemH = COSTUME_THUMB + 28 + 6;
                    int relY  = my - costumePanel.y - 36 - costumePanel.scrollOffset;
                    if (relY >= 0) {
                        int idx = relY / itemH;
                        if (idx >= 0 && idx < (int)sprite.costumes.size()) {
                            costumePanel.selectedIndex = idx;
                            sprite.currentCostume      = idx;
                            if (sprite.costumes[idx].texture)
                                sprite.texture = sprite.costumes[idx].texture;
                        }
                    }
                }
                else if (point_in_rect(mx, my, palette.catBarX, palette.catBarY,
                                       palette.catBarW, palette.catBarH)) {
                    handle_category_click(mx, my, palette);
                }
                else if (point_in_rect(mx, my, palette.blockListX, palette.blockListY,
                                       palette.blockListW, palette.blockListH)) {
                    Block* clicked = check_palette_click(mx, my, paletteBlocks, palette);
                    if (clicked) {
                        Block* nb = clone_block(clicked);
                        nb->x = mx - nb->w/2; nb->y = my - nb->h/2;
                        nb->isDragging = true;
                        nb->dragOffsetX = nb->w/2; nb->dragOffsetY = nb->h/2;
                        workspaceBlocks.push_back(nb);
                        draggedBlock = nb;
                    }
                }
                else if (point_in_rect(mx, my, workspace.x, workspace.y,
                                       workspace.w, workspace.h)) {
                    handle_mouse_down(e, workspaceBlocks, &draggedBlock);
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                handle_mouse_up(&draggedBlock, workspaceBlocks, workspace);
            }
            else if (e.type == SDL_MOUSEMOTION) {
                handle_mouse_motion(e, &draggedBlock, workspace);
            }
        }

        layout_palette_blocks(paletteBlocks, palette);
        scriptRunner.update(&sprite);
        if (sprite.sayTimer > 0) { sprite.sayTimer--; if (sprite.sayTimer == 0) sprite.sayText = ""; }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, COLOR_HEADER_BAR.r, COLOR_HEADER_BAR.g,
                               COLOR_HEADER_BAR.b, 255);
        SDL_Rect headerBar = {STAGE_X, 0, STAGE_WIDTH, STAGE_Y};
        SDL_RenderFillRect(renderer, &headerBar);

        draw_category_bar(renderer, fontSmall, palette);

        string activeName; SDL_Color activeCol;
        getActiveCatInfo(activeName, activeCol);
        draw_block_list_header(renderer, fontBig, palette, activeName, activeCol);

        for (Block* b : paletteBlocks)
            if (block_matches_category(b, palette.activeCategory))
                draw_block(renderer, fontSmall, b);

        draw_workspace_bg(renderer, workspace);
        for (Block* b : workspaceBlocks) draw_block(renderer, fontSmall, b);

        draw_stage(renderer, &stage);
        draw_sprite(renderer, &sprite, &stage, fontSmall);

        if (costumePanel.visible)
            draw_costume_panel(renderer, fontSmall, fontBig, costumePanel, &sprite);

        draw_play_stop_buttons(renderer, fontSmall, playBtn, stopBtn,
                               playTex, stopTex, scriptRunner.running);

        SDL_RenderPresent(renderer);
    }

    for (auto b : paletteBlocks)   delete b;
    for (auto b : workspaceBlocks) delete b;
    for (auto& c : sprite.costumes) if (c.texture) SDL_DestroyTexture(c.texture);
    if (playTex) SDL_DestroyTexture(playTex);
    if (stopTex) SDL_DestroyTexture(stopTex);
    TTF_CloseFont(fontSmall);
    if (fontBig) TTF_CloseFont(fontBig);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}



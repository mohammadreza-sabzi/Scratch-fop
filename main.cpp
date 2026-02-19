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

    SDL_Window* window = SDL_CreateWindow(
        "Scratch",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


#ifdef _WIN32
    const char* fontPathRegular = "C:\\Windows\\Fonts\\arial.ttf";
    const char* fontPathBold    = "C:\\Windows\\Fonts\\arialbd.ttf";
#else
    const char* fontPathRegular = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    const char* fontPathBold    = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
#endif
    TTF_Font* fontSmall = TTF_OpenFont(fontPathRegular, 12);
    TTF_Font* fontBig   = TTF_OpenFont(fontPathBold,    18);
    if (!fontSmall) cerr << "Font error: " << TTF_GetError() << endl;


    SDL_Texture* playTex = nullptr;
    SDL_Texture* stopTex = nullptr;
    {
        SDL_Surface* ps = IMG_Load("play1.png");
        if (ps) { playTex = SDL_CreateTextureFromSurface(renderer, ps);
                  SDL_FreeSurface(ps); }
        SDL_Surface* ss = IMG_Load("stop1.png");
        if (ss) { stopTex = SDL_CreateTextureFromSurface(renderer, ss);
                  SDL_FreeSurface(ss); }
    }

    SDL_Surface* spriteSurf = IMG_Load("sprite.png");
    SDL_Texture* spriteTex  = nullptr;
    int texW = 48, texH = 48;
    if (spriteSurf) {
        spriteTex = SDL_CreateTextureFromSurface(renderer, spriteSurf);
        texW = spriteSurf->w; texH = spriteSurf->h;
        SDL_FreeSurface(spriteSurf);
    }
    Sprite sprite = {
        (float)(STAGE_WIDTH/2 - texW/2),
        (float)(STAGE_HEIGHT/2 - texH/2),
        texW, texH, spriteTex,
        true, "", 0
    };

    Stage stage = {STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT,
                   {255,255,255,255}};


    Workspace workspace = {WORKSPACE_X, 0, WORKSPACE_W, SCREEN_HEIGHT};


    Palette palette;
    palette.activeCategory = CAT_MOTION;
    palette.scrollOffset   = 0;


    palette.catBarX = 0;
    palette.catBarY = 0;
    palette.catBarW = CAT_ICON_W;
    palette.catBarH = SCREEN_HEIGHT;


    palette.blockListX = CAT_ICON_W;
    palette.blockListY = 0;
    palette.blockListW = BLOCK_LIST_W;
    palette.blockListH = SCREEN_HEIGHT;


    int iy = 16;
    auto addCat = [&](CategoryType t, const string& n, SDL_Color col) {
        Category c;
        c.type     = t;
        c.name     = n;
        c.color    = col;
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
    auto addPB = [&](BlockType t, const string& txt) {
        paletteBlocks.push_back(new Block{
            (int)paletteBlocks.size()+1, t, txt,
            0, 0, BLOCK_W, BLOCK_H,
            false, 0, 0, nullptr, nullptr
        });
    };
    addPB(BLOCK_MOTION, "move 10 steps");
    addPB(BLOCK_MOTION, "turn 15 degrees");
    addPB(BLOCK_MOTION, "go to x:0 y:0");
    addPB(BLOCK_MOTION, "glide 1 secs to x:0 y:0");
    addPB(BLOCK_MOTION, "point in direction 90");
    addPB(BLOCK_MOTION, "change x by 10");
    addPB(BLOCK_MOTION, "set x to 0");
    addPB(BLOCK_MOTION, "change y by 10");
    addPB(BLOCK_MOTION, "set y to 0");

    addPB(BLOCK_LOOKS, "say Hello!");
    addPB(BLOCK_LOOKS, "say Hi for 2 secs");
    addPB(BLOCK_LOOKS, "show");
    addPB(BLOCK_LOOKS, "hide");
    addPB(BLOCK_LOOKS, "set size to 100%");
    addPB(BLOCK_LOOKS, "change size by 10");

    addPB(BLOCK_EVENT, "when flag clicked");
    addPB(BLOCK_EVENT, "when key pressed");
    addPB(BLOCK_EVENT, "when sprite clicked");

    addPB(BLOCK_CONTROL, "wait 1 secs");
    addPB(BLOCK_CONTROL, "repeat 10");
    addPB(BLOCK_CONTROL, "forever");
    addPB(BLOCK_CONTROL, "if <> then");
    addPB(BLOCK_CONTROL, "stop all");

    addPB(BLOCK_SOUND, "play sound");
    addPB(BLOCK_SOUND, "stop all sounds");

    addPB(BLOCK_SENSING, "touching mouse-pointer?");
    addPB(BLOCK_SENSING, "touching edge?");

    addPB(BLOCK_OPERATORS, "pick random 1 to 10");
    addPB(BLOCK_OPERATORS, "join hello world");

    SDL_Rect playBtn = {STAGE_X + 10,   8, 50, 50};
    SDL_Rect stopBtn = {STAGE_X + 60,   14, 38, 38};

    vector<Block*> workspaceBlocks;
    Block*  draggedBlock = nullptr;
    bool    quit         = false;
    SDL_Event e;
    ScriptRunner scriptRunner;

    auto getActiveCatInfo = [&](string& name, SDL_Color& col) {
        for (auto& c : palette.categories) {
            if (c.type == palette.activeCategory) {
                name = c.name;
                col  = c.color;
                return;
            }
        }
        name = ""; col = {100,100,100,255};
    };

    while (!quit) {

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) { quit = true; break; }

            if (e.type == SDL_MOUSEWHEEL) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (point_in_rect(mx, my,
                    palette.blockListX, palette.blockListY,
                    palette.blockListW, palette.blockListH)) {
                    handle_scroll_value(e, palette.scrollOffset);
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN &&
                e.button.button == SDL_BUTTON_LEFT)
            {
                int mx = e.button.x;
                int my = e.button.y;

                // Play
                if (point_in_rect(mx, my,
                    playBtn.x, playBtn.y, playBtn.w, playBtn.h)) {
                    Block* s = find_script_start(workspaceBlocks);
                    if (s) scriptRunner.start(s);
                }
                // Stop
                else if (point_in_rect(mx, my,
                    stopBtn.x, stopBtn.y, stopBtn.w, stopBtn.h)) {
                    scriptRunner.stop();
                }

                else if (point_in_rect(mx, my,
                    palette.catBarX, palette.catBarY,
                    palette.catBarW, palette.catBarH)) {
                    handle_category_click(mx, my, palette);
                }
                else if (point_in_rect(mx, my,
                    palette.blockListX, palette.blockListY,
                    palette.blockListW, palette.blockListH)) {
                    Block* clicked = check_palette_click(
                        mx, my, paletteBlocks, palette);
                    if (clicked) {
                        Block* nb = clone_block(clicked);
                        nb->x = mx - nb->w / 2;
                        nb->y = my - nb->h / 2;
                        nb->isDragging  = true;
                        nb->dragOffsetX = nb->w / 2;
                        nb->dragOffsetY = nb->h / 2;
                        workspaceBlocks.push_back(nb);
                        draggedBlock = nb;
                    }
                }
                // Workspace
                else if (point_in_rect(mx, my,
                    workspace.x, workspace.y,
                    workspace.w, workspace.h)) {
                    handle_mouse_down(e, workspaceBlocks, &draggedBlock);
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP &&
                     e.button.button == SDL_BUTTON_LEFT) {
                handle_mouse_up(&draggedBlock, workspaceBlocks, workspace);
            }
            else if (e.type == SDL_MOUSEMOTION) {
                handle_mouse_motion(e, &draggedBlock, workspace);
            }
        }

        layout_palette_blocks(paletteBlocks, palette);
        scriptRunner.update(&sprite);
        if (sprite.sayTimer > 0) {
            sprite.sayTimer--;
            if (sprite.sayTimer == 0) sprite.sayText = "";
        }

     //رندر
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        // ستون دایره‌ها
        draw_category_bar(renderer, fontSmall, palette);


        string activeName; SDL_Color activeCol;
        getActiveCatInfo(activeName, activeCol);
        draw_block_list_header(renderer, fontBig,
                               palette, activeName, activeCol);

        for (Block* b : paletteBlocks) {
            if (block_matches_category(b, palette.activeCategory)) {
                draw_block(renderer, fontSmall, b);
            }
        }

        // Workspace
        draw_workspace_bg(renderer, workspace);
        for (Block* b : workspaceBlocks) {
            draw_block(renderer, fontSmall, b);
        }

        // Stage
        draw_stage(renderer, &stage);
        draw_sprite(renderer, &sprite, &stage, fontSmall);

        // Play/Stop
        draw_play_stop_buttons(renderer, fontSmall,
                                playBtn, stopBtn,
                                playTex, stopTex,
                                scriptRunner.running);

        SDL_RenderPresent(renderer);
    }

    for (auto b : paletteBlocks)   delete b;
    for (auto b : workspaceBlocks) delete b;
    if (spriteTex) SDL_DestroyTexture(spriteTex);
    if (playTex)   SDL_DestroyTexture(playTex);
    if (stopTex)   SDL_DestroyTexture(stopTex);
    TTF_CloseFont(fontSmall);
    if (fontBig) TTF_CloseFont(fontBig);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}


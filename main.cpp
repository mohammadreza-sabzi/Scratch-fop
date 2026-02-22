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
#include "costume_editor.h"

using namespace std;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    SDL_Window*   window   = SDL_CreateWindow("Scratch",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    sprite.x = STAGE_WIDTH/2.0f - 48;
    sprite.y = STAGE_HEIGHT/2.0f - 48;
    sprite.w = 96; sprite.h = 96;

    CostumeEditor costumeEditor;
    ce_init(costumeEditor);

    Uint32 lastCostumeClickTime = 0;
    int    lastCostumeClickIdx  = -1;

    auto load_costume = [&](const char* path, const char* name) {
        SDL_Surface* s = IMG_Load(path);
        Costume c;
        c.name = name;
        if (s) {
            c.texture = SDL_CreateTextureFromSurface(renderer, s);
            c.w = s->w; c.h = s->h;
            SDL_FreeSurface(s);
        } else {
            c.texture = nullptr;
            c.w = 96; c.h = 96;
        }
        sprite.costumes.push_back(c);
    };
    load_costume("sprite.png",  "costume1");
    load_costume("sprite2.png", "costume2");

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
        Block* b = new Block{bid++, t, txt, 0,0,BLOCK_W,BLOCK_H,false,0,0,nullptr,nullptr};
        init_block_inputs(b);
        paletteBlocks.push_back(b);
    };

    //MOTION
    addPB(BLOCK_MOTION, "move 10 steps");
    addPB(BLOCK_MOTION, "turn 15 degrees");
    addPB(BLOCK_MOTION, "turn left 15 degrees");
    addPB(BLOCK_MOTION, "go to x:0 y:0");
    addPB(BLOCK_MOTION, "go to random position");
    addPB(BLOCK_MOTION, "go to mouse-pointer");
    addPB(BLOCK_MOTION, "glide 1 secs to x:0 y:0");
    addPB(BLOCK_MOTION, "point in direction 90");
    addPB(BLOCK_MOTION, "point towards mouse-pointer");
    addPB(BLOCK_MOTION, "change x by 10");
    addPB(BLOCK_MOTION, "set x to 0");
    addPB(BLOCK_MOTION, "change y by 10");
    addPB(BLOCK_MOTION, "set y to 0");
    addPB(BLOCK_MOTION, "if on edge, bounce");

    //LOOKS
    addPB(BLOCK_LOOKS, "say Hello!");
    addPB(BLOCK_LOOKS, "say Hello! for 2 secs");
    addPB(BLOCK_LOOKS, "think Hmm...");
    addPB(BLOCK_LOOKS, "show");
    addPB(BLOCK_LOOKS, "hide");
    addPB(BLOCK_LOOKS, "next costume");
    addPB(BLOCK_LOOKS, "switch costume to 0");
    addPB(BLOCK_LOOKS, "set size to 100");
    addPB(BLOCK_LOOKS, "change size by 10");

    //SOUND
    addPB(BLOCK_SOUND, "play sound Meow");
    addPB(BLOCK_SOUND, "play sound Meow until done");
    addPB(BLOCK_SOUND, "stop all sounds");
    addPB(BLOCK_SOUND, "change volume by 10");
    addPB(BLOCK_SOUND, "change volume by -10");
    addPB(BLOCK_SOUND, "set volume to 100");
    addPB(BLOCK_SOUND, "change pitch effect by 10");
    addPB(BLOCK_SOUND, "set pitch effect to 100");
    addPB(BLOCK_SOUND, "clear sound effects");

    //EVENTS
    addPB(BLOCK_EVENT, "when flag clicked");
    addPB(BLOCK_EVENT, "when key pressed");
    addPB(BLOCK_EVENT, "when sprite clicked");
    addPB(BLOCK_EVENT, "when backdrop switches to");
    addPB(BLOCK_EVENT, "broadcast message");
    addPB(BLOCK_EVENT, "broadcast message and wait");

    //CONTROL
    addPB(BLOCK_CONTROL, "wait 1 secs");
    addPB(BLOCK_CONTROL, "repeat 10");
    addPB(BLOCK_CONTROL, "repeat 3");
    addPB(BLOCK_CONTROL, "forever");
    addPB(BLOCK_CONTROL, "if <> then");
    addPB(BLOCK_CONTROL, "if <> then else");
    addPB(BLOCK_CONTROL, "wait until <>");
    addPB(BLOCK_CONTROL, "stop all");
    addPB(BLOCK_CONTROL, "stop this script");
    addPB(BLOCK_CONTROL, "stop other scripts");

    //SENSING
    addPB(BLOCK_SENSING, "ask What's your name? and wait");
    addPB(BLOCK_SENSING, "answer");
    addPB(BLOCK_SENSING, "touching mouse-pointer?");
    addPB(BLOCK_SENSING, "touching edge?");
    addPB(BLOCK_SENSING, "touching color?");
    addPB(BLOCK_SENSING, "key space pressed?");
    addPB(BLOCK_SENSING, "mouse down?");
    addPB(BLOCK_SENSING, "mouse x");
    addPB(BLOCK_SENSING, "mouse y");
    addPB(BLOCK_SENSING, "loudness");
    addPB(BLOCK_SENSING, "timer");
    addPB(BLOCK_SENSING, "reset timer");
    addPB(BLOCK_SENSING, "distance to mouse-pointer");

    //OPERATORS
    addPB(BLOCK_OPERATORS, "() + ()");
    addPB(BLOCK_OPERATORS, "() - ()");
    addPB(BLOCK_OPERATORS, "() * ()");
    addPB(BLOCK_OPERATORS, "() / ()");
    addPB(BLOCK_OPERATORS, "() mod ()");
    addPB(BLOCK_OPERATORS, "() < ()");
    addPB(BLOCK_OPERATORS, "() > ()");
    addPB(BLOCK_OPERATORS, "() = ()");
    addPB(BLOCK_OPERATORS, "<> and <>");
    addPB(BLOCK_OPERATORS, "<> or <>");
    addPB(BLOCK_OPERATORS, "not <>");
    addPB(BLOCK_OPERATORS, "pick random 1 to 10");
    addPB(BLOCK_OPERATORS, "pick random 1 to 360");
    addPB(BLOCK_OPERATORS, "round ()");
    addPB(BLOCK_OPERATORS, "abs of ()");
    addPB(BLOCK_OPERATORS, "sqrt of ()");
    addPB(BLOCK_OPERATORS, "floor of ()");
    addPB(BLOCK_OPERATORS, "ceiling of ()");
    addPB(BLOCK_OPERATORS, "sin of ()");
    addPB(BLOCK_OPERATORS, "cos of ()");
    addPB(BLOCK_OPERATORS, "join hello world");
    addPB(BLOCK_OPERATORS, "letter 1 of hello");
    addPB(BLOCK_OPERATORS, "length of hello");
    addPB(BLOCK_OPERATORS, "contains hello world");

    //VARIABLES
    addPB(BLOCK_VARIABLES, "set score to 0");
    addPB(BLOCK_VARIABLES, "change score by 1");
    addPB(BLOCK_VARIABLES, "show variable score");
    addPB(BLOCK_VARIABLES, "hide variable score");

    VariablesPanel varsPanel;
    varsPanel.x = 0; varsPanel.y = 0;
    varsPanel.w = VAR_PANEL_W; varsPanel.h = VAR_PANEL_H;
    varsPanel.visible = true;
    varsPanel.variables.push_back({"score", 0.0f, true});

    bool myBlocksCreating = false;
    string newBlockName = "";
    vector<string> customBlocks;

    CostumePanel costumePanel;
    costumePanel.x = COSTUME_PANEL_X;
    costumePanel.y = COSTUME_PANEL_Y;
    costumePanel.w = COSTUME_PANEL_W;
    costumePanel.h = COSTUME_PANEL_H;
    costumePanel.scrollOffset  = 0;
    costumePanel.selectedIndex = 0;
    costumePanel.visible       = true;

    SDL_Rect playBtn = {STAGE_X + 10, STAGE_Y - 52, 46, 46};
    SDL_Rect stopBtn = {STAGE_X + 62, STAGE_Y - 46, 34, 34};

    vector<Block*> workspaceBlocks;
    Block* draggedBlock = nullptr;
    BlockInput* activeInput = nullptr;
    bool   quit         = false;
    SDL_Event e;
    ScriptRunner scriptRunner;

    auto getActiveCatInfo = [&](string& name, SDL_Color& col) {
        for (auto& c : palette.categories) {
            if (c.type == palette.activeCategory) { name = c.name; col = c.color; return; }
        }
        name = ""; col = {100,100,100,255};
    };

    SDL_Rect makeVarBtn    = {0, 0, 0, 0};
    SDL_Rect makeBlockBtn  = {0, 0, 0, 0};
    bool isVarCat   = false;
    bool isMyBlocks = false;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { quit = true; break; }

            if (costumeEditor.isOpen) {
                ce_handle_event(costumeEditor, e, renderer, &sprite);
                continue;
            }

            if (activeInput && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER ||
                    e.key.keysym.sym == SDLK_ESCAPE) {
                    activeInput->editing = false;
                    activeInput = nullptr;
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && !activeInput->value.empty()) {
                    activeInput->value.pop_back();
                }
                continue;
            }
            if (activeInput && e.type == SDL_TEXTINPUT) {
                std::string ch = e.text.text;
                for (char c : ch) {
                    if (std::isdigit(c) || c == '.' || (c == '-' && activeInput->value.empty()))
                        activeInput->value += c;
                }
                continue;
            }

            if (varsPanel.creating && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (!varsPanel.newVarName.empty()) {
                        Variable nv;
                        nv.name = varsPanel.newVarName;
                        nv.value = 0.0f;
                        nv.showOnStage = true;
                        varsPanel.variables.push_back(nv);
                        addPB(BLOCK_VARIABLES, "set " + varsPanel.newVarName + " to 0");
                        addPB(BLOCK_VARIABLES, "change " + varsPanel.newVarName + " by 1");
                        addPB(BLOCK_VARIABLES, "show variable " + varsPanel.newVarName);
                        addPB(BLOCK_VARIABLES, "hide variable " + varsPanel.newVarName);
                    }
                    varsPanel.creating = false;
                    varsPanel.newVarName = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    varsPanel.creating = false;
                    varsPanel.newVarName = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && !varsPanel.newVarName.empty()) {
                    varsPanel.newVarName.pop_back();
                }
                continue;
            }
            if (varsPanel.creating && e.type == SDL_TEXTINPUT) {
                varsPanel.newVarName += e.text.text;
                continue;
            }

            if (myBlocksCreating && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (!newBlockName.empty()) {
                        customBlocks.push_back(newBlockName);
                        addPB(BLOCK_MYBLOCKS, newBlockName);
                    }
                    myBlocksCreating = false;
                    newBlockName = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    myBlocksCreating = false;
                    newBlockName = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && !newBlockName.empty()) {
                    newBlockName.pop_back();
                }
                continue;
            }
            if (myBlocksCreating && e.type == SDL_TEXTINPUT) {
                newBlockName += e.text.text;
                continue;
            }

            // SCROLL
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

            // MOUSE DOWN
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x, my = e.button.y;

                if (activeInput) {
                    activeInput->editing = false;
                    activeInput = nullptr;
                    SDL_StopTextInput();
                }

                if (point_in_rect(mx, my, playBtn.x, playBtn.y, playBtn.w, playBtn.h)) {
                    Block* s = find_script_start(workspaceBlocks);
                    if (s) scriptRunner.start(s, &varsPanel.variables);
                }
                else if (point_in_rect(mx, my, stopBtn.x, stopBtn.y, stopBtn.w, stopBtn.h)) {
                    scriptRunner.stop();
                    sprite.sayText = ""; sprite.sayTimer = 0;
                }
                else if (isVarCat && makeVarBtn.w > 0 &&
                         point_in_rect(mx, my, makeVarBtn.x, makeVarBtn.y,
                                       makeVarBtn.w, makeVarBtn.h)) {
                    varsPanel.creating = true;
                    varsPanel.newVarName = "";
                    SDL_StartTextInput();
                    continue;
                }
                else if (isMyBlocks && makeBlockBtn.w > 0 &&
                         point_in_rect(mx, my, makeBlockBtn.x, makeBlockBtn.y,
                                       makeBlockBtn.w, makeBlockBtn.h)) {
                    myBlocksCreating = true;
                    newBlockName = "";
                    SDL_StartTextInput();
                    continue;
                }

                if (costumePanel.visible &&
                    point_in_rect(mx, my, costumePanel.x, costumePanel.y,
                                  costumePanel.w, costumePanel.h)) {
                    int itemH = COSTUME_THUMB + 24 + 4;
                    int relY  = my - costumePanel.y - 36 - costumePanel.scrollOffset;
                    if (relY >= 0) {
                        int idx = relY / itemH;
                        if (idx >= 0 && idx < (int)sprite.costumes.size()) {
                            Uint32 now = SDL_GetTicks();
                            if (idx == lastCostumeClickIdx && (now - lastCostumeClickTime) < 400) {
                                ce_open(costumeEditor, renderer, &sprite, idx);
                                lastCostumeClickTime = 0;
                                lastCostumeClickIdx  = -1;
                            } else {
                                lastCostumeClickTime = now;
                                lastCostumeClickIdx  = idx;
                                costumePanel.selectedIndex = idx;
                                sprite.currentCostume      = idx;
                                if (sprite.costumes[idx].texture)
                                    sprite.texture = sprite.costumes[idx].texture;
                            }
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
                    BlockInput* clicked_inp = check_input_click(mx, my, workspaceBlocks);
                    if (clicked_inp) {
                        activeInput = clicked_inp;
                        activeInput->editing = true;
                        activeInput->value = "";
                        SDL_StartTextInput();
                        continue;
                    }
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

        isVarCat   = (palette.activeCategory == CAT_VARIABLES);
        isMyBlocks = (palette.activeCategory == CAT_MYBLOCKS);

        // RENDER
        SDL_SetRenderDrawColor(renderer, 200, 200, 205, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, COLOR_HEADER_BAR.r, COLOR_HEADER_BAR.g,
                               COLOR_HEADER_BAR.b, 255);
        SDL_Rect headerBar = {STAGE_X, 0, STAGE_WIDTH, STAGE_Y};
        SDL_RenderFillRect(renderer, &headerBar);

        draw_category_bar(renderer, fontSmall, palette);
        string activeName; SDL_Color activeCol;
        getActiveCatInfo(activeName, activeCol);

        if (isVarCat) {
            draw_block_list_header(renderer, fontBig, palette, activeName, activeCol,
                                   true, &makeVarBtn);
            makeBlockBtn = {0,0,0,0};
        } else if (isMyBlocks) {
            draw_block_list_header(renderer, fontBig, palette, activeName, activeCol,
                                   true, &makeBlockBtn);
            makeVarBtn = {0,0,0,0};
        } else {
            draw_block_list_header(renderer, fontBig, palette, activeName, activeCol,
                                   false, nullptr);
            makeVarBtn   = {0,0,0,0};
            makeBlockBtn = {0,0,0,0};
        }

        for (Block* b : paletteBlocks)
            if (block_matches_category(b, palette.activeCategory))
                draw_block(renderer, fontSmall, b);

        draw_workspace_bg(renderer, workspace);
        for (Block* b : workspaceBlocks) {
            update_block_input_rects(b);
            draw_block(renderer, fontSmall, b);
        }

        auto draw_name_dialog = [&](const string& title, const string& inputText) {
            int ox = workspace.x + workspace.w/2 - 160;
            int oy = workspace.y + workspace.h/2 - 40;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
            SDL_Rect overlay = {workspace.x, workspace.y, workspace.w, workspace.h};
            SDL_RenderFillRect(renderer, &overlay);

            SDL_SetRenderDrawColor(renderer, 250, 250, 255, 255);
            SDL_Rect box = {ox, oy, 320, 80};
            SDL_RenderFillRect(renderer, &box);
            SDL_SetRenderDrawColor(renderer, 150, 150, 200, 255);
            SDL_RenderDrawRect(renderer, &box);

            if (fontBig) draw_text(renderer, fontBig, title, ox+12, oy+10, COLOR_TEXT_DARK);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect field = {ox+12, oy+34, 220, 26};
            SDL_RenderFillRect(renderer, &field);
            SDL_SetRenderDrawColor(renderer, 100, 100, 220, 255);
            SDL_RenderDrawRect(renderer, &field);
            if (fontSmall)
                draw_text(renderer, fontSmall, inputText + "|", ox+16, oy+40, COLOR_TEXT_DARK);

            SDL_SetRenderDrawColor(renderer, 100, 100, 220, 255);
            SDL_Rect okBtn = {ox+244, oy+34, 64, 26};
            SDL_RenderFillRect(renderer, &okBtn);
            if (fontSmall) draw_text_centered(renderer, fontSmall, "OK", okBtn, COLOR_TEXT_WHITE);
        };

        if (varsPanel.creating)
            draw_name_dialog("New Variable Name:", varsPanel.newVarName);
        if (myBlocksCreating)
            draw_name_dialog("Block Name:", newBlockName);

        draw_stage(renderer, &stage);
        draw_sprite(renderer, &sprite, &stage, fontSmall);
        draw_variable_monitors(renderer, fontSmall, varsPanel, &stage);

        draw_costume_panel(renderer, fontSmall, fontBig, costumePanel, &sprite);
        draw_sprite_info_panel(renderer, fontSmall, fontBig, &sprite);

        draw_play_stop_buttons(renderer, fontSmall, playBtn, stopBtn,
                               playTex, stopTex, scriptRunner.running);

        ce_render(costumeEditor, renderer, fontSmall, fontBig);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    if (costumeEditor.canvasTex)  SDL_DestroyTexture(costumeEditor.canvasTex);
    if (costumeEditor.canvasSurf) SDL_FreeSurface(costumeEditor.canvasSurf);

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
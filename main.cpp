#include <iostream>
#include <vector>
#include <string>
#include <sstream>
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
#include "tab_bar.h"
#include "audio.h"
#include "Sound_panel.h"

using namespace std;

SoundsPanel* g_soundsPanel = nullptr;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
    audio_init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    SDL_Window* window = SDL_CreateWindow("Scratch",
                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                          SCREEN_WIDTH, SCREEN_HEIGHT,
                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

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

    ActiveTab activeTab = TAB_CODE;

    SDL_Rect gearBtn     = {0,0,0,0};
    SDL_Rect notesBtn    = {0,0,0,0};
    SDL_Rect penBtn      = {0,0,0,0};
    SDL_Rect bulbBtn     = {0,0,0,0};
    SDL_Rect saveIconBtn = {0,0,0,0};
    SDL_Rect loadIconBtn = {0,0,0,0};

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
    Workspace workspace = {WORKSPACE_X, STAGE_Y, WORKSPACE_W, SCREEN_HEIGHT - STAGE_Y};

    Palette palette;
    palette.activeCategory = CAT_MOTION;
    palette.scrollOffset   = 0;
    palette.catBarX = 0;
    palette.catBarY = STAGE_Y;
    palette.catBarW = CAT_ICON_W;
    palette.catBarH = SCREEN_HEIGHT - STAGE_Y;
    palette.blockListX = CAT_ICON_W;
    palette.blockListY = STAGE_Y;
    palette.blockListW = BLOCK_LIST_W;
    palette.blockListH = SCREEN_HEIGHT - STAGE_Y;

    int iy = STAGE_Y+8;
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
        Block* b = new Block();
        b->id = bid++;
        b->type = t;
        b->text = txt;
        b->x = 0; b->y = 0;
        b->w = BLOCK_W; b->h = BLOCK_H;
        b->isDragging = false;
        b->dragOffsetX = 0; b->dragOffsetY = 0;
        b->next = nullptr; b->prev = nullptr;
        b->isCShaped  = false;
        b->innerFirst = nullptr; b->innerLast = nullptr;
        b->elseFirst  = nullptr;
        b->hasElse    = false;
        b->innerH = 40; b->elseH = 40;
        init_block_inputs(b);
        paletteBlocks.push_back(b);
    };

   // ── MOTION ──
addPB(BLOCK_MOTION, "move () steps");
addPB(BLOCK_MOTION, "turn right () degrees");
addPB(BLOCK_MOTION, "turn left () degrees");
addPB(BLOCK_MOTION, "go to x:() y:()");
addPB(BLOCK_MOTION, "go to random position");
addPB(BLOCK_MOTION, "go to mouse-pointer");
addPB(BLOCK_MOTION, "glide () secs to x:() y:()");
addPB(BLOCK_MOTION, "point in direction ()");
addPB(BLOCK_MOTION, "point towards mouse-pointer");
addPB(BLOCK_MOTION, "change x by ()");
addPB(BLOCK_MOTION, "set x to ()");
addPB(BLOCK_MOTION, "change y by ()");
addPB(BLOCK_MOTION, "set y to ()");
addPB(BLOCK_MOTION, "if on edge, bounce");

// ── LOOKS ──
addPB(BLOCK_LOOKS, "say ()");
addPB(BLOCK_LOOKS, "say () for () secs");
addPB(BLOCK_LOOKS, "think ()");
addPB(BLOCK_LOOKS, "show");
addPB(BLOCK_LOOKS, "hide");
addPB(BLOCK_LOOKS, "next costume");
addPB(BLOCK_LOOKS, "switch costume to ()");
addPB(BLOCK_LOOKS, "set size to ()");
addPB(BLOCK_LOOKS, "change size by ()");

// ── SOUND ──
addPB(BLOCK_SOUND, "play sound ()");
addPB(BLOCK_SOUND, "play sound () until done");
addPB(BLOCK_SOUND, "stop all sounds");
addPB(BLOCK_SOUND, "change volume by ()");
addPB(BLOCK_SOUND, "set volume to ()");
addPB(BLOCK_SOUND, "clear sound effects");

// ── EVENTS ──
addPB(BLOCK_EVENT, "when flag clicked");
addPB(BLOCK_EVENT, "when key pressed");
addPB(BLOCK_EVENT, "when sprite clicked");

// ── CONTROL ──
addPB(BLOCK_CONTROL, "wait () secs");
addPB(BLOCK_CONTROL, "repeat ()");
addPB(BLOCK_CONTROL, "forever");
addPB(BLOCK_CONTROL, "if <> then");
addPB(BLOCK_CONTROL, "if <> then else");
addPB(BLOCK_CONTROL, "wait until <>");
addPB(BLOCK_CONTROL, "stop all");
addPB(BLOCK_CONTROL, "stop this script");
addPB(BLOCK_CONTROL, "stop other scripts");

// ── SENSING ──
addPB(BLOCK_SENSING, "ask () and wait");
addPB(BLOCK_SENSING, "answer");
addPB(BLOCK_SENSING, "touching mouse-pointer?");
addPB(BLOCK_SENSING, "touching edge?");
addPB(BLOCK_SENSING, "key space pressed?");
addPB(BLOCK_SENSING, "mouse down?");
addPB(BLOCK_SENSING, "mouse x");
addPB(BLOCK_SENSING, "mouse y");
addPB(BLOCK_SENSING, "timer");
addPB(BLOCK_SENSING, "reset timer");

// ── OPERATORS ──
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
addPB(BLOCK_OPERATORS, "pick random () to ()");
addPB(BLOCK_OPERATORS, "round ()");
addPB(BLOCK_OPERATORS, "abs of ()");
addPB(BLOCK_OPERATORS, "sqrt of ()");
addPB(BLOCK_OPERATORS, "floor of ()");
addPB(BLOCK_OPERATORS, "ceiling of ()");
addPB(BLOCK_OPERATORS, "sin of ()");
addPB(BLOCK_OPERATORS, "cos of ()");
addPB(BLOCK_OPERATORS, "tan of ()");
addPB(BLOCK_OPERATORS, "join () ()");
addPB(BLOCK_OPERATORS, "letter () of ()");
addPB(BLOCK_OPERATORS, "length of ()");

// ── VARIABLES ──
addPB(BLOCK_VARIABLES, "set score to ()");
addPB(BLOCK_VARIABLES, "change score by ()");
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

    SoundsPanel soundsPanel;
    soundsPanel.x = 0;
    soundsPanel.y = STAGE_Y;
    soundsPanel.w = PALETTE_WIDTH;
    soundsPanel.h = SCREEN_HEIGHT - STAGE_Y;
    soundsPanel.visible        = true;
    soundsPanel.selectedIndex  = -1;
    soundsPanel.scrollOffset   = 0;
    soundsPanel.uploadDialogOpen = false;
    soundsPanel.uploadEditing    = false;

    auto addDefaultSound = [&](const char* name, const char* path) {
        SoundClip sc;
        sc.name     = name;
        sc.filePath = path;
        sc.volume   = 100.0f;
        sc.chunk    = nullptr;
        sc.channel  = -1;
        sc.isPlaying = false;
        audio_load(sc);
        soundsPanel.sounds.push_back(sc);
    };
    addDefaultSound("Pop",  "pop.wav");
    addDefaultSound("Meow", "meow.wav");
    addDefaultSound("Drum", "drum.wav");

    g_soundsPanel = &soundsPanel;

    // متغیرهای ویرایش sprite info panel
    int   spriteInfoActiveField = -1;   // 0=x,1=y,2=dir,3=size  (-1=none)
    std::string spriteInfoEditText = "";

    SDL_Rect playBtn = {STAGE_X + 10,      STAGE_Y - TAB_H - 4, 38, 32};
    SDL_Rect stopBtn = {STAGE_X + 10 + 44, STAGE_Y - TAB_H - 4, 32, 32};

    vector<Block*> workspaceBlocks;
    Block* draggedBlock = nullptr;
    BlockInput* activeInput = nullptr;
    std::string askInputText = "";   // متنی که کاربر در ask تایپ می‌کند
    bool askInputActive = false;
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

    const std::string projectFile = "project.scratch";

    auto toggle_fullscreen = [&]() {
        Uint32 flags = SDL_GetWindowFlags(window);
        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
            SDL_SetWindowFullscreen(window, 0);
            SDL_RestoreWindow(window);
        } else {
            if (flags & SDL_WINDOW_MAXIMIZED) {
                SDL_RestoreWindow(window);
            }
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
    };

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { quit = true; break; }

            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    SDL_RestoreWindow(window);
                }
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_f) {
                    toggle_fullscreen();
                    continue;
                }
                if (e.key.keysym.sym == SDLK_ESCAPE) {

                    if (soundsPanel.uploadDialogOpen) {
                        soundsPanel.uploadDialogOpen = false;
                        soundsPanel.uploadEditing    = false;
                        soundsPanel.uploadPathInput  = "";
                        SDL_StopTextInput();
                        continue;
                    }
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        SDL_SetWindowFullscreen(window, 0);
                        SDL_RestoreWindow(window);
                        continue;
                    }
                }

                if (soundsPanel.uploadDialogOpen && soundsPanel.uploadEditing) {
                    if (e.key.keysym.sym == SDLK_RETURN ||
                        e.key.keysym.sym == SDLK_KP_ENTER) {

                        if (!soundsPanel.uploadPathInput.empty()) {
                            SoundClip nc;
                            std::string path = soundsPanel.uploadPathInput;
                            size_t slash = path.find_last_of("/\\");
                            std::string fname = (slash != std::string::npos)
                                                ? path.substr(slash + 1) : path;
                            size_t dot = fname.find_last_of('.');
                            nc.name     = (dot != std::string::npos)
                                          ? fname.substr(0, dot) : fname;
                            nc.filePath = path;
                            nc.volume   = 100.0f;
                            nc.chunk    = nullptr;
                            nc.channel  = -1;
                            nc.isPlaying = false;
                            audio_load(nc);
                            soundsPanel.sounds.push_back(nc);
                            soundsPanel.selectedIndex =
                                (int)soundsPanel.sounds.size() - 1;
                        }
                        soundsPanel.uploadDialogOpen = false;
                        soundsPanel.uploadEditing    = false;
                        soundsPanel.uploadPathInput  = "";
                        SDL_StopTextInput();
                        continue;
                    }
                    if (e.key.keysym.sym == SDLK_BACKSPACE &&
                        !soundsPanel.uploadPathInput.empty()) {
                        soundsPanel.uploadPathInput.pop_back();
                        continue;
                    }
                    continue;
                }
            }

            if (e.type == SDL_TEXTINPUT) {
                if (soundsPanel.uploadDialogOpen && soundsPanel.uploadEditing) {
                    soundsPanel.uploadPathInput += e.text.text;
                    continue;
                }
            }

            if (costumeEditor.isOpen) {
                ce_handle_event(costumeEditor, e, renderer, &sprite);
                continue;
            }

            // ─── handle ask/answer input ────────────────────────────────────────────
            if (g_askPending) {
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_RETURN ||
                        e.key.keysym.sym == SDLK_KP_ENTER) {
                        g_answer     = askInputText;
                        g_askPending = false;
                        askInputText = "";
                        askInputActive = false;
                        sprite.sayText = "";
                        sprite.sayTimer = 0;
                        SDL_StopTextInput();
                        } else if (e.key.keysym.sym == SDLK_BACKSPACE &&
                                   !askInputText.empty()) {
                            askInputText.pop_back();
                                   }
                    continue;
                }
                if (e.type == SDL_TEXTINPUT) {
                    askInputText += e.text.text;
                    continue;
                }
                if (!askInputActive) {
                    askInputActive = true;
                    SDL_StartTextInput();
                }
            }

            if (activeInput && e.type == SDL_TEXTINPUT) {
                activeInput->value += e.text.text;
                continue;
            }
            if (activeInput && e.type == SDL_TEXTINPUT) {
                activeInput->value += e.text.text;
                continue;
            }

            if (varsPanel.creating && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN ||
                    e.key.keysym.sym == SDLK_KP_ENTER) {
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
                    varsPanel.creating   = false;
                    varsPanel.newVarName = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    varsPanel.creating   = false;
                    varsPanel.newVarName = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_BACKSPACE &&
                           !varsPanel.newVarName.empty()) {
                    varsPanel.newVarName.pop_back();
                }
                continue;
            }
            if (varsPanel.creating && e.type == SDL_TEXTINPUT) {
                varsPanel.newVarName += e.text.text;
                continue;
            }

            if (myBlocksCreating && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN ||
                    e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (!newBlockName.empty()) {
                        customBlocks.push_back(newBlockName);
                        addPB(BLOCK_MYBLOCKS, newBlockName);
                    }
                    myBlocksCreating = false;
                    newBlockName     = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    myBlocksCreating = false;
                    newBlockName     = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_BACKSPACE &&
                           !newBlockName.empty()) {
                    newBlockName.pop_back();
                }
                continue;
            }
            if (myBlocksCreating && e.type == SDL_TEXTINPUT) {
                newBlockName += e.text.text;
                continue;
            }

            // ── ورودی کیبورد برای فیلدهای Sprite Info ─────────────────────
            if (spriteInfoActiveField >= 0 && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN ||
                    e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (!spriteInfoEditText.empty()) {
                        try {
                            float val = std::stof(spriteInfoEditText);
                            if (spriteInfoActiveField == 0)
                                sprite.x = val + STAGE_WIDTH/2.0f - sprite.w/2.0f;
                            else if (spriteInfoActiveField == 1)
                                sprite.y = -val + STAGE_HEIGHT/2.0f - sprite.h/2.0f;
                            else if (spriteInfoActiveField == 2)
                                sprite.direction = val;
                            else if (spriteInfoActiveField == 3) {
                                sprite.scale = val / 100.0f;
                                if (sprite.scale < 0.01f) sprite.scale = 0.01f;
                            }
                        } catch (...) {}
                    }
                    spriteInfoActiveField = -1;
                    spriteInfoEditText = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    spriteInfoActiveField = -1;
                    spriteInfoEditText = "";
                    SDL_StopTextInput();
                } else if (e.key.keysym.sym == SDLK_BACKSPACE &&
                           !spriteInfoEditText.empty()) {
                    spriteInfoEditText.pop_back();
                } else if (e.key.keysym.sym == SDLK_TAB) {
                    // Tab برای رفتن به فیلد بعدی
                    if (!spriteInfoEditText.empty()) {
                        try {
                            float val = std::stof(spriteInfoEditText);
                            if (spriteInfoActiveField == 0)
                                sprite.x = val + STAGE_WIDTH/2.0f - sprite.w/2.0f;
                            else if (spriteInfoActiveField == 1)
                                sprite.y = -val + STAGE_HEIGHT/2.0f - sprite.h/2.0f;
                            else if (spriteInfoActiveField == 2)
                                sprite.direction = val;
                            else if (spriteInfoActiveField == 3) {
                                sprite.scale = val / 100.0f;
                                if (sprite.scale < 0.01f) sprite.scale = 0.01f;
                            }
                        } catch (...) {}
                    }
                    spriteInfoActiveField = (spriteInfoActiveField + 1) % 4;
                    float scrX2 = sprite.x - STAGE_WIDTH/2.0f + sprite.w/2.0f;
                    float scrY2 = -(sprite.y - STAGE_HEIGHT/2.0f + sprite.h/2.0f);
                    std::ostringstream ss3;
                    ss3 << std::fixed << std::setprecision(1);
                    if (spriteInfoActiveField == 0) ss3 << scrX2;
                    else if (spriteInfoActiveField == 1) ss3 << scrY2;
                    else if (spriteInfoActiveField == 2) ss3 << sprite.direction;
                    else if (spriteInfoActiveField == 3) ss3 << sprite.scale*100.0f;
                    spriteInfoEditText = ss3.str();
                }
                continue;
            }
            if (spriteInfoActiveField >= 0 && e.type == SDL_TEXTINPUT) {
                spriteInfoEditText += e.text.text;
                continue;
            }

            if (e.type == SDL_MOUSEWHEEL) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);

                if (activeTab == TAB_SOUNDS &&
                    mx >= 0 && mx < PALETTE_WIDTH &&
                    my >= STAGE_Y) {
                    soundsPanel.scrollOffset += e.wheel.y * 20;
                    if (soundsPanel.scrollOffset > 0) soundsPanel.scrollOffset = 0;
                    continue;
                }

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

                if (soundsPanel.uploadDialogOpen) {
                    handle_upload_dialog_click(mx, my, soundsPanel, fontSmall);
                    continue;
                }

                if (activeInput) {
                    activeInput->editing = false;
                    activeInput = nullptr;
                    SDL_StopTextInput();
                }

                if (tab_bar_click(mx, my, activeTab)) {
                    if (activeTab == TAB_COSTUMES) {
                        int idx = costumePanel.selectedIndex;
                        if (idx < 0 || idx >= (int)sprite.costumes.size()) idx = 0;
                        if (!sprite.costumes.empty())
                            ce_open(costumeEditor, renderer, &sprite, idx);
                    }
                    continue;
                }

                if (point_in_rect(mx, my, saveIconBtn.x, saveIconBtn.y,
                                  saveIconBtn.w, saveIconBtn.h)) {
                    save_project(projectFile, workspaceBlocks, varsPanel);
                    continue;
                }
                if (point_in_rect(mx, my, loadIconBtn.x, loadIconBtn.y,
                                  loadIconBtn.w, loadIconBtn.h)) {
                    load_project(projectFile, workspaceBlocks, varsPanel, bid);
                    continue;
                }

                if (activeTab == TAB_SOUNDS) {
                    if (mx >= 0 && mx < PALETTE_WIDTH && my >= STAGE_Y) {
                        handle_sounds_panel_click(mx, my, soundsPanel);
                        continue;
                    }
                    if (soundsPanel.selectedIndex >= 0 &&
                        soundsPanel.selectedIndex < (int)soundsPanel.sounds.size()) {

                        SoundClip& sc = soundsPanel.sounds[soundsPanel.selectedIndex];
                        int wx2 = WORKSPACE_X, wy2 = STAGE_Y;
                        int ww2 = WORKSPACE_W;

                        SDL_Rect bigPlay = {wx2 + 20, wy2 + 220, 80, 36};
                        if (point_in_rect(mx, my,
                            bigPlay.x, bigPlay.y, bigPlay.w, bigPlay.h)) {
                            if (sc.isPlaying)
                                audio_stop(sc);
                            else {
                                audio_play(sc);
                                if (sc.channel >= 0)
                                    Mix_Volume(sc.channel,
                                        (int)(sc.volume / 100.0f * MIX_MAX_VOLUME));
                            }
                            continue;
                        }
                        // bigStop
                        SDL_Rect bigStop = {wx2 + 112, wy2 + 220, 80, 36};
                        if (point_in_rect(mx, my,
                            bigStop.x, bigStop.y, bigStop.w, bigStop.h)) {
                            audio_stop(sc);
                            continue;
                        }
                    }
                    continue; // بقیه کلیک‌ها در TAB_SOUNDS ignore
                }

                // ── Play / Stop script ────────────────────────────────────────
                if (point_in_rect(mx, my, playBtn.x, playBtn.y,
                                  playBtn.w, playBtn.h)) {
                    Block* s = find_script_start(workspaceBlocks);
                    if (s) scriptRunner.start(s, &varsPanel.variables);
                }
                else if (point_in_rect(mx, my, stopBtn.x, stopBtn.y,
                                       stopBtn.w, stopBtn.h)) {
                    scriptRunner.stop();
                    sprite.sayText = ""; sprite.sayTimer = 0;
                }
                else if (isVarCat && makeVarBtn.w > 0 &&
                         point_in_rect(mx, my, makeVarBtn.x, makeVarBtn.y,
                                       makeVarBtn.w, makeVarBtn.h)) {
                    varsPanel.creating   = true;
                    varsPanel.newVarName = "";
                    SDL_StartTextInput();
                    continue;
                }
                else if (isMyBlocks && makeBlockBtn.w > 0 &&
                         point_in_rect(mx, my, makeBlockBtn.x, makeBlockBtn.y,
                                       makeBlockBtn.w, makeBlockBtn.h)) {
                    myBlocksCreating = true;
                    newBlockName     = "";
                    SDL_StartTextInput();
                    continue;
                }

                // ── Sprite Info Panel (فیلدهای قابل ویرایش) ────────────────
                {
                    int siField = sprite_info_field_at(mx, my);
                    if (siField >= 0) {
                        if (spriteInfoActiveField >= 0 && !spriteInfoEditText.empty()) {
                            try {
                                float val = std::stof(spriteInfoEditText);
                                if (spriteInfoActiveField == 0)
                                    sprite.x = val + STAGE_WIDTH/2.0f - sprite.w/2.0f;
                                else if (spriteInfoActiveField == 1)
                                    sprite.y = -val + STAGE_HEIGHT/2.0f - sprite.h/2.0f;
                                else if (spriteInfoActiveField == 2)
                                    sprite.direction = val;
                                else if (spriteInfoActiveField == 3) {
                                    sprite.scale = val / 100.0f;
                                    if (sprite.scale < 0.01f) sprite.scale = 0.01f;
                                }
                            } catch (...) {}
                        }
                        spriteInfoActiveField = siField;
                        float scrX2 = sprite.x - STAGE_WIDTH/2.0f + sprite.w/2.0f;
                        float scrY2 = -(sprite.y - STAGE_HEIGHT/2.0f + sprite.h/2.0f);
                        std::ostringstream ss2;
                        ss2 << std::fixed << std::setprecision(1);
                        if (siField == 0) ss2 << scrX2;
                        else if (siField == 1) ss2 << scrY2;
                        else if (siField == 2) ss2 << sprite.direction;
                        else if (siField == 3) ss2 << sprite.scale*100.0f;
                        spriteInfoEditText = ss2.str();
                        SDL_StartTextInput();
                        continue;
                    } else if (spriteInfoActiveField >= 0) {
                        if (!spriteInfoEditText.empty()) {
                            try {
                                float val = std::stof(spriteInfoEditText);
                                if (spriteInfoActiveField == 0)
                                    sprite.x = val + STAGE_WIDTH/2.0f - sprite.w/2.0f;
                                else if (spriteInfoActiveField == 1)
                                    sprite.y = -val + STAGE_HEIGHT/2.0f - sprite.h/2.0f;
                                else if (spriteInfoActiveField == 2)
                                    sprite.direction = val;
                                else if (spriteInfoActiveField == 3) {
                                    sprite.scale = val / 100.0f;
                                    if (sprite.scale < 0.01f) sprite.scale = 0.01f;
                                }
                            } catch (...) {}
                        }
                        spriteInfoActiveField = -1;
                        spriteInfoEditText = "";
                        SDL_StopTextInput();
                    }
                }

                if (costumePanel.visible &&
                    point_in_rect(mx, my, costumePanel.x, costumePanel.y,
                                  costumePanel.w, costumePanel.h)) {
                    int itemH2 = COSTUME_THUMB + 24 + 4;
                    int relY   = my - costumePanel.y - 36 - costumePanel.scrollOffset;
                    if (relY >= 0) {
                        int idx = relY / itemH2;
                        if (idx >= 0 && idx < (int)sprite.costumes.size()) {
                            Uint32 now = SDL_GetTicks();
                            if (idx == lastCostumeClickIdx &&
                                (now - lastCostumeClickTime) < 400) {
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
                        nb->isDragging   = true;
                        nb->dragOffsetX  = nb->w/2;
                        nb->dragOffsetY  = nb->h/2;
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
                        activeInput->value   = "";
                        if (g_hasOperatorResult) {
                            activeInput->value = float_to_str(g_lastOperatorResult);
                            activeInput->editing = false;
                            g_hasOperatorResult = false;
                        } else {
                            SDL_StartTextInput();
                        }
                        continue;
                    }

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
        } // end event loop

        // ── Update ────────────────────────────────────────────────────────────
        layout_palette_blocks(paletteBlocks, palette);
        scriptRunner.update(&sprite);
        if (sprite.sayTimer > 0) {
            sprite.sayTimer--;
            if (sprite.sayTimer == 0) sprite.sayText = "";
        }

        audio_update(soundsPanel);

        isVarCat   = (palette.activeCategory == CAT_VARIABLES);
        isMyBlocks = (palette.activeCategory == CAT_MYBLOCKS);

        SDL_SetRenderDrawColor(renderer, 200, 200, 205, 255);
        SDL_RenderClear(renderer);

        // header bar
        SDL_SetRenderDrawColor(renderer,
            COLOR_HEADER_BAR.r, COLOR_HEADER_BAR.g, COLOR_HEADER_BAR.b, 255);
        SDL_Rect headerBar = { 0, 0, SCREEN_WIDTH, HEADER_H };
        SDL_RenderFillRect(renderer, &headerBar);

        draw_toolbar_icons(renderer, fontSmall,
                           gearBtn, notesBtn, penBtn, bulbBtn,
                           saveIconBtn, loadIconBtn);
        draw_tab_bar(renderer, fontSmall, activeTab);

        if (activeTab == TAB_CODE) {
            draw_category_bar(renderer, fontSmall, palette);
            string activeName; SDL_Color activeCol;
            getActiveCatInfo(activeName, activeCol);

            if (isVarCat) {
                draw_block_list_header(renderer, fontBig, palette,
                                       activeName, activeCol, true, &makeVarBtn);
                makeBlockBtn = {0,0,0,0};
            } else if (isMyBlocks) {
                draw_block_list_header(renderer, fontBig, palette,
                                       activeName, activeCol, true, &makeBlockBtn);
                makeVarBtn = {0,0,0,0};
            } else {
                draw_block_list_header(renderer, fontBig, palette,
                                       activeName, activeCol, false, nullptr);
                makeVarBtn   = {0,0,0,0};
                makeBlockBtn = {0,0,0,0};
            }

            for (Block* b : paletteBlocks)
                if (block_matches_category(b, palette.activeCategory))
                    draw_block(renderer, fontSmall, b);
        }
        else if (activeTab == TAB_COSTUMES) {
            SDL_SetRenderDrawColor(renderer, 245, 240, 255, 255);
            SDL_Rect leftPanel = {0, STAGE_Y, PALETTE_WIDTH, SCREEN_HEIGHT - STAGE_Y};
            SDL_RenderFillRect(renderer, &leftPanel);

            SDL_SetRenderDrawColor(renderer,
                COLOR_LOOKS.r, COLOR_LOOKS.g, COLOR_LOOKS.b, 255);
            SDL_Rect panHdr = {0, STAGE_Y, PALETTE_WIDTH, 32};
            SDL_RenderFillRect(renderer, &panHdr);
            if (fontBig)
                draw_text_centered(renderer, fontBig, "Costumes",
                                   panHdr, COLOR_TEXT_WHITE);

            int cy2 = STAGE_Y + 38;
            for (int i = 0; i < (int)sprite.costumes.size(); i++) {
                bool sel = (i == costumePanel.selectedIndex);
                SDL_SetRenderDrawColor(renderer,
                    sel?200:240, sel?210:240, sel?255:240, 255);
                SDL_Rect row = {4, cy2, PALETTE_WIDTH-8, 48};
                SDL_RenderFillRect(renderer, &row);
                SDL_SetRenderDrawColor(renderer,
                    sel?80:180, sel?100:180, sel?220:200, 255);
                SDL_RenderDrawRect(renderer, &row);
                if (fontSmall) {
                    std::string lbl = std::to_string(i+1) + ". " +
                                      sprite.costumes[i].name;
                    draw_text(renderer, fontSmall, lbl,
                              10, cy2+14, COLOR_TEXT_DARK);
                }
                cy2 += 54;
            }
            if (fontSmall)
                draw_text(renderer, fontSmall, "Click a costume to edit",
                          6, cy2+4, {150,120,180,255});
        }
        else if (activeTab == TAB_SOUNDS) {

            draw_sounds_left_panel(renderer, fontSmall, fontBig, soundsPanel);
            draw_sounds_workspace(renderer, fontSmall, fontBig, soundsPanel);
            draw_upload_dialog(renderer, fontSmall, fontBig, soundsPanel);
        }

        if (activeTab == TAB_CODE) {
            draw_workspace_bg(renderer, workspace);
            // ابتدا layout بلاک‌های داخل C ها
            for (Block* b : workspaceBlocks) {
                if (b->isCShaped) layout_inner_blocks(b);
            }
            // رندر همه بلاک‌ها
            for (Block* b : workspaceBlocks) {
                update_block_input_rects(b);
                draw_block(renderer, fontSmall, b);
                // رندر بلاک‌های داخل C
                if (b->isCShaped) {
                    Block* inner = b->innerFirst;
                    while (inner) {
                        update_block_input_rects(inner);
                        draw_block(renderer, fontSmall, inner);
                        inner = inner->next;
                    }
                    if (b->hasElse) {
                        Block* el = b->elseFirst;
                        while (el) {
                            update_block_input_rects(el);
                            draw_block(renderer, fontSmall, el);
                            el = el->next;
                        }
                    }
                }
            }

            auto draw_name_dialog = [&](const string& title,
                                        const string& inputText) {
                int ox = workspace.x + workspace.w/2 - 160;
                int oy = workspace.y + workspace.h/2 - 40;
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
                SDL_Rect overlay = {workspace.x, workspace.y,
                                    workspace.w, workspace.h};
                SDL_RenderFillRect(renderer, &overlay);
                SDL_SetRenderDrawColor(renderer, 250, 250, 255, 255);
                SDL_Rect box = {ox, oy, 320, 80};
                SDL_RenderFillRect(renderer, &box);
                SDL_SetRenderDrawColor(renderer, 150, 150, 200, 255);
                SDL_RenderDrawRect(renderer, &box);
                if (fontBig)
                    draw_text(renderer, fontBig, title,
                              ox+12, oy+10, COLOR_TEXT_DARK);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect field = {ox+12, oy+34, 220, 26};
                SDL_RenderFillRect(renderer, &field);
                SDL_SetRenderDrawColor(renderer, 100, 100, 220, 255);
                SDL_RenderDrawRect(renderer, &field);
                if (fontSmall)
                    draw_text(renderer, fontSmall, inputText + "|",
                              ox+16, oy+40, COLOR_TEXT_DARK);
                SDL_SetRenderDrawColor(renderer, 100, 100, 220, 255);
                SDL_Rect okBtn2 = {ox+244, oy+34, 64, 26};
                SDL_RenderFillRect(renderer, &okBtn2);
                if (fontSmall)
                    draw_text_centered(renderer, fontSmall, "OK",
                                       okBtn2, COLOR_TEXT_WHITE);
            };

            if (varsPanel.creating)
                draw_name_dialog("New Variable Name:", varsPanel.newVarName);
            if (myBlocksCreating)
                draw_name_dialog("Block Name:", newBlockName);
        }

        draw_stage(renderer, &stage);
        draw_sprite(renderer, &sprite, &stage, fontSmall);
        // رندر ask input اگر فعال است
        if (g_askPending) {
            draw_ask_input(renderer, fontSmall, &stage, g_askQuestion, askInputText);
        }
        draw_operator_result(renderer, fontSmall, &stage);
        draw_variable_monitors(renderer, fontSmall, varsPanel, &stage);
        draw_costume_panel(renderer, fontSmall, fontBig, costumePanel, &sprite);
        draw_sprite_info_panel(renderer, fontSmall, fontBig, &sprite,
                               spriteInfoActiveField, spriteInfoEditText);
        draw_play_stop_buttons(renderer, fontSmall, playBtn, stopBtn,
                               playTex, stopTex, scriptRunner.running);

        ce_render(costumeEditor, renderer, fontSmall, fontBig);

        SDL_RenderPresent(renderer);
    }

    audio_free_all(soundsPanel);
    audio_quit();

    if (costumeEditor.canvasTex)  SDL_DestroyTexture(costumeEditor.canvasTex);
    if (costumeEditor.canvasSurf) SDL_FreeSurface(costumeEditor.canvasSurf);
    for (auto b : paletteBlocks)   delete b;
    for (auto b : workspaceBlocks) delete b;
    for (auto& c : sprite.costumes)
        if (c.texture) SDL_DestroyTexture(c.texture);
    if (playTex) SDL_DestroyTexture(playTex);
    if (stopTex) SDL_DestroyTexture(stopTex);
    TTF_CloseFont(fontSmall);
    if (fontBig) TTF_CloseFont(fontBig);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
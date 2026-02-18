#include <iostream>
#include <vector>
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
        "Scratch C++ Engine",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


    SDL_Surface* spriteSurf = IMG_Load("sprite.png");
    SDL_Texture* spriteTexture = nullptr;
    int texW = 60, texH = 60;
    if (spriteSurf) {
        spriteTexture = SDL_CreateTextureFromSurface(renderer, spriteSurf);
        texW = spriteSurf->w;
        texH = spriteSurf->h;
        SDL_FreeSurface(spriteSurf);
    }

    Sprite sprite;
    sprite.x       = (STAGE_WIDTH  / 2) - texW / 2;
    sprite.y       = (STAGE_HEIGHT / 2) - texH / 2;
    sprite.w       = texW;
    sprite.h       = texH;
    sprite.texture = spriteTexture;
    sprite.visible = true;
    sprite.sayTimer = 0;


#ifdef _WIN32
    const char* fontPath = "C:\\Windows\\Fonts\\arialbd.ttf";
#else
    const char* fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
#endif
    TTF_Font* font = TTF_OpenFont(fontPath, 14);
    if (!font) {
        cerr << "Font error: " << TTF_GetError() << endl;
        // Fallback
        font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 14);
    }

    Stage stage;
    stage.x     = STAGE_X;
    stage.y     = TOOLBAR_HEIGHT;
    stage.w     = STAGE_WIDTH;
    stage.h     = STAGE_HEIGHT;
    stage.color = {255, 255, 255, 255};


    Workspace workspace;
    workspace.x = PALETTE_WIDTH;
    workspace.y = TOOLBAR_HEIGHT;
    workspace.w = SCREEN_WIDTH - PALETTE_WIDTH - STAGE_WIDTH;
    workspace.h = SCREEN_HEIGHT - TOOLBAR_HEIGHT;


    Palette palette;
    palette.x              = 0;
    palette.y              = TOOLBAR_HEIGHT;
    palette.w              = PALETTE_WIDTH;
    palette.h              = SCREEN_HEIGHT - TOOLBAR_HEIGHT;
    palette.activeCategory = CAT_MOTION;
    palette.scrollOffset   = 0;


    int cy = palette.y + 8;
    auto addCat = [&](CategoryType t, const string& name, SDL_Color col) {
        palette.categories.push_back({
            t, name,
            {palette.x + 4, cy, palette.w - 8, CATEGORY_HEIGHT},
            col
        });
        cy += CATEGORY_HEIGHT + 3;
    };

    addCat(CAT_MOTION,    "Motion",    COLOR_MOTION);
    addCat(CAT_LOOKS,     "Looks",     COLOR_LOOKS);
    addCat(CAT_SOUND,     "Sound",     COLOR_SOUND);
    addCat(CAT_EVENTS,    "Events",    COLOR_EVENTS);
    addCat(CAT_CONTROL,   "Control",   COLOR_CONTROL);
    addCat(CAT_SENSING,   "Sensing",   COLOR_SENSING);
    addCat(CAT_OPERATORS, "Operators", COLOR_OPERATORS);
    addCat(CAT_VARIABLES, "Variables", COLOR_VARIABLES);


    vector<Block*> paletteBlocks;
    auto addPBlock = [&](BlockType t, const string& txt) {
        paletteBlocks.push_back(new Block{
            (int)paletteBlocks.size() + 1, t, txt,
            0, 0, BLOCK_W, BLOCK_H,
            false, 0, 0, nullptr, nullptr
        });
    };

    // Motion
    addPBlock(BLOCK_MOTION, "Move 10 Steps");
    addPBlock(BLOCK_MOTION, "Turn 15 Degrees");
    addPBlock(BLOCK_MOTION, "Go to X:0 Y:0");
    addPBlock(BLOCK_MOTION, "Glide 1 Secs to X:0 Y:0");

    // Looks
    addPBlock(BLOCK_LOOKS, "Say \"Hello!\"");
    addPBlock(BLOCK_LOOKS, "Say \"Hi\" for 2 Secs");
    addPBlock(BLOCK_LOOKS, "Show");
    addPBlock(BLOCK_LOOKS, "Hide");

    // Events
    addPBlock(BLOCK_EVENT, "When Flag Clicked");
    addPBlock(BLOCK_EVENT, "When Key Pressed");
    addPBlock(BLOCK_EVENT, "When Sprite Clicked");

    // Control
    addPBlock(BLOCK_CONTROL, "Wait 1 Secs");
    addPBlock(BLOCK_CONTROL, "Repeat 10");
    addPBlock(BLOCK_CONTROL, "Forever");
    addPBlock(BLOCK_CONTROL, "If <> Then");
    addPBlock(BLOCK_CONTROL, "Stop All");


    vector<Block*> workspaceBlocks;
    Block*  draggedBlock = nullptr;
    bool    quit         = false;
    SDL_Event e;

    SDL_Rect playBtn = {SCREEN_WIDTH - STAGE_WIDTH + 10, 5, 56, 30};
    SDL_Rect stopBtn = {SCREEN_WIDTH - STAGE_WIDTH + 74, 5, 56, 30};


    ScriptRunner scriptRunner;


    while (!quit) {


        while (SDL_PollEvent(&e) != 0) {

            if (e.type == SDL_QUIT) { quit = true; break; }

            if (e.type == SDL_MOUSEWHEEL) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (point_in_rect(mx, my,
                    palette.x, palette.y, palette.w, palette.h)) {
                    handle_scroll_value(e, palette.scrollOffset);
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN &&
                e.button.button == SDL_BUTTON_LEFT)
            {
                int mx = e.button.x;
                int my = e.button.y;

                if (point_in_rect(mx, my,
                    playBtn.x, playBtn.y, playBtn.w, playBtn.h))
                {
                    Block* start = find_script_start(workspaceBlocks);
                    if (start) {
                        scriptRunner.start(start);
                        cout << "▶ Script started\n";
                    }
                }
                else if (point_in_rect(mx, my,
                    stopBtn.x, stopBtn.y, stopBtn.w, stopBtn.h))
                {
                    scriptRunner.stop();
                    cout << "■ Script stopped\n";
                }
                else if (handle_category_click(mx, my, palette)) {

                }

                else if (point_in_rect(mx, my,
                    palette.x, palette.y, palette.w, palette.h))
                {
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

                else if (point_in_rect(mx, my,
                    workspace.x, workspace.y, workspace.w, workspace.h))
                {
                    handle_mouse_down(e, workspaceBlocks, &draggedBlock);
                }
            }

            else if (e.type == SDL_MOUSEBUTTONUP &&
                     e.button.button == SDL_BUTTON_LEFT)
            {
                handle_mouse_up(&draggedBlock, workspaceBlocks, workspace);
            }

            else if (e.type == SDL_MOUSEMOTION) {
                handle_mouse_motion(e, &draggedBlock, workspace);
            }
        } // end event loop


        layout_palette_blocks(paletteBlocks, palette);
        scriptRunner.update(&sprite);

        if (sprite.sayTimer > 0) {
            sprite.sayTimer--;
            if (sprite.sayTimer == 0) sprite.sayText = "";
        }

        // ── Render ──
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        // پس‌زمینه‌ها
        draw_palette_bg(renderer, palette);
        draw_workspace_bg(renderer, workspace);

        // کتگوری‌ها
        draw_categories(renderer, font, palette);

        for (Block* b : paletteBlocks) {
            if (block_matches_category(b, palette.activeCategory)) {
                draw_block(renderer, font, b);
            }
        }

        // Workspace blocks
        for (Block* b : workspaceBlocks) {
            draw_block(renderer, font, b);
        }

        // Stage و Sprite
        draw_stage(renderer, &stage);
        draw_sprite(renderer, &sprite, &stage);

        // Toolbar
        draw_toolbar(renderer, font, playBtn, stopBtn);

        SDL_RenderPresent(renderer);
    }

    // ─── پاک‌سازی ─────────────────────────────────────────
    for (auto b : paletteBlocks)   delete b;
    for (auto b : workspaceBlocks) delete b;

    if (spriteTexture) SDL_DestroyTexture(spriteTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

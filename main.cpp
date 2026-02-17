#include <iostream>
#include "globals.h"
#include "input.h"
#include "render.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfx.h>

using namespace std;


int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Scratch CPP Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // لود فونت
    TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 16);
    if (!font) std::cout << "Font load error: " << TTF_GetError() << std::endl;

    // 2. ایجاد داده‌های اولیه (چند بلوک تستی)
    std::vector<Block> blocks;

    // بلوک 1: رویداد
    blocks.push_back({1, BLOCK_EVENT, "When Flag Clicked", 50, 50, 150, 40, false, 0, 0, nullptr});

    // بلوک 2: حرکت
    blocks.push_back({2, BLOCK_MOTION, "Move 10 Steps", 50, 120, 150, 40, false, 0, 0, nullptr});

    // بلوک 3: ظاهر
    blocks.push_back({3, BLOCK_LOOKS, "Say Hello!", 50, 190, 150, 40, false, 0, 0, nullptr});

    // متغیر برای نگهداری بلوکی که الان داریم میکشیم
    Block* draggedBlock = nullptr;

    bool quit = false;
    SDL_Event e;

    // 3. حلقه اصلی
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;

            // هندل کردن موس با استفاده از هدر input.h
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    handle_mouse_down(e, blocks, &draggedBlock);
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    handle_mouse_up(&draggedBlock);
                }
            }
            else if (e.type == SDL_MOUSEMOTION) {
                handle_mouse_motion(e, &draggedBlock);
            }
        }

        // 4. رسم (Render)
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255); // پس زمینه سفید خاکستری
        SDL_RenderClear(renderer);

        // رسم تمام بلوک‌ها
        for (auto& block : blocks) {
            draw_block(renderer, font, &block);
        }

        SDL_RenderPresent(renderer);
    }

    // خروج
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();


    return 0;
}
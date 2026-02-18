//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_ENGINE_H
#define SCRATCH_FOP_ENGINE_H

#include <iostream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include "structs.h"


struct ScriptRunner {
    Block*  currentBlock = nullptr;
    bool    running      = false;
    Uint32  nextRunTime  = 0;
    int     delayMs      = 150;

    void start(Block* startBlock) {
        currentBlock = startBlock;
        running      = true;
        nextRunTime  = SDL_GetTicks();
    }

    void stop() {
        running      = false;
        currentBlock = nullptr;
    }


    void update(Sprite* sprite) {
        if (!running || !currentBlock) {
            running = false;
            return;
        }

        Uint32 now = SDL_GetTicks();
        if (now < nextRunTime) return;

        Block* b = currentBlock;

        switch (b->type) {
            case BLOCK_MOTION: {

                int steps = 10;
                std::string t = b->text;
                size_t pos = t.find_first_of("0123456789");
                if (pos != std::string::npos) {
                    steps = std::stoi(t.substr(pos));
                }
                sprite->x += steps;
                std::cout << "[MOTION] Move " << steps << " steps\n";
                break;
            }
            case BLOCK_LOOKS: {
                // "Say Hello!" → نمایش حباب
                std::string msg = b->text;
                size_t q1 = msg.find('"');
                size_t q2 = msg.rfind('"');
                if (q1 != std::string::npos && q2 != q1) {
                    sprite->sayText = msg.substr(q1 + 1, q2 - q1 - 1);
                } else {
                    sprite->sayText = msg;
                }
                sprite->sayTimer = 120;
                std::cout << "[LOOKS] Say: " << sprite->sayText << "\n";
                break;
            }
            case BLOCK_EVENT:
                std::cout << "[EVENT] " << b->text << "\n";
                break;
            case BLOCK_CONTROL:
                std::cout << "[CONTROL] " << b->text << "\n";
                break;
            default:
                break;
        }

        currentBlock = b->next;
        nextRunTime  = now + delayMs;

        if (!currentBlock) {
            running = false;
            std::cout << "[ENGINE] Script finished.\n";
        }
    }
};

#endif

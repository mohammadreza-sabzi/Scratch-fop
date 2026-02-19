//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_ENGINE_H
#define SCRATCH_FOP_ENGINE_H

#include <string>
#include <cstdlib>
#include <SDL2/SDL.h>
#include "structs.h"
#include "globals.h"

struct ScriptRunner {
    bool   running    = false;
    Block* current    = nullptr;
    Uint32 waitUntil  = 0;
    int    repeatCount = 0;
    Block* repeatStart = nullptr;

    void start(Block* firstBlock) {
        running     = true;
        current     = firstBlock;
        waitUntil   = 0;
        repeatCount = 0;
        repeatStart = nullptr;
    }

    void stop() {
        running  = false;
        current  = nullptr;
    }

    void update(Sprite* sprite) {
        if (!running || !current || !sprite) return;

        Uint32 now = SDL_GetTicks();
        if (now < waitUntil) return;

        Block* b = current;

        execute_block(b, sprite, now);

        current = b->next;
        if (!current) {
            running = false;
        }
    }

    void execute_block(Block* b, Sprite* sprite, Uint32 now) {
        const std::string& txt = b->text;

        if (b->type == BLOCK_EVENT) {
            return;
        }

        if (b->type == BLOCK_MOTION) {
            if (txt.find("move") != std::string::npos) {
                int steps = 10;
                try {
                    size_t pos = txt.find("move ");
                    if (pos != std::string::npos)
                        steps = std::stoi(txt.substr(pos + 5));
                } catch (...) {}
                sprite->x += steps;

                if (sprite->x > STAGE_WIDTH  - sprite->w) sprite->x = 0;
                if (sprite->x < 0)                        sprite->x = (float)(STAGE_WIDTH - sprite->w);
            }
            else if (txt.find("change x by") != std::string::npos) {
                int val = 10;
                try {
                    size_t p = txt.find("by ");
                    if (p != std::string::npos) val = std::stoi(txt.substr(p + 3));
                } catch (...) {}
                sprite->x += val;
            }
            else if (txt.find("change y by") != std::string::npos) {
                int val = 10;
                try {
                    size_t p = txt.find("by ");
                    if (p != std::string::npos) val = std::stoi(txt.substr(p + 3));
                } catch (...) {}
                sprite->y += val;
            }
            else if (txt.find("go to x:") != std::string::npos) {
                sprite->x = STAGE_WIDTH  / 2.0f - sprite->w / 2.0f;
                sprite->y = STAGE_HEIGHT / 2.0f - sprite->h / 2.0f;
            }
        }

        else if (b->type == BLOCK_LOOKS) {
            if (txt.find("say") != std::string::npos) {

                size_t pos = txt.find("say ");
                if (pos != std::string::npos) {
                    std::string msg = txt.substr(pos + 4);

                    size_t forPos = msg.find(" for ");
                    if (forPos != std::string::npos) {
                        std::string secsStr = msg.substr(forPos + 5);
                        msg = msg.substr(0, forPos);
                        try {
                            int secs = std::stoi(secsStr);
                            waitUntil = SDL_GetTicks() + secs * 1000;
                            sprite->sayTimer = secs * 60;
                        } catch (...) {
                            sprite->sayTimer = 120;
                        }
                    } else {
                        sprite->sayTimer = 180;
                    }
                    sprite->sayText = msg;
                }
            }
            else if (txt == "show") { sprite->visible = true;  }
            else if (txt == "hide") { sprite->visible = false; }
        }

        else if (b->type == BLOCK_CONTROL) {
            if (txt.find("wait") != std::string::npos) {
                int secs = 1;
                try {
                    size_t p = txt.find("wait ");
                    if (p != std::string::npos) secs = std::stoi(txt.substr(p + 5));
                } catch (...) {}
                waitUntil = now + secs * 1000;
            }
            else if (txt == "stop all") {
                stop();
            }
        }
    }
};

#endif

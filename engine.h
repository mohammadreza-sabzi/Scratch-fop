//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_ENGINE_H
#define SCRATCH_FOP_ENGINE_H

#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL.h>
#include "structs.h"
#include "globals.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int parse_num(const std::string& txt, const std::string& after, int fallback = 0) {
    size_t pos = txt.find(after);
    if (pos == std::string::npos) return fallback;
    try { return std::stoi(txt.substr(pos + after.size())); }
    catch (...) { return fallback; }
}

static void clamp_sprite(Sprite* s) {
    if (s->x < 0) s->x = 0;
    if (s->y < 0) s->y = 0;
    if (s->x > STAGE_WIDTH  - s->w) s->x = (float)(STAGE_WIDTH  - s->w);
    if (s->y > STAGE_HEIGHT - s->h) s->y = (float)(STAGE_HEIGHT - s->h);
}

struct LoopFrame {
    Block* loopBody;
    int    remaining;
};

struct ScriptRunner {
    bool   running   = false;
    Block* current   = nullptr;
    Uint32 waitUntil = 0;
    bool   waiting   = false;
    bool   saySilent = false;
    std::vector<LoopFrame> loopStack;

    void start(Block* first) {
        running = true; current = first;
        waitUntil = 0; waiting = false; saySilent = false;
        loopStack.clear();
    }

    void stop() {
        running = false; current = nullptr;
        waiting = false; saySilent = false;
        loopStack.clear();
    }

    void update(Sprite* sprite) {
        if (!running || !sprite) return;
        Uint32 now = SDL_GetTicks();
        if (waiting) {
            if (now < waitUntil) return;
            waiting = false;
            if (saySilent) { sprite->sayText = ""; sprite->sayTimer = 0; saySilent = false; }
        }
        if (!current) { running = false; return; }
        bool jumped = execute_block(current, sprite, now);
        if (!jumped) advance(current);
    }

    bool execute_block(Block* b, Sprite* sprite, Uint32 now) {
        const std::string& txt = b->text;

        if (b->type == BLOCK_EVENT) return false;

        if (b->type == BLOCK_MOTION) {
            if (txt.find("move") != std::string::npos && txt.find("steps") != std::string::npos) {
                int steps = parse_num(txt, "move ", 10);
                double rad = (sprite->direction - 90.0) * M_PI / 180.0;
                sprite->x += (float)(steps * std::cos(rad));
                sprite->y += (float)(steps * std::sin(rad));
                clamp_sprite(sprite);
            } else if (txt.find("turn") != std::string::npos) {
                int deg = parse_num(txt, "turn ", 15);
                if (txt.find("left") != std::string::npos) sprite->direction -= deg;
                else sprite->direction += deg;
            } else if (txt.find("go to x:") != std::string::npos) {
                sprite->x = (float)(STAGE_WIDTH  / 2 + parse_num(txt, "x:", 0) - sprite->w / 2);
                sprite->y = (float)(STAGE_HEIGHT / 2 - parse_num(txt, "y:", 0) - sprite->h / 2);
                clamp_sprite(sprite);
            } else if (txt.find("change x by") != std::string::npos) {
                sprite->x += parse_num(txt, "by ", 10); clamp_sprite(sprite);
            } else if (txt.find("change y by") != std::string::npos) {
                sprite->y -= parse_num(txt, "by ", 10); clamp_sprite(sprite);
            } else if (txt.find("set x to") != std::string::npos) {
                sprite->x = (float)(STAGE_WIDTH  / 2 + parse_num(txt, "to ", 0) - sprite->w / 2);
                clamp_sprite(sprite);
            } else if (txt.find("set y to") != std::string::npos) {
                sprite->y = (float)(STAGE_HEIGHT / 2 - parse_num(txt, "to ", 0) - sprite->h / 2);
                clamp_sprite(sprite);
            } else if (txt.find("point in direction") != std::string::npos) {
                sprite->direction = (float)parse_num(txt, "direction ", 90);
            } else if (txt.find("if on edge") != std::string::npos) {
                if (sprite->x <= 0 || sprite->x >= STAGE_WIDTH  - sprite->w)
                    sprite->direction = 180.0f - sprite->direction;
                if (sprite->y <= 0 || sprite->y >= STAGE_HEIGHT - sprite->h)
                    sprite->direction = -sprite->direction;
            }
        }

        else if (b->type == BLOCK_LOOKS) {
            if (txt.find("say") != std::string::npos) {
                size_t sp = txt.find("say ");
                std::string msg = (sp != std::string::npos) ? txt.substr(sp + 4) : "Hello!";
                size_t fp = msg.find(" for ");
                if (fp != std::string::npos) {
                    int secs = 2;
                    try { secs = std::stoi(msg.substr(fp + 5)); } catch (...) {}
                    sprite->sayText  = msg.substr(0, fp);
                    sprite->sayTimer = secs * 60;
                    waitUntil = SDL_GetTicks() + (Uint32)(secs * 1000);
                    waiting = true; saySilent = true;
                } else {
                    sprite->sayText = msg; sprite->sayTimer = 999999;
                }
            } else if (txt == "show") { sprite->visible = true;
            } else if (txt == "hide") { sprite->visible = false;
            } else if (txt.find("set size to") != std::string::npos) {
                sprite->scale = parse_num(txt, "to ", 100) / 100.0f;
                if (sprite->scale < 0.05f) sprite->scale = 0.05f;
            } else if (txt.find("change size by") != std::string::npos) {
                sprite->scale += parse_num(txt, "by ", 10) / 100.0f;
                if (sprite->scale < 0.05f) sprite->scale = 0.05f;
            } else if (txt.find("next costume") != std::string::npos) {
                if (!sprite->costumes.empty()) {
                    sprite->currentCostume = (sprite->currentCostume + 1) % (int)sprite->costumes.size();
                    sprite->texture = sprite->costumes[sprite->currentCostume].texture;
                }
            } else if (txt.find("switch costume to") != std::string::npos) {
                int idx = parse_num(txt, "to ", 0);
                if (idx >= 0 && idx < (int)sprite->costumes.size()) {
                    sprite->currentCostume = idx;
                    sprite->texture = sprite->costumes[idx].texture;
                }
            }
        }

        else if (b->type == BLOCK_CONTROL) {
            if (txt.find("wait") != std::string::npos && txt.find("secs") != std::string::npos) {
                waitUntil = SDL_GetTicks() + (Uint32)(parse_num(txt, "wait ", 1) * 1000);
                waiting = true; saySilent = false;
            } else if (txt.find("repeat") != std::string::npos && txt.find("forever") == std::string::npos) {
                int count = parse_num(txt, "repeat ", 10);
                if (b->next) { loopStack.push_back({b->next, count}); current = b->next; return true; }
            } else if (txt.find("forever") != std::string::npos) {
                if (b->next) { loopStack.push_back({b->next, -1}); current = b->next; return true; }
            } else if (txt.find("stop") != std::string::npos) {
                stop(); return true;
            }
        }

        else if (b->type == BLOCK_OPERATORS) {
            if (txt.find("pick random") != std::string::npos) {
                int lo = parse_num(txt, "random ", 1);
                int hi = parse_num(txt, "to ", 10);
                if (hi < lo) std::swap(lo, hi);
                (void)(lo + rand() % (hi - lo + 1));
            }
        }

        return false;
    }

    void advance(Block* b) {
        if (b->next) { current = b->next; return; }
        if (!loopStack.empty()) {
            LoopFrame& f = loopStack.back();
            if (f.remaining == -1) { current = f.loopBody; return; }
            f.remaining--;
            if (f.remaining > 0) { current = f.loopBody; return; }
            loopStack.pop_back();
        }
        current = nullptr; running = false;
    }
};

Block* find_script_start(std::vector<Block*>& blocks) {
    for (Block* b : blocks)
        if (b->type == BLOCK_EVENT && b->prev == nullptr && b->next != nullptr)
            return b;
    for (Block* b : blocks)
        if (b->prev == nullptr) return b;
    return nullptr;
}

#endif
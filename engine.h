#ifndef SCRATCH_FOP_ENGINE_H
#define SCRATCH_FOP_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL.h>
#include "structs.h"
#include "globals.h"
#include "OperatorLogic.h"
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern map<string, float> g_vars;

inline int parse_num(const string& txt, const string& after, int fallback = 0) {
    size_t pos = txt.find(after);
    if (pos == string::npos) return fallback;
    pos += after.size();
    while (pos < txt.size() && txt[pos] == ' ') pos++;
    try { return stoi(txt.substr(pos)); }
    catch (...) { return fallback; }
}

inline float parse_float(const string& txt, const string& after, float fallback = 0.0f) {
    size_t pos = txt.find(after);
    if (pos == string::npos) return fallback;
    pos += after.size();
    while (pos < txt.size() && txt[pos] == ' ') pos++;
    try { return stof(txt.substr(pos)); }
    catch (...) { return fallback; }
}

inline void clamp_sprite(Sprite* s) {
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
    vector<LoopFrame> loopStack;
    vector<Variable>* vars = nullptr;

    void start(Block* first, vector<Variable>* varList = nullptr) {
        running = true; current = first;
        waitUntil = 0; waiting = false; saySilent = false;
        loopStack.clear();
        vars = varList;
        if (vars) for (auto& v : *vars) g_vars[v.name] = v.value;
    }

    void stop() {
        running = false; current = nullptr;
        waiting = false; saySilent = false;
        loopStack.clear();
    }

    void syncVarsBack() {
        if (!vars) return;
        for (auto& v : *vars) {
            auto it = g_vars.find(v.name);
            if (it != g_vars.end()) v.value = it->second;
        }
    }

    void update(Sprite* sprite) {
        if (!running || !sprite) return;
        Uint32 now = SDL_GetTicks();
        if (waiting) {
            if (now < waitUntil) return;
            waiting = false;
            if (saySilent) { sprite->sayText = ""; sprite->sayTimer = 0; saySilent = false; }
        }
        if (!current) { running = false; syncVarsBack(); return; }
        bool jumped = execute_block(current, sprite, now);
        if (!jumped) advance(current);
        syncVarsBack();
    }

    bool execute_block(Block* b, Sprite* sprite, Uint32 now) {
        const string& txt = b->text;

        if (b->type == BLOCK_EVENT) return false;

        if (b->type == BLOCK_MOTION) {
            if (txt.find("move") != string::npos && txt.find("steps") != string::npos) {
                int steps = parse_num(txt, "move ", 10);
                double rad = (sprite->direction - 90.0) * M_PI / 180.0;
                sprite->x += (float)(steps * cos(rad));
                sprite->y += (float)(steps * sin(rad));
                clamp_sprite(sprite);
            } else if (txt.find("turn") != string::npos) {
                int deg = parse_num(txt, "turn ", 15);
                if (txt.find("left") != string::npos) sprite->direction -= deg;
                else sprite->direction += deg;
            } else if (txt.find("go to x:") != string::npos) {
                int gx = parse_num(txt, "x:", 0);
                int gy = parse_num(txt, "y:", 0);
                sprite->x = (float)(STAGE_WIDTH  / 2 + gx - sprite->w / 2);
                sprite->y = (float)(STAGE_HEIGHT / 2 - gy - sprite->h / 2);
                clamp_sprite(sprite);
            } else if (txt.find("change x by") != string::npos) {
                sprite->x += parse_num(txt, "by ", 10); clamp_sprite(sprite);
            } else if (txt.find("change y by") != string::npos) {
                sprite->y -= parse_num(txt, "by ", 10); clamp_sprite(sprite);
            } else if (txt.find("set x to") != string::npos) {
                sprite->x = (float)(STAGE_WIDTH  / 2 + parse_num(txt, "to ", 0) - sprite->w / 2);
                clamp_sprite(sprite);
            } else if (txt.find("set y to") != string::npos) {
                sprite->y = (float)(STAGE_HEIGHT / 2 - parse_num(txt, "to ", 0) - sprite->h / 2);
                clamp_sprite(sprite);
            } else if (txt.find("point in direction") != string::npos) {
                sprite->direction = (float)parse_num(txt, "direction ", 90);
            } else if (txt.find("if on edge") != string::npos) {
                if (sprite->x <= 0 || sprite->x >= STAGE_WIDTH  - sprite->w)
                    sprite->direction = 180.0f - sprite->direction;
                if (sprite->y <= 0 || sprite->y >= STAGE_HEIGHT - sprite->h)
                    sprite->direction = -sprite->direction;
            }
        }
        else if (b->type == BLOCK_LOOKS) {
            if (txt.find("say") != string::npos || txt.find("think") != string::npos) {
                string keyword = (txt.find("say ") != string::npos) ? "say " : "think ";
                size_t sp = txt.find(keyword);
                string msg = (sp != string::npos) ? txt.substr(sp + keyword.size()) : "Hello!";
                size_t fp = msg.find(" for ");
                if (fp != string::npos) {
                    int secs = 2;
                    try { secs = stoi(msg.substr(fp + 5)); } catch (...) {}
                    sprite->sayText  = msg.substr(0, fp);
                    sprite->sayTimer = secs * 60;
                    waitUntil = SDL_GetTicks() + (Uint32)(secs * 1000);
                    waiting = true; saySilent = true;
                } else {
                    sprite->sayText = msg; sprite->sayTimer = 999999;
                }
            } else if (txt == "show") { sprite->visible = true;
            } else if (txt == "hide") { sprite->visible = false;
            } else if (txt.find("set size to") != string::npos) {
                sprite->scale = parse_float(txt, "to ", 100) / 100.0f;
                if (sprite->scale < 0.05f) sprite->scale = 0.05f;
            } else if (txt.find("change size by") != string::npos) {
                sprite->scale += parse_float(txt, "by ", 10) / 100.0f;
                if (sprite->scale < 0.05f) sprite->scale = 0.05f;
            } else if (txt.find("next costume") != string::npos) {
                if (!sprite->costumes.empty()) {
                    sprite->currentCostume = (sprite->currentCostume + 1) % (int)sprite->costumes.size();
                    if (sprite->costumes[sprite->currentCostume].texture)
                        sprite->texture = sprite->costumes[sprite->currentCostume].texture;
                }
            } else if (txt.find("switch costume to") != string::npos) {
                int idx = parse_num(txt, "to ", 0);
                if (idx >= 0 && idx < (int)sprite->costumes.size()) {
                    sprite->currentCostume = idx;
                    if (sprite->costumes[idx].texture)
                        sprite->texture = sprite->costumes[idx].texture;
                }
            }
        }
        else if (b->type == BLOCK_CONTROL) {
            if (txt.find("wait") != string::npos && txt.find("secs") != string::npos) {
                int secs = parse_num(txt, "wait ", 1);
                waitUntil = SDL_GetTicks() + (Uint32)(secs * 1000);
                waiting = true; saySilent = false;
            } else if (txt.find("if") != string::npos && txt.find("else") != string::npos) {
                bool cond = evaluate_condition(txt, sprite);
                if (!cond) {
                    Block* tmp = b->next;
                    while (tmp && tmp->text != "else") tmp = tmp->next;
                    if (tmp) { current = tmp->next; return true; }
                }
            } else if (txt.find("if") != string::npos) {
                bool cond = evaluate_condition(txt, sprite);
                if (!cond) {
                    Block* tmp = b->next;
                    while (tmp && tmp->text != "end if") tmp = tmp->next;
                    if (tmp) { current = tmp->next; return true; }
                }
            } else if (txt.find("repeat") != string::npos && txt.find("forever") == string::npos) {
                int count = parse_num(txt, "repeat ", 10);
                if (b->next) { loopStack.push_back({b->next, count}); current = b->next; return true; }
            } else if (txt.find("forever") != string::npos) {
                if (b->next) { loopStack.push_back({b->next, -1}); current = b->next; return true; }
            } else if (txt.find("stop") != string::npos) {
                stop(); return true;
            }
        }
        else if (b->type == BLOCK_VARIABLES) {
            if (txt.find("set ") == 0 && txt.find(" to ") != string::npos) {
                size_t toPos = txt.find(" to ");
                string varName = txt.substr(4, toPos - 4);
                g_vars[varName] = parse_float(txt, " to ", 0.0f);
            } else if (txt.find("change ") == 0 && txt.find(" by ") != string::npos) {
                size_t byPos = txt.find(" by ");
                string varName = txt.substr(7, byPos - 7);
                g_vars[varName] += parse_float(txt, " by ", 1.0f);
            }
        }
        else if (b->type == BLOCK_OPERATORS) {
            if (txt.find("pick random") != string::npos) {
                int lo = parse_num(txt, "random ", 1);
                int hi = parse_num(txt, "to ", 10);
                if (hi < lo) swap(lo, hi);
                int res = lo + rand() % (hi - lo + 1);
                sprite->sayText = to_string(res);
                sprite->sayTimer = 120;
            } else if (txt.find(" + ") != string::npos) {
                double v1 = parse_float(txt, "(", 0);
                double v2 = parse_float(txt, ") + (", 0);
                sprite->sayText = to_string(calculate(v1, v2, "+"));
                sprite->sayTimer = 120;
            } else if (txt.find(" - ") != string::npos) {
                double v1 = parse_float(txt, "(", 0);
                double v2 = parse_float(txt, ") - (", 0);
                sprite->sayText = to_string(calculate(v1, v2, "-"));
                sprite->sayTimer = 120;
            } else if (txt.find("join") != string::npos) {
                sprite->sayText = join("Hello ", "World");
                sprite->sayTimer = 120;
            } else if (txt.find("length of") != string::npos) {
                sprite->sayText = to_string(lengthOf("hello"));
                sprite->sayTimer = 120;
            }
        }

        return false;
    }

    bool evaluate_condition(const string& txt, Sprite* sprite) {
        if (txt.find(">") != string::npos) {
            float v1 = parse_float(txt, "(", 0);
            float v2 = parse_float(txt, "> ", 0);
            return v1 > v2;
        } else if (txt.find("<") != string::npos) {
            float v1 = parse_float(txt, "(", 0);
            float v2 = parse_float(txt, "< ", 0);
            return v1 < v2;
        } else if (txt.find("=") != string::npos) {
            float v1 = parse_float(txt, "(", 0);
            float v2 = parse_float(txt, "= ", 0);
            return v1 == v2;
        }
        return false;
    }

    void advance(Block* b) {
        if (b->next) { current = b->next; return; }
        while (!loopStack.empty()) {
            LoopFrame& f = loopStack.back();
            if (f.remaining == -1) { current = f.loopBody; return; }
            f.remaining--;
            if (f.remaining > 0) { current = f.loopBody; return; }
            loopStack.pop_back();
        }
        current = nullptr; running = false;
    }
};

inline Block* find_script_start(vector<Block*>& blocks) {
    for (Block* b : blocks)
        if (b->type == BLOCK_EVENT && b->prev == nullptr && b->next != nullptr)
            return b;
    for (Block* b : blocks)
        if (b->prev == nullptr) return b;
    return nullptr;
}

#endif
#ifndef SCRATCH_FOP_ENGINE_H
#define SCRATCH_FOP_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <SDL2/SDL.h>
#include "structs.h"
#include "globals.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ─── متغیرهای سراسری اسکریپت ────────────────────────────────────────────────
static std::map<std::string, float> g_vars;

// ─── نتیجه آخرین عملیات Operator (برای نمایش روی Stage) ─────────────────────
static float  g_lastOperatorResult = 0.0f;
static bool   g_hasOperatorResult  = false;
static std::string g_operatorResultText = "";

// ─── پارس اعداد از متن بلاک یا از inputs ─────────────────────────────────────
static float get_input_val(Block* b, int idx, float fallback = 0.0f) {
    if (idx < (int)b->inputs.size()) {
        try { return std::stof(b->inputs[idx].value); }
        catch (...) {}
    }
    return fallback;
}

static int parse_num(const std::string& txt,
                     const std::string& after, int fallback = 0) {
    size_t pos = txt.find(after);
    if (pos == std::string::npos) return fallback;
    pos += after.size();
    while (pos < txt.size() && txt[pos] == ' ') pos++;
    try { return std::stoi(txt.substr(pos)); }
    catch (...) { return fallback; }
}

static float parse_float(const std::string& txt,
                          const std::string& after, float fallback = 0.0f) {
    size_t pos = txt.find(after);
    if (pos == std::string::npos) return fallback;
    pos += after.size();
    while (pos < txt.size() && txt[pos] == ' ') pos++;
    try { return std::stof(txt.substr(pos)); }
    catch (...) { return fallback; }
}

static std::string float_to_str(float v) {
    // اگر عدد صحیح است بدون اعشار نشان بده
    if (v == std::floor(v) && std::abs(v) < 1e9f)
        return std::to_string((long long)v);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4) << v;
    // حذف صفرهای انتهایی
    std::string s = ss.str();
    size_t dot = s.find('.');
    if (dot != std::string::npos) {
        size_t last = s.find_last_not_of('0');
        if (last != std::string::npos && last > dot)
            s = s.substr(0, last + 1);
        else if (last == dot)
            s = s.substr(0, dot);
    }
    return s;
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

// ─── ScriptRunner ─────────────────────────────────────────────────────────────
struct ScriptRunner {
    bool   running   = false;
    Block* current   = nullptr;
    Uint32 waitUntil = 0;
    bool   waiting   = false;
    bool   saySilent = false;
    std::vector<LoopFrame> loopStack;
    std::vector<Variable>* vars = nullptr;

    void start(Block* first, std::vector<Variable>* varList = nullptr) {
        running = true; current = first;
        waitUntil = 0; waiting = false; saySilent = false;
        loopStack.clear();
        vars = varList;
        g_hasOperatorResult = false;
        g_operatorResultText = "";
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
            if (saySilent) {
                sprite->sayText = ""; sprite->sayTimer = 0; saySilent = false;
            }
        }
        if (!current) { running = false; syncVarsBack(); return; }
        bool jumped = execute_block(current, sprite, now);
        if (!jumped) advance(current);
        syncVarsBack();
    }

    // ─── اجرای یک بلاک ────────────────────────────────────────────────────────
    bool execute_block(Block* b, Sprite* sprite, Uint32 now) {
        const std::string& txt = b->type != BLOCK_OPERATORS
                                 ? b->text : b->text;

        if (b->type == BLOCK_EVENT) return false;

        // ── MOTION ────────────────────────────────────────────────────────────
        if (b->type == BLOCK_MOTION) {
            if (txt.find("move") != std::string::npos &&
                txt.find("steps") != std::string::npos) {
                float steps = b->inputs.empty()
                              ? (float)parse_num(txt, "move ", 10)
                              : get_input_val(b, 0, 10);
                double rad = (sprite->direction - 90.0) * M_PI / 180.0;
                sprite->x += (float)(steps * std::cos(rad));
                sprite->y += (float)(steps * std::sin(rad));
                clamp_sprite(sprite);
            }
            else if (txt.find("turn") != std::string::npos) {
                float deg = b->inputs.empty()
                            ? (float)parse_num(txt, "turn ", 15)
                            : get_input_val(b, 0, 15);
                if (txt.find("left") != std::string::npos)
                    sprite->direction -= deg;
                else
                    sprite->direction += deg;
            }
            else if (txt.find("go to x:") != std::string::npos) {
                float gx = (float)parse_num(txt, "x:", 0);
                float gy = (float)parse_num(txt, "y:", 0);
                sprite->x = STAGE_WIDTH  / 2.0f + gx - sprite->w / 2.0f;
                sprite->y = STAGE_HEIGHT / 2.0f - gy - sprite->h / 2.0f;
                clamp_sprite(sprite);
            }
            else if (txt.find("change x by") != std::string::npos) {
                sprite->x += b->inputs.empty()
                             ? (float)parse_num(txt, "by ", 10)
                             : get_input_val(b, 0, 10);
                clamp_sprite(sprite);
            }
            else if (txt.find("change y by") != std::string::npos) {
                sprite->y -= b->inputs.empty()
                             ? (float)parse_num(txt, "by ", 10)
                             : get_input_val(b, 0, 10);
                clamp_sprite(sprite);
            }
            else if (txt.find("set x to") != std::string::npos) {
                float val = b->inputs.empty()
                            ? (float)parse_num(txt, "to ", 0)
                            : get_input_val(b, 0, 0);
                sprite->x = STAGE_WIDTH / 2.0f + val - sprite->w / 2.0f;
                clamp_sprite(sprite);
            }
            else if (txt.find("set y to") != std::string::npos) {
                float val = b->inputs.empty()
                            ? (float)parse_num(txt, "to ", 0)
                            : get_input_val(b, 0, 0);
                sprite->y = STAGE_HEIGHT / 2.0f - val - sprite->h / 2.0f;
                clamp_sprite(sprite);
            }
            else if (txt.find("point in direction") != std::string::npos) {
                sprite->direction = b->inputs.empty()
                                    ? (float)parse_num(txt, "direction ", 90)
                                    : get_input_val(b, 0, 90);
            }
            else if (txt.find("if on edge") != std::string::npos) {
                if (sprite->x <= 0 || sprite->x >= STAGE_WIDTH  - sprite->w)
                    sprite->direction = 180.0f - sprite->direction;
                if (sprite->y <= 0 || sprite->y >= STAGE_HEIGHT - sprite->h)
                    sprite->direction = -sprite->direction;
            }
        }

        // ── LOOKS ─────────────────────────────────────────────────────────────
        else if (b->type == BLOCK_LOOKS) {
            if (txt.find("say") != std::string::npos ||
                txt.find("think") != std::string::npos) {
                std::string keyword = (txt.find("say ") != std::string::npos)
                                      ? "say " : "think ";
                size_t sp = txt.find(keyword);
                std::string msg = (sp != std::string::npos)
                                  ? txt.substr(sp + keyword.size()) : "Hello!";
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
            }
            else if (txt == "show") { sprite->visible = true; }
            else if (txt == "hide") { sprite->visible = false; }
            else if (txt.find("set size to") != std::string::npos) {
                float val = b->inputs.empty()
                            ? parse_float(txt, "to ", 100)
                            : get_input_val(b, 0, 100);
                sprite->scale = val / 100.0f;
                if (sprite->scale < 0.05f) sprite->scale = 0.05f;
            }
            else if (txt.find("change size by") != std::string::npos) {
                float val = b->inputs.empty()
                            ? parse_float(txt, "by ", 10)
                            : get_input_val(b, 0, 10);
                sprite->scale += val / 100.0f;
                if (sprite->scale < 0.05f) sprite->scale = 0.05f;
            }
            else if (txt.find("next costume") != std::string::npos) {
                if (!sprite->costumes.empty()) {
                    sprite->currentCostume =
                        (sprite->currentCostume + 1) % (int)sprite->costumes.size();
                    if (sprite->costumes[sprite->currentCostume].texture)
                        sprite->texture = sprite->costumes[sprite->currentCostume].texture;
                }
            }
            else if (txt.find("switch costume to") != std::string::npos) {
                int idx = parse_num(txt, "to ", 0);
                if (idx >= 0 && idx < (int)sprite->costumes.size()) {
                    sprite->currentCostume = idx;
                    if (sprite->costumes[idx].texture)
                        sprite->texture = sprite->costumes[idx].texture;
                }
            }
        }

        // ── CONTROL ───────────────────────────────────────────────────────────
        else if (b->type == BLOCK_CONTROL) {
            if (txt.find("wait") != std::string::npos &&
                txt.find("secs") != std::string::npos) {
                float secs = b->inputs.empty()
                             ? (float)parse_num(txt, "wait ", 1)
                             : get_input_val(b, 0, 1);
                waitUntil = SDL_GetTicks() + (Uint32)(secs * 1000);
                waiting = true; saySilent = false;
            }
            else if (txt.find("repeat") != std::string::npos &&
                     txt.find("forever") == std::string::npos) {
                int count = b->inputs.empty()
                            ? parse_num(txt, "repeat ", 10)
                            : (int)get_input_val(b, 0, 10);
                if (b->next) {
                    loopStack.push_back({b->next, count});
                    current = b->next;
                    return true;
                }
            }
            else if (txt.find("forever") != std::string::npos) {
                if (b->next) {
                    loopStack.push_back({b->next, -1});
                    current = b->next;
                    return true;
                }
            }
            else if (txt.find("stop") != std::string::npos) {
                stop(); return true;
            }
        }

        // ── VARIABLES ─────────────────────────────────────────────────────────
        else if (b->type == BLOCK_VARIABLES) {
            if (txt.find("set ") == 0 && txt.find(" to ") != std::string::npos) {
                size_t toPos = txt.find(" to ");
                std::string varName = txt.substr(4, toPos - 4);
                float val = b->inputs.empty()
                            ? parse_float(txt, " to ", 0.0f)
                            : get_input_val(b, 0, 0.0f);
                g_vars[varName] = val;
            }
            else if (txt.find("change ") == 0 &&
                     txt.find(" by ") != std::string::npos) {
                size_t byPos = txt.find(" by ");
                std::string varName = txt.substr(7, byPos - 7);
                float delta = b->inputs.empty()
                              ? parse_float(txt, " by ", 1.0f)
                              : get_input_val(b, 0, 1.0f);
                g_vars[varName] += delta;
            }
        }

        // ── OPERATORS – پیاده‌سازی کامل ─────────────────────────────────────
        else if (b->type == BLOCK_OPERATORS) {
            float a = get_input_val(b, 0, 0.0f);
            float c = get_input_val(b, 1, 0.0f);  // دومین ورودی
            float result = 0.0f;
            bool  computed = true;

            if (txt.find("() + ()") != std::string::npos ||
                txt.find(" + ") != std::string::npos) {
                result = a + c;
                g_operatorResultText = float_to_str(a) + " + " +
                                       float_to_str(c) + " = " +
                                       float_to_str(result);
            }
            else if (txt.find("() - ()") != std::string::npos ||
                     txt.find(" - ") != std::string::npos) {
                result = a - c;
                g_operatorResultText = float_to_str(a) + " - " +
                                       float_to_str(c) + " = " +
                                       float_to_str(result);
            }
            else if (txt.find("() * ()") != std::string::npos ||
                     txt.find(" * ") != std::string::npos) {
                result = a * c;
                g_operatorResultText = float_to_str(a) + " × " +
                                       float_to_str(c) + " = " +
                                       float_to_str(result);
            }
            else if (txt.find("() / ()") != std::string::npos ||
                     txt.find(" / ") != std::string::npos) {
                result = (c != 0.0f) ? a / c : 0.0f;
                g_operatorResultText = float_to_str(a) + " ÷ " +
                                       float_to_str(c) + " = " +
                                       (c != 0.0f ? float_to_str(result) : "ERR");
            }
            else if (txt.find("() mod ()") != std::string::npos) {
                result = (c != 0.0f) ? std::fmod(a, c) : 0.0f;
                g_operatorResultText = float_to_str(a) + " mod " +
                                       float_to_str(c) + " = " +
                                       float_to_str(result);
            }
            else if (txt.find("() < ()") != std::string::npos) {
                result = (a < c) ? 1.0f : 0.0f;
                g_operatorResultText = float_to_str(a) + " < " +
                                       float_to_str(c) + " → " +
                                       (result ? "true" : "false");
            }
            else if (txt.find("() > ()") != std::string::npos) {
                result = (a > c) ? 1.0f : 0.0f;
                g_operatorResultText = float_to_str(a) + " > " +
                                       float_to_str(c) + " → " +
                                       (result ? "true" : "false");
            }
            else if (txt.find("() = ()") != std::string::npos) {
                result = (a == c) ? 1.0f : 0.0f;
                g_operatorResultText = float_to_str(a) + " = " +
                                       float_to_str(c) + " → " +
                                       (result ? "true" : "false");
            }
            else if (txt.find("round ()") != std::string::npos) {
                result = std::round(a);
                g_operatorResultText = "round(" + float_to_str(a) + ") = " +
                                       float_to_str(result);
            }
            else if (txt.find("abs of ()") != std::string::npos) {
                result = std::abs(a);
                g_operatorResultText = "abs(" + float_to_str(a) + ") = " +
                                       float_to_str(result);
            }
            else if (txt.find("sqrt of ()") != std::string::npos) {
                result = (a >= 0) ? std::sqrt(a) : 0.0f;
                g_operatorResultText = "√(" + float_to_str(a) + ") = " +
                                       float_to_str(result);
            }
            else if (txt.find("floor of ()") != std::string::npos) {
                result = std::floor(a);
                g_operatorResultText = "floor(" + float_to_str(a) + ") = " +
                                       float_to_str(result);
            }
            else if (txt.find("ceiling of ()") != std::string::npos) {
                result = std::ceil(a);
                g_operatorResultText = "ceiling(" + float_to_str(a) + ") = " +
                                       float_to_str(result);
            }
            else if (txt.find("sin of ()") != std::string::npos) {
                result = std::sin(a * M_PI / 180.0);
                g_operatorResultText = "sin(" + float_to_str(a) + "°) = " +
                                       float_to_str(result);
            }
            else if (txt.find("cos of ()") != std::string::npos) {
                result = std::cos(a * M_PI / 180.0);
                g_operatorResultText = "cos(" + float_to_str(a) + "°) = " +
                                       float_to_str(result);
            }
            else if (txt.find("pick random") != std::string::npos) {
                int lo = (int)get_input_val(b, 0, 1);
                int hi = (int)get_input_val(b, 1, 10);
                if (hi < lo) std::swap(lo, hi);
                result = (float)(lo + rand() % (hi - lo + 1));
                g_operatorResultText = "random(" + std::to_string(lo) +
                                       "," + std::to_string(hi) + ") = " +
                                       float_to_str(result);
            }
            else {
                computed = false;
            }

            if (computed) {
                g_lastOperatorResult = result;
                g_hasOperatorResult  = true;
                // نمایش نتیجه روی sprite به عنوان say
                sprite->sayText  = g_operatorResultText;
                sprite->sayTimer = 180; // 3 ثانیه (60fps)
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

// ─── نمایش نتیجه Operator روی Stage ─────────────────────────────────────────
inline void draw_operator_result(SDL_Renderer* r, TTF_Font* font, Stage* stage) {
    if (!g_hasOperatorResult || !font || !stage) return;

    const std::string& txt = g_operatorResultText;
    int tw, th;
    TTF_SizeUTF8(font, txt.c_str(), &tw, &th);

    int bx = stage->x + stage->w / 2 - tw / 2 - 12;
    int by = stage->y + 8;
    int bw = tw + 24;
    int bh = th + 12;

    // سایه
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 60);
    SDL_Rect shadow = {bx + 2, by + 2, bw, bh};
    SDL_RenderFillRect(r, &shadow);

    // پس‌زمینه سبز Operator
    SDL_SetRenderDrawColor(r, 89, 192, 89, 230);
    SDL_Rect bg = {bx, by, bw, bh};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 50, 150, 50, 255);
    SDL_RenderDrawRect(r, &bg);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    draw_text(r, font, txt, bx + 12, by + 6, COLOR_TEXT_WHITE);
}

#endif

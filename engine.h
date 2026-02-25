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
#include "OperatorManager.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static std::map<std::string, float> g_vars;
static float  g_lastOperatorResult  = 0.0f;
static bool   g_hasOperatorResult   = false;
static std::string g_operatorResultText = "";

static std::string g_answer         = "";
static bool        g_askPending     = false;
static std::string g_askQuestion    = "";

static Uint32 g_timerStart = 0;

static Sprite* g_currentSprite = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static Stage*   g_stage         = nullptr;

static float eval_embedded_numeric(Block* b);
static bool  eval_embedded_boolean(Block* b);
static std::string float_to_str(float v);

static float get_input_val(Block* b, int idx, float fallback = 0.0f) {
    if (idx < (int)b->inputs.size()) {
        if (b->inputs[idx].embeddedBlock) {
            return eval_embedded_numeric(b->inputs[idx].embeddedBlock);
        }
        try { return std::stof(b->inputs[idx].value); }
        catch (...) {}
    }
    return fallback;
}

static bool get_input_bool(Block* b, int idx) {
    if (idx < (int)b->inputs.size()) {
        if (b->inputs[idx].embeddedBlock) {
            return eval_embedded_boolean(b->inputs[idx].embeddedBlock);
        }
        float val = 0;
        try { val = std::stof(b->inputs[idx].value); } catch (...) {}
        return val != 0.0f;
    }
    return false;
}

static float eval_embedded_numeric(Block* b) {
    if (!b) return 0.0f;
    const std::string& txt = b->text;
    float a = get_input_val(b, 0, 0.0f);
    float c = get_input_val(b, 1, 0.0f);

    if (txt.find("() + ()") != std::string::npos) return a + c;
    if (txt.find("() - ()") != std::string::npos) return a - c;
    if (txt.find("() * ()") != std::string::npos) return a * c;
    if (txt.find("() / ()") != std::string::npos) return (c != 0) ? a / c : 0;
    if (txt.find("() mod ()") != std::string::npos) return (c != 0) ? std::fmod(a, c) : 0;
    if (txt.find("round ()") != std::string::npos) return std::round(a);
    if (txt.find("abs of ()") != std::string::npos) return std::abs(a);
    if (txt.find("sqrt of ()") != std::string::npos) return (a >= 0) ? std::sqrt(a) : 0;
    if (txt.find("floor of ()") != std::string::npos) return std::floor(a);
    if (txt.find("ceiling of ()") != std::string::npos) return std::ceil(a);
    if (txt.find("sin of ()") != std::string::npos) return (float)std::sin(a * M_PI / 180.0);
    if (txt.find("cos of ()") != std::string::npos) return (float)std::cos(a * M_PI / 180.0);
    if (txt.find("tan of ()") != std::string::npos) return (float)std::tan(a * M_PI / 180.0);
    if (txt.find("pick random") != std::string::npos) {
        int lo = (int)a, hi = (int)c;
        if (hi < lo) std::swap(lo, hi);
        return (float)(lo + rand() % (hi - lo + 1));
    }
    if (txt == "timer" || txt.find("timer") != std::string::npos)
        return (float)(SDL_GetTicks() - g_timerStart) / 1000.0f;
    if (txt == "mouse x") {
        int mx2, my2; SDL_GetMouseState(&mx2, &my2);
        return (float)(mx2 - (STAGE_X + STAGE_WIDTH / 2));
    }
    if (txt == "mouse y") {
        int mx2, my2; SDL_GetMouseState(&mx2, &my2);
        return (float)((STAGE_Y + STAGE_HEIGHT / 2) - my2);
    }
    if (txt == "answer") {
        try { return std::stof(g_answer); } catch (...) { return 0.0f; }
    }
    if (txt.find("distance to mouse") != std::string::npos) {
        if (!g_currentSprite) return 0.0f;
        int mx2, my2; SDL_GetMouseState(&mx2, &my2);
        float scx = STAGE_X + g_currentSprite->x + g_currentSprite->w * g_currentSprite->scale / 2.0f;
        float scy = STAGE_Y + g_currentSprite->y + g_currentSprite->h * g_currentSprite->scale / 2.0f;
        float dx2 = (float)(mx2 - scx), dy2 = (float)(my2 - scy);
        return std::sqrt(dx2*dx2 + dy2*dy2);
    }
    if (txt.find("() < ()") != std::string::npos) return (a < c) ? 1.0f : 0.0f;
    if (txt.find("() > ()") != std::string::npos) return (a > c) ? 1.0f : 0.0f;
    if (txt.find("() = ()") != std::string::npos) return (a == c) ? 1.0f : 0.0f;
    if (txt.find("<> and <>") != std::string::npos) return (get_input_bool(b,0) && get_input_bool(b,1)) ? 1.0f : 0.0f;
    if (txt.find("<> or <>") != std::string::npos)  return (get_input_bool(b,0) || get_input_bool(b,1)) ? 1.0f : 0.0f;
    if (txt.find("not <>") != std::string::npos)    return (!get_input_bool(b,0)) ? 1.0f : 0.0f;
    return 0.0f;
}

static bool eval_embedded_boolean(Block* b) {
    if (!b) return false;
    const std::string& txt = b->text;
    float a = get_input_val(b, 0, 0.0f);
    float c = get_input_val(b, 1, 0.0f);

    if (txt.find("() < ()") != std::string::npos) return a < c;
    if (txt.find("() > ()") != std::string::npos) return a > c;
    if (txt.find("() = ()") != std::string::npos) return a == c;
    if (txt.find("<> and <>") != std::string::npos) return get_input_bool(b,0) && get_input_bool(b,1);
    if (txt.find("<> or <>") != std::string::npos)  return get_input_bool(b,0) || get_input_bool(b,1);
    if (txt.find("not <>") != std::string::npos)    return !get_input_bool(b,0);

    if (txt.find("touching mouse-pointer") != std::string::npos) {
        if (!g_currentSprite) return false;
        int mx2, my2; SDL_GetMouseState(&mx2, &my2);
        float sx = g_currentSprite->x + STAGE_X;
        float sy = g_currentSprite->y + STAGE_Y;
        float sw = g_currentSprite->w * g_currentSprite->scale;
        float sh = g_currentSprite->h * g_currentSprite->scale;
        return mx2 >= sx && mx2 <= sx + sw && my2 >= sy && my2 <= sy + sh;
    }
    if (txt.find("touching edge") != std::string::npos) {
        if (!g_currentSprite) return false;
        float maxX = (float)(STAGE_WIDTH  - (int)(g_currentSprite->w * g_currentSprite->scale));
        float maxY = (float)(STAGE_HEIGHT - (int)(g_currentSprite->h * g_currentSprite->scale));
        return g_currentSprite->x <= 0 || g_currentSprite->x >= maxX
            || g_currentSprite->y <= 0 || g_currentSprite->y >= maxY;
    }
    if (txt.find("mouse down") != std::string::npos ||
        (txt.find("down") != std::string::npos && txt.find("mouse") != std::string::npos)) {
        int mx2, my2;
        Uint32 mb = SDL_GetMouseState(&mx2, &my2);
        return (mb & SDL_BUTTON(1)) != 0;
    }
    if (txt.find("key") != std::string::npos &&
        txt.find("pressed") != std::string::npos) {
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        if (txt.find("space")  != std::string::npos) return keys[SDL_SCANCODE_SPACE]  != 0;
        if (txt.find("right")  != std::string::npos) return keys[SDL_SCANCODE_RIGHT]  != 0;
        if (txt.find("left")   != std::string::npos) return keys[SDL_SCANCODE_LEFT]   != 0;
        if (txt.find("up")     != std::string::npos) return keys[SDL_SCANCODE_UP]     != 0;
        if (txt.find("down")   != std::string::npos) return keys[SDL_SCANCODE_DOWN]   != 0;
        if (txt.find("enter")  != std::string::npos) return keys[SDL_SCANCODE_RETURN] != 0;
        if (txt.find("escape") != std::string::npos) return keys[SDL_SCANCODE_ESCAPE] != 0;
        for (char ch = 'a'; ch <= 'z'; ch++) {
            std::string letter(1, ch);
            if (txt.find(letter) != std::string::npos) {
                SDL_Scancode sc = SDL_GetScancodeFromKey(SDLK_a + (ch - 'a'));
                return keys[sc] != 0;
            }
        }
        for (char ch = '0'; ch <= '9'; ch++) {
            std::string digit(1, ch);
            if (txt.find(digit) != std::string::npos) {
                SDL_Scancode sc = SDL_GetScancodeFromKey(SDLK_0 + (ch - '0'));
                return keys[sc] != 0;
            }
        }
        return false;
    }

    return eval_embedded_numeric(b) != 0.0f;
}

static std::string get_input_str(Block* b, int idx,
                                  const std::string& fallback = "") {
    if (idx < (int)b->inputs.size()) {
        if (b->inputs[idx].embeddedBlock) {
            return float_to_str(eval_embedded_numeric(b->inputs[idx].embeddedBlock));
        }
        return b->inputs[idx].value;
    }
    return fallback;
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
    if (v == std::floor(v) && std::abs(v) < 1e9f)
        return std::to_string((long long)v);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4) << v;
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
    float maxX = (float)(STAGE_WIDTH  - (int)(s->w * s->scale));
    float maxY = (float)(STAGE_HEIGHT - (int)(s->h * s->scale));
    if (s->x < 0)    s->x = 0;
    if (s->y < 0)    s->y = 0;
    if (s->x > maxX) s->x = maxX;
    if (s->y > maxY) s->y = maxY;
}

struct LoopFrame {
    Block*  loopBlock;
    Block*  current;
    int     remaining;
    bool    inElse;
};

struct ScriptRunner {
    bool   running   = false;
    bool   paused    = false;
    Block* current   = nullptr;
    Uint32 waitUntil = 0;
    bool   waiting   = false;
    bool   saySilent = false;
    std::vector<LoopFrame> loopStack;
    std::vector<Variable>* vars = nullptr;

    Sprite* askSprite = nullptr;

    void start(Block* first, std::vector<Variable>* varList = nullptr) {
        running = true; paused = false; current = first;
        waitUntil = 0; waiting = false; saySilent = false;
        loopStack.clear();
        vars = varList;
        g_hasOperatorResult  = false;
        g_operatorResultText = "";
        g_timerStart = SDL_GetTicks();
        if (vars) for (auto& v : *vars) g_vars[v.name] = v.value;
    }

    void stop() {
        running = false; paused = false; current = nullptr;
        waiting = false; saySilent = false;
        loopStack.clear();
        g_askPending = false;
    }

    void togglePause() {
        if (!running) return;
        paused = !paused;
    }

    void syncVarsBack() {
        if (!vars) return;
        for (auto& v : *vars) {
            auto it = g_vars.find(v.name);
            if (it != g_vars.end()) v.value = it->second;
        }
    }

    void update(Sprite* sprite) {
        if (!running || paused || !sprite) return;
        askSprite = sprite;
        g_currentSprite = sprite;

        if (g_askPending) return;

        Uint32 now = SDL_GetTicks();
        if (waiting) {
            if (now < waitUntil) return;
            waiting = false;
            if (saySilent) {
                sprite->sayText = ""; sprite->sayTimer = 0; saySilent = false;
            }
        }
        if (!current) {
            if (!loopStack.empty()) {
                advance_loop(sprite);
                return;
            }
            running = false; syncVarsBack(); return;
        }
        bool jumped = execute_block(current, sprite, now);
        if (!jumped) advance(current, sprite);
        syncVarsBack();
    }

    bool eval_condition(Block* b) {
        if (b->inputs.empty()) return false;
        if (b->inputs[0].embeddedBlock) {
            return eval_embedded_boolean(b->inputs[0].embeddedBlock);
        }
        const std::string& val = b->inputs[0].value;
        float fval = 0;
        try { fval = std::stof(val); } catch (...) {}
        return fval != 0.0f;
    }

    bool execute_block(Block* b, Sprite* sprite, Uint32 now) {
        const std::string& txt = b->text;

        if (b->type == BLOCK_EVENT) return false;

        if (b->type == BLOCK_MOTION) {
            if (txt.find("move") != std::string::npos &&
                txt.find("steps") != std::string::npos) {
                float steps = get_input_val(b, 0, 10);
                double rad = (sprite->direction - 90.0) * M_PI / 180.0;
                sprite->x += (float)(steps * std::cos(rad));
                sprite->y += (float)(steps * std::sin(rad));
                clamp_sprite(sprite);
            }
            else if (txt.find("turn right") != std::string::npos ||
                     (txt.find("turn") != std::string::npos &&
                      txt.find("left") == std::string::npos)) {
                float deg = get_input_val(b, 0, 15);
                sprite->direction += deg;
            }
            else if (txt.find("turn left") != std::string::npos) {
                float deg = get_input_val(b, 0, 15);
                sprite->direction -= deg;
            }
            else if (txt.find("go to x:") != std::string::npos) {
                float gx = get_input_val(b, 0, 0);
                float gy = get_input_val(b, 1, 0);
                sprite->x = STAGE_WIDTH  / 2.0f + gx - sprite->w * sprite->scale / 2.0f;
                sprite->y = STAGE_HEIGHT / 2.0f - gy - sprite->h * sprite->scale / 2.0f;
                clamp_sprite(sprite);
            }
            else if (txt.find("go to random") != std::string::npos) {
                sprite->x = (float)(rand() % (STAGE_WIDTH  - (int)(sprite->w * sprite->scale)));
                sprite->y = (float)(rand() % (STAGE_HEIGHT - (int)(sprite->h * sprite->scale)));
            }
            else if (txt.find("go to mouse") != std::string::npos) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                sprite->x = (float)(mx - STAGE_X) - sprite->w * sprite->scale / 2.0f;
                sprite->y = (float)(my - STAGE_Y) - sprite->h * sprite->scale / 2.0f;
                clamp_sprite(sprite);
            }
            else if (txt.find("glide") != std::string::npos &&
                     txt.find("secs") != std::string::npos) {
                float secs = get_input_val(b, 0, 1);
                float gx   = get_input_val(b, 1, 0);
                float gy   = get_input_val(b, 2, 0);
                sprite->x = STAGE_WIDTH  / 2.0f + gx - sprite->w * sprite->scale / 2.0f;
                sprite->y = STAGE_HEIGHT / 2.0f - gy - sprite->h * sprite->scale / 2.0f;
                clamp_sprite(sprite);
                waitUntil = SDL_GetTicks() + (Uint32)(secs * 1000);
                waiting = true;
            }
            else if (txt.find("point in direction") != std::string::npos) {
                sprite->direction = get_input_val(b, 0, 90);
            }
            else if (txt.find("point towards mouse") != std::string::npos) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                float sx = STAGE_X + sprite->x + sprite->w * sprite->scale / 2.0f;
                float sy = STAGE_Y + sprite->y + sprite->h * sprite->scale / 2.0f;
                float dx = (float)(mx - sx), dy = (float)(my - sy);
                sprite->direction = (float)(std::atan2(dy, dx) * 180.0 / M_PI) + 90.0f;
            }
            else if (txt.find("change x by") != std::string::npos) {
                sprite->x += get_input_val(b, 0, 10);
                clamp_sprite(sprite);
            }
            else if (txt.find("change y by") != std::string::npos) {
                sprite->y -= get_input_val(b, 0, 10);
                clamp_sprite(sprite);
            }
            else if (txt.find("set x to") != std::string::npos) {
                float val = get_input_val(b, 0, 0);
                sprite->x = STAGE_WIDTH / 2.0f + val - sprite->w * sprite->scale / 2.0f;
                clamp_sprite(sprite);
            }
            else if (txt.find("set y to") != std::string::npos) {
                float val = get_input_val(b, 0, 0);
                sprite->y = STAGE_HEIGHT / 2.0f - val - sprite->h * sprite->scale / 2.0f;
                clamp_sprite(sprite);
            }
            else if (txt.find("if on edge") != std::string::npos) {
                float maxX = (float)(STAGE_WIDTH  - (int)(sprite->w * sprite->scale));
                float maxY = (float)(STAGE_HEIGHT - (int)(sprite->h * sprite->scale));
                if (sprite->x <= 0 || sprite->x >= maxX)
                    sprite->direction = 180.0f - sprite->direction;
                if (sprite->y <= 0 || sprite->y >= maxY)
                    sprite->direction = -sprite->direction;
                clamp_sprite(sprite);
            }
        }

        else if (b->type == BLOCK_LOOKS) {
            if (txt.find("say") != std::string::npos ||
                txt.find("think") != std::string::npos) {
                if (txt.find("for") != std::string::npos && b->inputs.size() >= 2) {
                    sprite->sayText  = get_input_str(b, 0, "Hello!");
                    float secs = get_input_val(b, 1, 2);
                    sprite->sayTimer = (int)(secs * 60);
                    waitUntil = SDL_GetTicks() + (Uint32)(secs * 1000);
                    waiting = true; saySilent = true;
                } else if (!b->inputs.empty()) {
                    sprite->sayText  = get_input_str(b, 0, "Hello!");
                    sprite->sayTimer = 999999;
                } else {
                    sprite->sayText  = "Hello!";
                    sprite->sayTimer = 999999;
                }
            }
            else if (txt == "show")        { sprite->visible = true; }
            else if (txt == "hide")        { sprite->visible = false; }
            else if (txt.find("set size to") != std::string::npos) {
                float val = get_input_val(b, 0, 100);
                sprite->scale = val / 100.0f;
                if (sprite->scale < 0.05f) sprite->scale = 0.05f;
            }
            else if (txt.find("change size by") != std::string::npos) {
                float val = get_input_val(b, 0, 10);
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
                int idx = (int)get_input_val(b, 0, 0);
                if (idx >= 1) idx--;
                if (idx >= 0 && idx < (int)sprite->costumes.size()) {
                    sprite->currentCostume = idx;
                    if (sprite->costumes[idx].texture)
                        sprite->texture = sprite->costumes[idx].texture;
                }
            }
            else if (txt.find("switch backdrop to") != std::string::npos) {
                int idx = (int)get_input_val(b, 0, 1);
                if (g_stage) {
                    SDL_Color bgColors[] = {
                        {255,255,255,255},
                        {173,216,230,255},
                        {255,228,196,255},
                        {144,238,144,255}
                    };
                    int ci = ((idx-1) % 4 + 4) % 4;
                    g_stage->color = bgColors[ci];
                }
            }
            else if (txt.find("next backdrop") != std::string::npos) {
                if (g_stage) {
                    static int bgIdx = 0;
                    bgIdx = (bgIdx + 1) % 4;
                    SDL_Color bgColors[] = {
                        {255,255,255,255},{173,216,230,255},
                        {255,228,196,255},{144,238,144,255}
                    };
                    g_stage->color = bgColors[bgIdx];
                }
            }
        }

        else if (b->type == BLOCK_EXTENSION) {
            if (txt.find("pen down") != std::string::npos) {
                sprite->penDown  = true;
                sprite->lastPenX = sprite->x;
                sprite->lastPenY = sprite->y;
            }
            else if (txt.find("pen up") != std::string::npos) {
                sprite->penDown  = false;
                sprite->lastPenX = -9999;
                sprite->lastPenY = -9999;
            }
            else if (txt.find("erase all") != std::string::npos) {
                if (g_renderer) pen_trail_clear(g_renderer);
            }
            else if (txt.find("stamp") != std::string::npos) {
                if (g_renderer) pen_stamp(g_renderer, sprite);
            }
            else if (txt.find("set pen color") != std::string::npos) {
                int idx = (int)get_input_val(b, 0, 0);
                SDL_Color colors[] = {
                    {0,0,0,255},{220,50,50,255},{50,180,50,255},
                    {50,50,220,255},{230,170,30,255},{150,60,200,255}
                };
                sprite->penColor = colors[((idx % 6) + 6) % 6];
            }
            else if (txt.find("change pen size by") != std::string::npos) {
                sprite->penSize += (int)get_input_val(b, 0, 1);
                if (sprite->penSize < 1) sprite->penSize = 1;
            }
            else if (txt.find("set pen size to") != std::string::npos) {
                sprite->penSize = (int)get_input_val(b, 0, 2);
                if (sprite->penSize < 1) sprite->penSize = 1;
            }
        }


        else if (b->type == BLOCK_SOUND) {
            if (g_soundsPanel) {
                if (txt.find("play sound") != std::string::npos) {
                    std::string sname = get_input_str(b, 0, "");
                    bool untilDone = (txt.find("until done") != std::string::npos);
                    for (auto& sc : g_soundsPanel->sounds) {
                        bool match = sname.empty()
                            || sc.name.find(sname) != std::string::npos
                            || sname.find(sc.name) != std::string::npos;
                        if (match) {
                            if (sc.chunk) {
                                int ch = Mix_PlayChannel(-1, sc.chunk, 0);
                                sc.channel   = ch;
                                sc.isPlaying = true;
                                if (untilDone && sc.durationSecs > 0) {
                                    waitUntil = SDL_GetTicks() + (Uint32)(sc.durationSecs * 1000);
                                    waiting   = true;
                                }
                            }
                            break;
                        }
                    }
                }
                else if (txt.find("stop all sounds") != std::string::npos) {
                    Mix_HaltChannel(-1);
                    for (auto& sc : g_soundsPanel->sounds)
                        sc.isPlaying = false;
                }
                else if (txt.find("change volume by") != std::string::npos) {
                    float delta = get_input_val(b, 0, 10);
                    for (auto& sc : g_soundsPanel->sounds) {
                        sc.volume = std::max(0.0f, std::min(100.0f, sc.volume + delta));
                        if (sc.channel >= 0 && sc.isPlaying)
                            Mix_Volume(sc.channel, (int)(sc.volume / 100.0f * MIX_MAX_VOLUME));
                    }
                }
                else if (txt.find("set volume to") != std::string::npos) {
                    float val = get_input_val(b, 0, 100);
                    for (auto& sc : g_soundsPanel->sounds) {
                        sc.volume = std::max(0.0f, std::min(100.0f, val));
                        if (sc.channel >= 0 && sc.isPlaying)
                            Mix_Volume(sc.channel, (int)(sc.volume / 100.0f * MIX_MAX_VOLUME));
                    }
                }
                else if (txt.find("clear sound effects") != std::string::npos) {
                    Mix_HaltChannel(-1);
                }
            }
        }

        else if (b->type == BLOCK_SENSING) {
            if (txt.find("ask") != std::string::npos &&
                txt.find("and wait") != std::string::npos) {
                std::string question = get_input_str(b, 0, "What's your name?");
                sprite->sayText  = question;
                sprite->sayTimer = 999999;
                g_askPending  = true;
                g_askQuestion = question;
                g_answer      = "";
            }
            else if (txt.find("reset timer") != std::string::npos) {
                g_timerStart = SDL_GetTicks();
            }
        }

        else if (b->type == BLOCK_CONTROL) {
            if (txt.find("wait") != std::string::npos &&
                txt.find("secs") != std::string::npos &&
                txt.find("until") == std::string::npos) {
                float secs = get_input_val(b, 0, 1);
                waitUntil = SDL_GetTicks() + (Uint32)(secs * 1000);
                waiting = true; saySilent = false;
            }
            else if (txt.find("wait until") != std::string::npos) {
                if (!eval_condition(b)) {
                    return true;
                }
            }
            else if (txt.find("repeat") != std::string::npos &&
                     txt.find("forever") == std::string::npos) {
                int count = (int)get_input_val(b, 0, 10);
                if (b->isCShaped && b->innerFirst) {
                    loopStack.push_back({b, b->innerFirst, count, false});
                    current = b->innerFirst;
                    return true;
                }
            }
            else if (txt.find("forever") != std::string::npos) {
                if (b->isCShaped && b->innerFirst) {
                    loopStack.push_back({b, b->innerFirst, -1, false});
                    current = b->innerFirst;
                    return true;
                }
            }
            else if (txt.find("if") != std::string::npos &&
                     txt.find("then") != std::string::npos) {
                bool cond = eval_condition(b);
                if (b->hasElse) {
                    if (cond && b->innerFirst) {
                        loopStack.push_back({b, b->innerFirst, 1, false});
                        current = b->innerFirst;
                        return true;
                    } else if (!cond && b->elseFirst) {
                        loopStack.push_back({b, b->elseFirst, 1, true});
                        current = b->elseFirst;
                        return true;
                    }
                } else {
                    if (cond && b->innerFirst) {
                        loopStack.push_back({b, b->innerFirst, 1, false});
                        current = b->innerFirst;
                        return true;
                    }
                }
            }
            else if (txt.find("stop all") != std::string::npos &&
                     txt.find("script") == std::string::npos) {
                stop(); return true;
            }
            else if (txt.find("stop this script") != std::string::npos ||
                     txt.find("stop other") != std::string::npos) {
                stop(); return true;
            }
        }

        else if (b->type == BLOCK_VARIABLES) {
            if (txt.find("set ") == 0 && txt.find(" to ") != std::string::npos) {
                size_t toPos = txt.find(" to ");
                std::string varName = txt.substr(4, toPos - 4);
                float val = get_input_val(b, 0, 0.0f);
                g_vars[varName] = val;
            }
            else if (txt.find("change ") == 0 &&
                     txt.find(" by ") != std::string::npos) {
                size_t byPos = txt.find(" by ");
                std::string varName = txt.substr(7, byPos - 7);
                float delta = get_input_val(b, 0, 1.0f);
                g_vars[varName] += delta;
            }
            else if (txt.find("show variable") != std::string::npos) {
                if (vars) {
                    size_t p = txt.find("variable ") + 9;
                    std::string vname = txt.substr(p);
                    for (auto& v : *vars)
                        if (v.name == vname) v.showOnStage = true;
                }
            }
            else if (txt.find("hide variable") != std::string::npos) {
                if (vars) {
                    size_t p = txt.find("variable ") + 9;
                    std::string vname = txt.substr(p);
                    for (auto& v : *vars)
                        if (v.name == vname) v.showOnStage = false;
                }
            }
        }

        else if (b->type == BLOCK_OPERATORS) {
            float a = get_input_val(b, 0, 0.0f);
            float c = get_input_val(b, 1, 0.0f);
            float result = 0.0f;
            bool  computed = true;

            if      (txt.find("() + ()") != std::string::npos) {
                result = a + c;
                g_operatorResultText = float_to_str(a)+" + "+float_to_str(c)+" = "+float_to_str(result);
            }
            else if (txt.find("() - ()") != std::string::npos) {
                result = a - c;
                g_operatorResultText = float_to_str(a)+" - "+float_to_str(c)+" = "+float_to_str(result);
            }
            else if (txt.find("() * ()") != std::string::npos) {
                result = a * c;
                g_operatorResultText = float_to_str(a)+" × "+float_to_str(c)+" = "+float_to_str(result);
            }
            else if (txt.find("() / ()") != std::string::npos) {
                result = (c != 0) ? a / c : 0;
                g_operatorResultText = float_to_str(a)+" ÷ "+float_to_str(c)+" = "+(c!=0?float_to_str(result):"ERR");
            }
            else if (txt.find("() mod ()") != std::string::npos) {
                result = (c != 0) ? std::fmod(a, c) : 0;
                g_operatorResultText = float_to_str(a)+" mod "+float_to_str(c)+" = "+float_to_str(result);
            }
            else if (txt.find("() < ()") != std::string::npos) {
                result = (a < c) ? 1 : 0;
                g_operatorResultText = float_to_str(a)+" < "+float_to_str(c)+" → "+(result?"true":"false");
            }
            else if (txt.find("() > ()") != std::string::npos) {
                result = (a > c) ? 1 : 0;
                g_operatorResultText = float_to_str(a)+" > "+float_to_str(c)+" → "+(result?"true":"false");
            }
            else if (txt.find("() = ()") != std::string::npos) {
                result = (a == c) ? 1 : 0;
                g_operatorResultText = float_to_str(a)+" = "+float_to_str(c)+" → "+(result?"true":"false");
            }
            else if (txt.find("and") != std::string::npos) {
                bool ba = get_input_bool(b, 0);
                bool bc = get_input_bool(b, 1);
                result = (ba && bc) ? 1 : 0;
                g_operatorResultText = std::string(ba?"true":"false") + " AND " + std::string(bc?"true":"false") + " → " + std::string(result?"true":"false");            }
            else if (txt.find("or") != std::string::npos) {
                bool ba = get_input_bool(b, 0);
                bool bc = get_input_bool(b, 1);
                result = (ba || bc) ? 1 : 0;
                g_operatorResultText = std::string(ba?"true":"false") + " OR " + std::string(bc?"true":"false") + " → " + std::string(result?"true":"false");
            }
            else if (txt.find("not <>") != std::string::npos) {
                bool ba = get_input_bool(b, 0);
                result = (!ba) ? 1 : 0;
                g_operatorResultText = std::string("NOT ") + std::string(ba?"true":"false") + " → " + std::string(result?"true":"false");
            }
            else if (txt.find("round ()") != std::string::npos) {
                result = std::round(a);
                g_operatorResultText = "round("+float_to_str(a)+") = "+float_to_str(result);
            }
            else if (txt.find("abs of ()") != std::string::npos) {
                result = std::abs(a);
                g_operatorResultText = "abs("+float_to_str(a)+") = "+float_to_str(result);
            }
            else if (txt.find("sqrt of ()") != std::string::npos) {
                result = (a >= 0) ? std::sqrt(a) : 0;
                g_operatorResultText = "√("+float_to_str(a)+") = "+float_to_str(result);
            }
            else if (txt.find("floor of ()") != std::string::npos) {
                result = std::floor(a);
                g_operatorResultText = "floor("+float_to_str(a)+") = "+float_to_str(result);
            }
            else if (txt.find("ceiling of ()") != std::string::npos) {
                result = std::ceil(a);
                g_operatorResultText = "ceiling("+float_to_str(a)+") = "+float_to_str(result);
            }
            else if (txt.find("sin of ()") != std::string::npos) {
                result = (float)std::sin(a * M_PI / 180.0);
                g_operatorResultText = "sin("+float_to_str(a)+"°) = "+float_to_str(result);
            }
            else if (txt.find("cos of ()") != std::string::npos) {
                result = (float)std::cos(a * M_PI / 180.0);
                g_operatorResultText = "cos("+float_to_str(a)+"°) = "+float_to_str(result);
            }
            else if (txt.find("tan of ()") != std::string::npos) {
                result = (float)std::tan(a * M_PI / 180.0);
                g_operatorResultText = "tan("+float_to_str(a)+"°) = "+float_to_str(result);
            }
            else if (txt.find("pick random") != std::string::npos) {
                int lo = (int)get_input_val(b, 0, 1);
                int hi = (int)get_input_val(b, 1, 10);
                if (hi < lo) std::swap(lo, hi);
                result = (float)(lo + rand() % (hi - lo + 1));
                g_operatorResultText = "random("+std::to_string(lo)+","+std::to_string(hi)+") = "+float_to_str(result);
            }
            else if (txt.find("join () ()") != std::string::npos) {
                std::string s1 = get_input_str(b, 0, "hello");
                std::string s2 = get_input_str(b, 1, "world");
                std::string res = s1 + s2;
                g_operatorResultText = "join: \"" + res + "\"";
                sprite->sayText  = res;
                sprite->sayTimer = 180;
                g_hasOperatorResult = true;
                return false;
            }
            else if (txt.find("length of ()") != std::string::npos) {
                std::string s = get_input_str(b, 0, "");
                result = (float)s.size();
                g_operatorResultText = "length(\""+s+"\") = "+float_to_str(result);
            }
            else if (txt.find("letter () of ()") != std::string::npos) {
                int idx2 = (int)get_input_val(b, 0, 1) - 1;
                std::string s = get_input_str(b, 1, "");
                std::string res = (idx2 >= 0 && idx2 < (int)s.size())
                                  ? std::string(1, s[idx2]) : "";
                g_operatorResultText = "letter "+std::to_string(idx2+1)+" of \""+s+"\" = \""+res+"\"";
                sprite->sayText = res; sprite->sayTimer = 180;
                g_hasOperatorResult = true;
                return false;
            }
            else { computed = false; }

            if (computed) {
                g_lastOperatorResult = result;
                g_hasOperatorResult  = true;
                sprite->sayText  = g_operatorResultText;
                sprite->sayTimer = 180;
            }
        }

        return false;
    }

    void advance(Block* b, Sprite* /*sprite*/) {
        if (b->next) {
            current = b->next;
            return;
        }
        if (!loopStack.empty()) {
            advance_loop(nullptr);
            return;
        }
        current = nullptr;
        running = false;
    }

    void advance_loop(Sprite* /*sprite*/) {
        if (loopStack.empty()) { current = nullptr; running = false; return; }

        LoopFrame& f = loopStack.back();

        Block* nextInner = f.current ? f.current->next : nullptr;

        if (nextInner) {
            f.current = nextInner;
            current   = nextInner;
            return;
        }

        if (f.remaining == -1) {
            Block* first = f.inElse ? f.loopBlock->elseFirst
                                     : f.loopBlock->innerFirst;
            f.current = first;
            current   = first;
            return;
        }
        if (f.remaining > 1) {
            f.remaining--;
            Block* first = f.loopBlock->innerFirst;
            f.current = first;
            current   = first;
            return;
        }
        Block* afterC = f.loopBlock->next;
        loopStack.pop_back();
        if (afterC) {
            current = afterC;
        } else if (!loopStack.empty()) {
            advance_loop(nullptr);
        } else {
            current = nullptr; running = false;
        }
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

Block* find_key_event(std::vector<Block*>& blocks, const std::string& keyName) {
    for (Block* b : blocks)
        if (b->type == BLOCK_EVENT && b->prev == nullptr && b->next != nullptr &&
            b->text.find("when key pressed") != std::string::npos) {
            if (!b->inputs.empty() && b->inputs[0].value == keyName) return b;
            if (b->inputs.empty()) return b;
        }
    return nullptr;
}

Block* find_sprite_click_event(std::vector<Block*>& blocks) {
    for (Block* b : blocks)
        if (b->type == BLOCK_EVENT && b->prev == nullptr && b->next != nullptr &&
            b->text.find("when sprite clicked") != std::string::npos)
            return b;
    return nullptr;
}

inline void draw_operator_result(SDL_Renderer* r, TTF_Font* font, Stage* stage) {
    if (!g_hasOperatorResult || !font || !stage) return;
    const std::string& txt = g_operatorResultText;
    int tw, th;
    TTF_SizeUTF8(font, txt.c_str(), &tw, &th);
    int bx = stage->x + stage->w/2 - tw/2 - 12;
    int by = stage->y + 8;
    int bw = tw + 24, bh = th + 12;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0,0,0,60);
    SDL_Rect shadow = {bx+2, by+2, bw, bh};
    SDL_RenderFillRect(r, &shadow);
    bool isBool = (g_operatorResultText.find("true") != std::string::npos ||
                       g_operatorResultText.find("false") != std::string::npos);
    SDL_Color resBg = isBool ? SDL_Color{60,130,220,230} : SDL_Color{89,192,89,230};
    SDL_SetRenderDrawColor(r, resBg.r, resBg.g, resBg.b, resBg.a);
    SDL_Rect bg = {bx, by, bw, bh};
    SDL_RenderFillRect(r, &bg);
    SDL_SetRenderDrawColor(r, 50,150,50,255);
    SDL_RenderDrawRect(r, &bg);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    extern void draw_text(SDL_Renderer*, TTF_Font*, const std::string&, int, int, SDL_Color);
    draw_text(r, font, txt, bx+12, by+6, {255,255,255,255});
}


#endif
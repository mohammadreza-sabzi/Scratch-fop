//
// Created by Domim on 2/24/2026.
//

#pragma once
#ifndef TAB_BAR_H
#define TAB_BAR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cmath>
#include "globals.h"
#include "structs.h"
#include "render.h"

enum ActiveTab {
    TAB_CODE = 0,
    TAB_COSTUMES,
    TAB_SOUNDS
};

inline void draw_tab_bar(SDL_Renderer* r, TTF_Font* font, ActiveTab active)
{
    const int stripY = HEADER_H;

    SDL_SetRenderDrawColor(r, 220, 215, 235, 255);
    SDL_Rect strip = { 0, stripY, SCREEN_WIDTH, TAB_H };
    SDL_RenderFillRect(r, &strip);

    struct TabInfo {
        const char* label;
        int x, w;
        ActiveTab id;
        SDL_Color accent;
    };
    const TabInfo tabs[] = {
        { "Code",     TAB_CODE_X, TAB_CODE_W, TAB_CODE,
          { 89, 192, 89, 255 } },
        { "Costumes", TAB_COST_X, TAB_COST_W, TAB_COSTUMES,
          { 155, 89, 182, 255 } },
        { "Sounds",   TAB_SND_X,  TAB_SND_W,  TAB_SOUNDS,
          { 207, 74, 217, 255 } },
    };

    for (const auto& t : tabs) {
        const bool active_tab = (t.id == active);
        SDL_Rect tabRect = { t.x, stripY, t.w, TAB_H };

        SDL_SetRenderDrawColor(r,
            active_tab ? 255 : 210,
            active_tab ? 255 : 205,
            active_tab ? 255 : 225, 255);
        SDL_RenderFillRect(r, &tabRect);

        if (active_tab) {
            SDL_SetRenderDrawColor(r,
                t.accent.r, t.accent.g, t.accent.b, 255);
            SDL_Rect accent = { t.x, stripY + TAB_H - 4, t.w, 4 };
            SDL_RenderFillRect(r, &accent);
        }

        SDL_SetRenderDrawColor(r, 180, 170, 200, 255);
        SDL_RenderDrawRect(r, &tabRect);

        const int dotR  = 5;
        const int dotCX = t.x + 14;
        const int dotCY = stripY + TAB_H / 2;
        SDL_SetRenderDrawColor(r,
            t.accent.r, t.accent.g, t.accent.b,
            active_tab ? 255 : 160);
        for (int dy = -dotR; dy <= dotR; dy++) {
            int dx = (int)std::sqrt((double)(dotR*dotR - dy*dy));
            SDL_RenderDrawLine(r, dotCX-dx, dotCY+dy, dotCX+dx, dotCY+dy);
        }

        if (font) {
            SDL_Color tc = active_tab
                ? COLOR_TEXT_DARK
                : SDL_Color{110, 95, 140, 255};
            int tw = 0, th = 0;
            TTF_SizeUTF8(font, t.label, &tw, &th);
            int lx = t.x + (t.w - tw) / 2 + 8;
            int ly = stripY + (TAB_H - th) / 2;
            draw_text(r, font, t.label, lx, ly, tc);
        }
    }

    SDL_SetRenderDrawColor(r, 160, 150, 185, 255);
    SDL_RenderDrawLine(r, 0, stripY + TAB_H - 1,
                          SCREEN_WIDTH, stripY + TAB_H - 1);
}

inline void draw_toolbar_icons(SDL_Renderer* r, TTF_Font* font,
                                SDL_Rect& gearBtn,  SDL_Rect& notesBtn,
                                SDL_Rect& penBtn,   SDL_Rect& bulbBtn,
                                SDL_Rect& saveBtn,  SDL_Rect& loadBtn)
{
    const int by = ICON_BTN_Y;

    saveBtn = { 8,          by, 56, ICON_BTN_SIZE };
    loadBtn = { 8 + 56 + 8, by, 56, ICON_BTN_SIZE };

    int bx = STAGE_X - 12 - ICON_BTN_SIZE;
    bulbBtn  = { bx, by, ICON_BTN_SIZE, ICON_BTN_SIZE }; bx -= ICON_BTN_SIZE + 8;
    penBtn   = { bx, by, ICON_BTN_SIZE, ICON_BTN_SIZE }; bx -= ICON_BTN_SIZE + 8;
    notesBtn = { bx, by, ICON_BTN_SIZE, ICON_BTN_SIZE }; bx -= ICON_BTN_SIZE + 8;
    gearBtn  = { bx, by, ICON_BTN_SIZE, ICON_BTN_SIZE };

    struct BtnDef { SDL_Rect& rect; const char* sym; SDL_Color col; };
    BtnDef defs[] = {
        { saveBtn,  "Save", {  60, 180,  80, 255 } },
        { loadBtn,  "Load", {  60, 140, 220, 255 } },
        { gearBtn,  "\xE2\x9A\x99", { 130,  90, 200, 255 } },
        { notesBtn, "Prj",  { 130,  90, 200, 255 } },
        { penBtn,   "Edt",  { 130,  90, 200, 255 } },
        { bulbBtn,  "Tip",  { 220, 180,  40, 255 } },
    };

    for (auto& d : defs) {
        SDL_SetRenderDrawColor(r,
            d.col.r, d.col.g, d.col.b, 210);
        SDL_RenderFillRect(r, &d.rect);
        SDL_SetRenderDrawColor(r,
            (Uint8)(d.col.r * 0.65f),
            (Uint8)(d.col.g * 0.65f),
            (Uint8)(d.col.b * 0.65f), 255);
        SDL_RenderDrawRect(r, &d.rect);
        if (font)
            draw_text_centered(r, font, d.sym, d.rect, COLOR_TEXT_WHITE);
    }
}

inline bool tab_bar_click(int mx, int my, ActiveTab& active)
{
    const int stripY = HEADER_H;
    if (my < stripY || my >= stripY + TAB_H) return false;

    struct T { int x, w; ActiveTab id; };
    const T tabs[] = {
        { TAB_CODE_X, TAB_CODE_W, TAB_CODE     },
        { TAB_COST_X, TAB_COST_W, TAB_COSTUMES },
        { TAB_SND_X,  TAB_SND_W,  TAB_SOUNDS   },
    };
    for (const auto& t : tabs) {
        if (mx >= t.x && mx < t.x + t.w) {
            active = t.id;
            return true;
        }
    }
    return false;
}


inline std::string escape_str(const std::string& s) {
    // جایگزینی \n با \\n و | با \pipe
    std::string out;
    for (char c : s) {
        if (c == '|')  out += "\\pipe";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

inline std::string unescape_str(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i+1 < s.size()) {
            if (s.substr(i+1, 4) == "pipe") { out += '|'; i += 4; }
            else if (s[i+1] == 'n') { out += '\n'; i += 1; }
            else out += s[i];
        } else {
            out += s[i];
        }
    }
    return out;
}

inline bool save_project(const std::string& path,
                          const std::vector<Block*>& wsBlocks,
                          const VariablesPanel& vars)
{
    std::ofstream f(path);
    if (!f.is_open()) return false;

    f << "# Scratch-CPP project v3\n";

    for (const Block* b : wsBlocks) {
        if (!b) continue;
        int prevId = b->prev ? b->prev->id : -1;

        f << "BLOCK|"
          << b->id       << "|"
          << (int)b->type << "|"
          << b->x        << "|"
          << b->y        << "|"
          << prevId      << "|"
          << escape_str(b->text) << "\n";

        for (const auto& inp : b->inputs) {
            f << "INPUT|"
              << b->id      << "|"
              << inp.index  << "|"
              << escape_str(inp.value) << "\n";
        }
    }

    for (const auto& v : vars.variables) {
        f << "VAR|"
          << escape_str(v.name) << "|"
          << v.value << "|"
          << (v.showOnStage ? 1 : 0) << "\n";
    }

    f.flush();
    return true;
}

inline bool load_project(const std::string& path,
                          std::vector<Block*>& wsBlocks,
                          VariablesPanel& vars,
                          int& nextId)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;

    // پاک کردن workspace فعلی
    for (auto* b : wsBlocks) delete b;
    wsBlocks.clear();

    std::map<int, Block*> idMap;
    std::map<int, int>    prevMap;

    auto split_line = [](const std::string& line, char delim)
        -> std::vector<std::string>
    {
        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string part;
        while (std::getline(ss, part, delim))
            parts.push_back(part);
        return parts;
    };

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto parts = split_line(line, '|');
        if (parts.empty()) continue;

        const std::string& token = parts[0];

        if (token == "BLOCK" && parts.size() >= 7) {
            int  id      = std::stoi(parts[1]);
            int  typeInt = std::stoi(parts[2]);
            int  bx      = std::stoi(parts[3]);
            int  by      = std::stoi(parts[4]);
            int  prevId  = std::stoi(parts[5]);
            std::string text = unescape_str(parts[6]);

            Block* b = new Block();
            b->id           = id;
            b->type         = (BlockType)typeInt;
            b->text         = text;
            b->x            = bx;
            b->y            = by;
            b->w            = BLOCK_W;
            b->h            = BLOCK_H;
            b->isDragging   = false;
            b->dragOffsetX  = 0;
            b->dragOffsetY  = 0;
            b->next         = nullptr;
            b->prev         = nullptr;

            // مقداردهی inputs از متن بلاک
            b->inputs.clear();
            int idx = 0;
            for (size_t i = 0; i + 1 < text.size(); i++) {
                if (text[i] == '(' && text[i+1] == ')') {
                    BlockInput inp;
                    inp.value   = "10";
                    inp.editing = false;
                    inp.index   = idx++;
                    b->inputs.push_back(inp);
                    ++i;
                }
            }

            idMap[id]   = b;
            prevMap[id] = prevId;
            wsBlocks.push_back(b);

            if (id >= nextId) nextId = id + 1;
        }
        else if (token == "INPUT" && parts.size() >= 4) {
            int blockId  = std::stoi(parts[1]);
            int inputIdx = std::stoi(parts[2]);
            std::string val = unescape_str(parts[3]);

            auto it = idMap.find(blockId);
            if (it != idMap.end()) {
                Block* b = it->second;
                if (inputIdx >= 0 && inputIdx < (int)b->inputs.size())
                    b->inputs[inputIdx].value = val;
            }
        }
        else if (token == "VAR" && parts.size() >= 4) {
            std::string name = unescape_str(parts[1]);
            float val        = std::stof(parts[2]);
            int   show       = std::stoi(parts[3]);

            bool found = false;
            for (auto& v : vars.variables) {
                if (v.name == name) {
                    v.value       = val;
                    v.showOnStage = (show != 0);
                    found = true;
                    break;
                }
            }
            if (!found)
                vars.variables.push_back({ name, val, (show != 0) });
        }
    }

    for (auto& kv : idMap) {
        int    id  = kv.first;
        Block* b   = kv.second;
        int    pid = prevMap[id];

        if (pid >= 0) {
            auto pit = idMap.find(pid);
            if (pit != idMap.end()) {
                b->prev            = pit->second;
                pit->second->next  = b;
            }
        }
    }

    return true;
}

#endif // TAB_BAR_H


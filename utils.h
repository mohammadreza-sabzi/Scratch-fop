




#ifndef SCRATCH_FOP_UTILS_H
#define SCRATCH_FOP_UTILS_H

#include <vector>
#include <string>
#include <algorithm>
#include "structs.h"
#include "globals.h"

bool point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

bool is_c_shaped(const std::string& txt) {
    return txt == "forever"
        || txt.find("repeat") != std::string::npos
        || txt.find("if <>") != std::string::npos
        || txt.find("if <> then") != std::string::npos
        || txt.find("if <> then else") != std::string::npos
        || txt.find("wait until <>") != std::string::npos;
}

bool has_else_section(const std::string& txt) {
    return txt.find("if <> then else") != std::string::npos;
}

bool is_boolean_operator(const std::string& txt) {
    return txt.find("() < ()") != std::string::npos
        || txt.find("() > ()") != std::string::npos
        || txt.find("() = ()") != std::string::npos
        || txt.find("<> and <>") != std::string::npos
        || txt.find("<> or <>") != std::string::npos
        || txt.find("not <>") != std::string::npos;
}

bool is_numeric_operator(const std::string& txt) {
    if (txt.find("BLOCK_OPERATORS") != std::string::npos) return false;
    return (txt.find("() + ()") != std::string::npos
         || txt.find("() - ()") != std::string::npos
         || txt.find("() * ()") != std::string::npos
         || txt.find("() / ()") != std::string::npos
         || txt.find("() mod ()") != std::string::npos
         || txt.find("pick random") != std::string::npos
         || txt.find("round ()") != std::string::npos
         || txt.find("abs of ()") != std::string::npos
         || txt.find("sqrt of ()") != std::string::npos
         || txt.find("floor of ()") != std::string::npos
         || txt.find("ceiling of ()") != std::string::npos
         || txt.find("sin of ()") != std::string::npos
         || txt.find("cos of ()") != std::string::npos
         || txt.find("tan of ()") != std::string::npos
         || txt.find("join () ()") != std::string::npos
         || txt.find("length of ()") != std::string::npos
         || txt.find("letter () of ()") != std::string::npos);
}

int embedded_block_width(Block* b);

int compute_block_width(Block* b) {
    const std::string& txt = b->text;
    const int CHAR_W   = 7;
    const int INPUT_W_PER_CHAR = 8;
    const int INPUT_MIN_W = 30;
    const int BOOL_SLOT_W = 40;
    const int PADDING  = 24;

    int textLen = 0;
    int inputIdx = 0;
    for (size_t i = 0; i < txt.size(); i++) {
        if (txt[i] == '(' && i + 1 < txt.size() && txt[i+1] == ')') {
            int vw = INPUT_MIN_W;
            if (inputIdx < (int)b->inputs.size()) {
                if (b->inputs[inputIdx].embeddedBlock) {
                    vw = embedded_block_width(b->inputs[inputIdx].embeddedBlock) + 4;
                } else {
                    int chars = (int)b->inputs[inputIdx].value.size();
                    vw = std::max(INPUT_MIN_W, chars * INPUT_W_PER_CHAR + 8);
                }
            }
            textLen += vw + 4;
            inputIdx++;
            i++;
        } else if (txt[i] == '<' && i + 1 < txt.size() && txt[i+1] == '>') {
            int vw = BOOL_SLOT_W;
            if (inputIdx < (int)b->inputs.size() && b->inputs[inputIdx].embeddedBlock) {
                vw = embedded_block_width(b->inputs[inputIdx].embeddedBlock) + 4;
            }
            textLen += vw + 4;
            inputIdx++;
            i++;
        } else {
            textLen += CHAR_W;
        }
    }
    return std::max(BLOCK_W, textLen + PADDING);
}

int embedded_block_width(Block* b) {
    if (!b) return 30;
    return compute_block_width(b);
}

std::string default_input_val(const std::string& txt, int inputIdx) {
    if (txt.find("move") != std::string::npos)                return "10";
    if (txt.find("turn") != std::string::npos)                return "15";
    if (txt.find("go to x:") != std::string::npos)            return "0";
    if (txt.find("glide") != std::string::npos) {
        if (inputIdx == 0) return "1";
        return "0";
    }
    if (txt.find("point in direction") != std::string::npos)  return "90";
    if (txt.find("change x by") != std::string::npos)         return "10";
    if (txt.find("change y by") != std::string::npos)         return "10";
    if (txt.find("set x to") != std::string::npos)            return "0";
    if (txt.find("set y to") != std::string::npos)            return "0";
    if (txt.find("say") != std::string::npos ||
        txt.find("think") != std::string::npos) {
        if (inputIdx == 0) return "Hello!";
        return "2";
    }
    if (txt.find("switch costume") != std::string::npos)      return "0";
    if (txt.find("set size to") != std::string::npos)         return "100";
    if (txt.find("change size by") != std::string::npos)      return "10";
    if (txt.find("wait") != std::string::npos &&
        txt.find("until") == std::string::npos)               return "1";
    if (txt.find("repeat") != std::string::npos)              return "10";
    if (txt.find("set volume") != std::string::npos)          return "100";
    if (txt.find("change volume") != std::string::npos)       return "10";
    if (txt.find("change pitch") != std::string::npos)        return "10";
    if (txt.find("set pitch") != std::string::npos)           return "100";
    if (txt.find("pick random") != std::string::npos) {
        if (inputIdx == 0) return "1";
        return "10";
    }
    if (txt.find("set ") == 0 && txt.find(" to ") != std::string::npos) return "0";
    if (txt.find("change ") == 0 && txt.find(" by ") != std::string::npos) return "1";
    return "0";
}

void init_block_inputs(Block* b) {
    b->inputs.clear();
    const std::string& txt = b->text;
    int idx = 0;
    for (size_t i = 0; i < txt.size(); i++) {
        if (txt[i] == '(' && i + 1 < txt.size() && txt[i+1] == ')') {
            BlockInput inp;
            inp.value   = default_input_val(txt, idx);
            inp.editing = false;
            inp.index   = idx++;
            inp.slotType = SLOT_NUMERIC;
            inp.embeddedBlock = nullptr;
            b->inputs.push_back(inp);
            i++;
        } else if (txt[i] == '<' && i + 1 < txt.size() && txt[i+1] == '>') {
            BlockInput inp;
            inp.value   = "0";
            inp.editing = false;
            inp.index   = idx++;
            inp.slotType = SLOT_BOOLEAN;
            inp.embeddedBlock = nullptr;
            b->inputs.push_back(inp);
            i++;
        }
    }
    b->isCShaped = is_c_shaped(txt);
    b->hasElse   = has_else_section(txt);
    b->innerH    = 40;
    b->elseH     = 40;
    b->w         = compute_block_width(b);
}

Block* clone_block(Block* src);

static Block* clone_embedded(Block* src) {
    if (!src) return nullptr;
    Block* nb = new Block(*src);
    nb->next = nullptr; nb->prev = nullptr;
    nb->innerFirst = nullptr; nb->innerLast = nullptr;
    nb->elseFirst = nullptr;
    nb->isDragging = false;
    for (auto& inp : nb->inputs) {
        inp.editing = false;
        if (inp.embeddedBlock) {
            inp.embeddedBlock = clone_embedded(inp.embeddedBlock);
        }
    }
    return nb;
}

Block* clone_block(Block* src) {
    Block* nb = new Block(*src);
    nb->next       = nullptr;
    nb->prev       = nullptr;
    nb->innerFirst = nullptr;
    nb->innerLast  = nullptr;
    nb->elseFirst  = nullptr;
    nb->isDragging = false;
    for (auto& inp : nb->inputs) {
        inp.editing = false;
        if (inp.embeddedBlock) {
            inp.embeddedBlock = clone_embedded(inp.embeddedBlock);
        }
    }
    return nb;
}

bool block_matches_category(Block* b, CategoryType cat) {
    switch (cat) {
        case CAT_MOTION:    return b->type == BLOCK_MOTION;
        case CAT_LOOKS:     return b->type == BLOCK_LOOKS;
        case CAT_SOUND:     return b->type == BLOCK_SOUND;
        case CAT_EVENTS:    return b->type == BLOCK_EVENT;
        case CAT_CONTROL:   return b->type == BLOCK_CONTROL;
        case CAT_SENSING:   return b->type == BLOCK_SENSING;
        case CAT_OPERATORS: return b->type == BLOCK_OPERATORS;
        case CAT_VARIABLES: return b->type == BLOCK_VARIABLES;
        case CAT_MYBLOCKS:  return b->type == BLOCK_MYBLOCKS;
        case CAT_EXTENSION: return b->type == BLOCK_EXTENSION;
        default: return false;
    }
}

int block_total_height(Block* b) {
    if (!b->isCShaped) return b->h;
    int total = b->h;
    total += b->innerH;
    total += 8;
    if (b->hasElse) {
        total += 20;
        total += b->elseH;
        total += 8;
    }
    return total;
}

void recalc_inner_height(Block* b) {
    if (!b->isCShaped) return;
    int h = 0;
    Block* inner = b->innerFirst;
    while (inner) {
        h += block_total_height(inner);
        inner = inner->next;
    }
    b->innerH = std::max(40, h);

    if (b->hasElse) {
        h = 0;
        Block* el = b->elseFirst;
        while (el) {
            h += block_total_height(el);
            el = el->next;
        }
        b->elseH = std::max(40, h);
    }
}

int c_inner_y(Block* b) {
    return b->y + b->h;
}

int c_else_y(Block* b) {
    return c_inner_y(b) + b->innerH + 8;
}

int c_bottom_y(Block* b) {
    if (!b->isCShaped) return b->y + b->h;
    int base = c_inner_y(b) + b->innerH + 8;
    if (b->hasElse) base += 20 + b->elseH + 8;
    return base;
}

void layout_inner_blocks(Block* cblock) {
    if (!cblock->isCShaped) return;
    int innerY = c_inner_y(cblock) + 4;
    int innerX = cblock->x + 16;
    int innerW = cblock->w - 16;
    Block* b = cblock->innerFirst;
    int h = 0;
    while (b) {
        b->x = innerX;
        b->y = innerY;
        b->w = innerW;
        h += block_total_height(b);
        innerY += block_total_height(b);
        layout_inner_blocks(b);
        b = b->next;
    }
    cblock->innerH = std::max(40, h + 4);

    if (cblock->hasElse) {
        int elseY = c_else_y(cblock) + 4 + 20;
        h = 0;
        b = cblock->elseFirst;
        while (b) {
            b->x = innerX;
            b->y = elseY;
            b->w = innerW;
            h += block_total_height(b);
            elseY += block_total_height(b);
            layout_inner_blocks(b);
            b = b->next;
        }
        cblock->elseH = std::max(40, h + 4);
    }
}

void update_block_input_rects(Block* b) {
    if (b->inputs.empty()) return;
    const std::string& txt = b->text;
    int inputIdx = 0;
    const int CHAR_W = 7;
    const int BOOL_SLOT_W = 40;
    int xCursor  = b->x + 10;
    int yCenter  = b->y + (b->h - 18) / 2;

    for (size_t i = 0; i < txt.size() && inputIdx < (int)b->inputs.size(); i++) {
        if (txt[i] == '(' && i + 1 < txt.size() && txt[i+1] == ')') {
            BlockInput& inp = b->inputs[inputIdx];
            int valW;
            if (inp.embeddedBlock) {
                valW = embedded_block_width(inp.embeddedBlock) + 4;
            } else {
                const std::string& val = inp.value;
                valW = std::max(30, (int)val.size() * 8 + 8);
            }
            inp.rect = {xCursor, yCenter - 1, valW, 18};
            if (inp.embeddedBlock) {
                inp.embeddedBlock->x = xCursor + 2;
                inp.embeddedBlock->y = yCenter - 1;
                inp.embeddedBlock->h = 18;
                inp.embeddedBlock->w = valW - 4;
                update_block_input_rects(inp.embeddedBlock);
            }
            xCursor += valW + 4;
            inputIdx++;
            i++;
        } else if (txt[i] == '<' && i + 1 < txt.size() && txt[i+1] == '>') {
            BlockInput& inp = b->inputs[inputIdx];
            int valW;
            if (inp.embeddedBlock) {
                valW = embedded_block_width(inp.embeddedBlock) + 4;
            } else {
                valW = BOOL_SLOT_W;
            }
            inp.rect = {xCursor, yCenter - 2, valW, 20};
            if (inp.embeddedBlock) {
                inp.embeddedBlock->x = xCursor + 2;
                inp.embeddedBlock->y = yCenter - 2;
                inp.embeddedBlock->h = 20;
                inp.embeddedBlock->w = valW - 4;
                update_block_input_rects(inp.embeddedBlock);
            }
            xCursor += valW + 4;
            inputIdx++;
            i++;
        } else {
            xCursor += CHAR_W;
        }
    }
}

void layout_palette_blocks(std::vector<Block*>& paletteBlocks,
                            const Palette& palette)
{
    int hdrH = (palette.activeCategory == CAT_VARIABLES ||
                palette.activeCategory == CAT_MYBLOCKS) ? 88 : 44;

    int startY = palette.blockListY + hdrH + BLOCK_LIST_TOP_OFFSET;
    int y = startY + palette.scrollOffset;
    int x = palette.blockListX + 10;
    int bw = palette.blockListW - 20;

    for (Block* b : paletteBlocks) {
        if (!block_matches_category(b, palette.activeCategory)) {
            b->x = palette.blockListX - 9999;
            b->y = -9999;
            b->w = bw;
            b->h = BLOCK_H;
            continue;
        }
        b->x = x;
        b->y = y;
        b->w = std::max(bw, compute_block_width(b));
        b->h = BLOCK_H;
        y += BLOCK_H + 6;
    }
}

Block* check_palette_click(int mx, int my,
                            std::vector<Block*>& blocks,
                            Palette& palette) {
    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        Block* b = blocks[i];
        if (!block_matches_category(b, palette.activeCategory)) continue;
        if (b->x < palette.blockListX) continue;
        if (b->y < palette.blockListY) continue;
        if (point_in_rect(mx, my, b->x, b->y, b->w, b->h)) return b;
    }
    return nullptr;
}

void handle_category_click(int mx, int my, Palette& palette) {
    for (auto& cat : palette.categories) {
        if (point_in_rect(mx, my, cat.iconRect.x, cat.iconRect.y,
                          cat.iconRect.w, cat.iconRect.h)) {
            palette.activeCategory = cat.type;
            palette.scrollOffset   = 0;
            break;
        }
    }
}

void handle_scroll_value(SDL_Event& e, int& scrollOffset) {
    scrollOffset += e.wheel.y * 20;
    if (scrollOffset > 0) scrollOffset = 0;
}

#endif

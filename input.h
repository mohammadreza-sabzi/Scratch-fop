//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_INPUT_H
#define SCRATCH_FOP_INPUT_H

#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include "structs.h"
#include "globals.h"
#include "utils.h"

// ──────────────────────────────────────────────────────────────────────────────
// تمام C-blockها در workspace را پیدا کن (برای snap)
static void collect_all_c_blocks(std::vector<Block*>& blocks,
                                  std::vector<Block*>& result) {
    for (Block* b : blocks) {
        if (b->isCShaped) result.push_back(b);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// آیا نقطه (px,py) داخل ناحیه inner یک C-block است؟
static bool point_in_c_inner(int px, int py, Block* cb) {
    int innerX = cb->x + 16;
    int innerY = c_inner_y(cb);
    int innerW = cb->w - 16;
    return point_in_rect(px, py, innerX, innerY, innerW, cb->innerH);
}

static bool point_in_c_else(int px, int py, Block* cb) {
    if (!cb->hasElse) return false;
    int elseX = cb->x + 16;
    int elseY = c_else_y(cb) + 20; // +20 برای نوار "else"
    int elseW = cb->w - 16;
    return point_in_rect(px, py, elseX, elseY, elseW, cb->elseH);
}

// ──────────────────────────────────────────────────────────────────────────────
// حذف بلاک از هر C-block که ممکنه داخلش باشه
static void detach_from_c_blocks(Block* dragged, std::vector<Block*>& blocks) {
    for (Block* cb : blocks) {
        if (!cb->isCShaped) continue;

        // چک inner
        Block* prev2 = nullptr;
        Block* cur = cb->innerFirst;
        while (cur) {
            if (cur == dragged) {
                if (prev2) prev2->next = cur->next;
                else cb->innerFirst = cur->next;
                if (cur->next) cur->next->prev = prev2;
                if (cb->innerLast == cur) cb->innerLast = prev2;
                cur->prev = nullptr; cur->next = nullptr;
                goto done;
            }
            prev2 = cur; cur = cur->next;
        }

        // چک else
        if (cb->hasElse) {
            prev2 = nullptr; cur = cb->elseFirst;
            while (cur) {
                if (cur == dragged) {
                    if (prev2) prev2->next = cur->next;
                    else cb->elseFirst = cur->next;
                    if (cur->next) cur->next->prev = prev2;
                    cur->prev = nullptr; cur->next = nullptr;
                    goto done;
                }
                prev2 = cur; cur = cur->next;
            }
        }
    }
    done:;
}

// ──────────────────────────────────────────────────────────────────────────────
// snap بلاک به داخل C-block
static bool try_snap_into_c(Block* dragged, Block* cb) {
    if (!cb->isCShaped || cb == dragged) return false;
    const int SNAP_D = SNAP_DISTANCE + 8;
    int indent = 16;

    // ─ snap به inner ─
    {
        int dropX = cb->x + indent;
        int dropY = c_inner_y(cb) + 4;

        if (cb->innerFirst == nullptr) {
            // inner خالی: چک نزدیکی به ناحیه inner
            if (std::abs(dragged->x - dropX) < SNAP_D &&
                std::abs(dragged->y - dropY) < SNAP_D) {
                dragged->x = dropX;
                dragged->y = dropY;
                dragged->prev = nullptr;
                dragged->next = nullptr;
                cb->innerFirst = dragged;
                cb->innerLast  = dragged;
                return true;
            }
        } else {
            // snap به آخر inner
            Block* last = cb->innerFirst;
            while (last->next) last = last->next;
            int lastBottomY = last->y + block_total_height(last);
            if (std::abs(dragged->x - last->x) < SNAP_D &&
                std::abs(dragged->y - lastBottomY) < SNAP_D) {
                dragged->x    = last->x;
                dragged->y    = lastBottomY;
                dragged->prev = last;
                dragged->next = nullptr;
                last->next    = dragged;
                cb->innerLast = dragged;
                return true;
            }
        }
    }

    // ─ snap به else (اگر دارد) ─
    if (cb->hasElse) {
        int dropX = cb->x + indent;
        int dropY = c_else_y(cb) + 20 + 4; // +20 = نوار else

        if (cb->elseFirst == nullptr) {
            if (std::abs(dragged->x - dropX) < SNAP_D &&
                std::abs(dragged->y - dropY) < SNAP_D) {
                dragged->x = dropX;
                dragged->y = dropY;
                dragged->prev = nullptr;
                dragged->next = nullptr;
                cb->elseFirst = dragged;
                return true;
            }
        } else {
            Block* last = cb->elseFirst;
            while (last->next) last = last->next;
            int lastBottomY = last->y + block_total_height(last);
            if (std::abs(dragged->x - last->x) < SNAP_D &&
                std::abs(dragged->y - lastBottomY) < SNAP_D) {
                dragged->x    = last->x;
                dragged->y    = lastBottomY;
                dragged->prev = last;
                dragged->next = nullptr;
                last->next    = dragged;
                return true;
            }
        }
    }

    return false;
}

// ──────────────────────────────────────────────────────────────────────────────
void handle_snap(Block* dragged, std::vector<Block*>& blocks) {
    if (!dragged) return;

    // ابتدا از هر C-block جدا کن
    detach_from_c_blocks(dragged, blocks);
    if (dragged->prev) { dragged->prev->next = nullptr; dragged->prev = nullptr; }
    if (dragged->next) { dragged->next->prev = nullptr; dragged->next = nullptr; }

    // ── اول تلاش برای snap به داخل C-block ──
    for (Block* b : blocks) {
        if (b == dragged || b->isDragging || !b->isCShaped) continue;
        if (try_snap_into_c(dragged, b)) {
            layout_inner_blocks(b);
            return;
        }
    }

    // ── snap معمولی (بالا/پایین زنجیر) ──
    for (Block* b : blocks) {
        if (b == dragged || b->isDragging) continue;
        int dx = std::abs(dragged->x - b->x);

        // snap پایین b (با block_total_height برای C-block)
        int bBottom = b->y + block_total_height(b);
        int dy1 = std::abs(dragged->y - bBottom);
        if (dx < SNAP_DISTANCE && dy1 < SNAP_DISTANCE && b->next == nullptr) {
            dragged->x    = b->x;
            dragged->y    = bBottom;
            b->next       = dragged;
            dragged->prev = b;
            return;
        }

        // snap بالای b
        int dy2 = std::abs((dragged->y + block_total_height(dragged)) - b->y);
        if (dx < SNAP_DISTANCE && dy2 < SNAP_DISTANCE && b->prev == nullptr) {
            dragged->x    = b->x;
            dragged->y    = b->y - block_total_height(dragged);
            dragged->next = b;
            b->prev       = dragged;
            return;
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// کلیک روی C-block: هم هدر، هم ناحیه inner چک میشه
static Block* find_clicked_block(int mx, int my, std::vector<Block*>& blocks) {
    // از آخر به اول (بالاترین لایه اول)
    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        Block* b = blocks[i];

        // ناحیه header بلاک
        if (point_in_rect(mx, my, b->x, b->y, b->w, b->h))
            return b;

        // برای C-block، بلاک‌های داخل inner هم چک میشن
        if (b->isCShaped) {
            Block* inner = b->innerFirst;
            while (inner) {
                if (point_in_rect(mx, my, inner->x, inner->y, inner->w, inner->h))
                    return inner;
                // اگر inner خودش C-block باشه (nested)
                if (inner->isCShaped) {
                    Block* nested = inner->innerFirst;
                    while (nested) {
                        if (point_in_rect(mx, my, nested->x, nested->y, nested->w, nested->h))
                            return nested;
                        nested = nested->next;
                    }
                }
                inner = inner->next;
            }
            if (b->hasElse) {
                Block* el = b->elseFirst;
                while (el) {
                    if (point_in_rect(mx, my, el->x, el->y, el->w, el->h))
                        return el;
                    el = el->next;
                }
            }
        }
    }
    return nullptr;
}

// ──────────────────────────────────────────────────────────────────────────────
void handle_mouse_down(SDL_Event& e, std::vector<Block*>& blocks, Block** draggedBlock) {
    int mx = e.button.x, my = e.button.y;

    Block* clicked = find_clicked_block(mx, my, blocks);
    if (!clicked) return;

    // اگر بلاک داخل یک C-block بود، از C-block جداش کن
    detach_from_c_blocks(clicked, blocks);

    // از زنجیر اصلی جدا کن
    if (clicked->prev) { clicked->prev->next = nullptr; clicked->prev = nullptr; }
    if (clicked->next) { clicked->next->prev = nullptr; clicked->next = nullptr; }

    clicked->isDragging  = true;
    clicked->dragOffsetX = mx - clicked->x;
    clicked->dragOffsetY = my - clicked->y;
    *draggedBlock = clicked;

    // به انتهای لیست ببر (روی بقیه رندر بشه)
    auto it = std::find(blocks.begin(), blocks.end(), clicked);
    if (it != blocks.end()) {
        blocks.erase(it);
        blocks.push_back(clicked);
    }
}

// ──────────────────────────────────────────────────────────────────────────────
void handle_mouse_motion(SDL_Event& e, Block** draggedBlock, Workspace&) {
    if (!*draggedBlock) return;
    (*draggedBlock)->x = e.motion.x - (*draggedBlock)->dragOffsetX;
    (*draggedBlock)->y = e.motion.y - (*draggedBlock)->dragOffsetY;
}

// ──────────────────────────────────────────────────────────────────────────────
void handle_mouse_up(Block** draggedBlock, std::vector<Block*>& blocks,
                     Workspace& workspace) {
    if (!*draggedBlock) return;
    Block* b = *draggedBlock;
    b->isDragging = false;

    // اگر خارج از workspace رها شد، حذفش کن
    bool inWS = point_in_rect(b->x + b->w/2, b->y + b->h/2,
                               workspace.x, workspace.y,
                               workspace.w, workspace.h);
    if (!inWS) {
        detach_from_c_blocks(b, blocks);
        blocks.erase(std::remove(blocks.begin(), blocks.end(), b), blocks.end());
        if (b->prev) b->prev->next = nullptr;
        if (b->next) b->next->prev = nullptr;
        delete b;
        *draggedBlock = nullptr;
        return;
    }

    handle_snap(b, blocks);
    *draggedBlock = nullptr;
}

// ──────────────────────────────────────────────────────────────────────────────
// check_input_click: در workspace blocks + بلاک‌های داخل C-block هم چک کن
BlockInput* check_input_click(int mx, int my, std::vector<Block*>& blocks) {
    // لامبدا برای چک کردن یک بلاک
    auto checkBlock = [&](Block* b) -> BlockInput* {
        if (!b || b->isDragging) return nullptr;
        for (auto& inp : b->inputs) {
            if (inp.rect.w > 0 &&
                point_in_rect(mx, my, inp.rect.x, inp.rect.y,
                              inp.rect.w, inp.rect.h))
                return &inp;
        }
        return nullptr;
    };

    for (Block* b : blocks) {
        if (auto* r = checkBlock(b)) return r;
        if (b->isCShaped) {
            Block* inner = b->innerFirst;
            while (inner) {
                if (auto* r = checkBlock(inner)) return r;
                inner = inner->next;
            }
            if (b->hasElse) {
                Block* el = b->elseFirst;
                while (el) {
                    if (auto* r = checkBlock(el)) return r;
                    el = el->next;
                }
            }
        }
    }
    return nullptr;
}

#endif
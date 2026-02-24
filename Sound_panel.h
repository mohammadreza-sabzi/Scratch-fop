//
// Created by Domim on 2/24/2026.
//

#ifndef SCRATCH_FOP_SOUND_PANEL_H
#define SCRATCH_FOP_SOUND_PANEL_H
#ifndef SCRATCH_SOUND_PANEL_H
#define SCRATCH_SOUND_PANEL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "structs.h"
#include "globals.h"
#include "render.h"
#include "audio.h"

struct SoundPanelButtons {
    SDL_Rect playBtns[32];
    SDL_Rect stopBtns[32];
    SDL_Rect deleteBtns[32];
    SDL_Rect uploadBtn;
    SDL_Rect waveformArea[32];
    int      count = 0;
};

static SoundPanelButtons g_soundPanelBtns;

static void draw_waveform(SDL_Renderer* r, SDL_Rect area, SoundClip& clip,
                          bool isPlaying)
{
    SDL_SetRenderDrawColor(r, 240, 235, 255, 255);
    SDL_RenderFillRect(r, &area);
    SDL_SetRenderDrawColor(r, 200, 180, 230, 255);
    SDL_RenderDrawRect(r, &area);

    if (!clip.chunk || area.w < 4) return;

    Sint16* samples  = (Sint16*)clip.chunk->abuf;
    int     numSamps = (int)(clip.chunk->alen / sizeof(Sint16));
    if (numSamps <= 0) return;

    SDL_Color wCol = isPlaying
                     ? SDL_Color{120, 60, 200, 255}
                     : SDL_Color{160, 100, 220, 180};
    SDL_SetRenderDrawColor(r, wCol.r, wCol.g, wCol.b, wCol.a);

    int midY = area.y + area.h / 2;
    for (int px = 0; px < area.w; px++) {
        int idx = (int)((float)px / area.w * numSamps);
        if (idx >= numSamps) idx = numSamps - 1;
        float normalized = (float)samples[idx] / 32768.0f;
        int   amp        = (int)(normalized * (area.h / 2 - 2));
        SDL_RenderDrawLine(r, area.x + px, midY, area.x + px, midY - amp);
    }

    SDL_SetRenderDrawColor(r, 150, 100, 200, 120);
    SDL_RenderDrawLine(r, area.x, midY, area.x + area.w, midY);
}

static void draw_sounds_left_panel(SDL_Renderer* r, TTF_Font* font,
                                   TTF_Font* fontBig, SoundsPanel& panel)
{
    int px = 0, py = STAGE_Y;
    int pw = PALETTE_WIDTH, ph = SCREEN_HEIGHT - STAGE_Y;

    SDL_SetRenderDrawColor(r, 250, 240, 255, 255);
    SDL_Rect bg = {px, py, pw, ph};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r,
        COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
    SDL_Rect hdr = {px, py, pw, 34};
    SDL_RenderFillRect(r, &hdr);
    if (fontBig)
        draw_text_centered(r, fontBig, "Sounds", hdr, {255,255,255,255});

    SDL_Rect uploadBtn = {px + 8, py + 40, pw - 16, 28};
    SDL_SetRenderDrawColor(r,
        COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 200);
    SDL_RenderFillRect(r, &uploadBtn);
    SDL_SetRenderDrawColor(r, 150, 50, 160, 255);
    SDL_RenderDrawRect(r, &uploadBtn);
    if (font)
        draw_text_centered(r, font, "+ Upload Sound", uploadBtn, {255,255,255,255});
    g_soundPanelBtns.uploadBtn = uploadBtn;

    g_soundPanelBtns.count = 0;
    int itemH = 56;
    int startY = py + 76 + panel.scrollOffset;

    for (int i = 0; i < (int)panel.sounds.size(); i++) {
        SoundClip& sc = panel.sounds[i];
        int iy = startY + i * (itemH + 4);
        if (iy + itemH < py + 34 || iy > py + ph) continue;

        bool sel = (i == panel.selectedIndex);

        SDL_SetRenderDrawColor(r,
            sel ? 230 : 248, sel ? 215 : 240, sel ? 255 : 252, 255);
        SDL_Rect card = {px + 4, iy, pw - 8, itemH};
        SDL_RenderFillRect(r, &card);
        SDL_SetRenderDrawColor(r,
            sel ? 140 : 200, sel ? 80 : 170, sel ? 200 : 210, 255);
        SDL_RenderDrawRect(r, &card);

        if (font) {
            std::string lbl = std::to_string(i + 1) + ". " + sc.name;
            draw_text(r, font, lbl, px + 10, iy + 5, {80, 40, 100, 255});

            if (sc.durationSecs > 0.0f) {
                char dur[32];
                snprintf(dur, sizeof(dur), "%.2fs", sc.durationSecs);
                draw_text(r, font, dur, px + pw - 44, iy + 5, {140, 100, 160, 255});
            }
        }

        SDL_Rect playB = {px + 8, iy + 22, 28, 22};
        SDL_SetRenderDrawColor(r,
            sc.isPlaying ? 50 : 80, sc.isPlaying ? 160 : 180,
            sc.isPlaying ? 50 : 80, 255);
        SDL_RenderFillRect(r, &playB);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        for (int t = 0; t < 7; t++)
            SDL_RenderDrawLine(r, playB.x+8+t, playB.y+5+t,
                                  playB.x+8+t, playB.y+17-t);
        if (font && sc.isPlaying)
            draw_text(r, font, "▶", playB.x+6, playB.y+4, {255,255,255,255});

        SDL_Rect stopB = {px + 42, iy + 22, 28, 22};
        SDL_SetRenderDrawColor(r, 200, 50, 50, 255);
        SDL_RenderFillRect(r, &stopB);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_Rect sq = {stopB.x+7, stopB.y+6, stopB.w-14, stopB.h-12};
        SDL_RenderFillRect(r, &sq);

        SDL_Rect delB = {px + pw - 34, iy + 22, 26, 22};
        SDL_SetRenderDrawColor(r, 220, 80, 80, 200);
        SDL_RenderFillRect(r, &delB);
        if (font)
            draw_text_centered(r, font, "X", delB, {255,255,255,255});

        if (sc.isPlaying && font) {
            draw_text(r, font, "● playing", px + 76, iy + 26, {100, 180, 100, 255});
        }

        if (i < 32) {
            g_soundPanelBtns.playBtns[i]   = playB;
            g_soundPanelBtns.stopBtns[i]   = stopB;
            g_soundPanelBtns.deleteBtns[i] = delB;
            g_soundPanelBtns.count         = i + 1;
        }
    }

    if (panel.sounds.empty() && font) {
        draw_text(r, font, "No sounds yet",
                  px + 16, py + 80, {160, 120, 180, 255});
        draw_text(r, font, "Click '+' to upload",
                  px + 10, py + 100, {180, 140, 200, 255});
    }
}

static void draw_sounds_workspace(SDL_Renderer* r, TTF_Font* font,
                                  TTF_Font* fontBig, SoundsPanel& panel)
{
    int wx = PALETTE_WIDTH, wy = STAGE_Y;
    int ww = SCREEN_WIDTH - PALETTE_WIDTH - (STAGE_X - PALETTE_WIDTH);
    ww = STAGE_X - PALETTE_WIDTH;
    if (ww < 100) ww = WORKSPACE_W; // fallback

    wx = WORKSPACE_X; wy = STAGE_Y;
    ww = WORKSPACE_W; int wh = SCREEN_HEIGHT - STAGE_Y;

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_Rect bg = {wx, wy, ww, wh};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 215, 210, 225, 255);
    for (int x = wx + 12; x < wx + ww; x += 24)
        for (int y = wy + 12; y < wy + wh; y += 24)
            SDL_RenderDrawPoint(r, x, y);

    if (panel.selectedIndex < 0 ||
        panel.selectedIndex >= (int)panel.sounds.size()) {
        if (font)
            draw_text(r, font, "Select a sound to preview",
                      wx + 40, wy + 40, {180, 150, 200, 255});
        return;
    }

    SoundClip& sc = panel.sounds[panel.selectedIndex];

    if (fontBig) {
        SDL_Rect titleR = {wx + 20, wy + 16, ww - 40, 30};
        draw_text(r, fontBig, sc.name, wx + 20, wy + 16, {100, 50, 140, 255});
    }

    if (font && sc.durationSecs > 0) {
        char info[64];
        snprintf(info, sizeof(info), "Duration: %.2f sec", sc.durationSecs);
        draw_text(r, font, info, wx + 20, wy + 46, {150, 100, 170, 255});
        draw_text(r, font, sc.filePath, wx + 20, wy + 62, {170, 130, 190, 255});
    }

    SDL_Rect waveArea = {wx + 20, wy + 84, ww - 40, 120};
    draw_waveform(r, waveArea, sc, sc.isPlaying);

    SDL_Rect bigPlay = {wx + 20, wy + 220, 80, 36};
    SDL_Rect bigStop = {wx + 112, wy + 220, 80, 36};

    SDL_SetRenderDrawColor(r,
        sc.isPlaying ? 40 : 70, sc.isPlaying ? 150 : 170,
        sc.isPlaying ? 40 : 70, 255);
    SDL_RenderFillRect(r, &bigPlay);
    SDL_SetRenderDrawColor(r, 30, 120, 30, 255);
    SDL_RenderDrawRect(r, &bigPlay);
    if (font)
        draw_text_centered(r, font,
            sc.isPlaying ? "▶ Playing" : "▶ Play",
            bigPlay, {255,255,255,255});

    SDL_SetRenderDrawColor(r, 200, 50, 50, 255);
    SDL_RenderFillRect(r, &bigStop);
    SDL_SetRenderDrawColor(r, 160, 30, 30, 255);
    SDL_RenderDrawRect(r, &bigStop);
    if (font)
        draw_text_centered(r, font, "■ Stop", bigStop, {255,255,255,255});

    if (font) {
        char vol[32];
        snprintf(vol, sizeof(vol), "Volume: %.0f%%", sc.volume);
        draw_text(r, font, vol, wx + 20, wy + 270, {120, 80, 150, 255});

        SDL_Rect track = {wx + 20, wy + 290, ww - 40, 12};
        SDL_SetRenderDrawColor(r, 210, 190, 230, 255);
        SDL_RenderFillRect(r, &track);
        int fillW = (int)(track.w * sc.volume / 100.0f);
        SDL_SetRenderDrawColor(r,
            COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
        SDL_Rect fill = {track.x, track.y, fillW, track.h};
        SDL_RenderFillRect(r, &fill);
        SDL_SetRenderDrawColor(r, 150, 80, 170, 255);
        SDL_RenderDrawRect(r, &track);
    }
}

static void draw_upload_dialog(SDL_Renderer* r, TTF_Font* font,
                               TTF_Font* fontBig, SoundsPanel& panel)
{
    if (!panel.uploadDialogOpen) return;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 140);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(r, &overlay);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    int dw = 460, dh = 160;
    int dx = SCREEN_WIDTH/2  - dw/2;
    int dy = SCREEN_HEIGHT/2 - dh/2;

    SDL_SetRenderDrawColor(r, 0, 0, 0, 80);
    SDL_Rect shad = {dx+4, dy+4, dw, dh};
    SDL_RenderFillRect(r, &shad);

    SDL_SetRenderDrawColor(r, 252, 248, 255, 255);
    SDL_Rect box = {dx, dy, dw, dh};
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
    SDL_RenderDrawRect(r, &box);

    SDL_Rect hdr = {dx, dy, dw, 34};
    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
    SDL_RenderFillRect(r, &hdr);
    if (fontBig)
        draw_text_centered(r, fontBig, "Upload Sound File", hdr, {255,255,255,255});

    if (font)
        draw_text(r, font, "File path (.wav / .ogg / .mp3):",
                  dx + 14, dy + 44, {80, 40, 100, 255});

    SDL_Rect field = {dx + 14, dy + 62, dw - 28, 28};
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderFillRect(r, &field);
    SDL_Color bc = panel.uploadEditing
                   ? SDL_Color{120, 60, 200, 255}
                   : SDL_Color{180, 150, 200, 255};
    SDL_SetRenderDrawColor(r, bc.r, bc.g, bc.b, 255);
    SDL_RenderDrawRect(r, &field);
    if (font) {
        std::string disp = panel.uploadPathInput +
                           (panel.uploadEditing ? "|" : "");
        draw_text(r, font, disp, field.x + 6, field.y + 7, {60, 30, 90, 255});
    }

    SDL_Rect okBtn = {dx + dw - 100, dy + dh - 38, 86, 28};
    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
    SDL_RenderFillRect(r, &okBtn);
    SDL_SetRenderDrawColor(r, 150, 50, 170, 255);
    SDL_RenderDrawRect(r, &okBtn);
    if (font) draw_text_centered(r, font, "Upload", okBtn, {255,255,255,255});

    SDL_Rect cancelBtn = {dx + dw - 196, dy + dh - 38, 86, 28};
    SDL_SetRenderDrawColor(r, 180, 180, 190, 255);
    SDL_RenderFillRect(r, &cancelBtn);
    SDL_SetRenderDrawColor(r, 140, 140, 150, 255);
    SDL_RenderDrawRect(r, &cancelBtn);
    if (font) draw_text_centered(r, font, "Cancel", cancelBtn, {60, 60, 70, 255});

    if (font)
        draw_text(r, font, "Tip: use full path or relative path",
                  dx + 14, dy + dh - 18, {160, 120, 180, 255});
}

static bool handle_upload_dialog_click(int mx, int my, SoundsPanel& panel,
                                       TTF_Font* font)
{
    if (!panel.uploadDialogOpen) return false;

    int dw = 460, dh = 160;
    int dx = SCREEN_WIDTH/2  - dw/2;
    int dy = SCREEN_HEIGHT/2 - dh/2;

    SDL_Rect field = {dx + 14, dy + 62, dw - 28, 28};
    if (mx >= field.x && mx < field.x+field.w &&
        my >= field.y && my < field.y+field.h) {
        panel.uploadEditing = true;
        SDL_StartTextInput();
        return true;
    }

    SDL_Rect okBtn = {dx + dw - 100, dy + dh - 38, 86, 28};
    if (mx >= okBtn.x && mx < okBtn.x+okBtn.w &&
        my >= okBtn.y && my < okBtn.y+okBtn.h) {
        if (!panel.uploadPathInput.empty()) {
            SoundClip nc;
            std::string path = panel.uploadPathInput;
            size_t slash = path.find_last_of("/\\");
            std::string fname = (slash != std::string::npos)
                                ? path.substr(slash + 1) : path;
            size_t dot = fname.find_last_of('.');
            nc.name     = (dot != std::string::npos)
                          ? fname.substr(0, dot) : fname;
            nc.filePath = path;
            nc.volume   = 100.0f;
            if (audio_load(nc)) {
                panel.sounds.push_back(nc);
                panel.selectedIndex = (int)panel.sounds.size() - 1;
            } else {
                panel.sounds.push_back(nc);
                panel.selectedIndex = (int)panel.sounds.size() - 1;
            }
        }
        panel.uploadDialogOpen  = false;
        panel.uploadEditing     = false;
        panel.uploadPathInput   = "";
        SDL_StopTextInput();
        return true;
    }

    SDL_Rect cancelBtn = {dx + dw - 196, dy + dh - 38, 86, 28};
    if (mx >= cancelBtn.x && mx < cancelBtn.x+cancelBtn.w &&
        my >= cancelBtn.y && my < cancelBtn.y+cancelBtn.h) {
        panel.uploadDialogOpen = false;
        panel.uploadEditing    = false;
        panel.uploadPathInput  = "";
        SDL_StopTextInput();
        return true;
    }

    return true;
}

static bool handle_sounds_panel_click(int mx, int my, SoundsPanel& panel)
{
    SDL_Rect& ub = g_soundPanelBtns.uploadBtn;
    if (mx >= ub.x && mx < ub.x+ub.w && my >= ub.y && my < ub.y+ub.h) {
        panel.uploadDialogOpen = true;
        panel.uploadEditing    = true;
        panel.uploadPathInput  = "";
        SDL_StartTextInput();
        return true;
    }

    for (int i = 0; i < g_soundPanelBtns.count; i++) {
        // Play
        SDL_Rect& pb = g_soundPanelBtns.playBtns[i];
        if (mx >= pb.x && mx < pb.x+pb.w && my >= pb.y && my < pb.y+pb.h) {
            panel.selectedIndex = i;
            audio_play(panel.sounds[i]);
            if (panel.sounds[i].channel >= 0)
                Mix_Volume(panel.sounds[i].channel,
                           (int)(panel.sounds[i].volume / 100.0f * MIX_MAX_VOLUME));
            return true;
        }
        // Stop
        SDL_Rect& sb = g_soundPanelBtns.stopBtns[i];
        if (mx >= sb.x && mx < sb.x+sb.w && my >= sb.y && my < sb.y+sb.h) {
            audio_stop(panel.sounds[i]);
            return true;
        }
        // Delete
        SDL_Rect& db = g_soundPanelBtns.deleteBtns[i];
        if (mx >= db.x && mx < db.x+db.w && my >= db.y && my < db.y+db.h) {
            audio_stop(panel.sounds[i]);
            if (panel.sounds[i].chunk)
                Mix_FreeChunk(panel.sounds[i].chunk);
            panel.sounds.erase(panel.sounds.begin() + i);
            if (panel.selectedIndex >= (int)panel.sounds.size())
                panel.selectedIndex = (int)panel.sounds.size() - 1;
            return true;
        }
    }
    return false;
}

#endif // SCRATCH_SOUND_PANEL_H

#endif //SCRATCH_FOP_SOUND_PANEL_H
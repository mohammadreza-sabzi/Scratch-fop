

#ifndef SCRATCH_FOP_SOUND_PANEL_H
#define SCRATCH_FOP_SOUND_PANEL_H

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
    int      count = 0;

    SDL_Rect bigPlay;
    SDL_Rect bigStop;
    SDL_Rect volumeTrack;
    SDL_Rect pitchTrack;
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

static void draw_control_slider(SDL_Renderer* r, TTF_Font* font,
                                SDL_Rect track, float value, float minV, float maxV,
                                SDL_Color fillColor)
{
    float ratio = (value - minV) / (maxV - minV);
    if (ratio < 0) ratio = 0.f;
    if (ratio > 1) ratio = 1.f;
    int fillW = (int)(track.w * ratio);

    SDL_SetRenderDrawColor(r, 210, 190, 230, 255);
    SDL_RenderFillRect(r, &track);

    SDL_SetRenderDrawColor(r, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_Rect fill = {track.x, track.y, fillW, track.h};
    SDL_RenderFillRect(r, &fill);

    SDL_SetRenderDrawColor(r, 140, 90, 170, 255);
    SDL_RenderDrawRect(r, &track);

    SDL_SetRenderDrawColor(r, 80, 30, 150, 255);
    SDL_Rect handle = {track.x + fillW - 5, track.y - 4, 10, track.h + 8};
    SDL_RenderFillRect(r, &handle);
    SDL_SetRenderDrawColor(r, 240, 220, 255, 220);
    SDL_RenderDrawRect(r, &handle);
}

static void draw_sounds_left_panel(SDL_Renderer* r, TTF_Font* font,
                                   TTF_Font* fontBig, SoundsPanel& panel)
{
    int px = 0, py = STAGE_Y;
    int pw = PALETTE_WIDTH, ph = SCREEN_HEIGHT - STAGE_Y;

    SDL_SetRenderDrawColor(r, 250, 240, 255, 255);
    SDL_Rect bg = {px, py, pw, ph};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
    SDL_Rect hdr = {px, py, pw, 34};
    SDL_RenderFillRect(r, &hdr);
    if (fontBig)
        draw_text_centered(r, fontBig, "Sounds", hdr, {255,255,255,255});

    SDL_Rect uploadBtn = {px + 8, py + 40, pw - 16, 28};
    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 200);
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

        if (sc.isPlaying && font)
            draw_text(r, font, "● playing", px + 76, iy + 26, {100, 180, 100, 255});

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
    int wx = WORKSPACE_X, wy = STAGE_Y;
    int ww = WORKSPACE_W; int wh = SCREEN_HEIGHT - STAGE_Y;

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_Rect bg = {wx, wy, ww, wh};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 215, 210, 225, 255);
    for (int x = wx + 12; x < wx + ww; x += 24)
        for (int y = wy + 12; y < wy + wh; y += 24)
            SDL_RenderDrawPoint(r, x, y);

    g_soundPanelBtns.bigPlay     = {0,0,0,0};
    g_soundPanelBtns.bigStop     = {0,0,0,0};
    g_soundPanelBtns.volumeTrack = {0,0,0,0};
    g_soundPanelBtns.pitchTrack  = {0,0,0,0};

    if (panel.selectedIndex < 0 ||
        panel.selectedIndex >= (int)panel.sounds.size()) {
        if (font)
            draw_text(r, font, "Select a sound to preview",
                      wx + 40, wy + 40, {180, 150, 200, 255});
        return;
    }

    SoundClip& sc = panel.sounds[panel.selectedIndex];

    if (fontBig)
        draw_text(r, fontBig, sc.name, wx + 20, wy + 16, {100, 50, 140, 255});

    if (font && sc.durationSecs > 0) {
        char info[64];
        snprintf(info, sizeof(info), "Duration: %.2f sec", sc.durationSecs);
        draw_text(r, font, info, wx + 20, wy + 46, {150, 100, 170, 255});
        draw_text(r, font, sc.filePath, wx + 20, wy + 62, {170, 130, 190, 255});
    }

    SDL_Rect waveArea = {wx + 20, wy + 84, ww - 40, 96};
    draw_waveform(r, waveArea, sc, sc.isPlaying);

    SDL_Rect bigPlay = {wx + 20, wy + 196, 90, 34};
    SDL_Rect bigStop = {wx + 122, wy + 196, 90, 34};
    g_soundPanelBtns.bigPlay = bigPlay;
    g_soundPanelBtns.bigStop = bigStop;

    SDL_SetRenderDrawColor(r,
        sc.isPlaying ? 40 : 70, sc.isPlaying ? 150 : 170,
        sc.isPlaying ? 40 : 70, 255);
    SDL_RenderFillRect(r, &bigPlay);
    SDL_SetRenderDrawColor(r, 30, 120, 30, 255);
    SDL_RenderDrawRect(r, &bigPlay);
    if (font)
        draw_text_centered(r, font,
            sc.isPlaying ? "|| Pause" : "▶ Play", bigPlay, {255,255,255,255});

    SDL_SetRenderDrawColor(r, 200, 50, 50, 255);
    SDL_RenderFillRect(r, &bigStop);
    SDL_SetRenderDrawColor(r, 160, 30, 30, 255);
    SDL_RenderDrawRect(r, &bigStop);
    if (font)
        draw_text_centered(r, font, "■ Stop", bigStop, {255,255,255,255});

    SDL_Color labelCol = {100, 60, 140, 255};
    int sliderW = ww - 60;

    int volY = wy + 250;
    SDL_Rect volTrack = {wx + 20, volY, sliderW, 14};
    g_soundPanelBtns.volumeTrack = volTrack;

    if (font) {
        char vlbl[32];
        snprintf(vlbl, sizeof(vlbl), "Volume: %.0f%%", sc.volume);
        draw_text(r, font, vlbl, wx + 20, volY - 16, labelCol);
    }
    draw_control_slider(r, font, volTrack,
                        sc.volume, 0.f, 100.f,
                        SDL_Color{COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255});

    if (font) {
        char pct[16];
        snprintf(pct, sizeof(pct), "%.0f%%", sc.volume);
        draw_text(r, font, pct, wx + 20 + sliderW + 6, volY, labelCol);
    }

    int pitchY = wy + 310;
    SDL_Rect pitchTrack = {wx + 20, pitchY, sliderW, 14};
    g_soundPanelBtns.pitchTrack = pitchTrack;

    float pitchPct = sc.pitch * 100.0f;
    if (font) {
        char plbl[48];
        snprintf(plbl, sizeof(plbl), "Pitch: %.0f%%", pitchPct);
        draw_text(r, font, plbl, wx + 20, pitchY - 16, labelCol);
    }
    draw_control_slider(r, font, pitchTrack,
                        sc.pitch, 0.5f, 2.0f,
                        SDL_Color{80, 150, 210, 255});

    if (font) {
        char ppct[16];
        snprintf(ppct, sizeof(ppct), "%.0f%%", pitchPct);
        draw_text(r, font, ppct, wx + 20 + sliderW + 6, pitchY, labelCol);
    }


    {
        int normX = pitchTrack.x + (int)(pitchTrack.w * (1.0f - 0.5f) / (2.0f - 0.5f));
        SDL_SetRenderDrawColor(r, 60, 120, 180, 180);
        SDL_RenderDrawLine(r, normX, pitchTrack.y - 6, normX, pitchTrack.y + pitchTrack.h + 6);
        if (font)
            draw_text(r, font, "100%", normX - 14, pitchTrack.y + pitchTrack.h + 3,
                      {80, 130, 190, 200});
    }

    if (font)
        draw_text(r, font, "Pitch applies on next Play",
                  wx + 20, pitchY + 22, {160, 130, 180, 200});
}

static bool handle_sounds_workspace_click(int mx, int my, SoundsPanel& panel)
{
    if (panel.selectedIndex < 0 ||
        panel.selectedIndex >= (int)panel.sounds.size())
        return false;

    SoundClip& sc = panel.sounds[panel.selectedIndex];

    SDL_Rect& vt = g_soundPanelBtns.volumeTrack;
    if (vt.w > 0) {
        SDL_Rect hit = {vt.x, vt.y - 10, vt.w, vt.h + 20};
        if (mx >= hit.x && mx < hit.x+hit.w && my >= hit.y && my < hit.y+hit.h) {
            float ratio = (float)(mx - vt.x) / vt.w;
            if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
            sc.volume = ratio * 100.0f;
            if (sc.channel >= 0 && sc.isPlaying)
                Mix_Volume(sc.channel, (int)(ratio * MIX_MAX_VOLUME));
            return true;
        }
    }

    SDL_Rect& pt = g_soundPanelBtns.pitchTrack;
    if (pt.w > 0) {
        SDL_Rect hit = {pt.x, pt.y - 10, pt.w, pt.h + 20};
        if (mx >= hit.x && mx < hit.x+hit.w && my >= hit.y && my < hit.y+hit.h) {
            float ratio = (float)(mx - pt.x) / pt.w;
            if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
            sc.pitch = 0.5f + ratio * (2.0f - 0.5f);
            return true;
        }
    }

    SDL_Rect& bp = g_soundPanelBtns.bigPlay;
    if (bp.w > 0 && mx >= bp.x && mx < bp.x+bp.w && my >= bp.y && my < bp.y+bp.h) {
        if (sc.isPlaying)
            audio_stop(sc);
        else {
            audio_play(sc);
            if (sc.channel >= 0)
                Mix_Volume(sc.channel, (int)(sc.volume / 100.0f * MIX_MAX_VOLUME));
        }
        return true;
    }

    SDL_Rect& bs = g_soundPanelBtns.bigStop;
    if (bs.w > 0 && mx >= bs.x && mx < bs.x+bs.w && my >= bs.y && my < bs.y+bs.h) {
        audio_stop(sc);
        return true;
    }

    return false;
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
    if (font) draw_text_centered(r, font, "Upload", okBtn, {255,255,255,255});

    SDL_Rect cancelBtn = {dx + dw - 196, dy + dh - 38, 86, 28};
    SDL_SetRenderDrawColor(r, 180, 180, 190, 255);
    SDL_RenderFillRect(r, &cancelBtn);
    if (font) draw_text_centered(r, font, "Cancel", cancelBtn, {60, 60, 70, 255});

    if (font)
        draw_text(r, font, "Tip: use full path or relative path",
                  dx + 14, dy + dh - 18, {160, 120, 180, 255});
}

static bool handle_upload_dialog_click(int mx, int my, SoundsPanel& panel,
                                       TTF_Font*)
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
            nc.name     = (dot != std::string::npos) ? fname.substr(0, dot) : fname;
            nc.filePath = path;
            nc.volume   = 100.0f;
            nc.pitch    = 1.0f;
            audio_load(nc);
            panel.sounds.push_back(nc);
            panel.selectedIndex = (int)panel.sounds.size() - 1;
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
        SDL_Rect& pb = g_soundPanelBtns.playBtns[i];
        if (mx >= pb.x && mx < pb.x+pb.w && my >= pb.y && my < pb.y+pb.h) {
            panel.selectedIndex = i;
            audio_play(panel.sounds[i]);
            if (panel.sounds[i].channel >= 0)
                Mix_Volume(panel.sounds[i].channel,
                           (int)(panel.sounds[i].volume / 100.0f * MIX_MAX_VOLUME));
            return true;
        }
        SDL_Rect& sb = g_soundPanelBtns.stopBtns[i];
        if (mx >= sb.x && mx < sb.x+sb.w && my >= sb.y && my < sb.y+sb.h) {
            audio_stop(panel.sounds[i]);
            return true;
        }
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

#endif

#ifndef SCRATCH_FOP_SOUND_PANEL_H
#define SCRATCH_FOP_SOUND_PANEL_H

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
    int      count = 0;

    SDL_Rect bigPlay;
    SDL_Rect bigStop;
    SDL_Rect volumeTrack;
    SDL_Rect pitchTrack;
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

static void draw_control_slider(SDL_Renderer* r, TTF_Font* font,
                                SDL_Rect track, float value, float minV, float maxV,
                                SDL_Color fillColor)
{
    float ratio = (value - minV) / (maxV - minV);
    if (ratio < 0) ratio = 0.f;
    if (ratio > 1) ratio = 1.f;
    int fillW = (int)(track.w * ratio);

    SDL_SetRenderDrawColor(r, 210, 190, 230, 255);
    SDL_RenderFillRect(r, &track);

    SDL_SetRenderDrawColor(r, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_Rect fill = {track.x, track.y, fillW, track.h};
    SDL_RenderFillRect(r, &fill);

    SDL_SetRenderDrawColor(r, 140, 90, 170, 255);
    SDL_RenderDrawRect(r, &track);

    SDL_SetRenderDrawColor(r, 80, 30, 150, 255);
    SDL_Rect handle = {track.x + fillW - 5, track.y - 4, 10, track.h + 8};
    SDL_RenderFillRect(r, &handle);
    SDL_SetRenderDrawColor(r, 240, 220, 255, 220);
    SDL_RenderDrawRect(r, &handle);
}

static void draw_sounds_left_panel(SDL_Renderer* r, TTF_Font* font,
                                   TTF_Font* fontBig, SoundsPanel& panel)
{
    int px = 0, py = STAGE_Y;
    int pw = PALETTE_WIDTH, ph = SCREEN_HEIGHT - STAGE_Y;

    SDL_SetRenderDrawColor(r, 250, 240, 255, 255);
    SDL_Rect bg = {px, py, pw, ph};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255);
    SDL_Rect hdr = {px, py, pw, 34};
    SDL_RenderFillRect(r, &hdr);
    if (fontBig)
        draw_text_centered(r, fontBig, "Sounds", hdr, {255,255,255,255});

    SDL_Rect uploadBtn = {px + 8, py + 40, pw - 16, 28};
    SDL_SetRenderDrawColor(r, COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 200);
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

        if (sc.isPlaying && font)
            draw_text(r, font, "● playing", px + 76, iy + 26, {100, 180, 100, 255});

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
    int wx = WORKSPACE_X, wy = STAGE_Y;
    int ww = WORKSPACE_W; int wh = SCREEN_HEIGHT - STAGE_Y;

    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_Rect bg = {wx, wy, ww, wh};
    SDL_RenderFillRect(r, &bg);

    SDL_SetRenderDrawColor(r, 215, 210, 225, 255);
    for (int x = wx + 12; x < wx + ww; x += 24)
        for (int y = wy + 12; y < wy + wh; y += 24)
            SDL_RenderDrawPoint(r, x, y);

    g_soundPanelBtns.bigPlay     = {0,0,0,0};
    g_soundPanelBtns.bigStop     = {0,0,0,0};
    g_soundPanelBtns.volumeTrack = {0,0,0,0};
    g_soundPanelBtns.pitchTrack  = {0,0,0,0};

    if (panel.selectedIndex < 0 ||
        panel.selectedIndex >= (int)panel.sounds.size()) {
        if (font)
            draw_text(r, font, "Select a sound to preview",
                      wx + 40, wy + 40, {180, 150, 200, 255});
        return;
    }

    SoundClip& sc = panel.sounds[panel.selectedIndex];

    if (fontBig)
        draw_text(r, fontBig, sc.name, wx + 20, wy + 16, {100, 50, 140, 255});

    if (font && sc.durationSecs > 0) {
        char info[64];
        snprintf(info, sizeof(info), "Duration: %.2f sec", sc.durationSecs);
        draw_text(r, font, info, wx + 20, wy + 46, {150, 100, 170, 255});
        draw_text(r, font, sc.filePath, wx + 20, wy + 62, {170, 130, 190, 255});
    }

    SDL_Rect waveArea = {wx + 20, wy + 84, ww - 40, 96};
    draw_waveform(r, waveArea, sc, sc.isPlaying);

    SDL_Rect bigPlay = {wx + 20, wy + 196, 90, 34};
    SDL_Rect bigStop = {wx + 122, wy + 196, 90, 34};
    g_soundPanelBtns.bigPlay = bigPlay;
    g_soundPanelBtns.bigStop = bigStop;

    SDL_SetRenderDrawColor(r,
        sc.isPlaying ? 40 : 70, sc.isPlaying ? 150 : 170,
        sc.isPlaying ? 40 : 70, 255);
    SDL_RenderFillRect(r, &bigPlay);
    SDL_SetRenderDrawColor(r, 30, 120, 30, 255);
    SDL_RenderDrawRect(r, &bigPlay);
    if (font)
        draw_text_centered(r, font,
            sc.isPlaying ? "|| Pause" : "▶ Play", bigPlay, {255,255,255,255});

    SDL_SetRenderDrawColor(r, 200, 50, 50, 255);
    SDL_RenderFillRect(r, &bigStop);
    SDL_SetRenderDrawColor(r, 160, 30, 30, 255);
    SDL_RenderDrawRect(r, &bigStop);
    if (font)
        draw_text_centered(r, font, "■ Stop", bigStop, {255,255,255,255});

    SDL_Color labelCol = {100, 60, 140, 255};
    int sliderW = ww - 60;

    int volY = wy + 250;
    SDL_Rect volTrack = {wx + 20, volY, sliderW, 14};
    g_soundPanelBtns.volumeTrack = volTrack;

    if (font) {
        char vlbl[32];
        snprintf(vlbl, sizeof(vlbl), "Volume: %.0f%%", sc.volume);
        draw_text(r, font, vlbl, wx + 20, volY - 16, labelCol);
    }
    draw_control_slider(r, font, volTrack,
                        sc.volume, 0.f, 100.f,
                        SDL_Color{COLOR_SOUND.r, COLOR_SOUND.g, COLOR_SOUND.b, 255});

    if (font) {
        char pct[16];
        snprintf(pct, sizeof(pct), "%.0f%%", sc.volume);
        draw_text(r, font, pct, wx + 20 + sliderW + 6, volY, labelCol);
    }

    int pitchY = wy + 310;
    SDL_Rect pitchTrack = {wx + 20, pitchY, sliderW, 14};
    g_soundPanelBtns.pitchTrack = pitchTrack;

    float pitchPct = sc.pitch * 100.0f;
    if (font) {
        char plbl[48];
        snprintf(plbl, sizeof(plbl), "Pitch: %.0f%%", pitchPct);
        draw_text(r, font, plbl, wx + 20, pitchY - 16, labelCol);
    }
    draw_control_slider(r, font, pitchTrack,
                        sc.pitch, 0.5f, 2.0f,
                        SDL_Color{80, 150, 210, 255});

    if (font) {
        char ppct[16];
        snprintf(ppct, sizeof(ppct), "%.0f%%", pitchPct);
        draw_text(r, font, ppct, wx + 20 + sliderW + 6, pitchY, labelCol);
    }


    {
        int normX = pitchTrack.x + (int)(pitchTrack.w * (1.0f - 0.5f) / (2.0f - 0.5f));
        SDL_SetRenderDrawColor(r, 60, 120, 180, 180);
        SDL_RenderDrawLine(r, normX, pitchTrack.y - 6, normX, pitchTrack.y + pitchTrack.h + 6);
        if (font)
            draw_text(r, font, "100%", normX - 14, pitchTrack.y + pitchTrack.h + 3,
                      {80, 130, 190, 200});
    }

    if (font)
        draw_text(r, font, "Pitch applies on next Play",
                  wx + 20, pitchY + 22, {160, 130, 180, 200});
}

static bool handle_sounds_workspace_click(int mx, int my, SoundsPanel& panel)
{
    if (panel.selectedIndex < 0 ||
        panel.selectedIndex >= (int)panel.sounds.size())
        return false;

    SoundClip& sc = panel.sounds[panel.selectedIndex];

    SDL_Rect& vt = g_soundPanelBtns.volumeTrack;
    if (vt.w > 0) {
        SDL_Rect hit = {vt.x, vt.y - 10, vt.w, vt.h + 20};
        if (mx >= hit.x && mx < hit.x+hit.w && my >= hit.y && my < hit.y+hit.h) {
            float ratio = (float)(mx - vt.x) / vt.w;
            if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
            sc.volume = ratio * 100.0f;
            if (sc.channel >= 0 && sc.isPlaying)
                Mix_Volume(sc.channel, (int)(ratio * MIX_MAX_VOLUME));
            return true;
        }
    }

    SDL_Rect& pt = g_soundPanelBtns.pitchTrack;
    if (pt.w > 0) {
        SDL_Rect hit = {pt.x, pt.y - 10, pt.w, pt.h + 20};
        if (mx >= hit.x && mx < hit.x+hit.w && my >= hit.y && my < hit.y+hit.h) {
            float ratio = (float)(mx - pt.x) / pt.w;
            if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
            sc.pitch = 0.5f + ratio * (2.0f - 0.5f);
            return true;
        }
    }

    SDL_Rect& bp = g_soundPanelBtns.bigPlay;
    if (bp.w > 0 && mx >= bp.x && mx < bp.x+bp.w && my >= bp.y && my < bp.y+bp.h) {
        if (sc.isPlaying)
            audio_stop(sc);
        else {
            audio_play(sc);
            if (sc.channel >= 0)
                Mix_Volume(sc.channel, (int)(sc.volume / 100.0f * MIX_MAX_VOLUME));
        }
        return true;
    }

    SDL_Rect& bs = g_soundPanelBtns.bigStop;
    if (bs.w > 0 && mx >= bs.x && mx < bs.x+bs.w && my >= bs.y && my < bs.y+bs.h) {
        audio_stop(sc);
        return true;
    }

    return false;
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
    if (font) draw_text_centered(r, font, "Upload", okBtn, {255,255,255,255});

    SDL_Rect cancelBtn = {dx + dw - 196, dy + dh - 38, 86, 28};
    SDL_SetRenderDrawColor(r, 180, 180, 190, 255);
    SDL_RenderFillRect(r, &cancelBtn);
    if (font) draw_text_centered(r, font, "Cancel", cancelBtn, {60, 60, 70, 255});

    if (font)
        draw_text(r, font, "Tip: use full path or relative path",
                  dx + 14, dy + dh - 18, {160, 120, 180, 255});
}

static bool handle_upload_dialog_click(int mx, int my, SoundsPanel& panel,
                                       TTF_Font*)
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
            nc.name     = (dot != std::string::npos) ? fname.substr(0, dot) : fname;
            nc.filePath = path;
            nc.volume   = 100.0f;
            nc.pitch    = 1.0f;
            audio_load(nc);
            panel.sounds.push_back(nc);
            panel.selectedIndex = (int)panel.sounds.size() - 1;
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
        SDL_Rect& pb = g_soundPanelBtns.playBtns[i];
        if (mx >= pb.x && mx < pb.x+pb.w && my >= pb.y && my < pb.y+pb.h) {
            panel.selectedIndex = i;
            audio_play(panel.sounds[i]);
            if (panel.sounds[i].channel >= 0)
                Mix_Volume(panel.sounds[i].channel,
                           (int)(panel.sounds[i].volume / 100.0f * MIX_MAX_VOLUME));
            return true;
        }
        SDL_Rect& sb = g_soundPanelBtns.stopBtns[i];
        if (mx >= sb.x && mx < sb.x+sb.w && my >= sb.y && my < sb.y+sb.h) {
            audio_stop(panel.sounds[i]);
            return true;
        }
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

#endif

//
// Created by Domim on 2/22/2026.
//

#ifndef SCRATCH_FOP_AUDIO_H
#define SCRATCH_FOP_AUDIO_H
#include <SDL2/SDL_mixer.h>
#include <string>
#include <iostream>
#include "structs.h"

inline bool audio_init()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "[Audio] SDL_Init AUDIO failed: " << SDL_GetError() << "\n";
        return false;
    }

    int flags = MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC;
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags)
        std::cerr << "[Audio] Mix_Init warning: " << Mix_GetError() << "\n";


    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "[Audio] Mix_OpenAudio failed: " << Mix_GetError() << "\n";
        return false;
    }

    Mix_AllocateChannels(32);
    std::cout << "[Audio] SDL_mixer initialized OK\n";
    return true;
}

inline void audio_quit()
{
    Mix_CloseAudio();
    Mix_Quit();
}

inline bool audio_load(SoundClip& clip)
{
    if (clip.filePath.empty()) return false;
    if (clip.chunk) {
        Mix_FreeChunk(clip.chunk);
        clip.chunk = nullptr;
    }

    clip.chunk = Mix_LoadWAV(clip.filePath.c_str());
    if (!clip.chunk) {
        std::cerr << "[Audio] Cannot load '" << clip.filePath
                  << "': " << Mix_GetError() << "\n";
        return false;
    }

    clip.durationSecs = (float)clip.chunk->alen / (44100.0f * 2.0f * 2.0f);
    return true;
}

inline void audio_play(SoundClip& clip)
{
    if (!clip.chunk) {
        std::cerr << "[Audio] No chunk loaded for '" << clip.name << "'\n";
        return;
    }

    if (clip.isPlaying && clip.channel >= 0)
        Mix_HaltChannel(clip.channel);

    clip.channel   = Mix_PlayChannel(-1, clip.chunk, 0); // -1 = اتوماتیک
    clip.isPlaying = (clip.channel >= 0);

    if (clip.channel < 0)
        std::cerr << "[Audio] Mix_PlayChannel failed: " << Mix_GetError() << "\n";
}

inline void audio_stop(SoundClip& clip)
{
    if (clip.channel >= 0)
        Mix_HaltChannel(clip.channel);
    clip.isPlaying = false;
    clip.channel   = -1;
}

inline void audio_update(SoundsPanel& panel)
{
    for (auto& s : panel.sounds) {
        if (s.isPlaying && s.channel >= 0) {
            // وقتی SDL_mixer پخش تموم کرد، کانال آزاد می‌شه
            if (!Mix_Playing(s.channel)) {
                s.isPlaying = false;
                s.channel   = -1;
            }
        }
    }
}

inline void audio_free_all(SoundsPanel& panel)
{
    for (auto& s : panel.sounds) {
        if (s.chunk) {
            Mix_FreeChunk(s.chunk);
            s.chunk   = nullptr;
            s.channel = -1;
        }
    }
}

#endif //SCRATCH_FOP_AUDIO_H
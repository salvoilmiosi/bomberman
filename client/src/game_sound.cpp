#include "game_sound.h"

#include "resources.h"

#include <map>

Mix_Music *music_battle = nullptr;

static std::map<wave_id, Mix_Chunk*> wave_map;

void loadSounds() {
    music_battle = Mix_LoadMUS_RW(getResourceRW("IDM_BATTLE"), 1);

    wave_map[WAV_PLANT]    = Mix_LoadWAV_RW(getResourceRW("IDW_PLANT"), 1);
    wave_map[WAV_EXPLODE]  = Mix_LoadWAV_RW(getResourceRW("IDW_EXPLODE"), 1);
    wave_map[WAV_PICKUP]   = Mix_LoadWAV_RW(getResourceRW("IDW_PICKUP"),  1);
    wave_map[WAV_SKULL]    = Mix_LoadWAV_RW(getResourceRW("IDW_SKULL"),  1);
    wave_map[WAV_DEATH]    = Mix_LoadWAV_RW(getResourceRW("IDW_DEATH"),  1);
    wave_map[WAV_PUNCH]    = Mix_LoadWAV_RW(getResourceRW("IDW_PUNCH"),  1);
    wave_map[WAV_SLIDE]    = Mix_LoadWAV_RW(getResourceRW("IDW_SLIDE"),  1);
    wave_map[WAV_HARDHIT]  = Mix_LoadWAV_RW(getResourceRW("IDW_HARDHIT"),  1);
    wave_map[WAV_BOUNCE]   = Mix_LoadWAV_RW(getResourceRW("IDW_BOUNCE"),  1);
    wave_map[WAV_JUMP]     = Mix_LoadWAV_RW(getResourceRW("IDW_JUMP"), 1);
    wave_map[WAV_WALK]     = Mix_LoadWAV_RW(getResourceRW("IDW_WALK"), 1);
    wave_map[WAV_SELECT]   = Mix_LoadWAV_RW(getResourceRW("IDW_SELECT"), 1);
}

void clearSounds() {
    Mix_FreeMusic(music_battle);

    for (auto it : wave_map) {
        Mix_FreeChunk(it.second);
    }
}

void playMusic(Mix_Music *music) {
    Mix_PlayMusic(music, -1);
}

void stopMusic() {
    Mix_HaltMusic();
}

void playWave(wave_id id, int channel) {
    auto it = wave_map.find(id);
    if (it == wave_map.end()) return;

    Mix_PlayChannel(-1, it->second, 0);
}

void setVolume(int volume) {
    Mix_Volume(-1, volume);
}

void setMusicVolume(int volume) {
    Mix_VolumeMusic(volume);
}

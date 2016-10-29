#include "game_sound.h"

#include "resources.h"

#include <map>

Mix_Music *music_battle = nullptr;

static std::map<wave_id, Mix_Chunk*> wave_map;

void loadSounds() {
    music_battle = Mix_LoadMUS_RW(getResourceRW("IDM_BATTLE"), 1);

    static const std::map<wave_id, const char *> wav_resource_map = {
        {WAV_PLANT,     "IDW_PLANT"},
        {WAV_EXPLODE,   "IDW_EXPLODE"},
        {WAV_PICKUP,    "IDW_PICKUP"},
        {WAV_SKULL,     "IDW_SKULL"},
        {WAV_DEATH,     "IDW_DEATH"},
        {WAV_PUNCH,     "IDW_PUNCH"},
        {WAV_SLIDE,     "IDW_SLIDE"},
        {WAV_HARDHIT,   "IDW_HARDHIT"},
        {WAV_BOUNCE,    "IDW_BOUNCE"},
        {WAV_JUMP,      "IDW_JUMP"},
        {WAV_WALK,      "IDW_WALK"},
        {WAV_SELECT,    "IDW_SELECT"}
    };

    for (auto it : wav_resource_map) {
        wave_map[it.first] = Mix_LoadWAV_RW(getResourceRW(it.second), 1);
    }
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
    try {
        Mix_PlayChannel(-1, wave_map.at(id), 0);
    } catch (std::out_of_range) {}
}

void setVolume(int volume) {
    Mix_Volume(-1, volume);
}

void setMusicVolume(int volume) {
    Mix_VolumeMusic(volume);
}

#include "game_sound.h"

#include "resources.h"

Mix_Music *music_battle = nullptr;
Mix_Music *music_level1 = nullptr;

Mix_Chunk *wav_plant = nullptr;
Mix_Chunk *wav_explode = nullptr;
Mix_Chunk *wav_pickup = nullptr;
Mix_Chunk *wav_skull = nullptr;
Mix_Chunk *wav_death = nullptr;
Mix_Chunk *wav_punch = nullptr;
Mix_Chunk *wav_slide = nullptr;
Mix_Chunk *wav_hardhit = nullptr;
Mix_Chunk *wav_bounce = nullptr;
Mix_Chunk *wav_jump = nullptr;

void loadSounds() {
    music_battle = Mix_LoadMUS_RW(getResourceRW(IDM_BATTLE, "OGG"), 1);
    //music_level1 = Mix_LoadMUS_RW(getResourceRW(IDM_LEVEL1, "OGG"), 1);

    wav_plant = Mix_LoadWAV_RW(getResourceRW(IDW_PLANT, "WAV"), 1);
    wav_explode = Mix_LoadWAV_RW(getResourceRW(IDW_EXPLODE, "WAV"), 1);
    wav_pickup = Mix_LoadWAV_RW(getResourceRW(IDW_PICKUP, "WAV"), 1);
    wav_skull = Mix_LoadWAV_RW(getResourceRW(IDW_SKULL, "WAV"), 1);
    wav_death = Mix_LoadWAV_RW(getResourceRW(IDW_DEATH, "WAV"), 1);
    wav_punch = Mix_LoadWAV_RW(getResourceRW(IDW_PUNCH, "WAV"), 1);
    wav_slide = Mix_LoadWAV_RW(getResourceRW(IDW_SLIDE, "WAV"), 1);
    wav_hardhit = Mix_LoadWAV_RW(getResourceRW(IDW_HARDHIT, "WAV"), 1);
    wav_bounce = Mix_LoadWAV_RW(getResourceRW(IDW_BOUNCE, "WAV"), 1);
    wav_jump = Mix_LoadWAV_RW(getResourceRW(IDW_JUMP, "WAV"), 1);
}

void clearSounds() {
    Mix_FreeMusic(music_battle);
    //Mix_FreeMusic(music_level1);
    Mix_FreeChunk(wav_plant);
    Mix_FreeChunk(wav_explode);
    Mix_FreeChunk(wav_pickup);
    Mix_FreeChunk(wav_skull);
    Mix_FreeChunk(wav_death);
    Mix_FreeChunk(wav_punch);
    Mix_FreeChunk(wav_slide);
    Mix_FreeChunk(wav_hardhit);
    Mix_FreeChunk(wav_bounce);
    Mix_FreeChunk(wav_jump);
}

void playMusic(Mix_Music *music) {
    Mix_PlayMusic(music, -1);
}

void stopMusic() {
    Mix_HaltMusic();
}

void playWave(Mix_Chunk *wave) {
    Mix_PlayChannel(-1, wave, 0);
}

void playWaveById(uint8_t sound_id) {
    switch(sound_id) {
    case WAV_PLANT:
        playWave(wav_plant);
        break;
    case WAV_EXPLODE:
        playWave(wav_explode);
        break;
    case WAV_PICKUP:
        playWave(wav_pickup);
        break;
    case WAV_SKULL:
        playWave(wav_skull);
        break;
    case WAV_DEATH:
        playWave(wav_death);
        break;
    case WAV_PUNCH:
        playWave(wav_punch);
        break;
    case WAV_SLIDE:
        playWave(wav_slide);
        break;
    case WAV_HARDHIT:
        playWave(wav_hardhit);
        break;
    case WAV_BOUNCE:
        playWave(wav_bounce);
        break;
    case WAV_JUMP:
        playWave(wav_jump);
        break;
    default:
        break;
    }
}

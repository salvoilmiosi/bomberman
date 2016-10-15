#ifndef __GAME_SOUND_H__
#define __GAME_SOUND_H__

#include <SDL2/SDL_mixer.h>

extern Mix_Music *music_battle;

extern Mix_Chunk *wav_plant;
extern Mix_Chunk *wav_explode;
extern Mix_Chunk *wav_pickup;
extern Mix_Chunk *wav_skull;
extern Mix_Chunk *wav_death;
extern Mix_Chunk *wav_slide;
extern Mix_Chunk *wav_hardhit;
extern Mix_Chunk *wav_bounce;
extern Mix_Chunk *wav_jump;

extern Mix_Chunk *wav_select;

static const uint8_t WAV_PLANT = 1;
static const uint8_t WAV_EXPLODE = 2;
static const uint8_t WAV_PICKUP = 3;
static const uint8_t WAV_SKULL = 4;
static const uint8_t WAV_DEATH = 5;
static const uint8_t WAV_PUNCH = 6;
static const uint8_t WAV_SLIDE = 7;
static const uint8_t WAV_HARDHIT = 8;
static const uint8_t WAV_BOUNCE = 9;
static const uint8_t WAV_JUMP = 10;

void loadSounds();

void clearSounds();

void playMusic(Mix_Music *music);
void stopMusic();

void playWave(Mix_Chunk *wave, int channel = -1);
void playWaveById(uint8_t sound_id);

void setMusicVolume(int volume);

#endif // __GAME_SOUND_H__

#ifndef __GAME_SOUND_H__
#define __GAME_SOUND_H__

#include <SDL2/SDL_mixer.h>

extern Mix_Music *music_battle;

enum wave_id {
    WAVE_NONE,
    WAV_PLANT,
    WAV_EXPLODE,
    WAV_PICKUP,
    WAV_SKULL,
    WAV_DEATH,
    WAV_PUNCH,
    WAV_SLIDE,
    WAV_HARDHIT,
    WAV_BOUNCE,
    WAV_JUMP,
    WAV_WALK,
    WAV_SELECT
};

void loadSounds();

void clearSounds();

void playMusic(Mix_Music *music);
void stopMusic();

void playWave(wave_id id, int channel = -1);

void setVolume(int volume);
void setMusicVolume(int volume);

#endif // __GAME_SOUND_H__

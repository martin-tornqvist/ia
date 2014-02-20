#ifndef AUDIO_H
#define AUDIO_H

#include <SDL_mixer.h>

#include "CommonTypes.h"
#include "AudioIds.h"

class Engine;

namespace Audio {

void init();
void cleanup();

//Returns channel playing on
int play(const SfxId sfx, const int VOL_PERCENT_TOT = 100,
         const int VOL_PERCENT_L = 50);

void play(const SfxId sfx, const Dir dir, const int DISTANCE_PERCENT);

void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY, Engine& eng);

void fadeOutChannel(const int CHANNEL_NR);

}

#endif

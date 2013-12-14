#ifndef AUDIO_H
#define AUDIO_H

#include "SDL/SDL_mixer.h"

#include "CommonTypes.h"
#include "AudioIds.h"

class Engine;

using namespace std;

class Audio {
public:
  Audio(Engine& engine);
  ~Audio();

  void initAndClearPrev();

  //Returns channel playing on
  int play(const Sfx_t sfx, const int VOL_PERCENT_TOT = 100,
           const int VOL_PERCENT_L = 50);

  void playFromDir(const Sfx_t sfx, const Dir_t dir,
                         const int DISTANCE_PERCENT);

  void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY);

  void fadeOutChannel(const int CHANNEL_NR);

private:
  Sfx_t getAmbSfxSuitableForDlvl() const;

  void loadAudioFile(const Sfx_t sfx, const string& filename);

  void freeAssets();

  int curChannel;

  Mix_Chunk* audioChunks[endOfSfx];

  int timeAtLastAmb;

  Engine& eng;
};

#endif

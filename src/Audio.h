#ifndef AUDIO_H
#define AUDIO_H

#include <string>

#include "SDL/SDL_Mixer.h"

#include "CommonTypes.h"

class Engine;

using namespace std;

enum Sfx_t {
  sfxDogSnarl,
  sfxWolfHowl,
  sfxZombieGrowl,
  sfxGhoulGrowl,

  sfxPistolFire,
  sfxPistolReload,
  sfxShotgunSawedOffFire,
  sfxShotgunPumpFire,

  sfxRicochet,

  sfxDoorOpen,
  sfxDoorClose,
  sfxDoorBang,
  sfxDoorBreak,

  sfxExplosion,
  sfxExplosionMolotov,

  startOfAmbSfx,
  endOfAmbSfx,

  endOfSfx
};

class Audio {
public:
  Audio(Engine* engine);
  ~Audio();

  void play(const Sfx_t sfx);

  void play(const Sfx_t sfx, const Direction_t direction,
            const int DISTANCE_PERCENT);

  void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY);

private:
  void play(const Sfx_t, const int VOL_PERCENT);

  Sfx_t getAmbSfxSuitableForDlvl() const;

  void loadAllAudio();

  void loadAudioFile(const Sfx_t sfx, const string& filename);

  void freeAssets();

  int curChannel;

  Mix_Chunk* audioChunks[endOfSfx];

  int timeAtLastAmb;

  Engine* eng;
};

#endif

#ifndef AUDIO_H
#define AUDIO_H

#include <string>

#include "SDL/SDL_Mixer.h"

#include "CommonTypes.h"

class Engine;

using namespace std;

enum Sfx_t {
  //Monster sounds
  sfxDogSnarl,
  sfxWolfHowl,
  sfxZombieGrowl,
  sfxGhoulGrowl,
  sfxOozeGurgle,

  //Weapon and attack sounds
  sfxPistolFire,
  sfxPistolReload,
  sfxShotgunSawedOffFire,
  sfxShotgunPumpFire,

  //Environment sounds
  sfxRicochet,
  sfxExplosion,
  sfxExplosionMolotov,
  sfxDoorOpen,
  sfxDoorClose,
  sfxDoorBang,
  sfxDoorBreak,

  //User interface sounds
  sfxBackpack,
  sfxPickup,
  sfxElectricLantern,
  sfxPotionQuaff,

  //Ambient sounds
  startOfAmbSfx,
  endOfAmbSfx,

  endOfSfx
};

class Audio {
public:
  Audio(Engine* engine);
  ~Audio();

  void play(const Sfx_t sfx, const int VOL_PERCENT_TOT = 100,
            const int VOL_PERCENT_L = 50);

  void playFromDirection(const Sfx_t sfx, const Direction_t direction,
                         const int DISTANCE_PERCENT);

  void tryPlayAmb(const int ONE_IN_N_CHANCE_TO_PLAY);

private:
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

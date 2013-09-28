#ifndef AUDIO_H
#define AUDIO_H

#include <string>

#include "SDL/SDL_Mixer.h"

class Engine;

using namespace std;

enum Sfx_t {
  sfxPistolFire,
  sfxShotgunSawedOffFire,
  sfxShotgunPumpFire,

  sfxRicochet,

  sfxDoorOpen,
  sfxDoorClose,
  sfxDoorBang,
  sfxDoorBreak,

  endOfSfx
};

class Audio {
public:
  Audio(Engine* engine);
  ~Audio();

  void play(const Sfx_t sfx);

private:
  void loadAllAudio();

  void loadAudioFile(const Sfx_t sfx, const string& filename);

  void freeAssets();

  Mix_Chunk* audioChunks[endOfSfx];

  Engine* eng;
};

#endif

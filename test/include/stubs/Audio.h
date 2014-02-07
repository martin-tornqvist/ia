#ifndef AUDIO_H
#define AUDIO_H

#include "AudioIds.h"
#include "CommonTypes.h"

class Engine;

class Audio {
public:
  Audio(Engine& engine) {(void)engine;}
  ~Audio() {}

  void initAndClearPrev() {}

  int play(const SfxId, const int = 100, const int = 50) {return -1;}

  void playFromDir(const SfxId, const Dir, const int) {}

  void tryPlayAmb(const int) {}
};

#endif

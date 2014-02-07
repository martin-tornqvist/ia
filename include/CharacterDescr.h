#ifndef CHARACTER_DESCR_H
#define CHARACTER_DESCR_H

#include <vector>

#include "CommonTypes.h"
#include "ActorPlayer.h"

class Engine;

class CharacterDescr {
public:
  CharacterDescr(Engine& engine) : eng(engine) {}
  void run();
private:
  void getShockResSrcTitle(const ShockSrc shockSrc, string& strRef);

  void drawInterface();
  void makeLines();
  vector<StrAndClr> lines;
  Engine& eng;
};

#endif

#ifndef CHARACTER_DESCR_H
#define CHARACTER_DESCR_H

#include <vector>

#include "CmnTypes.h"
#include "ActorPlayer.h"



class CharacterDescr {
public:
  CharacterDescr() {}
  void run();
private:
  void getShockResSrcTitle(const ShockSrc shockSrc, string& strRef);

  void drawInterface();
  void makeLines();
  vector<StrAndClr> lines;

};

#endif

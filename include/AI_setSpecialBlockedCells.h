#ifndef AI_SET_SPECIAL_BLOCKED_CELLS_H
#define AI_SET_SPECIAL_BLOCKED_CELLS_H

#include "Engine.h"

class AI_setSpecialBlockedCells {
public:
  static void learn(
    Monster& monster, bool a[MAP_W][MAP_H], Engine& eng) {
    eng.basicUtils->resetArray(a, false);
  }
private:
};

#endif

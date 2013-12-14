#ifndef AI_SET_SPECIAL_BLOCKED_CELLS_H
#define AI_SET_SPECIAL_BLOCKED_CELLS_H

#include "Engine.h"

class AI_setSpecialBlockedCells {
public:
  static void learn(
    Monster& monster, bool a[MAP_X_CELLS][MAP_Y_CELLS], Engine& eng) {
    eng.basicUtils->resetArray(a, false);
  }
private:
};

#endif

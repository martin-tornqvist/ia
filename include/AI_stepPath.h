#ifndef AI_STEP_PATH
#define AI_STEP_PATH

#include "Engine.h"

class AI_stepPath {
public:
  static bool action(Monster& monster, vector<Pos>* path) {
    if(monster.deadState == actorDeadState_alive) {
      if(path->empty() == false) {
        monster.moveToCell(path->back());
        return true;
      }
    }
    return false;
  }

private:

};

#endif

#ifndef AI_STEP_PATH
#define AI_STEP_PATH

#include "Engine.h"

class AI_stepPath {
public:
  static bool action(Monster& monster, vector<Pos>& path) {
    if(monster.deadState == actorDeadState_alive) {
      if(path.empty() == false) {
        const Pos delta = path.back() - monster.pos;
        monster.moveDir(DirConverter().getDir(delta));
        return true;
      }
    }
    return false;
  }

private:

};

#endif

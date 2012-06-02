#ifndef AI_STEP_PATH
#define AI_STEP_PATH

#include "Engine.h"

class AI_stepPath
{
public:
   static bool action(Monster* monster, vector<coord>* path) {
      if(monster->deadState == actorDeadState_alive) {
         if(path->size() > 0) {
            monster->moveToCell(path->back());
            return true;
         }
      }
      return false;
   }

private:

};

#endif

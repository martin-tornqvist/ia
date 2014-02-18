#ifndef AI_SET_INTEREST_IF_SEE_TARGET_H
#define AI_SET_INTEREST_IF_SEE_TARGET_H

#include "Engine.h"

#include "ActorPlayer.h"
#include "Log.h"

class AI_look_becomePlayerAware {
public:
  static bool action(Monster& monster, Engine& engine) {
    if(monster.deadState == actorDeadState_alive) {

      const bool WAS_AWARE_BEFORE = monster.awareOfPlayerCounter_ > 0;

      vector<Actor*> spottedEnemies;
      monster.getSpottedEnemies(spottedEnemies);

      if(spottedEnemies.empty() == false && WAS_AWARE_BEFORE) {
        monster.becomeAware(false);
        return false;
      }

      for(Actor * actor : spottedEnemies) {
        if(actor == engine.player) {
          if(monster.isSpottingHiddenActor(*actor)) {
            monster.becomeAware(true);
            if(WAS_AWARE_BEFORE) {
              return false;
            } else {
              engine.gameTime->actorDidAct();
              return true;
            }
          }
        } else {
          monster.becomeAware(false);
          if(WAS_AWARE_BEFORE) {
            return false;
          } else {
            engine.gameTime->actorDidAct();
            return true;
          }
        }
      }
    }
    return false;
  }
};

#endif

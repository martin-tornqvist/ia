#ifndef AI_SET_INTEREST_IF_SEE_TARGET_H
#define AI_SET_INTEREST_IF_SEE_TARGET_H

#include "Engine.h"
#include "ActorPlayer.h"

class AI_look_becomePlayerAware {
public:
  static void learn(Monster* monster, Engine* engine) {
    if(monster->deadState == actorDeadState_alive) {

      monster->getSpotedEnemies();

      if(monster->spotedEnemies.size() > 0 && monster->playerAwarenessCounter > 0) {
        monster->becomeAware();
        return;
      }

      for(unsigned int i = 0; i < monster->spotedEnemies.size(); i++) {
        Actor* const actor = monster->spotedEnemies.at(i);
        if(actor == engine->player) {
          const int PLAYER_SNEAK = engine->player->getDef()->abilityValues.getAbilityValue(ability_stealth, true, *(engine->player));
          if(engine->abilityRoll->roll(PLAYER_SNEAK) <= failSmall) {
            monster->becomeAware();
            return;
          }
        } else {
          monster->becomeAware();
          return;
        }
      }
    }
  }

private:

};

#endif

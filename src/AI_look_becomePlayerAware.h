#ifndef AI_SET_INTEREST_IF_SEE_TARGET_H
#define AI_SET_INTEREST_IF_SEE_TARGET_H

#include "Engine.h"
#include "ActorPlayer.h"

class AI_look_becomePlayerAware {
public:
  static bool action(Monster* monster, Engine* engine) {
    const bool IS_AWARE_BEFORE = monster->playerAwarenessCounter > 0;

    if(monster->deadState == actorDeadState_alive) {

      monster->getSpotedEnemies();

      if(monster->spotedEnemies.size() > 0 && IS_AWARE_BEFORE) {
        monster->becomeAware();
        if(IS_AWARE_BEFORE) {
          return false;
        } else {
          engine->gameTime->endTurnOfCurrentActor();
          return true;
        }
      }

      for(unsigned int i = 0; i < monster->spotedEnemies.size(); i++) {
        Actor* const actor = monster->spotedEnemies.at(i);
        if(actor == engine->player) {
          const coord playerPos = engine->player->pos;
          const bool IS_LGT_AT_PLAYER = engine->map->light[playerPos.x][playerPos.y];
          const int PLAYER_SNEAK = IS_LGT_AT_PLAYER ? 0 : engine->player->getDef()->abilityVals.getVal(ability_stealth, true, *(engine->player));
          if(engine->abilityRoll->roll(PLAYER_SNEAK) <= failSmall) {
            monster->becomeAware();
            if(IS_AWARE_BEFORE) {
              return false;
            } else {
              engine->gameTime->endTurnOfCurrentActor();
              return true;
            }
          }
        } else {
          monster->becomeAware();
          if(IS_AWARE_BEFORE) {
            return false;
          } else {
            engine->gameTime->endTurnOfCurrentActor();
            return true;
          }
        }
      }
    }
    return false;
  }

private:

};

#endif

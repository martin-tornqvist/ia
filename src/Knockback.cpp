#include "Knockback.h"

#include "Engine.h"
#include "Attack.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Config.h"
#include "GameTime.h"
#include "Renderer.h"
#include "MapParsing.h"

void KnockBack::tryKnockBack(Actor& defender, const Pos& attackedFromPos,
                             const bool IS_SPIKE_GUN,
                             const bool IS_KNOCKBACK_MESSAGE_ALLOWED) {
  const bool DEFENDER_IS_MONSTER = &defender != eng->player;

  if(DEFENDER_IS_MONSTER || eng->config->isBotPlaying == false) {
    if(defender.getData()->actorSize <= actorSize_giant) {

      const BodyType_t defenderBodyType = defender.getBodyType();
      const bool WALKTYPE_CAN_BE_KNOCKED_BACK =
        defenderBodyType != bodyType_ethereal &&
        defenderBodyType != bodyType_ooze;

      const Pos delta = (defender.pos - attackedFromPos).getSigns();

      const int KNOCK_BACK_RANGE = 2;

      const bool IS_NAILED = defender.getPropHandler()->hasProp(propNailed);

      for(int i = 0; i < KNOCK_BACK_RANGE; i++) {

        const Pos newPos = defender.pos + delta;

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
        MapParser::parse(
          CellPredBlocksBodyType(defender.getBodyType(), true, eng), blockers);
        const bool CELL_BLOCKED = blockers[newPos.x][newPos.y];
        const bool CELL_IS_BOTTOMLESS =
          eng->map->cells[newPos.x][newPos.y].featureStatic->isBottomless();

        if(
          (WALKTYPE_CAN_BE_KNOCKED_BACK) &&
          (CELL_BLOCKED == false || CELL_IS_BOTTOMLESS)) {
          if(i == 0) {
            if(IS_KNOCKBACK_MESSAGE_ALLOWED) {
              if(DEFENDER_IS_MONSTER) {
                eng->log->addMsg(
                  defender.getNameThe() + " is knocked back!");
              } else {
                eng->log->addMsg("I am knocked back!");
              }
            }
            defender.getPropHandler()->tryApplyProp(
              new PropParalyzed(eng, propTurnsSpecified, 1),
              false, false);
            defender.getPropHandler()->tryApplyProp(
              new PropConfused(eng, propTurnsSpecified, 5),
              false, false);
          }

          defender.pos = newPos;

          eng->renderer->drawMapAndInterface();

          eng->sleep(eng->config->delayProjectileDraw);

          if(CELL_IS_BOTTOMLESS) {
            if(DEFENDER_IS_MONSTER) {
              eng->log->addMsg(
                defender.getNameThe() + " plummets down the depths.",
                clrMessageGood);
            } else {
              eng->log->addMsg(
                "I plummet down the depths!", clrMessageBad);
            }
            defender.die(true, false, false);
            return;
          }

          // Bump features (e.g. so monsters can be knocked back into traps)
          vector<FeatureMob*> featureMobs =
            eng->gameTime->getFeatureMobsAtPos(defender.pos);
          for(
            unsigned int featureMobIndex = 0;
            featureMobIndex < featureMobs.size();
            featureMobIndex++) {
            featureMobs.at(featureMobIndex)->bump(defender);
          }

          if(defender.deadState != actorDeadState_alive) {
            return;
          }

          FeatureStatic* const f =
            eng->map->cells[defender.pos.x][defender.pos.y].featureStatic;
          f->bump(defender);

          if(defender.deadState != actorDeadState_alive) {
            return;
          }
        } else {
          // Defender nailed to a wall from a spike gun?
          if(IS_SPIKE_GUN) {
            FeatureStatic* const f =
              eng->map->cells[newPos.x][newPos.y].featureStatic;
            if(f->isVisionPassable() == false) {
              defender.getPropHandler()->tryApplyProp(
                new PropNailed(eng, propTurnsIndefinite));
            }
          }
          return;
        }
      }
    }
  }
}

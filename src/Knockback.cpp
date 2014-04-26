#include "Knockback.h"

#include <algorithm>

#include "Attack.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Config.h"
#include "GameTime.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "SdlWrapper.h"

void KnockBack::tryKnockBack(Actor& defender, const Pos& attackedFromPos,
                             const bool IS_SPIKE_GUN,
                             const bool IS_MSG_ALLOWED) {
  const bool DEFENDER_IS_MONSTER = &defender != Map::player;

  if(DEFENDER_IS_MONSTER || Config::isBotPlaying() == false) {
    if(defender.getData().actorSize <= actorSize_giant) {

      vector<PropId> props;
      defender.getPropHandler().getAllActivePropIds(props);


      const bool ACTOR_CAN_BE_KNOCKED_BACK =
        find(props.begin(), props.end(), propEthereal)  == props.end() &&
        find(props.begin(), props.end(), propOoze)      == props.end();

      const Pos delta = (defender.pos - attackedFromPos).getSigns();

      const int KNOCK_BACK_RANGE = 2;

//      const bool IS_NAILED = defender.getPropHandler().hasProp(propNailed);

      for(int i = 0; i < KNOCK_BACK_RANGE; i++) {

        const Pos newPos = defender.pos + delta;

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(defender, true, eng), blockers);
        const bool CELL_BLOCKED = blockers[newPos.x][newPos.y];
        const bool CELL_IS_BOTTOMLESS =
          Map::cells[newPos.x][newPos.y].featureStatic->isBottomless();

        if(
          (ACTOR_CAN_BE_KNOCKED_BACK) &&
          (CELL_BLOCKED == false || CELL_IS_BOTTOMLESS)) {

          bool visionBlockers[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksVision(), visionBlockers);
          const bool PLAYER_SEE_DEFENDER =
            DEFENDER_IS_MONSTER == false ? true :
            Map::player->isSeeingActor(defender, blockers);

          if(i == 0) {
            if(IS_MSG_ALLOWED) {
              if(DEFENDER_IS_MONSTER && PLAYER_SEE_DEFENDER) {
                eng.log->addMsg(
                  defender.getNameThe() + " is knocked back!");
              } else {
                eng.log->addMsg("I am knocked back!");
              }
            }
            defender.getPropHandler().tryApplyProp(
              new PropParalyzed(eng, propTurnsSpecific, 1), false, false);
          }

          defender.pos = newPos;

          Renderer::drawMapAndInterface();

          SdlWrapper::sleep(Config::getDelayProjectileDraw());

          if(CELL_IS_BOTTOMLESS) {
            if(DEFENDER_IS_MONSTER && PLAYER_SEE_DEFENDER) {
              eng.log->addMsg(
                defender.getNameThe() + " plummets down the depths.",
                clrMsgGood);
            } else {
              eng.log->addMsg(
                "I plummet down the depths!", clrMsgBad);
            }
            defender.die(true, false, false);
            return;
          }

          // Bump features (e.g. so monsters can be knocked back into traps)
          vector<FeatureMob*> featureMobs;
          GameTime::getFeatureMobsAtPos(defender.pos, featureMobs);
          for(
            unsigned int featureMobIndex = 0;
            featureMobIndex < featureMobs.size();
            featureMobIndex++) {
            featureMobs.at(featureMobIndex)->bump(defender);
          }

          if(defender.deadState != ActorDeadState::alive) {
            return;
          }

          FeatureStatic* const f =
            Map::cells[defender.pos.x][defender.pos.y].featureStatic;
          f->bump(defender);

          if(defender.deadState != ActorDeadState::alive) {
            return;
          }
        } else {
          // Defender nailed to a wall from a spike gun?
          if(IS_SPIKE_GUN) {
            FeatureStatic* const f =
              Map::cells[newPos.x][newPos.y].featureStatic;
            if(f->isVisionPassable() == false) {
              defender.getPropHandler().tryApplyProp(
                new PropNailed(eng, propTurnsIndefinite));
            }
          }
          return;
        }
      }
    }
  }
}

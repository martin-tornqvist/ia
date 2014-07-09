#include "Knockback.h"

#include <algorithm>
#include <vector>

#include "Attack.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Config.h"
#include "GameTime.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "SdlWrapper.h"
#include "FeatureStatic.h"
#include "FeatureMob.h"

using namespace std;

namespace KnockBack {

void tryKnockBack(Actor& defender, const Pos& attackedFromPos,
                  const bool IS_SPIKE_GUN, const bool IS_MSG_ALLOWED) {

  const bool DEFENDER_IS_MON = &defender != Map::player;

  if(DEFENDER_IS_MON || !Config::isBotPlaying()) {
    if(defender.getData().actorSize <= actorSize_giant) {

      vector<PropId> props;
      defender.getPropHandler().getAllActivePropIds(props);


      const bool ACTOR_CAN_BE_KNOCKED_BACK =
        find(begin(props), end(props), propEthereal)  == end(props) &&
        find(begin(props), end(props), propOoze)      == end(props);

      const Pos delta = (defender.pos - attackedFromPos).getSigns();

      const int KNOCK_BACK_RANGE = 2;

//      const bool IS_NAILED = defender.getPropHandler().hasProp(propNailed);

      for(int i = 0; i < KNOCK_BACK_RANGE; ++i) {

        const Pos newPos = defender.pos + delta;

        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksActor(defender, true), blocked);
        const bool CELL_BLOCKED = blocked[newPos.x][newPos.y];
        const bool CELL_IS_BOTTOMLESS =
          Map::cells[newPos.x][newPos.y].featureStatic->isBottomless();

        if(
          (ACTOR_CAN_BE_KNOCKED_BACK) &&
          (!CELL_BLOCKED || CELL_IS_BOTTOMLESS)) {

          bool visionBlockers[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksVision(), visionBlockers);
          const bool PLAYER_SEE_DEFENDER =
            DEFENDER_IS_MON ? Map::player->isSeeingActor(defender, blocked) :
            true;

          if(i == 0) {
            if(IS_MSG_ALLOWED) {
              if(DEFENDER_IS_MON && PLAYER_SEE_DEFENDER) {
                Log::addMsg(defender.getNameThe() + " is knocked back!");
              } else {
                Log::addMsg("I am knocked back!");
              }
            }
            defender.getPropHandler().tryApplyProp(
              new PropParalyzed(propTurnsSpecific, 1), false, false);
          }

          defender.pos = newPos;

          Renderer::drawMapAndInterface();

          SdlWrapper::sleep(Config::getDelayProjectileDraw());

          if(CELL_IS_BOTTOMLESS) {
            if(DEFENDER_IS_MON && PLAYER_SEE_DEFENDER) {
              Log::addMsg(
                defender.getNameThe() + " plummets down the depths.",
                clrMsgGood);
            } else {
              Log::addMsg("I plummet down the depths!", clrMsgBad);
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
            if(!f->isVisionPassable()) {
              defender.getPropHandler().tryApplyProp(
                new PropNailed(propTurnsIndefinite));
            }
          }
          return;
        }
      }
    }
  }
}

} //KnockBack

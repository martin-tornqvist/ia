#include "Bash.h"

#include "Init.h"
#include "GameTime.h"
#include "FeatureStatic.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace Bash {

void playerBash() {
  TRACE_FUNC_BEGIN;

  Log::clearLog();
  Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Renderer::drawMapAndInterface();
  Pos bashPos(Map::player->pos + Query::dir());
  Log::clearLog();

  if(bashPos != Map::player->pos) {
    //Bash living actor?
    Actor* livingActor =
      Utils::getActorAtPos(bashPos, ActorDeadState::alive);
    if(livingActor) {
      TRACE << "Actor found at bash pos, attempting to kick actor" << endl;
      if(Map::player->getPropHandler().allowAttackMelee(true)) {
        TRACE << "Player is allowed to do melee attack" << endl;
        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blocked);

        TRACE << "Player can see actor" << endl;
        Map::player->kick(*livingActor);
      }
      TRACE_FUNC_END;
      return;
    }

    //Bash corpse?
    Actor* deadActor =
      Utils::getActorAtPos(bashPos, ActorDeadState::corpse);
    if(deadActor) {
      const bool IS_SEEING_CORPSE =
        Map::cells[bashPos.x][bashPos.y].isSeenByPlayer;

      const string actorNameA = deadActor->getNameA();

      const string name =
        IS_SEEING_CORPSE ? ("the body of " + actorNameA) : "a corpse";

      Log::addMsg("I bash " + name + ".");

      pair<int, int> kickDmg =
        ItemData::data[int(ItemId::playerKick)]->meleeDmg;
      deadActor->hit(kickDmg.first * kickDmg.second, DmgType::physical, false);
      GameTime::actorDidAct();
      TRACE_FUNC_END;
      return;
    }

    //Bash feature
    TRACE << "No actor at bash pos, attempting to bash feature instead" << endl;
    Cell& cell = Map::cells[bashPos.x][bashPos.y];
    cell.featureStatic->bash(*Map::player);
  }
  TRACE_FUNC_END;
}

} //Bash

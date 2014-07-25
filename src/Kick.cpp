#include "Kick.h"

#include "Init.h"
#include "GameTime.h"
#include "FeatureRigid.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace Kick {

void playerKick() {
  TRACE_FUNC_BEGIN;

  Log::clearLog();
  Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Renderer::drawMapAndInterface();
  Pos kickPos(Map::player->pos + Query::dir());
  Log::clearLog();

  if(kickPos != Map::player->pos) {
    //Kick living actor?
    Actor* livingActor = Utils::getFirstActorAtPos(kickPos, ActorDeadState::alive);
    if(livingActor) {
      TRACE << "Actor found at kick pos, attempting to kick actor" << endl;
      if(Map::player->getPropHandler().allowAttackMelee(true)) {
        TRACE << "Player is allowed to do melee attack" << endl;
        bool blocked[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blocked);

        TRACE << "Player can see actor" << endl;
        Map::player->kickMonster(*livingActor);
      }
      TRACE_FUNC_END;
      return;
    }

    //Kick corpse?
    Actor* deadActor = Utils::getFirstActorAtPos(kickPos, ActorDeadState::corpse);
    if(deadActor) {
      const bool IS_SEEING_CORPSE =
        Map::cells[kickPos.x][kickPos.y].isSeenByPlayer;

      const string actorNameA = deadActor->getNameA();

      const string name =
        IS_SEEING_CORPSE ? ("the body of " + actorNameA) : "a corpse";

      Log::addMsg("I bash " + name + ".");

      pair<int, int> kickDmg = ItemData::data[int(ItemId::playerKick)]->meleeDmg;
      deadActor->hit(kickDmg.first * kickDmg.second, DmgType::physical, false);
      GameTime::actorDidAct();
      TRACE_FUNC_END;
      return;
    }

    //Kick feature
    TRACE << "No actor at kick pos, attempting to kick feature instead" << endl;
    auto* const f = Map::cells[kickPos.x][kickPos.y].rigid;
    f->hit(DmgType::physical, DmgMethod::kick, Map::player);
  }
  TRACE_FUNC_END;
}

} //Kick

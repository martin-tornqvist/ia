#include "Bash.h"

#include "Init.h"
#include "GameTime.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

void Bash::playerBash() const {
  trace << "Bash::playerBash()" << endl;

  Log::clearLog();
  Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Renderer::drawMapAndInterface();
  Pos bashPos(Map::player->pos + Query::dir());
  Log::clearLog();

  if(bashPos != Map::player->pos) {
    //Bash living actor?
    Actor* livingActor =
      Utils::getActorAtPos(bashPos, ActorDeadState::alive);
    if(livingActor != NULL) {
      trace << "Bash: Actor found at bash pos, attempt kicking actor" << endl;
      if(Map::player->getPropHandler().allowAttackMelee(true)) {
        trace << "Bash: Player is allowed to do melee attack" << endl;
        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(), blockers);

        trace << "Bash: Player can see actor" << endl;
        Map::player->kick(*livingActor);
      }
      return;
    }

    //Bash corpse?
    Actor* deadActor =
      Utils::getActorAtPos(bashPos, ActorDeadState::corpse);
    if(deadActor != NULL) {
      const bool IS_SEEING_CORPSE =
        Map::cells[bashPos.x][bashPos.y].isSeenByPlayer;

      const string actorNameA = deadActor->getNameA();

      const string name =
        IS_SEEING_CORPSE ? ("the body of " + actorNameA) : "a corpse";

      Log::addMsg("I bash " + name + ".");

      pair<int, int> kickDmg =
        ItemData::dataList[int(ItemId::playerKick)]->meleeDmg;
      deadActor->hit(kickDmg.first * kickDmg.second, DmgType::physical, false);
      GameTime::actorDidAct();
      return;
    }

    //Bash feature
    trace << "Bash: No actor at bash pos, ";
    trace << "attempting to bash feature instead" << endl;
    Cell& cell = Map::cells[bashPos.x][bashPos.y];
    cell.featureStatic->bash(*Map::player);
  }
}

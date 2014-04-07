#include "Bash.h"

#include "Engine.h"

#include "GameTime.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "Utils.h"

void Bash::playerBash() const {
  trace << "Bash::playerBash()" << endl;

  eng.log->clearLog();
  eng.log->addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Renderer::drawMapAndInterface();
  Pos bashPos(eng.player->pos + eng.query->dir());
  eng.log->clearLog();

  if(bashPos != eng.player->pos) {
    //Bash living actor?
    Actor* livingActor =
      Utils::getActorAtPos(bashPos, eng, ActorDeadState::alive);
    if(livingActor != NULL) {
      trace << "Bash: Actor found at bash pos, attempt kicking actor" << endl;
      if(eng.player->getPropHandler().allowAttackMelee(true)) {
        trace << "Bash: Player is allowed to do melee attack" << endl;
        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(eng), blockers);

        trace << "Bash: Player can see actor" << endl;
        eng.player->kick(*livingActor);
      }
      return;
    }

    //Bash corpse?
    Actor* deadActor =
      Utils::getActorAtPos(bashPos, eng, ActorDeadState::corpse);
    if(deadActor != NULL) {
      const bool IS_SEEING_CORPSE =
        eng.map->cells[bashPos.x][bashPos.y].isSeenByPlayer;

      const string actorNameA = deadActor->getNameA();

      const string name =
        IS_SEEING_CORPSE ? ("the body of " + actorNameA) : "a corpse";

      eng.log->addMsg("I bash " + name + ".");

      pair<int, int> kickDmg =
        eng.itemDataHandler->dataList[int(ItemId::playerKick)]->meleeDmg;
      deadActor->hit(kickDmg.first * kickDmg.second, DmgType::physical, false);
      eng.gameTime->actorDidAct();
      return;
    }

    //Bash feature
    trace << "Bash: No actor at bash pos, ";
    trace << "attempting to bash feature instead" << endl;
    Cell& cell = eng.map->cells[bashPos.x][bashPos.y];
    cell.featureStatic->bash(*eng.player);
  }
}

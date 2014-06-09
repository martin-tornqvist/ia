#include "Disarm.h"

#include "GameTime.h"
#include "Log.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "Query.h"
#include "Map.h"
#include "FeatureTrap.h"
#include "Utils.h"
#include "Inventory.h"

namespace Disarm {

void playerDisarm() {
  //TODO It would probably be more fun if examine were allowed while blind,
  //with some potentially horrible results

  //Abort if blind
  if(!Map::player->getPropHandler().allowSee()) {
    Log::addMsg("Not while blind.");
    Renderer::drawMapAndInterface();
    return;
  }

  //Abort if held by spider web
  const Pos playerPos = Map::player->pos;
  const auto* const featureAtPlayer =
    Map::cells[playerPos.x][playerPos.y].featureStatic;
  if(featureAtPlayer->getId() == FeatureId::trap) {
    const Trap* const trap = dynamic_cast<const Trap*>(featureAtPlayer);
    if(trap->getTrapType() == trap_spiderWeb) {
      const auto* const web =
        dynamic_cast<const TrapSpiderWeb*>(trap->getSpecificTrap());
      if(web->isHolding()) {
        Log::addMsg("Not while entangled in a spider web.");
        Renderer::drawMapAndInterface();
        return;
      }
    }
  }

  //Abort if encumbered
  if(Map::player->getEncPercent() >= 100) {
    Log::addMsg("Not while encumbered.");
    Renderer::drawMapAndInterface();
    return;
  }

  Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Renderer::drawMapAndInterface();

  const Pos pos(Map::player->pos + Query::dir());

  if(pos != Map::player->pos) {

    //Abort if cell is unseen
    if(!Map::cells[pos.x][pos.y].isSeenByPlayer) {
      Log::addMsg("I cannot see there.");
      Renderer::drawMapAndInterface();
      return;
    }

    Log::clearLog();

    Actor* actorOnTrap = Utils::getActorAtPos(pos);

    //Abort if trap blocked by monster
    if(actorOnTrap) {
      if(Map::player->isSeeingActor(*actorOnTrap, nullptr)) {
        Log::addMsg("It's blocked.");
      } else {
        Log::addMsg("Something is blocking it.");
      }
      Renderer::drawMapAndInterface();
      return;
    }

    Log::clearLog();
    Map::cells[pos.x][pos.y].featureStatic->disarm();
    Renderer::drawMapAndInterface();
  }
}

} //Disarm


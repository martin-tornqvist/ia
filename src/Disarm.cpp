#include "Disarm.h"

#include "Engine.h"

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

void playerDisarm(Engine& eng) {
  //TODO It would probably be more fun if examine were allowed while blind,
  //with some potentially horrible results

  //Abort if blind
  if(eng.player->getPropHandler().allowSee() == false) {
    eng.log->addMsg("Not while blind.");
    Renderer::drawMapAndInterface();
    return;
  }

  //Abort if held by spider web
  const Pos playerPos = eng.player->pos;
  const FeatureStatic* const featureAtPlayer =
    eng.map->cells[playerPos.x][playerPos.y].featureStatic;
  if(featureAtPlayer->getId() == feature_trap) {
    const Trap* const trap = dynamic_cast<const Trap*>(featureAtPlayer);
    if(trap->getTrapType() == trap_spiderWeb) {
      const TrapSpiderWeb* const web =
        dynamic_cast<const TrapSpiderWeb*>(trap->getSpecificTrap());
      if(web->isHolding()) {
        eng.log->addMsg("Not while entangled in a spider web.");
        Renderer::drawMapAndInterface();
        return;
      }
    }
  }

  //Abort if encumbered
  if(eng.player->getEncPercent() >= 100) {
    eng.log->addMsg("Not while encumbered.");
    Renderer::drawMapAndInterface();
    return;
  }

  eng.log->addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Renderer::drawMapAndInterface();

  const Pos pos(eng.player->pos + eng.query->dir());

  //Abort if cell is unseen
  if(eng.map->cells[pos.x][pos.y].isSeenByPlayer == false) {
    eng.log->addMsg("I cannot see there.");
    Renderer::drawMapAndInterface();
    return;
  }

  eng.log->clearLog();

  Actor* actorOnTrap = Utils::getActorAtPos(pos, eng);

  //Abort if trap blocked by monster
  if(actorOnTrap != NULL) {
    if(eng.player->isSeeingActor(*actorOnTrap, NULL)) {
      eng.log->addMsg("It's blocked.");
    } else {
      eng.log->addMsg("Something is blocking it.");
    }
    Renderer::drawMapAndInterface();
    return;
  }

  eng.log->clearLog();
  if(pos != eng.player->pos) {
    eng.map->cells[pos.x][pos.y].featureStatic->disarm();
  }
  Renderer::drawMapAndInterface();
}

} //Disarm


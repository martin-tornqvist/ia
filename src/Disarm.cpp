#include "Disarm.h"

#include "Engine.h"

#include "GameTime.h"
#include "Log.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "Query.h"
#include "Map.h"
#include "FeatureTrap.h"

namespace Disarm {

void playerDisarm(Engine& eng) {
  //TODO It would probably be more fun if examine were allowed while blind,
  //with some potentially horrible results
  if(eng.player->getPropHandler().allowSee()) {
    eng.log->addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
    eng.renderer->drawMapAndInterface();

    const Pos disarmPos(eng.player->pos + eng.query->dir());

    eng.log->clearLog();
    if(disarmPos != eng.player->pos) {
      eng.map->cells[disarmPos.x][disarmPos.y].featureStatic->disarm();
    }
    eng.renderer->drawMapAndInterface();
  } else {
    eng.log->addMsg("Not while blind.");
  }
  eng.renderer->drawMapAndInterface();
}

} //Disarm


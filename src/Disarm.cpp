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
  eng.log->addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  eng.renderer->drawMapAndInterface();
  const Pos disarmPos(eng.player->pos + eng.query->dir());
  eng.log->clearLog();

  eng.map->cells[disarmPos.x][disarmPos.y].featureStatic->playerTryDisarm();

  eng.renderer->drawMapAndInterface();
}

} //Disarm


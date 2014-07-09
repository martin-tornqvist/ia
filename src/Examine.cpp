#include "Examine.h"

#include "FeatureStatic.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"

namespace Examine {

void playerExamine() {
  //TODO It would probably be more fun if examine were allowed while blind,
  //with some potentially horrible results
  if(Map::player->getPropHandler().allowSee()) {
    Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
    Renderer::drawMapAndInterface();

    Pos examinePos = Map::player->pos + Query::dir();

    Log::clearLog();

    if(examinePos != Map::player->pos) {
      Map::cells[examinePos.x][examinePos.y].featureStatic->examine();
    }
  } else {
    Log::addMsg("Not while blind.");
  }
  Renderer::drawMapAndInterface();
}

} //Examine

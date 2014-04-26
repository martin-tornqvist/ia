#include "Examine.h"

#include "Feature.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"

void Examine::playerExamine() const {
  //TODO It would probably be more fun if examine were allowed while blind,
  //with some potentially horrible results
  if(Map::player->getPropHandler().allowSee()) {
    eng.log->addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
    Renderer::drawMapAndInterface();

    Pos examinePos = Map::player->pos + eng.query->dir();

    eng.log->clearLog();

    if(examinePos != Map::player->pos) {
      Map::cells[examinePos.x][examinePos.y].featureStatic->examine();
    }
  } else {
    eng.log->addMsg("Not while blind.");
  }
  Renderer::drawMapAndInterface();
}

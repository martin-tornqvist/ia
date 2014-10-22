#include "Examine.h"

#include "FeatureRigid.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Render.h"
#include "Utils.h"

namespace Examine {

void playerExamine() {
  //TODO It would probably be more fun if examine were allowed while blind,
  //with some potentially horrible results
  if(Map::player->getPropHandler().allowSee()) {
    Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
    Render::drawMapAndInterface();

    Pos examinePos = Map::player->pos + DirUtils::getOffset(Query::dir());

    Log::clearLog();

    if(examinePos != Map::player->pos) {
      Map::cells[examinePos.x][examinePos.y].rigid->examine();
    }
  } else {
    Log::addMsg("Not while blind.");
  }
  Render::drawMapAndInterface();
}

} //Examine

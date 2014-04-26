#include "Close.h"

#include "GameTime.h"
#include "Feature.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "FeatureDoor.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Renderer.h"

void Close::playerClose() const {
  eng.log->clearLog();
  eng.log->addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Renderer::drawMapAndInterface();
  Pos closePos(Map::player->pos + eng.query->dir());
  eng.log->clearLog();

  if(closePos != Map::player->pos) {
    playerCloseFeature(Map::cells[closePos.x][closePos.y].featureStatic);
  }

  Renderer::drawMapAndInterface();
}

void Close::playerCloseFeature(Feature* const feature) const {
  bool closeAbleObjectFound = false;

  if(feature->getId() == feature_door) {
    Door* const door = dynamic_cast<Door*>(feature);
    door->tryClose(Map::player);
    closeAbleObjectFound = true;
  }

  if(closeAbleObjectFound == false) {
    const bool PLAYER_CAN_SEE = Map::player->getPropHandler().allowSee();
    if(PLAYER_CAN_SEE) {
      eng.log->addMsg("I see nothing there to close.");
    } else {
      eng.log->addMsg("I find nothing there to close.");
    }
  }
}


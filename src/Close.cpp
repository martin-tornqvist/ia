#include "Close.h"

#include "Engine.h"
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
  eng.log->addMsg("Which direction? | space/esc to cancel", clrWhiteHigh);
  eng.renderer->drawMapAndInterface();
  Pos closePos(eng.player->pos + eng.query->dir());
  eng.log->clearLog();

  if(closePos != eng.player->pos) {
    playerCloseFeature(eng.map->cells[closePos.x][closePos.y].featureStatic);
  }
}

void Close::playerCloseFeature(Feature* const feature) const {
  bool closeAbleObjectFound = false;

  if(feature->getId() == feature_door) {
    Door* const door = dynamic_cast<Door*>(feature);
    door->tryClose(eng.player);
    closeAbleObjectFound = true;
  }

  if(closeAbleObjectFound == false) {
    const bool PLAYER_CAN_SEE = eng.player->getPropHandler().allowSee();
    if(PLAYER_CAN_SEE) {
      eng.log->addMsg("I see nothing there to close.");
    } else {
      eng.log->addMsg("I find nothing there to close.");
    }
  }
}


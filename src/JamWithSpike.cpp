#include "JamWithSpike.h"

#include "Engine.h"
#include "GameTime.h"
#include "Feature.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Query.h"
#include "Inventory.h"
#include "Renderer.h"

void JamWithSpike::playerJam() const {
  eng.log->clearLog();

  if(eng.player->getInv().hasItemInGeneral(item_ironSpike) == false) {
    eng.log->addMsg("I have no spikes to jam with.", clrWhite);
    eng.renderer->drawMapAndInterface();
    return;
  }

  eng.log->addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  eng.renderer->drawMapAndInterface();
  const Pos jamPos(eng.player->pos + eng.query->dir());
  eng.log->clearLog();

  playerJamFeature(eng.map->cells[jamPos.x][jamPos.y].featureStatic);

  eng.renderer->drawMapAndInterface();
}

void JamWithSpike::playerJamFeature(Feature* const feature) const {
  bool jamableObjectFound = false;

  if(feature->getId() == feature_door) {
    Door* const door = dynamic_cast<Door*>(feature);
    const bool DOOR_SPIKED = door->trySpike(eng.player);

    if(DOOR_SPIKED == true) {

      jamableObjectFound = true;

      eng.player->getInv().decrItemTypeInGeneral(item_ironSpike);
      const int SPIKES_LEFT =
        eng.player->getInv().getItemStackSizeInGeneral(item_ironSpike);
      if(SPIKES_LEFT == 0) {
        eng.log->addMsg("I have no iron spikes left.");
      } else {
        eng.log->addMsg("I have " + toString(SPIKES_LEFT) + " iron spikes left.");
      }
    }
  }

  if(jamableObjectFound == false) {
    const bool PLAYER_IS_BLIND = eng.player->getPropHandler().allowSee();
    if(PLAYER_IS_BLIND == false) {
      eng.log->addMsg("I see nothing there to jam with a spike.");
    } else {
      eng.log->addMsg("I find nothing there to jam with a spike.");
    }
  }
}


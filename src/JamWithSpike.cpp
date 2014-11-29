#include "JamWithSpike.h"

#include "GameTime.h"
#include "Feature.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "Query.h"
#include "Inventory.h"
#include "Render.h"
#include "FeatureDoor.h"
#include "Utils.h"

namespace JamWithSpike
{

void playerJamFeature(Feature* const feature)
{
  bool jamableObjectFound = false;

  if (feature->getId() == FeatureId::door)
  {
    Door* const door = static_cast<Door*>(feature);
    const bool DOOR_SPIKED = door->trySpike(Map::player);

    if (DOOR_SPIKED)
    {

      jamableObjectFound = true;

      Map::player->getInv().decrItemTypeInGeneral(ItemId::ironSpike);
      const int SPIKES_LEFT =
        Map::player->getInv().getItemStackSizeInGeneral(ItemId::ironSpike);
      if (SPIKES_LEFT == 0)
      {
        Log::addMsg("I have no iron spikes left.");
      }
      else
      {
        Log::addMsg("I have " + toStr(SPIKES_LEFT) + " iron spikes left.");
      }
    }
  }

  if (!jamableObjectFound)
  {
    const bool PLAYER_IS_BLIND = Map::player->getPropHandler().allowSee();
    if (!PLAYER_IS_BLIND)
    {
      Log::addMsg("I see nothing there to jam with a spike.");
    }
    else
    {
      Log::addMsg("I find nothing there to jam with a spike.");
    }
  }
}

void playerJam()
{
  Log::clearLog();

  if (!Map::player->getInv().hasItemInBackpack(ItemId::ironSpike))
  {
    Log::addMsg("I have no spikes to jam with.", clrWhite);
    Render::drawMapAndInterface();
    return;
  }

  Log::addMsg("Which direction?" + cancelInfoStr, clrWhiteHigh);
  Render::drawMapAndInterface();
  const Pos jamPos(Map::player->pos + DirUtils::getOffset(Query::dir()));
  Log::clearLog();

  playerJamFeature(Map::cells[jamPos.x][jamPos.y].rigid);

  Render::drawMapAndInterface();
}

} //JamWithSpike

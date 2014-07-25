#include "DungeonClimb.h"

#include <iostream>

#include "Init.h"
#include "Map.h"
#include "MapGen.h"
#include "PopulateItems.h"
#include "Renderer.h"
#include "Log.h"
#include "FeatureRigid.h"

using namespace std;

namespace DungeonClimb {

namespace {

void mkLvl() {
  TRACE_FUNC_BEGIN;

  bool levelBuilt = false;

  //------------------------------------- TRAPEZOHEDRON LVL
  if(!levelBuilt) {
    if(Map::dlvl > LAST_CAVERN_LVL) {
      while(!MapGen::TrapezohedronLvl::run()) {}
      levelBuilt = true;
    }
  }

  //------------------------------------- KINGS TOMB
  if(!levelBuilt) {
    if(Map::dlvl == LAST_ROOM_AND_CORRIDOR_LVL + 1) {
      while(!MapGen::Egypt::run()) {}
      levelBuilt = true;
    }
  }

  //------------------------------------- DUNGEON LVLS
  if(!levelBuilt) {
    if(Map::dlvl < FIRST_CAVERN_LVL) {
      while(!MapGen::Std::run()) {}
      levelBuilt = true;
    }
  }
  //------------------------------------- CAVERN LVLS
  if(!levelBuilt) {
    if(Map::dlvl >= FIRST_CAVERN_LVL) {
      while(!MapGen::CaveLvl::run()) {}
    }
  }
  if(Map::dlvl > 0 && Map::dlvl <= LAST_CAVERN_LVL) {
    TRACE << "Calling PopulateItems::mkItems()" << endl;
    PopulateItems::mkItems();
  }
  TRACE_FUNC_END;
}

} //namespace

void travelDown(const int LVLS) {
  TRACE_FUNC_BEGIN;

  Map::player->restoreShock(999, true);

  Map::dlvl += LVLS;

  mkLvl();

  Map::player->target = nullptr;
  Map::player->updateFov();
  Map::player->updateColor();
  Renderer::drawMapAndInterface();
  Audio::tryPlayAmb(1);
  TRACE_FUNC_END;
}

void tryUseDownStairs() {
  TRACE_FUNC_BEGIN;

  Log::clearLog();

  const Pos& playerPos = Map::player->pos;

  const auto featureIdAtPlayer =
    Map::cells[playerPos.x][playerPos.y].rigid->getId();

  if(featureIdAtPlayer == FeatureId::stairs) {
    TRACE << "DungeonClimb: Player is on stairs" << endl;
    if(Map::dlvl >= FIRST_CAVERN_LVL && Map::dlvl <= LAST_CAVERN_LVL) {
      Log::addMsg("I climb downwards.");
    } else {
      Log::addMsg("I descend the stairs.");
    }
//    Renderer::updateScreen();
    travelDown();

    if(Map::player->phobias[int(Phobia::deepPlaces)]) {
      Log::addMsg("I am plagued by my phobia of deep places!");
      Map::player->getPropHandler().tryApplyProp(
        new PropTerrified(PropTurns::standard));
      return;
    }
  } else {
    if(Map::dlvl >= FIRST_CAVERN_LVL && Map::dlvl <= LAST_CAVERN_LVL) {
      Log::addMsg("I see no path leading downwards here.");
    } else {
      Log::addMsg("I see no stairs leading downwards here.");
    }
//    Renderer::updateScreen();
  }
  TRACE_FUNC_END;
}

} //DungeonClimb

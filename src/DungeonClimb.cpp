#include "DungeonClimb.h"

#include <iostream>

#include "Init.h"
#include "Map.h"
#include "MapGen.h"
#include "PopulateItems.h"
#include "Renderer.h"
#include "Log.h"

using namespace std;

namespace DungeonClimb {

namespace {

void makeLevel() {
  trace << "DungeonClimb::makeLevel()..." << endl;

  bool levelBuilt = false;

  //------------------------------------- TRAPEZOHEDRON LEVEL
  if(levelBuilt == false) {
    if(Map::dlvl > LAST_CAVERN_LEVEL) {
      while(MapGen::TrapezohedronLvl::run() == false) {}
      levelBuilt = true;
    }
  }

  //------------------------------------- KINGS TOMB
  if(levelBuilt == false) {
    if(Map::dlvl == LAST_ROOM_AND_CORRIDOR_LEVEL + 1) {
      while(MapGen::EgyptTomb::run() == false) {}
      levelBuilt = true;
    }
  }

  //------------------------------------- DUNGEON LEVELS
  if(levelBuilt == false) {
    if(Map::dlvl < FIRST_CAVERN_LEVEL) {
      while(MapGen::Bsp::run() == false) {}
      levelBuilt = true;
    }
  }
  //------------------------------------- CAVERN LEVELS
  if(levelBuilt == false) {
    if(Map::dlvl >= FIRST_CAVERN_LEVEL) {
      while(MapGen::CaveLvl::run() == false) {}
    }
  }
  if(Map::dlvl > 0 && Map::dlvl <= LAST_CAVERN_LEVEL) {
    trace << "DungeonClimb: Calling PopulateItems::spawnItems()" << endl;
    PopulateItems::spawnItems();
  }
  trace << "DungeonClimb::makeLevel() [DONE]" << endl;
}

} //namespace

void travelDown(const int LEVELS) {
  trace << "DungeonClimb::travelDown()..." << endl;

  Map::player->restoreShock(999, true);

  Map::dlvl += LEVELS;

  makeLevel();

  Map::player->target = nullptr;
  Map::player->updateFov();
  Map::player->updateColor();
  Renderer::drawMapAndInterface();
  Audio::tryPlayAmb(1);
  trace << "DungeonClimb::travelDown() [DONE]" << endl;
}

void tryUseDownStairs() {
  trace << "DungeonClimb::tryUseDownStairs()..." << endl;

  Log::clearLog();

  const Pos& playerPos = Map::player->pos;

  const FeatureId featureIdAtPlayer =
    Map::cells[playerPos.x][playerPos.y].featureStatic->getId();

  if(featureIdAtPlayer == FeatureId::stairs) {
    trace << "DungeonClimb: Player is on stairs" << endl;
    if(Map::dlvl >= FIRST_CAVERN_LEVEL && Map::dlvl <= LAST_CAVERN_LEVEL) {
      Log::addMsg("I climb downwards.");
    } else {
      Log::addMsg("I descend the stairs.");
    }
//    Renderer::updateScreen();
    travelDown();

    if(Map::player->phobias[int(Phobia::deepPlaces)]) {
      Log::addMsg("I am plagued by my phobia of deep places!");
      Map::player->getPropHandler().tryApplyProp(
        new PropTerrified(propTurnsStd));
      return;
    }
  } else {
    if(Map::dlvl >= FIRST_CAVERN_LEVEL && Map::dlvl <= LAST_CAVERN_LEVEL) {
      Log::addMsg("I see no path leading downwards here.");
    } else {
      Log::addMsg("I see no stairs leading downwards here.");
    }
//    Renderer::updateScreen();
  }
  trace << "DungeonClimb::tryUseDownStairs() [DONE]" << endl;
}

} //DungeonClimb

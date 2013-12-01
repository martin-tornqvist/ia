#include "DungeonClimb.h"

#include "Engine.h"
#include "Map.h"
#include "MapGen.h"
#include "PopulateMonsters.h"
#include "PopulateItems.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Item.h"
#include "Renderer.h"
#include "Audio.h"

void DungeonClimb::makeLevel() {
  trace << "DungeonClimb::makeLevel()..." << endl;

  const int DLVL = eng->map->getDLVL();

  bool levelBuilt = false;

  //------------------------------------- TRAPEZOHEDRON LEVEL
  if(levelBuilt == false) {
    if(DLVL > LAST_CAVERN_LEVEL) {
      while(MapGenTrapezohedronLvl(eng).run() == false) {}
      levelBuilt = true;
    }
  }

  //------------------------------------- KINGS TOMB
  if(levelBuilt == false) {
    if(DLVL == LAST_ROOM_AND_CORRIDOR_LEVEL + 1) {
      while(MapGenEgyptTomb(eng).run() == false) {}
      levelBuilt = true;
    }
  }

  //------------------------------------- DUNGEON LEVELS
  if(levelBuilt == false) {
    if(DLVL < FIRST_CAVERN_LEVEL) {
      while(MapGenBsp(eng).run() == false) {}
      levelBuilt = true;
    }
  }
  //------------------------------------- CAVERN LEVELS
  if(levelBuilt == false) {
    if(DLVL >= FIRST_CAVERN_LEVEL) {
      while(MapGenCaveLvl(eng).run() == false) {}
    }
  }
  if(DLVL > 0 && DLVL <= LAST_CAVERN_LEVEL) {
    trace << "DungeonClimb: Calling PopulateItems::spawnItems()" << endl;
    eng->populateItems->spawnItems();
  }
  trace << "DungeonClimb::makeLevel() [DONE]" << endl;
}

void DungeonClimb::travelDown(const int levels) {
  trace << "DungeonClimb::travelDown()..." << endl;

  eng->player->restoreShock(999, true);

  eng->map->incrDlvl(levels);

  makeLevel();

  eng->player->target = NULL;
  eng->player->updateFov();
  eng->player->updateColor();
  eng->renderer->drawMapAndInterface();
  eng->audio->tryPlayAmb(2);
  trace << "DungeonClimb::travelDown() [DONE]" << endl;
}

void DungeonClimb::tryUseDownStairs() {
  trace << "DungeonClimb::tryUseDownStairs()..." << endl;

  eng->log->clearLog();

  const int DLVL = eng->map->getDLVL();
  const Pos& playerPos = eng->player->pos;

  const Feature_t featureIdAtPlayer =
    eng->map->cells[playerPos.x][playerPos.y].featureStatic->getId();

  if(featureIdAtPlayer == feature_stairsDown) {
    trace << "DungeonClimb: Player is on stairs" << endl;
    if(DLVL >= FIRST_CAVERN_LEVEL && DLVL <= LAST_CAVERN_LEVEL) {
      eng->log->addMsg("I climb downwards.");
    } else {
      eng->log->addMsg("I descend the stairs.");
    }
//    eng->renderer->updateScreen();
    travelDown();

    if(eng->player->insanityPhobias[insanityPhobia_deepPlaces]) {
      eng->log->addMsg("I am plagued by my phobia of deep places!");
      eng->player->getPropHandler()->tryApplyProp(
        new PropTerrified(eng, propTurnsStandard));
      return;
    }
  } else {
    if(DLVL >= FIRST_CAVERN_LEVEL && DLVL <= LAST_CAVERN_LEVEL) {
      eng->log->addMsg("I see no path leading downwards here.");
    } else {
      eng->log->addMsg("I see no stairs leading downwards here.");
    }
//    eng->renderer->updateScreen();
  }
  trace << "DungeonClimb::tryUseDownStairs() [DONE]" << endl;
}


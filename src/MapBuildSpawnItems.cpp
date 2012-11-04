#include "MapBuildSpawnItems.h"
#include "Engine.h"

#include "ActorPlayer.h"
#include "Map.h"
#include "ItemFactory.h"
#include "PlayerBonuses.h"

void MapBuildSpawnItems::spawnItems() {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeItemBlockerArray(blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);

  const int CELLS_PER_SPAWN = 125;

  int nrOfSpawns = freeCells.size() / CELLS_PER_SPAWN;
  nrOfSpawns = max(1, nrOfSpawns);
  nrOfSpawns += eng->dice(1, (nrOfSpawns / 2) + 2) - 1;

  if(eng->playerBonusHandler->isBonusPicked(playerBonus_treasureHunter)) {
    nrOfSpawns = static_cast<int>(static_cast<double>(nrOfSpawns) * 1.2);
  }

  buildCandidateList();

  //Spawn randomly from the coord-vector
  int n = 0;
  ItemDevNames_t id;

  for(int i = 0; i < nrOfSpawns; i++) {
    if(freeCells.size() > 0) {

      //Roll the dice for random element
      n = eng->dice(1, freeCells.size()) - 1;
      const coord pos(freeCells.at(n));

      //Get type to spawn
      id = getFromCandidateList();

      //Spawn
      eng->itemFactory->spawnItemOnMap(id, pos);

      //Erase coordinate from the vector
      freeCells.erase(freeCells.begin() + n);
    }
  }
}

void MapBuildSpawnItems::buildCandidateList() {
  candidates.resize(0);

  ItemDefinition** defs = eng->itemData->itemDefinitions;

  const unsigned int NUMBER_DEFINED = static_cast<unsigned int>(endOfItemDevNames);

  for(unsigned int i = 1; i < NUMBER_DEFINED; i++) {
    if(eng->map->getDungeonLevel() >= defs[i]->spawnStandardMinDLVL && eng->map->getDungeonLevel() <= defs[i]->spawnStandardMaxDLVL
        && defs[i]->isIntrinsicWeapon == false) {
      if(eng->dice(1, 100) < defs[i]->chanceToIncludeInSpawnList) {
        candidates.push_back(static_cast<ItemDevNames_t>(i));
      }
    }
  }
}

ItemDevNames_t MapBuildSpawnItems::getFromCandidateList() {
  const int NUMBER_CANDIDATES = static_cast<int>(candidates.size());
  return candidates.at(eng->dice(1, NUMBER_CANDIDATES) - 1);
}

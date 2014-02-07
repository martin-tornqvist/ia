#include "PopulateItems.h"
#include "Engine.h"

#include "ActorPlayer.h"
#include "Map.h"
#include "ItemFactory.h"
#include "PlayerBonuses.h"
#include "MapParsing.h"

void PopulateItems::spawnItems() {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksItems(eng), blockers);
  vector<Pos> freeCells;
  eng.basicUtils->makeVectorFromBoolMap(false, blockers, freeCells);

  const int CELLS_PER_SPAWN = 135;

  int nrOfSpawns = freeCells.size() / CELLS_PER_SPAWN;
  nrOfSpawns = max(1, nrOfSpawns);
  nrOfSpawns += eng.dice(1, (nrOfSpawns / 2) + 2) - 1;

  if(eng.playerBonHandler->hasTrait(traitTreasureHunter)) {
    nrOfSpawns = (nrOfSpawns * 3) / 2;
  }

  buildCandidateList();

  //Spawn randomly from the Pos-vector
  int n = 0;
  ItemId id;

  for(int i = 0; i < nrOfSpawns; i++) {
    if(freeCells.size() > 0) {

      //Roll the dice for random element
      n = eng.dice(1, freeCells.size()) - 1;
      const Pos pos(freeCells.at(n));

      //Get type to spawn
      id = getFromCandidateList();

      //Spawn
      eng.itemFactory->spawnItemOnMap(id, pos);

      //Erase position from the vector
      freeCells.erase(freeCells.begin() + n);
    }
  }
}

void PopulateItems::buildCandidateList() {
  candidates.resize(0);

  ItemData** dataList = eng.itemDataHandler->dataList;

  const unsigned int NUMBER_DEFINED = (unsigned int)endOfItemIds;

  for(unsigned int i = 1; i < NUMBER_DEFINED; i++) {
    if(
      eng.map->getDlvl() >= dataList[i]->spawnStandardMinDLVL &&
      eng.map->getDlvl() <= dataList[i]->spawnStandardMaxDLVL &&
      dataList[i]->isIntrinsic == false) {
      if(eng.dice.percentile() < dataList[i]->chanceToIncludeInSpawnList) {
        candidates.push_back(static_cast<ItemId>(i));
      }
    }
  }
}

ItemId PopulateItems::getFromCandidateList() {
  const int NUMBER_CANDIDATES = int(candidates.size());
  return candidates.at(eng.dice(1, NUMBER_CANDIDATES) - 1);
}

#include "PopulateItems.h"

#include "ActorPlayer.h"
#include "Map.h"
#include "ItemFactory.h"
#include "PlayerBon.h"
#include "MapParsing.h"
#include "Utils.h"

namespace PopulateItems {

void spawnItems() {
  int nrOfSpawns = Rnd::range(6, 8);

  if(PlayerBon::hasTrait(Trait::treasureHunter)) {
    nrOfSpawns += 2;
  }

  vector<ItemId> candidates;
  candidates.resize(0);

  ItemDataT** dataList = ItemData::dataList;

  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    if(
      DLVL >= dataList[i]->spawnStandardMinDLVL &&
      DLVL <= dataList[i]->spawnStandardMaxDLVL &&
      dataList[i]->isIntrinsic == false) {
      if(Rnd::percentile() < dataList[i]->chanceToIncludeInSpawnList) {
        candidates.push_back(ItemId(i));
      }
    }
  }

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksItems(), blockers);
  vector<Pos> freeCells;
  Utils::makeVectorFromBoolMap(false, blockers, freeCells);

  for(int i = 0; i < nrOfSpawns; i++) {
    if(freeCells.empty()) {break;}

    const int ELEMENT       = Rnd::dice(1, freeCells.size()) - 1;
    const Pos& pos          = freeCells.at(ELEMENT);
    const int NR_CANDIDATES = int(candidates.size());
    const ItemId id         = candidates.at(Rnd::range(0, NR_CANDIDATES - 1));

    ItemFactory::spawnItemOnMap(id, pos);

    freeCells.erase(freeCells.begin() + ELEMENT);
  }
}

} //PopulateItems

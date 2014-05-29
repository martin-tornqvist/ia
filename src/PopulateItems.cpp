#include "PopulateItems.h"

#include <vector>

#include "ActorPlayer.h"
#include "Map.h"
#include "ItemFactory.h"
#include "PlayerBon.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace PopulateItems {

void mkItems() {
  int nrOfSpawns = Rnd::range(6, 8);

  if(PlayerBon::hasTrait(Trait::treasureHunter)) {
    nrOfSpawns += 2;
  }

  vector<ItemId> candidates;
  candidates.resize(0);

  ItemDataT** data = ItemData::data;

  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    if(
      Map::dlvl >= data[i]->spawnStandardMinDLVL &&
      Map::dlvl <= data[i]->spawnStandardMaxDLVL && !data[i]->isIntrinsic) {
      if(Rnd::percentile() < data[i]->chanceToIncludeInSpawnList) {
        candidates.push_back(ItemId(i));
      }
    }
  }

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksItems(), blocked);
  vector<Pos> freeCells;
  Utils::mkVectorFromBoolMap(false, blocked, freeCells);

  for(int i = 0; i < nrOfSpawns; i++) {
    if(freeCells.empty()) {break;}

    const int ELEMENT       = Rnd::dice(1, freeCells.size()) - 1;
    const Pos& pos          = freeCells.at(ELEMENT);
    const int NR_CANDIDATES = int(candidates.size());
    const ItemId id         = candidates.at(Rnd::range(0, NR_CANDIDATES - 1));

    ItemFactory::mkItemOnMap(id, pos);

    freeCells.erase(freeCells.begin() + ELEMENT);
  }
}

} //PopulateItems

#include "PopulateItems.h"

#include <vector>

#include "ActorPlayer.h"
#include "Map.h"
#include "ItemFactory.h"
#include "PlayerBon.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace PopulateItems
{

void mkItemsOnFloor()
{
    int nrSpawns = Rnd::range(6, 8);

    if (PlayerBon::traits[int(Trait::treasureHunter)])
    {
        nrSpawns += 2;
    }

    vector<ItemId> itemBucket;
    itemBucket.clear();

    for (int i = 0; i < int(ItemId::END); ++i)
    {
        const ItemDataT* const data = ItemData::data[i];

        //The following items are NOT allowed to spawn on the floor:
        // * Intrinsic items
        // * Items with a dlvl range not matching current dlvl
        // * Items forbidden to spawn (e.g. unique items already spawned)

        if (
            int(data->type) < int(ItemType::END_OF_EXTR_ITEMS)  &&
            Utils::isValInRange(Map::dlvl, data->spawnStdRange) &&
            data->allowSpawn                                    &&
            Rnd::percent(data->chanceToIncludeInFloorSpawnList))
        {
            itemBucket.push_back(ItemId(i));
        }
    }

    bool blocked[MAP_W][MAP_H];
    MapParse::run(CellCheck::BlocksItems(), blocked);
    vector<Pos> freeCells;
    Utils::mkVectorFromBoolMap(false, blocked, freeCells);

    for (int i = 0; i < nrSpawns; ++i)
    {
        if (freeCells.empty() || itemBucket.empty())
        {
            break;
        }

        const size_t  CELL_IDX        = Rnd::range(0, freeCells.size() - 1);
        const Pos&    pos             = freeCells[CELL_IDX];
        const size_t  ITEM_IDX        = Rnd::range(0, itemBucket.size() - 1);
        const ItemId  id              = itemBucket[ITEM_IDX];

        //Check if this item is no longer allowed to spawn (e.g. a unique item)
        if (!ItemData::data[int(id)]->allowSpawn)
        {
            itemBucket.erase(begin(itemBucket) + ITEM_IDX);
        }
        else
        {
            ItemFactory::mkItemOnFloor(id, pos);

            freeCells.erase(begin(freeCells) + CELL_IDX);
        }
    }
}

} //PopulateItems

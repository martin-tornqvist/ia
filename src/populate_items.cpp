#include "populate_items.h"

#include <vector>

#include "actor_player.h"
#include "map.h"
#include "item_factory.h"
#include "player_bon.h"
#include "map_parsing.h"
#include "utils.h"

using namespace std;

namespace Populate_items
{

void mk_items_on_floor()
{
    int nr_spawns = Rnd::range(6, 8);

    if (Player_bon::traits[int(Trait::treasure_hunter)])
    {
        nr_spawns += 2;
    }

    vector<Item_id> item_bucket;
    item_bucket.clear();

    for (int i = 0; i < int(Item_id::END); ++i)
    {
        const Item_data_t* const data = Item_data::data[i];

        //The following items are NOT allowed to spawn on the floor:
        // * Intrinsic items
        // * Items with a dlvl range not matching current dlvl
        // * Items forbidden to spawn (e.g. unique items already spawned)

        if (
            int(data->type) < int(Item_type::END_OF_EXTR_ITEMS)  &&
            Utils::is_val_in_range(Map::dlvl, data->spawn_std_range) &&
            data->allow_spawn                                    &&
            Rnd::percent(data->chance_to_include_in_floor_spawn_list))
        {
            item_bucket.push_back(Item_id(i));
        }
    }

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_items(), blocked);
    vector<Pos> free_cells;
    Utils::mk_vector_from_bool_map(false, blocked, free_cells);

    for (int i = 0; i < nr_spawns; ++i)
    {
        if (free_cells.empty() || item_bucket.empty())
        {
            break;
        }

        const size_t  CELL_IDX        = Rnd::range(0, free_cells.size() - 1);
        const Pos&    pos             = free_cells[CELL_IDX];
        const size_t  ITEM_IDX        = Rnd::range(0, item_bucket.size() - 1);
        const Item_id  id              = item_bucket[ITEM_IDX];

        //Check if this item is no longer allowed to spawn (e.g. a unique item)
        if (!Item_data::data[int(id)]->allow_spawn)
        {
            item_bucket.erase(begin(item_bucket) + ITEM_IDX);
        }
        else
        {
            Item_factory::mk_item_on_floor(id, pos);

            free_cells.erase(begin(free_cells) + CELL_IDX);
        }
    }
}

} //Populate_items

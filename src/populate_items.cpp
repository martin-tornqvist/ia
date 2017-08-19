#include "populate_items.hpp"

#include <vector>

#include "mapgen.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "item_factory.hpp"
#include "player_bon.hpp"
#include "map_parsing.hpp"
#include "feature_rigid.hpp"
#include "feature_door.hpp"
#include "feature_trap.hpp"

namespace populate_items
{

namespace
{

int nr_items()
{
    int nr = rnd::range(3, 5);

    if (player_bon::traits[(size_t)Trait::treasure_hunter])
    {
        nr += rnd::range(1, 2);
    }

    return nr;
}

std::vector<ItemId> mk_item_bucket()
{
    std::vector<ItemId> item_bucket;
    item_bucket.clear();

    for (int i = 0; i < (int)ItemId::END; ++i)
    {
        const ItemDataT& data = item_data::data[i];

        // The following items are NOT allowed to spawn on the floor:
        // * Intrinsic items
        // * Items with a dlvl range not matching current dlvl
        // * Items forbidden to spawn (e.g. unique items already spawned)

        if ((int)data.type < (int)ItemType::END_OF_EXTR_ITEMS &&
            data.spawn_std_range.is_in_range(map::dlvl) &&
            data.allow_spawn &&
            rnd::percent(data.chance_to_incl_in_spawn_list))
        {
            item_bucket.push_back(ItemId(i));
        }
    }

    return item_bucket;
}

void mk_blocked_map(bool out[map_w][map_h])
{
    map_parsers::BlocksItems()
        .run(out);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            // Shallow liquids doesn't block items, but let's not spawn there...
            const FeatureId id = map::cells[x][y].rigid->id();

            if (id == FeatureId::liquid_shallow)
            {
                out[x][y] = true;
            }
        }
    }

    const P& player_p = map::player->pos;

    out[player_p.x][player_p.y] = true;
}

} // namespace

void mk_items_on_floor()
{
    auto item_bucket = mk_item_bucket();

    // Spawn items with a weighted random choice

    //
    // NOTE: Each index in the position vector corresponds to the same index in
    //       the weights vector.
    //
    std::vector<P> positions;

    std::vector<int> position_weights;

    bool blocked[map_w][map_h];

    mk_blocked_map(blocked);

    mapgen::mk_explore_spawn_weights(
        blocked,
        positions,
        position_weights);

    const int nr = nr_items();

    for (int i = 0; i < nr; ++i)
    {
        if (positions.empty() || item_bucket.empty())
        {
            break;
        }

        const size_t p_idx = rnd::weighted_choice(position_weights);

        const P& p = positions[p_idx];

        const size_t item_idx = rnd::range(0, item_bucket.size() - 1);

        const ItemId id = item_bucket[item_idx];

        if (item_data::data[(size_t)id].allow_spawn)
        {
            item_factory::mk_item_on_floor(id, p);

            positions.erase(begin(positions) + p_idx);
            position_weights.erase(begin(position_weights) + p_idx);
        }
        else
        {
            item_bucket.erase(begin(item_bucket) + item_idx);
        }
    }
}

} // populate_items

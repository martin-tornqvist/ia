#include "populate_items.hpp"

#include <vector>

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
    int nr = rnd::range(7, 9);

    if (player_bon::traits[(size_t)Trait::treasure_hunter])
    {
        nr += 2;
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

        //The following items are NOT allowed to spawn on the floor:
        // * Intrinsic items
        // * Items with a dlvl range not matching current dlvl
        // * Items forbidden to spawn (e.g. unique items already spawned)

        if (
            (int)data.type < (int)ItemType::END_OF_EXTR_ITEMS       &&
            data.spawn_std_range.is_in_range(map::dlvl)             &&
            data.allow_spawn                                        &&
            rnd::percent(data.chance_to_incl_in_floor_spawn_list))
        {
            item_bucket.push_back(ItemId(i));
        }
    }

    return item_bucket;
}

void mk_position_weights(std::vector<P>& positions_out,
                         std::vector<int>& weights_out)
{
    bool blocked[map_w][map_h];
    map_parse::run(cell_check::BlocksItems(), blocked);

    int weight_map[map_w][map_h] = {};

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            //Shallow liquids doesn't block items, but let's not spawn there...
            const FeatureId id = map::cells[x][y].rigid->id();

            if (id == FeatureId::liquid_shallow)
            {
                blocked[x][y] = true;
            }

            if (!blocked[x][y])
            {
                weight_map[x][y] = 1;

                //Increase weight for dark cells
                if (map::cells[x][y].is_dark)
                {
                    weight_map[x][y] += 10;
                }
            }
        }
    }

    //Put extra weight for positions behind choke points
    for (const auto& choke_point : map::choke_point_data)
    {
        //If the player and the stairs are on the same side of the choke point,
        //this means that the "other" side is an optional map branch.
        if (choke_point.player_side == choke_point.stairs_side)
        {
            ASSERT(choke_point.player_side == 0 ||
                   choke_point.player_side == 1);

            //Robustness for release mode
            if (
                choke_point.player_side != 0 &&
                choke_point.player_side != 1)
            {
                continue;
            }

            const int other_side_idx = choke_point.player_side == 0 ? 1 : 0;

            const auto& other_side_positions = choke_point.sides[other_side_idx];

            //NOTE: To avoid leaning heavily towards only putting items in big
            //      hidden areas (it looks more nice and "natural" with small
            //      hidden rooms filled with items), we divide the weight given
            //      per cell based on the total number of cells in the area.

            const int weight_div = other_side_positions.size() / 2;

            //Increase weight for being in an optional map branch
            int weight_inc = std::max(1, (200 / weight_div));

            Rigid* const rigid = map::cells[choke_point.p.x][choke_point.p.y].rigid;

            //Increase weight if behind a hidden door.
            if (rigid->id() == FeatureId::door)
            {
                Door* const door = static_cast<Door*>(rigid);

                if (door->is_secret())
                {
                    weight_inc += std::max(1, (200 / weight_div));
                }

                //Also increase weight a bit if the door is stuck
                if (door->is_stuck())
                {
                    weight_inc += std::max(1, (200 / weight_div));
                }
            }

            for (const P& p : other_side_positions)
            {
                weight_map[p.x][p.y] += weight_inc;
            }
        }
    }

    //Prepare for at least worst case of push-backs
    positions_out.reserve(nr_map_cells);
    weights_out.reserve(nr_map_cells);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            const int weight = weight_map[x][y];

            if (weight > 0)
            {
                //OK, we can spawn here, save the position and the corresponding
                //spawn chance weight
                positions_out.push_back(P(x, y));
                weights_out.push_back(weight);
            }
        }
    }

    // std::vector<P> free_cells;

    // to_vec((bool*)blocked,
    //        false,
    //        map_w,
    //        map_h,
    //        free_cells);
    
}

}

void mk_items_on_floor()
{
    auto item_bucket = mk_item_bucket();

    //Spawn items with a weighted random choice
    //NOTE: Each index in the position vector corresponds to the same index in
    //      the weights vector.
    std::vector<P>      positions;
    std::vector<int>    position_weights;

    mk_position_weights(positions, position_weights);

    const int nr = nr_items();

    for (int i = 0; i < nr; ++i)
    {
        if (positions.empty() || item_bucket.empty())
        {
            break;
        }

        const size_t    p_idx       = rnd::weighted_choice(position_weights);
        const P&        p           = positions[p_idx];
        const size_t    item_idx    = rnd::range(0, item_bucket.size() - 1);
        const ItemId    id          = item_bucket[item_idx];

        if (item_data::data[size_t(id)].allow_spawn)
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

} //populate_items

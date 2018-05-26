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

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static int nr_items()
{
        int nr = rnd::range(4, 5);

        if (player_bon::traits[(size_t)Trait::treasure_hunter])
        {
                nr += rnd::range(1, 2);
        }

        return nr;
}

static std::vector<ItemId> make_item_bucket()
{
        std::vector<ItemId> item_bucket;
        item_bucket.clear();

        for (int i = 0; i < (int)ItemId::END; ++i)
        {
                const ItemData& data = item_data::data[i];

                if (data.type < ItemType::END_OF_EXTRINSIC_ITEMS &&
                    data.spawn_std_range.is_in_range(map::dlvl) &&
                    data.allow_spawn &&
                    rnd::percent(data.chance_to_incl_in_spawn_list))
                {
                        item_bucket.push_back(ItemId(i));
                }
        }

        return item_bucket;
}

static Array2<bool> make_blocked_map()
{
        Array2<bool> result(map::dims());

        map_parsers::BlocksItems()
                .run(result, result.rect());

        for (size_t i = 0; i < map::nr_cells(); ++i)
        {
                // Shallow liquids doesn't block items, but let's not
                // spawn there...
                const FeatureId id = map::cells.at(i).rigid->id();

                if (id == FeatureId::liquid_shallow)
                {
                        result.at(i) = true;
                }
        }

        const P& player_p = map::player->pos;

        result.at(player_p) = true;

        return result;
}

// -----------------------------------------------------------------------------
// populate_items
// -----------------------------------------------------------------------------
namespace populate_items
{

void make_items_on_floor()
{
        auto item_bucket = make_item_bucket();

        // Spawn items with a weighted random choice

        // NOTE: Each index in the position vector corresponds to the same index
        // in the weights vector.
        std::vector<P> positions;

        std::vector<int> position_weights;

        const auto blocked = make_blocked_map();

        mapgen::make_explore_spawn_weights(
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
                        item_factory::make_item_on_floor(id, p);

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

#include "jam_with_spike.hpp"

#include "game_time.hpp"
#include "feature.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "log.hpp"
#include "map.hpp"
#include "query.hpp"
#include "inventory.hpp"
#include "render.hpp"
#include "feature_door.hpp"
#include "utils.hpp"

namespace Jam_with_spike
{

void player_jam_feature(Feature* const feature)
{
    bool jamable_object_found = false;

    if (feature->get_id() == Feature_id::door)
    {
        Door* const door = static_cast<Door*>(feature);
        const bool DOOR_SPIKED = door->try_spike(Map::player);

        if (DOOR_SPIKED)
        {

            jamable_object_found = true;

            Map::player->get_inv().decr_item_type_in_general(Item_id::iron_spike);
            const int SPIKES_LEFT =
                Map::player->get_inv().get_item_stack_size_in_general(Item_id::iron_spike);
            if (SPIKES_LEFT == 0)
            {
                Log::add_msg("I have no iron spikes left.");
            }
            else
            {
                Log::add_msg("I have " + to_str(SPIKES_LEFT) + " iron spikes left.");
            }
        }
    }

    if (!jamable_object_found)
    {
        const bool PLAYER_IS_BLIND = Map::player->get_prop_handler().allow_see();
        if (!PLAYER_IS_BLIND)
        {
            Log::add_msg("I see nothing there to jam with a spike.");
        }
        else
        {
            Log::add_msg("I find nothing there to jam with a spike.");
        }
    }
}

void player_jam()
{
    Log::clear_log();

    if (!Map::player->get_inv().has_item_in_backpack(Item_id::iron_spike))
    {
        Log::add_msg("I have no spikes to jam with.", clr_white);
        Render::draw_map_and_interface();
        return;
    }

    Log::add_msg("Which direction?" + cancel_info_str, clr_white_high);
    Render::draw_map_and_interface();
    const Pos jam_pos(Map::player->pos + Dir_utils::get_offset(Query::dir()));
    Log::clear_log();

    player_jam_feature(Map::cells[jam_pos.x][jam_pos.y].rigid);

    Render::draw_map_and_interface();
}

} //Jam_with_spike

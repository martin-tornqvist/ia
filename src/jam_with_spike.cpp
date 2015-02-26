#include "jam_with_spike.hpp"

#include "game_time.hpp"
#include "feature.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "query.hpp"
#include "inventory.hpp"
#include "render.hpp"
#include "feature_door.hpp"
#include "utils.hpp"

namespace jam_with_spike
{

void player_jam_feature(Feature* const feature)
{
    bool jamable_object_found = false;

    if (feature->get_id() == Feature_id::door)
    {
        Door* const door = static_cast<Door*>(feature);
        const bool DOOR_SPIKED = door->try_spike(map::player);

        if (DOOR_SPIKED)
        {

            jamable_object_found = true;

            map::player->get_inv().decr_item_type_in_general(Item_id::iron_spike);
            const int SPIKES_LEFT =
                map::player->get_inv().get_item_stack_size_in_general(Item_id::iron_spike);

            if (SPIKES_LEFT == 0)
            {
                msg_log::add("I have no iron spikes left.");
            }
            else
            {
                msg_log::add("I have " + to_str(SPIKES_LEFT) + " iron spikes left.");
            }
        }
    }

    if (!jamable_object_found)
    {
        const bool PLAYER_IS_BLIND = map::player->get_prop_handler().allow_see();

        if (!PLAYER_IS_BLIND)
        {
            msg_log::add("I see nothing there to jam with a spike.");
        }
        else
        {
            msg_log::add("I find nothing there to jam with a spike.");
        }
    }
}

void player_jam()
{
    msg_log::clear();

    if (!map::player->get_inv().has_item_in_backpack(Item_id::iron_spike))
    {
        msg_log::add("I have no spikes to jam with.", clr_white);
        render::draw_map_and_interface();
        return;
    }

    msg_log::add("Which direction?" + cancel_info_str, clr_white_high);
    render::draw_map_and_interface();
    const Pos jam_pos(map::player->pos + dir_utils::get_offset(query::dir()));
    msg_log::clear();

    player_jam_feature(map::cells[jam_pos.x][jam_pos.y].rigid);

    render::draw_map_and_interface();
}

} //Jam_with_spike

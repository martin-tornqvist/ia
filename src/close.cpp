#include "close.hpp"

#include "game_time.hpp"
#include "feature.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "feature_door.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "render.hpp"

namespace close_door
{

namespace
{

void player_try_close_or_jam_feature(Feature* const feature)
{
    if (feature->id() != Feature_id::door)
    {
        const bool player_can_see = map::player->prop_handler().allow_see();

        if (player_can_see)
        {
            msg_log::add("I see nothing there to close or jam.");
        }
        else //Player cannot see
        {
            msg_log::add("I find nothing there to close or jam.");
        }

        return;
    }

    //This point reached means it's a door

    Door* const door = static_cast<Door*>(feature);

    if (door->is_open())
    {
        //Door is open, try to close it
        door->try_close(map::player);
    }
    else //Door is closed - try to jam it
    {
        const bool has_spike = map::player->inv().has_item_in_backpack(Item_id::iron_spike);

        if (has_spike)
        {
            const bool did_spike_door = door->try_jam(map::player);

            if (did_spike_door)
            {
                map::player->inv().decr_item_type_in_backpack(Item_id::iron_spike);

                const int spikes_left_after =
                    map::player->inv().item_stack_size_in_backpack(Item_id::iron_spike);

                if (spikes_left_after == 0)
                {
                    msg_log::add("I have no iron spikes left.");
                }
                else //Has spikes left
                {
                    msg_log::add("I have " + to_str(spikes_left_after) + " iron spikes left.");
                }
            }
        }
        else //Has no spikes to jam with
        {
            msg_log::add("I have nothing to jam the door with.");
        }
    }
}

} //namespace

void player_try_close_or_jam()
{
    msg_log::clear();

    msg_log::add("Which direction?" + cancel_info_str, clr_white_high);

    render::draw_map_state();

    const Dir input_dir = query::dir(Allow_center::no);

    msg_log::clear();

    if (input_dir != Dir::END && input_dir != Dir::center)
    {
        //Valid direction
        const P p(map::player->pos + dir_utils::offset(input_dir));

        player_try_close_or_jam_feature(map::cells[p.x][p.y].rigid);
    }

    render::draw_map_state();
}

} //close

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
#include "utils.hpp"

namespace close
{

void player_close_feature(Feature* const feature)
{
    bool close_able_object_found = false;

    if (feature->id() == Feature_id::door)
    {
        Door* const door = static_cast<Door*>(feature);
        door->try_close(map::player);
        close_able_object_found = true;
    }

    if (!close_able_object_found)
    {
        const bool PLAYER_CAN_SEE = map::player->prop_handler().allow_see();

        if (PLAYER_CAN_SEE)
        {
            msg_log::add("I see nothing there to close.");
        }
        else
        {
            msg_log::add("I find nothing there to close.");
        }
    }
}

void player_close()
{
    msg_log::clear();
    msg_log::add("Which direction?" + cancel_info_str, clr_white_high);
    render::draw_map_and_interface();
    Pos close_pos(map::player->pos + dir_utils::offset(query::dir()));
    msg_log::clear();

    if (close_pos != map::player->pos)
    {
        player_close_feature(map::cells[close_pos.x][close_pos.y].rigid);
    }

    render::draw_map_and_interface();
}

} //Close

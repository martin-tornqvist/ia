#include "close.h"

#include "game_time.h"
#include "feature.h"
#include "actor.h"
#include "actor_player.h"
#include "feature_door.h"
#include "map.h"
#include "log.h"
#include "query.h"
#include "render.h"
#include "utils.h"

namespace Close
{

void player_close_feature(Feature* const feature)
{
    bool close_able_object_found = false;

    if (feature->get_id() == Feature_id::door)
    {
        Door* const door = static_cast<Door*>(feature);
        door->try_close(Map::player);
        close_able_object_found = true;
    }

    if (!close_able_object_found)
    {
        const bool PLAYER_CAN_SEE = Map::player->get_prop_handler().allow_see();
        if (PLAYER_CAN_SEE)
        {
            Log::add_msg("I see nothing there to close.");
        }
        else
        {
            Log::add_msg("I find nothing there to close.");
        }
    }
}

void player_close()
{
    Log::clear_log();
    Log::add_msg("Which direction?" + cancel_info_str, clr_white_high);
    Render::draw_map_and_interface();
    Pos close_pos(Map::player->pos + Dir_utils::get_offset(Query::dir()));
    Log::clear_log();

    if (close_pos != Map::player->pos)
    {
        player_close_feature(Map::cells[close_pos.x][close_pos.y].rigid);
    }

    Render::draw_map_and_interface();
}

} //Close

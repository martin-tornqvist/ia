#include "disarm.hpp"

#include "game_time.hpp"
#include "msg_log.hpp"
#include "render.hpp"
#include "actor_player.hpp"
#include "query.hpp"
#include "map.hpp"
#include "feature_trap.hpp"
#include "utils.hpp"
#include "inventory.hpp"

namespace disarm
{

void player_disarm()
{
    //TODO: It would probably be more fun if examine were allowed while blind,
    //with some potentially horrible results

    //Abort if blind
    if (!map::player->get_prop_handler().allow_see())
    {
        msg_log::add("Not while blind.");
        render::draw_map_and_interface();
        return;
    }

    //Abort if held by spider web
    const Pos player_pos = map::player->pos;
    const auto* const feature_at_player =
        map::cells[player_pos.x][player_pos.y].rigid;
    if (feature_at_player->get_id() == Feature_id::trap)
    {
        const Trap* const trap = static_cast<const Trap*>(feature_at_player);
        if (trap->get_trap_type() == Trap_id::web)
        {
            const auto* const web = static_cast<const Trap_web*>(trap->get_specific_trap());
            if (web->is_holding())
            {
                msg_log::add("Not while entangled in a spider web.");
                render::draw_map_and_interface();
                return;
            }
        }
    }

    //Abort if encumbered
    if (map::player->get_enc_percent() >= 100)
    {
        msg_log::add("Not while encumbered.");
        render::draw_map_and_interface();
        return;
    }

    msg_log::add("Which direction?" + cancel_info_str, clr_white_high);
    render::draw_map_and_interface();

    const Pos pos(map::player->pos + dir_utils::get_offset(query::dir()));

    if (pos == map::player->pos)
    {
        msg_log::clear();
        render::draw_map_and_interface();
    }
    else
    {
        //Abort if cell is unseen
        if (!map::cells[pos.x][pos.y].is_seen_by_player)
        {
            msg_log::add("I cannot see there.");
            render::draw_map_and_interface();
            return;
        }

        msg_log::clear();

        Actor* actor_on_trap = utils::get_actor_at_pos(pos);

        //Abort if trap blocked by monster
        if (actor_on_trap)
        {
            if (map::player->can_see_actor(*actor_on_trap, nullptr))
            {
                msg_log::add("It's blocked.");
            }
            else
            {
                msg_log::add("Something is blocking it.");
            }
        }
        else //No actor on the trap
        {
            map::cells[pos.x][pos.y].rigid->disarm();
        }

        render::draw_map_and_interface();
    }
}

} //Disarm


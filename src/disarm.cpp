#include "disarm.hpp"

#include "game_time.hpp"
#include "log.hpp"
#include "render.hpp"
#include "actor_player.hpp"
#include "query.hpp"
#include "map.hpp"
#include "feature_trap.hpp"
#include "utils.hpp"
#include "inventory.hpp"

namespace Disarm
{

void player_disarm()
{
    //TODO: It would probably be more fun if examine were allowed while blind,
    //with some potentially horrible results

    //Abort if blind
    if (!Map::player->get_prop_handler().allow_see())
    {
        Log::add_msg("Not while blind.");
        Render::draw_map_and_interface();
        return;
    }

    //Abort if held by spider web
    const Pos player_pos = Map::player->pos;
    const auto* const feature_at_player =
        Map::cells[player_pos.x][player_pos.y].rigid;
    if (feature_at_player->get_id() == Feature_id::trap)
    {
        const Trap* const trap = static_cast<const Trap*>(feature_at_player);
        if (trap->get_trap_type() == Trap_id::web)
        {
            const auto* const web = static_cast<const Trap_web*>(trap->get_specific_trap());
            if (web->is_holding())
            {
                Log::add_msg("Not while entangled in a spider web.");
                Render::draw_map_and_interface();
                return;
            }
        }
    }

    //Abort if encumbered
    if (Map::player->get_enc_percent() >= 100)
    {
        Log::add_msg("Not while encumbered.");
        Render::draw_map_and_interface();
        return;
    }

    Log::add_msg("Which direction?" + cancel_info_str, clr_white_high);
    Render::draw_map_and_interface();

    const Pos pos(Map::player->pos + Dir_utils::get_offset(Query::dir()));

    if (pos == Map::player->pos)
    {
        Log::clear_log();
        Render::draw_map_and_interface();
    }
    else
    {
        //Abort if cell is unseen
        if (!Map::cells[pos.x][pos.y].is_seen_by_player)
        {
            Log::add_msg("I cannot see there.");
            Render::draw_map_and_interface();
            return;
        }

        Log::clear_log();

        Actor* actor_on_trap = Utils::get_actor_at_pos(pos);

        //Abort if trap blocked by monster
        if (actor_on_trap)
        {
            if (Map::player->can_see_actor(*actor_on_trap, nullptr))
            {
                Log::add_msg("It's blocked.");
            }
            else
            {
                Log::add_msg("Something is blocking it.");
            }
        }
        else //No actor on the trap
        {
            Map::cells[pos.x][pos.y].rigid->disarm();
        }

        Render::draw_map_and_interface();
    }
}

} //Disarm


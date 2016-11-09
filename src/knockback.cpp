#include "knockback.hpp"

#include <algorithm>
#include <vector>

#include "attack.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "config.hpp"
#include "game_time.hpp"
#include "io.hpp"
#include "map_parsing.hpp"
#include "sdl_base.hpp"
#include "feature_rigid.hpp"
#include "feature_trap.hpp"
#include "feature_mob.hpp"

namespace knock_back
{

void try_knock_back(Actor& defender,
                    const P& attacked_from_pos,
                    const bool is_spike_gun,
                    const bool is_msg_allowed)
{
    TRACE_FUNC_BEGIN;

    const bool is_defender_player = defender.is_player();
    const auto& defender_data = defender.data();

    if (defender_data.prevent_knockback ||
        defender_data.actor_size >= ActorSize::giant ||
        defender.has_prop(PropId::ethereal) ||
        defender.has_prop(PropId::ooze) ||
        // Do not knock back player if bot is playing
        (is_defender_player && config::is_bot_playing()))
    {
        // Defender is not knockable

        TRACE_FUNC_END;
        return;
    }

    //Check if actor is held by a trap
    Rigid* const rigid = map::cells[defender.pos.x][defender.pos.y].rigid;

    if (rigid->id() == FeatureId::trap)
    {
        Trap* const trap = static_cast<Trap*>(rigid);

        if (trap->is_holding_actor())
        {
            if (trap->type() == TrapId::web)
            {
                //Held by a web, just destroy the web
                trap->destroy();
            }
            else //Not a web
            {
                //Held by some other trap, prevent knockback
                TRACE_FUNC_END;
                return;
            }
        }
    }

    const P d = (defender.pos - attacked_from_pos).signs();

    const P new_pos = defender.pos + d;

    bool blocked[map_w][map_h];

    map_parsers::BlocksActor(defender, ParseActors::yes)
        .run(blocked);

    const bool is_cell_bottomless =
        map::cells[new_pos.x][new_pos.y].rigid->is_bottomless();

    const bool is_cell_blocked =
        blocked[new_pos.x][new_pos.y] && !is_cell_bottomless;

    if (is_cell_blocked)
    {
        //Defender nailed to a wall from a spike gun?
        if (is_spike_gun)
        {
            Rigid* const f = map::cells[new_pos.x][new_pos.y].rigid;

            if (!f->is_los_passable())
            {
                defender.prop_handler().try_add(
                    new PropNailed(PropTurns::indefinite));
            }
        }

        TRACE_FUNC_END;
        return;
    }
    else //Target cell is free
    {
        const bool player_see_defender = is_defender_player ?
                                         true :
                                         map::player->can_see_actor(defender);

        if (is_msg_allowed && player_see_defender)
        {
            if (is_defender_player)
            {
                msg_log::add("I am knocked back!");
            }
            else
            {
                msg_log::add(defender.name_the() + " is knocked back!");
            }
        }

        defender.prop_handler().try_add(
            new PropParalyzed(PropTurns::specific, 1));

        defender.pos = new_pos;

        if (is_cell_bottomless &&
            !defender.has_prop(PropId::flying)  &&
            player_see_defender)
        {
            if (is_defender_player)
            {
                msg_log::add("I plummet down the depths!", clr_msg_bad);
            }
            else
            {
                msg_log::add(defender.name_the() + " plummets down the depths.",
                             clr_msg_good);
            }

            defender.die(true, false, false);

            TRACE_FUNC_END;
            return;
        }

        //Bump features (e.g. so monsters can be knocked back into traps)
        std::vector<Mob*> mobs;

        game_time::mobs_at_pos(defender.pos, mobs);

        for (Mob* const mob : mobs)
        {
            mob->bump(defender);
        }

        if (!defender.is_alive())
        {
            TRACE_FUNC_END;
            return;
        }

        Rigid* const f = map::cells[defender.pos.x][defender.pos.y].rigid;

        f->bump(defender);

        if (!defender.is_alive())
        {
            TRACE_FUNC_END;
            return;
        }
    }

    TRACE_FUNC_END;
}

} // knockBack

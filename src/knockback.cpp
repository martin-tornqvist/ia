#include "knockback.hpp"

#include <algorithm>
#include <vector>

#include "attack.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "config.hpp"
#include "game_time.hpp"
#include "render.hpp"
#include "map_parsing.hpp"
#include "sdl_wrapper.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"


namespace knock_back
{

void try_knock_back(Actor& defender,
                    const P& attacked_from_pos,
                    const bool IS_SPIKE_GUN,
                    const bool IS_MSG_ALLOWED)
{
    TRACE_FUNC_BEGIN;

    const bool  IS_DEFENDER_PLAYER  = defender.is_player();
    const auto& defender_data       = defender.data();

    if (
        defender_data.prevent_knockback                 ||
        defender_data.actor_size >= Actor_size::giant   ||
        defender.has_prop(Prop_id::ethereal)            ||
        defender.has_prop(Prop_id::ooze)                ||
        //Do not knock back if bot is playing
        (IS_DEFENDER_PLAYER && config::is_bot_playing()))
    {
        //Defender is not knockable

        TRACE_FUNC_END;
        return;
    }

    const P d = (defender.pos - attacked_from_pos).signs();

    const int KNOCK_RANGE = 2;

    for (int i = 0; i < KNOCK_RANGE; ++i)
    {
        const P new_pos = defender.pos + d;

        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_actor(defender, true), blocked);

        const bool IS_CELL_BOTTOMLESS =
            map::cells[new_pos.x][new_pos.y].rigid->is_bottomless();

        const bool IS_CELL_BLOCKED =
            blocked[new_pos.x][new_pos.y] && !IS_CELL_BOTTOMLESS;

        if (IS_CELL_BLOCKED)
        {
            //Defender nailed to a wall from a spike gun?
            if (IS_SPIKE_GUN)
            {
                Rigid* const f = map::cells[new_pos.x][new_pos.y].rigid;

                if (!f->is_los_passable())
                {
                    defender.prop_handler().try_add(
                        new Prop_nailed(Prop_turns::indefinite));
                }
            }

            TRACE_FUNC_END;
            return;
        }
        else //Target cell is free
        {
            const bool PLAYER_SEE_DEFENDER = IS_DEFENDER_PLAYER ?
                                             true :
                                             map::player->can_see_actor(defender);

            if (i == 0)
            {
                if (IS_MSG_ALLOWED && PLAYER_SEE_DEFENDER)
                {
                    if (IS_DEFENDER_PLAYER)
                    {
                        msg_log::add("I am knocked back!");
                    }
                    else
                    {
                        msg_log::add(defender.name_the() + " is knocked back!");
                    }
                }

                defender.prop_handler().try_add(
                    new Prop_paralyzed(Prop_turns::specific, 1));
            }

            defender.pos = new_pos;

            if (i == KNOCK_RANGE - 1)
            {
                render::draw_map_state();
                sdl_wrapper::sleep(config::delay_projectile_draw());
            }

            if (
                IS_CELL_BOTTOMLESS                  &&
                !defender.has_prop(Prop_id::flying) &&
                PLAYER_SEE_DEFENDER)
            {
                if (IS_DEFENDER_PLAYER)
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
    }

    TRACE_FUNC_END;
}

} //Knock_back

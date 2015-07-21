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

using namespace std;

namespace knock_back
{

void try_knock_back(Actor&        defender,
                    const Pos&    attacked_from_pos,
                    const bool    IS_SPIKE_GUN,
                    const bool    IS_MSG_ALLOWED)
{
    const bool  IS_DEF_MON    = !defender.is_player();
    const auto& defender_data  = defender.data();

    bool props[size_t(Prop_id::END)];
    defender.prop_handler().prop_ids(props);

    if (
        defender_data.prevent_knockback               ||
        defender_data.actor_size >= Actor_size::giant  ||
        props[int(Prop_id::ethereal)]                ||
        props[int(Prop_id::ooze)]                    ||
        /*Do not knock back if bot is playing*/
        (!IS_DEF_MON && config::is_bot_playing()))
    {
        //Defender is not knockable
        return;
    }

    const Pos d = (defender.pos - attacked_from_pos).signs();

    const int KNOCK_RANGE = 2;

    for (int i = 0; i < KNOCK_RANGE; ++i)
    {
        const Pos new_pos = defender.pos + d;

        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_actor(defender, true), blocked);

        const bool IS_CELL_BOTTOMLESS =
            map::cells[new_pos.x][new_pos.y].rigid->is_bottomless();

        const bool IS_CELL_BLOCKED    =
            blocked[new_pos.x][new_pos.y] && !IS_CELL_BOTTOMLESS;

        if (IS_CELL_BLOCKED)
        {
            //Defender nailed to a wall from a spike gun?
            if (IS_SPIKE_GUN)
            {
                Rigid* const f = map::cells[new_pos.x][new_pos.y].rigid;

                if (!f->is_los_passable())
                {
                    defender.prop_handler().try_apply_prop(
                        new Prop_nailed(Prop_turns::indefinite));
                }
            }

            return;
        }
        else //Target cell is free
        {
            const bool IS_PLAYER_SEE_DEF = IS_DEF_MON ?
                                           map::player->can_see_actor(defender, nullptr) :
                                           true;

            if (i == 0)
            {
                if (IS_MSG_ALLOWED)
                {
                    if (IS_DEF_MON && IS_PLAYER_SEE_DEF)
                    {
                        msg_log::add(defender.name_the() + " is knocked back!");
                    }
                    else
                    {
                        msg_log::add("I am knocked back!");
                    }
                }

                defender.prop_handler().try_apply_prop(
                    new Prop_paralyzed(Prop_turns::specific, 1), false, Verbosity::verbose);
            }

            defender.pos = new_pos;

            if (i == KNOCK_RANGE - 1)
            {
                render::draw_map_and_interface();
                sdl_wrapper::sleep(config::delay_projectile_draw());
            }

            if (IS_CELL_BOTTOMLESS && !props[int(Prop_id::flying)])
            {
                if (IS_DEF_MON && IS_PLAYER_SEE_DEF)
                {
                    msg_log::add(defender.name_the() + " plummets down the depths.",
                                 clr_msg_good);
                }
                else
                {
                    msg_log::add("I plummet down the depths!", clr_msg_bad);
                }

                defender.die(true, false, false);
                return;
            }

            // Bump features (e.g. so monsters can be knocked back into traps)
            vector<Mob*> mobs;
            game_time::mobs_at_pos(defender.pos, mobs);

            for (Mob* const mob : mobs)
            {
                mob->bump(defender);
            }

            if (!defender.is_alive())
            {
                return;
            }

            Rigid* const f = map::cells[defender.pos.x][defender.pos.y].rigid;
            f->bump(defender);

            if (!defender.is_alive())
            {
                return;
            }
        }
    }
}

} //Knock_back

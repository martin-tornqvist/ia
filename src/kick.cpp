#include "kick.hpp"

#include "init.hpp"
#include "game_time.hpp"
#include "feature_rigid.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "io.hpp"
#include "map_parsing.hpp"
#include "text_format.hpp"
#include "item_factory.hpp"

namespace kick
{

void player_kick()
{
    TRACE_FUNC_BEGIN;

    msg_log::clear();

    msg_log::add("Which direction?" + cancel_info_str,
                 clr_white_high);

    const Dir input_dir = query::dir(AllowCenter::yes);

    msg_log::clear();

    if (input_dir == Dir::END)
    {
        //Invalid direction
        io::update_screen();
    }
    else //Valid direction
    {
        P kick_pos(map::player->pos + dir_utils::offset(input_dir));

        TRACE << "Checking if player is kicking a living actor" << std::endl;
        if (input_dir != Dir::center)
        {
            Actor* living_actor =
                map::actor_at_pos(kick_pos, ActorState::alive);

            if (living_actor)
            {
                TRACE << "Actor found at kick pos, "
                      << "attempting to kick actor" << std::endl;

                const bool allowed =
                    map::player->prop_handler().allow_attack_melee(
                        Verbosity::verbose);

                if (allowed)
                {
                    TRACE << "Player is allowed to do melee attack"
                          << std::endl;

                    bool blocked[map_w][map_h];

                    map_parsers::BlocksLos()
                        .run(blocked);

                    TRACE << "Player can see actor" << std::endl;
                    map::player->kick_mon(*living_actor);
                }

                TRACE_FUNC_END;
                return;
            }
        }

        TRACE << "Checking if player is kicking a corpse" << std::endl;
        Actor* corpse = nullptr;

        // Check all corpses here, stop at any corpse which is prioritized for
        // bashing (Zombies)
        for (Actor* const actor : game_time::actors)
        {
            if (actor->pos == kick_pos && actor->state() == ActorState::corpse)
            {
                corpse = actor;

                if (actor->data().prio_corpse_bash)
                {
                    break;
                }
            }
        }

        if (corpse)
        {
            const bool is_seeing_cell =
                map::cells[kick_pos.x][kick_pos.y].is_seen_by_player;

            std::string corpse_name =
                is_seeing_cell ?
                corpse->corpse_name_a() : "a corpse";

            text_format::first_to_upper(corpse_name);

            msg_log::add("I bash " + corpse_name + ".");

            Item* kick_item = item_factory::mk(ItemId::player_kick);

            const DiceParam kick_dmg_dice =
                kick_item->dmg(AttMode::melee, map::player);

            delete kick_item;

            const int kick_dmg = kick_dmg_dice.roll();

            corpse->hit(kick_dmg, DmgType::physical, DmgMethod::kick);

            game_time::tick();
            TRACE_FUNC_END;
            return;
        }

        // Kick feature
        TRACE << "Checking if player is kicking a feature" << std::endl;

        if (input_dir != Dir::center)
        {
            auto* const f = map::cells[kick_pos.x][kick_pos.y].rigid;

            f->hit(DmgType::physical, DmgMethod::kick, map::player);
        }
    }

    TRACE_FUNC_END;
}

} //kick

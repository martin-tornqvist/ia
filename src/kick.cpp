#include "kick.hpp"

#include "init.hpp"
#include "game_time.hpp"
#include "feature_rigid.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "render.hpp"
#include "map_parsing.hpp"
#include "utils.hpp"
#include "text_format.hpp"
#include "item_factory.hpp"

namespace kick
{

void player_kick()
{
    TRACE_FUNC_BEGIN;

    msg_log::clear();
    msg_log::add("Which direction?" + cancel_info_str, clr_white_high);

    render::draw_map_and_interface();

    const Dir input_dir = query::dir(Allow_center::yes);

    msg_log::clear();

    if (input_dir == Dir::END)
    {
        //Invalid direction
        render::update_screen();
    }
    else //Valid direction
    {
        P kick_pos(map::player->pos + dir_utils::offset(input_dir));

        TRACE << "Checking if player is kicking a living actor" << std::endl;
        if (input_dir != Dir::center)
        {
            Actor* living_actor = utils::actor_at_pos(kick_pos, Actor_state::alive);

            if (living_actor)
            {
                TRACE << "Actor found at kick pos, attempting to kick actor" << std::endl;

                if (map::player->prop_handler().allow_attack_melee(Verbosity::verbose))
                {
                    TRACE << "Player is allowed to do melee attack" << std::endl;
                    bool blocked[MAP_W][MAP_H];
                    map_parse::run(cell_check::Blocks_los(), blocked);

                    TRACE << "Player can see actor" << std::endl;
                    map::player->kick_mon(*living_actor);
                }

                TRACE_FUNC_END;
                return;
            }
        }

        TRACE << "Checking if player is kicking a corpse" << std::endl;
        Actor* corpse = nullptr;

        //Check all corpses here, stop at any corpse which is prioritized for bashing (Zombies)
        for (Actor* const actor : game_time::actors)
        {
            if (actor->pos == kick_pos && actor->state() == Actor_state::corpse)
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
            const bool IS_SEEING_CELL = map::cells[kick_pos.x][kick_pos.y].is_seen_by_player;

            std::string corpse_name = IS_SEEING_CELL ?
                                      corpse->corpse_name_a() : "a corpse";

            text_format::first_to_upper(corpse_name);

            msg_log::add("I bash " + corpse_name + ".");

            Item* kick_item = item_factory::mk(Item_id::player_kick);

            const Dice_param kick_dmg_dice = kick_item->dmg(Att_mode::melee, map::player);

            delete kick_item;

            const int KICK_DMG = kick_dmg_dice.roll();

            corpse->hit(KICK_DMG, Dmg_type::physical, Dmg_method::kick);

            game_time::tick();
            TRACE_FUNC_END;
            return;
        }

        //Kick feature
        TRACE << "Checking if player is kicking a feature" << std::endl;

        if (input_dir != Dir::center)
        {
            auto* const f = map::cells[kick_pos.x][kick_pos.y].rigid;
            f->hit(Dmg_type::physical, Dmg_method::kick, map::player);
        }
    }

    TRACE_FUNC_END;
}

} //kick

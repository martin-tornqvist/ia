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
    Pos kick_pos(map::player->pos + dir_utils::offset(query::dir()));
    msg_log::clear();

    if (kick_pos != map::player->pos)
    {
        //Kick living actor?
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

        //Kick corpse?
        Actor* dead_actor = utils::actor_at_pos(kick_pos, Actor_state::corpse);

        if (dead_actor)
        {
            const bool IS_SEEING_CELL = map::cells[kick_pos.x][kick_pos.y].is_seen_by_player;

            std::string corpse_name = IS_SEEING_CELL ?
                                      dead_actor->corpse_name_a() : "a corpse";

            text_format::first_to_upper(corpse_name);

            msg_log::add("I bash " + corpse_name + ".");

            Item* kick_item = item_factory::mk(Item_id::player_kick);

            const Dice_param kick_dmg_dice = kick_item->dmg(Att_mode::melee, map::player);

            delete kick_item;

            const int KICK_DMG = rnd::dice(kick_dmg_dice);

            dead_actor->hit(KICK_DMG, Dmg_type::physical, Dmg_method::kick);

            game_time::tick();
            TRACE_FUNC_END;
            return;
        }

        //Kick feature
        TRACE << "No actor at kick pos, attempting to kick feature instead" << std::endl;
        auto* const f = map::cells[kick_pos.x][kick_pos.y].rigid;
        f->hit(Dmg_type::physical, Dmg_method::kick, map::player);
    }

    TRACE_FUNC_END;
}

} //Kick

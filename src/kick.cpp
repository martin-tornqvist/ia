#include "Kick.h"

#include "Init.h"
#include "Game_time.h"
#include "Feature_rigid.h"
#include "Actor_player.h"
#include "Map.h"
#include "Log.h"
#include "Query.h"
#include "Render.h"
#include "Map_parsing.h"
#include "Utils.h"
#include "Text_format.h"

using namespace std;

namespace Kick
{

void player_kick()
{
    TRACE_FUNC_BEGIN;

    Log::clear_log();
    Log::add_msg("Which direction?" + cancel_info_str, clr_white_high);
    Render::draw_map_and_interface();
    Pos kick_pos(Map::player->pos + Dir_utils::get_offset(Query::dir()));
    Log::clear_log();

    if (kick_pos != Map::player->pos)
    {
        //Kick living actor?
        Actor* living_actor = Utils::get_actor_at_pos(kick_pos, Actor_state::alive);
        if (living_actor)
        {
            TRACE << "Actor found at kick pos, attempting to kick actor" << endl;
            if (Map::player->get_prop_handler().allow_attack_melee(true))
            {
                TRACE << "Player is allowed to do melee attack" << endl;
                bool blocked[MAP_W][MAP_H];
                Map_parse::run(Cell_check::Blocks_los(), blocked);

                TRACE << "Player can see actor" << endl;
                Map::player->kick_mon(*living_actor);
            }
            TRACE_FUNC_END;
            return;
        }

        //Kick corpse?
        Actor* dead_actor = Utils::get_actor_at_pos(kick_pos, Actor_state::corpse);
        if (dead_actor)
        {
            const bool  IS_SEEING_CELL  = Map::cells[kick_pos.x][kick_pos.y].is_seen_by_player;
            string      corpse_name      = IS_SEEING_CELL ?
                                          dead_actor->get_corpse_name_a() :
                                          "a corpse";

            Text_format::first_to_upper(corpse_name);

            Log::add_msg("I bash " + corpse_name + ".");

            pair<int, int> kick_dmg = Item_data::data[int(Item_id::player_kick)]->melee.dmg;
            dead_actor->hit(kick_dmg.first * kick_dmg.second, Dmg_type::physical,
                           Dmg_method::kick);

            Game_time::tick();
            TRACE_FUNC_END;
            return;
        }

        //Kick feature
        TRACE << "No actor at kick pos, attempting to kick feature instead" << endl;
        auto* const f = Map::cells[kick_pos.x][kick_pos.y].rigid;
        f->hit(Dmg_type::physical, Dmg_method::kick, Map::player);
    }
    TRACE_FUNC_END;
}

} //Kick

#include "disarm.hpp"

#include "game_time.hpp"
#include "msg_log.hpp"
#include "io.hpp"
#include "actor_player.hpp"
#include "query.hpp"
#include "map.hpp"
#include "feature_trap.hpp"
#include "inventory.hpp"
#include "property_factory.hpp"

namespace disarm
{

void player_disarm()
{
        // TODO: Disarming should be allowed while blind
        if (!map::player->properties().allow_see())
        {
                msg_log::add("Not while blind.");

                return;
        }

        if (map::player->has_prop(PropId::entangled))
        {
                msg_log::add("Not while entangled.");

                return;
        }

        if (map::player->enc_percent() >= 100)
        {
                msg_log::add("Not while encumbered.");

                return;
        }

        msg_log::add("Which direction?" +
                     cancel_info_str, colors::light_white());


        const Dir input_dir = query::dir(AllowCenter::no);

        if (input_dir == Dir::END || input_dir == Dir::center)
        {
                // Invalid direction
                msg_log::clear();
        }
        else // Valid direction
        {
                const P pos(map::player->pos + dir_utils::offset(input_dir));

                // Abort if cell is unseen
                if (!map::cells[pos.x][pos.y].is_seen_by_player)
                {
                        msg_log::add("I cannot see there.");

                        return;
                }

                msg_log::clear();

                Actor* actor_on_trap = map::actor_at_pos(pos);

                // Abort if trap blocked by monster
                if (actor_on_trap)
                {
                        if (map::player->can_see_actor(*actor_on_trap))
                        {
                                msg_log::add("It's blocked.");
                        }
                        else
                        {
                                msg_log::add("Something is blocking it.");
                        }
                }
                else // No actor on the trap
                {
                        map::cells[pos.x][pos.y].rigid->disarm();
                }
        }
} // player_disarm

} // disarm

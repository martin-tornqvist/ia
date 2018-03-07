#include "map_controller.hpp"

#include "global.hpp"
#include "rl_utils.hpp"
#include "msg_log.hpp"
#include "feature_rigid.hpp"
#include "actor_factory.hpp"
#include "game_time.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "map.hpp"

// -----------------------------------------------------------------------------
// MapController
// -----------------------------------------------------------------------------

// TODO: For standard level 'on_start':
// audio::try_play_amb(1)


void MapControllerBoss::on_start()
{
        audio::play(SfxId::boss_voice1);

        for (auto* const actor : game_time::actors)
        {
                if (!actor->is_player())
                {
                        static_cast<Mon*>(actor)->become_aware_player(false);
                }
        }
}

void MapControllerBoss::on_std_turn()
{
        const P stair_pos(map_w - 2, 11);

        const auto feature_at_stair_pos =
                map::cells[stair_pos.x][stair_pos.y].rigid->id();

        if (feature_at_stair_pos == FeatureId::stairs)
        {
                // Stairs already created
                return;
        }

        for (const auto* const actor : game_time::actors)
        {
                if ((actor->id() == ActorId::the_high_priest) &&
                    actor->is_alive())
                {
                        // The boss is still alive
                        return;
                }
        }

        // The boss is dead, and stairs have not yet been created

        msg_log::add("The ground rumbles...",
                     colors::white(),
                     false,
                     MorePromptOnMsg::yes);

        map::put(new Stairs(stair_pos));

        map::put(new RubbleLow(stair_pos - P(1, 0)));

        // TODO: This was in the 'on_death' hook for TheHighPriest - it should
        // be a property if this event should still exist
        // const size_t nr_snakes = rnd::range(4, 5);

        // actor_factory::spawn(
        //         pos,
        //         {nr_snakes, ActorId::pit_viper});
}

// -----------------------------------------------------------------------------
// map_control
// -----------------------------------------------------------------------------
namespace map_control
{

std::unique_ptr<MapController> controller = nullptr;

}

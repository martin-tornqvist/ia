#include "map_travel.hpp"

#include "init.hpp"

#include <list>

#include "map.hpp"
#include "map_builder.hpp"
#include "map_controller.hpp"
#include "draw_map.hpp"
#include "mapgen.hpp"
#include "populate_items.hpp"
#include "io.hpp"
#include "msg_log.hpp"
#include "feature_rigid.hpp"
#include "saving.hpp"
#include "actor_factory.hpp"
#include "actor_player.hpp"
#include "actor_mon.hpp"
#include "property.hpp"
#include "property_handler.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static void trigger_insanity_sympts_for_descent()
{
        // Phobia of deep places
        if (insanity::has_sympt(InsSymptId::phobia_deep))
        {
                msg_log::add("I am plagued by my phobia of deep places!");

                map::player->apply_prop(new PropTerrified());
        }

        // Babbling
        for (const auto* const sympt : insanity::active_sympts())
        {
                if (sympt->id() == InsSymptId::babbling)
                {
                        static_cast<const InsBabbling*>(sympt)->babble();
                }
        }
}

// -----------------------------------------------------------------------------
// map_travel
// -----------------------------------------------------------------------------
namespace map_travel
{

std::vector<MapData> map_list;

void init()
{
        // Forest + dungeon + boss + trapezohedron
        const size_t nr_lvl_tot = dlvl_last + 3;

        {
                MapData map_data = {
                        MapType::std,
                        AllowSpawnMonOverTime::yes
                };

                map_list = std::vector<MapData>(nr_lvl_tot, map_data);
        }

        // Forest intro level
        map_list[0] = {
                MapType::intro,
                AllowSpawnMonOverTime::no
        };

        // Occasionally set rats-in-the-walls level as intro to late game
        if (rnd::one_in(5))
        {
                map_list[dlvl_first_late_game - 1] = {
                        MapType::rat_cave,
                        AllowSpawnMonOverTime::no
                };
        }

        // "Pharaoh chamber" is the first late game level
        map_list[dlvl_first_late_game] = {
                MapType::egypt,
                AllowSpawnMonOverTime::no
        };

        map_list[dlvl_last + 1] = {
                MapType::boss,
                AllowSpawnMonOverTime::no
        };

        map_list[dlvl_last + 2] = {
                MapType::trapez,
                AllowSpawnMonOverTime::no
        };
}

void save()
{
        saving::put_int(map_list.size());

        for (const auto& map_data : map_list)
        {
                saving::put_int((int)map_data.type);

                saving::put_int((int)map_data.allow_spawn_mon_over_time);
        }
}

void load()
{
        const int nr_maps = saving::get_int();

        map_list.resize((size_t)nr_maps);

        for (auto& map_data : map_list)
        {
                map_data.type = (MapType)saving::get_int();

                map_data.allow_spawn_mon_over_time =
                        (AllowSpawnMonOverTime)saving::get_int();
        }
}

void go_to_nxt()
{
        TRACE_FUNC_BEGIN;

        io::clear_screen();
        io::update_screen();

        draw_map::clear();

        map_list.erase(map_list.begin());

        const auto& map_data = map_list.front();

        ++map::dlvl;

        const auto map_builder = map_builder::make(map_data.type);

        map_builder->build();

        if (map::player->has_prop(PropId::descend))
        {
                msg_log::add("My sinking feeling disappears.");

                map::player->properties().end_prop_silent(PropId::descend);
        }

        game_time::is_magic_descend_nxt_std_turn = false;

        map::player->tgt_ = nullptr;

        map::update_vision();

        map::player->restore_shock(999, true);

        msg_log::add("I have discovered a new area.");

        // NOTE: When the "intro level" is skipped, "go_to_nxt" is called when
        // the game starts - so no XP is missed in that case (same thing when
        // loading the game)
        game::incr_player_xp(5, Verbosity::verbose);

        map::player->on_new_dlvl_reached();

        game::add_history_event("Reached dungeon level " +
                                std::to_string(map::dlvl));

        trigger_insanity_sympts_for_descent();

        if (map_control::controller)
        {
                map_control::controller->on_start();
        }

        TRACE_FUNC_END;
}

MapData current_map_data()
{
        return map_list.front();
}

} // map_travel

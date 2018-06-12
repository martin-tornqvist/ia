#include "map_builder.hpp"

#ifndef NDEBUG
#include <chrono>
#endif // NDEBUG

#include "map.hpp"
#include "map_templates.hpp"
#include "map_controller.hpp"
#include "game_time.hpp"
#include "gods.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "actor_factory.hpp"
#include "mapgen.hpp"

// -----------------------------------------------------------------------------
// MapBuilder
// -----------------------------------------------------------------------------
void MapBuilder::build()
{
        TRACE_FUNC_BEGIN;

        bool map_ok = false;

#ifndef NDEBUG
        int nr_attempts = 0;
        auto start_time = std::chrono::steady_clock::now();
#endif // NDEBUG

        // TODO: When the map is invalid, any unique items spawned are lost
        // forever. Currently, the only effect of this should be that slightly
        // fewever unique items are found by the player.

        while (!map_ok)
        {
#ifndef NDEBUG
                ++nr_attempts;
#endif // NDEBUG

                map_ok = build_specific();

                if (map_ok)
                {
                        map_templates::on_map_ok();
                }
                else
                {
                        map_templates::on_map_discarded();
                }
        }

        gods::set_random_god();

        // Spawn starting allies
        for (size_t i = 0; i < game_time::actors.size(); ++i)
        {
                Actor* const actor = game_time::actors[i];

                const auto& allies = actor->data().starting_allies;

                if (allies.empty())
                {
                        continue;
                }

                actor_factory::spawn(actor->pos, allies, map::rect())
                        .set_leader(actor)
                        .for_each([](Mon* mon)
                        {
                                mon->is_player_feeling_msg_allowed_ = false;
                        });
        }

        map_control::controller = map_controller();

#ifndef NDEBUG
        auto diff_time = std::chrono::steady_clock::now() - start_time;

        const double duration =
                std::chrono::duration<double, std::milli>(diff_time)
                .count();

        TRACE << "Map built after " << nr_attempts << " attempt(s)."
              << std::endl
              << "Total time taken: " <<  duration << " ms"
              << std::endl;
#endif // NDEBUG

        TRACE_FUNC_END;
}

std::unique_ptr<MapController> MapBuilder::map_controller() const
{
        return nullptr;
}

// -----------------------------------------------------------------------------
// MapBuilderTemplateLevel
// -----------------------------------------------------------------------------
bool MapBuilderTemplateLevel::build_specific()
{
        template_ = &map_templates::level_templ(template_id());

        const P templ_dims = template_->dims();

        map::reset(templ_dims);

        for (int x = 0; x < templ_dims.x; ++x)
        {
                for (int y = 0; y < templ_dims.y; ++y)
                {
                        const P p(x, y);

                        handle_template_pos(p, template_->at(p));
                }
        }

        on_template_built();

        return true;
}

// -----------------------------------------------------------------------------
// map_builder
// -----------------------------------------------------------------------------
namespace map_builder
{

std::unique_ptr<MapBuilder> make(const MapType map_type)
{
        switch (map_type)
        {
        case MapType::intro:
                return std::make_unique<MapBuilderIntroForest>();

        case MapType::std:
                return std::make_unique<MapBuilderStd>();

        case MapType::egypt:
                return std::make_unique<MapBuilderEgypt>();

        case MapType::rat_cave:
                return std::make_unique<MapBuilderRatCave>();

        case MapType::boss:
                return std::make_unique<MapBuilderBoss>();

        case MapType::trapez:
                return std::make_unique<MapBuilderTrapez>();
        }

        return nullptr;
}

} // map_builder

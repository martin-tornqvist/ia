#include "init.hpp"

#include "bot.hpp"

#include <algorithm>
#include <vector>

#include "property.hpp"
#include "property_data.hpp"
#include "property_handler.hpp"
#include "property_factory.hpp"
#include "actor.hpp"
#include "actor_mon.hpp"
#include "feature.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "actor_factory.hpp"
#include "attack.hpp"
#include "feature_door.hpp"
#include "inventory.hpp"
#include "actor_mon.hpp"
#include "map_parsing.hpp"
#include "game_time.hpp"
#include "map_travel.hpp"
#include "explosion.hpp"
#include "io.hpp"
#include "sdl_base.hpp"
#include "item_factory.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static std::vector<P> path_;

static void show_map_and_freeze(const std::string& msg)
{
        TRACE_FUNC_BEGIN;

        for (auto& cell : map::cells)
        {
                cell.is_explored = true;
                cell.is_seen_by_player = true;
        }

        for (Actor* const actor : game_time::actors)
        {
                if (!actor->is_player())
                {
                        Mon* const mon = static_cast<Mon*>(actor);

                        mon->player_aware_of_me_counter_ = 999;
                }
        }

        while (true)
        {
                io::draw_text("[" + msg + "]",
                              Panel::screen,
                              P(0, 0),
                              colors::light_red());

                io::update_screen();

                sdl_base::sleep(1);

                io::flush_input();
        }
} // show_map_and_freeze

static void find_stair_path()
{
        Array2<bool> blocked(map::dims());

        map_parsers::BlocksMoveCommon(ParseActors::no)
                .run(blocked, blocked.rect());

        P stair_p(-1, -1);

        for (int x = 0; x < map::w(); ++x)
        {
                for (int y = 0; y < map::h(); ++y)
                {
                        const auto id = map::cells.at(x, y).rigid->id();

                        if (id == FeatureId::stairs)
                        {
                                blocked.at(x, y) = false;

                                stair_p.set(x, y);
                        }
                        else if (id == FeatureId::door)
                        {
                                blocked.at(x, y) = false;
                        }
                }
        }

        if (stair_p.x == -1)
        {
                show_map_and_freeze("Could not find stairs");
        }

        const P& player_p = map::player->pos;

        if (blocked.at(player_p))
        {
                show_map_and_freeze("Player on blocked position");
        }

        path_ = pathfind(player_p, stair_p, blocked);

        if (path_.empty())
        {
                show_map_and_freeze("Could not find path to stairs");
        }

        ASSERT(path_.front() == stair_p);
} // find_stair_path

static bool walk_to_adj_cell(const P& p)
{
        ASSERT(is_pos_adj(map::player->pos, p, true));

        char key = '0' + (int)dir_utils::dir(p - map::player->pos);

        // Occasionally randomize movement
        if (rnd::one_in(5))
        {
                key = '0' + rnd::range(1, 9);
        }

        game::handle_player_input(InputData(key));

        return map::player->pos == p;
}

// -----------------------------------------------------------------------------
// bot
// -----------------------------------------------------------------------------
namespace bot
{

void init()
{
        path_.clear();
}

void act()
{
        // =====================================================================
        // TESTS
        // =====================================================================
#ifndef NDEBUG
        for (size_t outer_idx = 0;
             outer_idx < game_time::actors.size();
             ++outer_idx)
        {
                const Actor* const actor = game_time::actors[outer_idx];

                ASSERT(map::is_pos_inside_map(actor->pos));

                for (size_t inner_idx = 0;
                     inner_idx < game_time::actors.size();
                     ++inner_idx)
                {
                        const Actor* const other_actor =
                                game_time::actors[inner_idx];

                        if (outer_idx == inner_idx ||
                            !actor->is_alive() ||
                            !other_actor->is_alive())
                        {
                                continue;
                        }

                        if (actor == other_actor)
                        {
                                show_map_and_freeze(
                                        "Same actor encountered twice in list");
                        }

                        if (actor->pos == other_actor->pos)
                        {
                                show_map_and_freeze(
                                        "Two living actors at same pos (" +
                                        std::to_string(actor->pos.x) + ", " +
                                        std::to_string(actor->pos.y) + ")");
                        }
                }
        }
#endif
        // =====================================================================

        // Abort?
        // TODO: Reimplement this
//    if(io::is_key_held(SDLK_ESCAPE))
//    {
//        config::toggle_bot_playing();
//    }

        // If we are finished with the current run, go back to dlvl 1
        if (map::dlvl >= dlvl_last)
        {
                TRACE << "Starting new run on first dungeon level" << std::endl;
                map_travel::init();

                map::dlvl = 1;

                return;
        }

        auto& inv = map::player->inv();

        // Use an Incinerator as ranged weapon
        {
                auto* wpn_item = inv.item_in_slot(SlotId::wpn);

                if (!wpn_item || wpn_item->data().ranged.is_ranged_wpn)
                {
                        delete inv.slots_[(size_t)SlotId::wpn].item;

                        inv.slots_[(size_t)SlotId::wpn].item = nullptr;

                        inv.put_in_slot(
                                SlotId::wpn,
                                item_factory::make(ItemId::incinerator),
                                Verbosity::silent);
                }
        }

        // If no armor, occasionally equip an asbesthos suite (helps not getting
        // stuck on e.g. Energy Hounds)
        if (!inv.slots_[(size_t)SlotId::body].item &&
            rnd::one_in(20))
        {
                inv.put_in_slot(
                        SlotId::body,
                        item_factory::make(ItemId::armor_asb_suit),
                        Verbosity::silent);
        }

        PropHandler& properties = map::player->properties();

        // Keep an allied Mi-go around (to help getting out of sticky
        // situations, and for some allied monster code exercise)
        bool has_allied_mon = false;

        for (const Actor* const actor : game_time::actors)
        {
                if (map::player->is_leader_of(actor))
                {
                        has_allied_mon = true;
                        break;
                }
        }

        if (!has_allied_mon)
        {
                actor_factory::spawn(
                        map::player->pos, {ActorId::mi_go}, map::rect())
                        .set_leader(map::player)
                        .make_aware_of_player();
        }

        // Occasionally apply rFear to avoid getting stuck
        if (rnd::one_in(7))
        {
                auto prop = new PropRFear();

                prop->set_duration(4);

                properties.apply(prop);
        }

        // Occasionally apply Burning to a random actor (to avoid getting stuck)
        if (rnd::one_in(10))
        {
                const int element = rnd::range(0, game_time::actors.size() - 1);

                Actor* const actor = game_time::actors[element];

                if (actor != map::player)
                {
                        actor->apply_prop(new PropBurning());
                }
        }

        // Occasionally teleport (to avoid getting stuck)
        if (rnd::one_in(200))
        {
                map::player->teleport();
        }

        // Occasionally send a TAB command to attack nearby monsters
        if (rnd::coin_toss())
        {
                game::handle_player_input(InputData(SDLK_TAB));

                return;
        }

        // Occasionally send a 'wait 5 turns' command (just code exercise)
        if (rnd::one_in(50))
        {
                game::handle_player_input(InputData('s'));

                return;
        }

        // Occasionally fire at a random position
        if (rnd::one_in(5))
        {
                auto& inv = map::player->inv();

                auto* wpn_item = inv.item_in_slot(SlotId::wpn);

                if (wpn_item && wpn_item->data().ranged.is_ranged_wpn)
                {
                        auto* wpn = static_cast<Wpn*>(wpn_item);

                        wpn->ammo_loaded_ = wpn->data().ranged.max_ammo;

                        game::handle_player_input(InputData('f'));

                        return;
                }
        }

        // Occasionally apply a random property (to exercise the prop code)
        if (rnd::one_in(20))
        {
                std::vector<PropId> prop_bucket;

                for (size_t i = 0; i < (size_t)PropId::END; ++i)
                {
                        if (property_data::data[i].allow_test_on_bot)
                        {
                                prop_bucket.push_back(PropId(i));
                        }
                }

                const PropId prop_id = rnd::element(prop_bucket);

                auto* const prop = property_factory::make(prop_id);

                prop->set_duration(5);

                properties.apply(prop);
        }

        // Occasionally swap weapon (just some code exercise)
        if (rnd::one_in(50))
        {
                game::handle_player_input(InputData('z'));

                return;
        }

        // Occasionally cause shock spikes (code exercise)
        if (rnd::one_in(100))
        {
                map::player->incr_shock(200, ShockSrc::misc);
                return;
        }

        // Occasionally run an explosion around the player (code exercise, and
        // to avoid getting stuck)
        if (rnd::one_in(50))
        {
                explosion::run(map::player->pos, ExplType::expl);

                return;
        }

        // Handle blocking door
        for (const P& d : dir_utils::dir_list)
        {
                const P p(map::player->pos + d);

                auto* const f = map::cells.at(p).rigid;

                if (f->id() == FeatureId::door)
                {
                        Door* const door = static_cast<Door*>(f);

                        door->reveal(Verbosity::silent);

                        if (door->is_stuck())
                        {
                                f->hit(6, // Arbitrary
                                       DmgType::physical,
                                       DmgMethod::blunt,
                                       map::player);

                                return;
                        }
                }
        }

        // If we are terrified, wait in place
        if (map::player->has_prop(PropId::terrified))
        {
                if (walk_to_adj_cell(map::player->pos))
                {
                        return;
                }
        }

        find_stair_path();

        walk_to_adj_cell(path_.back());
}

} // bot

#include "mapgen.hpp"

#include <vector>

#include "actor_player.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "actor_factory.hpp"
#include "feature_rigid.hpp"
#include "map_parsing.hpp"
#include "highscore.hpp"
#include "fov.hpp"
#include "text_format.hpp"
#include "populate_monsters.hpp"
#include "populate_items.hpp"
#include "item_factory.hpp"
#include "feature_door.hpp"
#include "feature_event.hpp"
#include "game_time.hpp"
#include "init.hpp"

namespace mapgen
{

//------------------------------------------------------------------- FOREST
bool mk_intro_lvl()
{
    map::reset_map();

    const Map_templ&    templ       = map_templ_handling::templ(Map_templ_id::intro_forest);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const Map_templ_cell&   templ_cell  = templ.cell(p);
            const Feature_id        f_id        = templ_cell.feature_id;

            switch (templ_cell.ch)
            {
            case '=':
            case '@':
            {
                Floor* const floor = new Floor(p);

                floor->type_ = Floor_type::stone_path;

                map::put(floor);

                if (templ_cell.ch == '@')
                {
                    map::player->pos = p;
                }
            }
            break;

            case '_':
            {
                Grass* const grass = new Grass(p);
                grass->type_ = Grass_type::withered;
                map::put(grass);
            }
            break;

            case '&':
            case ',':
            {
                if (rnd::one_in(8))
                {
                    map::put(new Bush(p));
                }
                else
                {
                    map::put(new Grass(p));
                }
            }
            break;

            case '+':
            {
                Door* const door = new Door(p, new Wall(p), Door_spawn_state::closed);
                map::put(door);
            }
            break;

            case '.':
            {
                if (rnd::one_in(6))
                {
                    if (rnd::one_in(6))
                    {
                        map::put(new Bush(p));
                    }
                    else
                    {
                        map::put(new Grass(p));
                    }
                }
                else //Normal stone floor
                {
                    map::put(new Floor(p));
                }
            }
            break;

            case '#':
            {
                bool is_door_adj = false;

                for (const P& d : dir_utils::dir_list)
                {
                    const Map_templ_cell& adj_cell = templ.cell(p + d);

                    if (adj_cell.ch == '+')
                    {
                        is_door_adj = true;
                        break;
                    }
                }

                Rigid* rigid = nullptr;

                if (!is_door_adj)
                {
                    if (rnd::one_in(16))
                    {
                        rigid = map::put(new Rubble_low(p));
                    }
                    else if (rnd::one_in(4))
                    {
                        rigid = map::put(new Rubble_high(p));
                    }
                }

                if (!rigid)
                {
                    Wall* const wall = new Wall(p);
                    wall->set_random_is_moss_grown();
                    map::put(wall);
                }
            }
            break;

            default:
            {
                const auto& d = feature_data::data(f_id);
                map::put(static_cast<Rigid*>(d.mk_obj(p)));
            }
            break;
            }
        }
    }

    //Place graves
    std::vector<Highscore_entry> entries = highscore::entries_sorted();

    const int NR_NON_WIN = count_if(begin(entries), end(entries), [](const Highscore_entry & e)
    {
        return !e.is_win();
    });

    if (NR_NON_WIN > 0)
    {
        size_t  entry_idx   = 0;
        int     nr_placed   = 0;
        bool    is_done     = false;

        for (int x = (templ_dims.x - 1); x >= 0; --x)
        {
            for (int y = 0; y < templ_dims.y; ++y)
            {
                const P p(x, y);

                const Map_templ_cell& templ_cell = templ.cell(p);

                if (templ_cell.ch == '&')
                {
                    Grave_stone* grave = new Grave_stone(p);

                    Highscore_entry entry = entries[entry_idx];

                    //Skip winning entries
                    while (entry.is_win())
                    {
                        ++entry_idx;
                        entry = entries[entry_idx];
                    }

                    const std::string name = entry.name();
                    std::vector<std::string> date_str_vector;

                    date_str_vector.clear();
                    text_format::space_separated_list(entry.date_and_time(), date_str_vector);
                    const std::string date_str  = date_str_vector[0];
                    const std::string score_str = to_str(entry.score());
                    const std::string class_str = player_bon::bg_title(entry.bg());

                    grave->set_inscription(
                        "RIP "      + name      +
                        ", "        + class_str +
                        ", "        + date_str  +
                        ", Score: " + score_str);

                    map::put(grave);

                    ++nr_placed;
                    ++entry_idx;

                    if (nr_placed == NR_NON_WIN)
                    {
                        is_done = true;
                        break;
                    }
                }
            }

            if (is_done)
            {
                break;
            }
        }
    }

    populate_mon::populate_intro_lvl();

    return true;
}

//------------------------------------------------------------------- EGYPT
bool mk_egypt_lvl()
{
    map::reset_map();

    const Map_templ&    templ       = map_templ_handling::templ(Map_templ_id::egypt);
    const P             templ_dims  = templ.dims();
    const char          STAIR_CH    = '1' + rnd::range(0, 1);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const Map_templ_cell& templ_cell = templ.cell(p);

            if (templ_cell.feature_id != Feature_id::END)
            {
                if (templ_cell.ch == STAIR_CH)
                {
                    map::put(new Stairs(p));
                }
                else //Not stair pos
                {
                    const auto& d = feature_data::data(templ_cell.feature_id);
                    map::put(static_cast<Rigid*>(d.mk_obj(p)));
                }
            }

            if (templ_cell.actor_id != Actor_id::END)
            {
                Actor* const actor = actor_factory::mk(templ_cell.actor_id, p);
                static_cast<Mon*>(actor)->is_roaming_allowed_ = false;
            }

            if (templ_cell.ch == '@')
            {
                map::player->pos = p;
            }
        }
    }

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Rigid* const f = map::cells[x][y].rigid;

            if (f->id() == Feature_id::wall)
            {
                static_cast<Wall*>(f)->type_ = Wall_type::egypt;
            }
        }
    }

    populate_items::mk_items_on_floor();

    return true;
}

//------------------------------------------------------------------- LENG
bool mk_leng_lvl()
{
    map::reset_map();

    const Map_templ&    templ       = map_templ_handling::templ(Map_templ_id::leng);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const Map_templ_cell&   templ_cell  = templ.cell(p);
            const Feature_id        f_id        = templ_cell.feature_id;

            if (f_id != Feature_id::END)
            {
                const auto& d = feature_data::data(f_id);
                auto* const f = map::put(static_cast<Rigid*>(d.mk_obj(p)));

                if (f_id == Feature_id::grass)
                {
                    if (rnd::one_in(50))
                    {
                        map::put(new Bush(p));
                    }
                }
                else if (f_id == Feature_id::wall)
                {
                    auto* const wall = static_cast<Wall*>(f);

                    if (templ_cell.ch == '%')
                    {
                        wall->type_ = Wall_type::cliff;
                    }
                    else if (templ_cell.ch == '#' || templ_cell.ch == '2')
                    {
                        wall->type_ = Wall_type::leng_monestary;
                    }
                }
            }

            if (templ_cell.actor_id != Actor_id::END)
            {
                actor_factory::mk(templ_cell.actor_id, p);
            }

            switch (templ_cell.ch)
            {
            case '@':
                map::player->pos = p;
                break;

            case ',':
                map::cells[x][y].is_dark = true;
                break;

            case '+':
            {
                Wall* mimic   = new Wall(p);
                mimic->type_  = Wall_type::leng_monestary;
                map::put(new Door(p, mimic, Door_spawn_state::closed));
            }
            break;

            default:
                break;
            }
        }
    }

    return true;
}

//------------------------------------------------------------------- RATS IN THE WALLS
bool mk_rats_in_the_walls_lvl()
{
    map::reset_map();

    const Map_templ&    templ       = map_templ_handling::templ(Map_templ_id::rats_in_the_walls);
    const P             templ_dims  = templ.dims();

    const int       RAT_THING_ONE_IN_N_RAT = 6;
    const Fraction  bones_one_in_n(1, 2);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const Map_templ_cell&   templ_cell  = templ.cell(p);
            const Feature_id        f_id        = templ_cell.feature_id;

            if (f_id != Feature_id::END)
            {
                const auto& d = feature_data::data(f_id);
                auto* const f = map::put(static_cast<Rigid*>(d.mk_obj(p)));

                if (f->id() == Feature_id::wall)
                {
                    if (templ_cell.ch == 'x') //Constructed walls
                    {
                        if (rnd::one_in(2))
                        {
                            map::put(new Rubble_low(p));
                        }
                        else if (rnd::one_in(5))
                        {
                            map::put(new Rubble_high(p));
                        }
                        else
                        {
                            static_cast<Wall*>(f)->type_ = Wall_type::cmn;
                        }
                    }
                    else //Cave walls
                    {
                        static_cast<Wall*>(f)->type_ = Wall_type::cave;
                    }
                }
                else if (f->id() == Feature_id::floor)
                {
                    if (
                        (templ_cell.ch == ',' || templ_cell.ch == 'r') &&
                        bones_one_in_n.roll())
                    {
                        map::put(new Bones(p));
                    }
                    else //Not bones
                    {
                        static_cast<Floor*>(f)->type_ = Floor_type::cave;
                    }
                }

                if (templ_cell.actor_id == Actor_id::rat)
                {
                    if (rnd::one_in(RAT_THING_ONE_IN_N_RAT))
                    {
                        actor_factory::mk(Actor_id::rat_thing, p);
                    }
                    else
                    {
                        actor_factory::mk(Actor_id::rat, p);
                    }
                }
            }

            switch (templ_cell.ch)
            {
            case '@':
                map::player->pos = p;
                break;

            case '1':
                game_time::add_mob(new Event_rats_in_the_walls_discovery(p));
                break;

            default:
                break;
            }
        }
    }

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::Blocks_move_cmn(true), blocked);

    //Spawn extra rats in the rightmost part of the map
    const Fraction fraction_spawned(2, 3);

    const int X0 = (map_w * 7) / 8;

    for (int x = X0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (!blocked[x][y] && fraction_spawned.roll())
            {
                const P p(x, y);

                if (rnd::one_in(RAT_THING_ONE_IN_N_RAT))
                {
                    actor_factory::mk(Actor_id::rat_thing, p);
                }
                else
                {
                    actor_factory::mk(Actor_id::rat, p);
                }
            }
        }
    }

    //Set all actors to non-roaming (will be set to roaming by the discovery event)
    for (Actor* const actor : game_time::actors)
    {
        if (!actor->is_player())
        {
            static_cast<Mon*>(actor)->is_roaming_allowed_ = false;
        }
    }

    populate_items::mk_items_on_floor();

    return true;
}

//------------------------------------------------------------------- BOSS
bool mk_boss_lvl()
{
    map::reset_map();

    const Map_templ&    templ       = map_templ_handling::templ(Map_templ_id::boss_level);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const Map_templ_cell&   templ_cell  = templ.cell(p);
            const Feature_id        f_id        = templ_cell.feature_id;

            if (f_id != Feature_id::END)
            {
                const auto& d = feature_data::data(f_id);
                map::put(static_cast<Rigid*>(d.mk_obj(p)));
            }

            if (templ_cell.actor_id != Actor_id::END)
            {
                actor_factory::mk(templ_cell.actor_id, P(x, y));
            }

            if (templ_cell.ch == '@')
            {
                map::player->pos = p;
            }
        }
    }

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Rigid* const f = map::cells[x][y].rigid;

            if (f->id() == Feature_id::wall)
            {
                static_cast<Wall*>(f)->type_ = Wall_type::egypt;
            }
        }
    }

    return true;
}

//------------------------------------------------------------------- TRAPEZOHEDRON
bool mk_trapez_lvl()
{
    map::reset_map();

    const Map_templ&    templ       = map_templ_handling::templ(Map_templ_id::trapez_level);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            map::cells[x][y].is_dark = true;

            const P p(x, y);

            const Map_templ_cell&   templ_cell  = templ.cell(p);
            const Feature_id        f_id        = templ_cell.feature_id;
            const Item_id           item_id     = templ_cell.item_id;

            if (f_id != Feature_id::END)
            {
                const auto& d = feature_data::data(f_id);
                map::put(static_cast<Rigid*>(d.mk_obj(p)));
            }

            if (item_id != Item_id::END)
            {
                item_factory::mk_item_on_floor(item_id, p);
            }

            if (templ_cell.ch == '@')
            {
                map::player->pos = p;
            }
        }
    }

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            Rigid* const f = map::cells[x][y].rigid;

            if (f->id() == Feature_id::wall)
            {
                static_cast<Wall*>(f)->type_ = Wall_type::egypt;
            }
        }
    }

    return true;
}

} //mapgen

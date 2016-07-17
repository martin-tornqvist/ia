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

    const MapTempl&    templ       = map_templ_handling::templ(MapTemplId::intro_forest);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const MapTemplCell&   templ_cell  = templ.cell(p);
            const FeatureId        f_id        = templ_cell.feature_id;

            switch (templ_cell.ch)
            {
            case '=':
            case '@':
            {
                Floor* const floor = new Floor(p);

                floor->type_ = FloorType::stone_path;

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
                grass->type_ = GrassType::withered;
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
                Door* const door = new Door(p, new Wall(p), DoorSpawnState::closed);
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
                    const MapTemplCell& adj_cell = templ.cell(p + d);

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
                        rigid = map::put(new RubbleLow(p));
                    }
                    else if (rnd::one_in(4))
                    {
                        rigid = map::put(new RubbleHigh(p));
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
    std::vector<HighscoreEntry> entries = highscore::entries_sorted();

    const int nr_non_win = count_if(begin(entries), end(entries), [](const HighscoreEntry & e)
    {
        return !e.is_win();
    });

    if (nr_non_win > 0)
    {
        size_t  entry_idx   = 0;
        int     nr_placed   = 0;
        bool    is_done     = false;

        for (int x = (templ_dims.x - 1); x >= 0; --x)
        {
            for (int y = 0; y < templ_dims.y; ++y)
            {
                const P p(x, y);

                const MapTemplCell& templ_cell = templ.cell(p);

                if (templ_cell.ch == '&')
                {
                    GraveStone* grave = new GraveStone(p);

                    HighscoreEntry entry = entries[entry_idx];

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

                    if (nr_placed == nr_non_win)
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

    const MapTempl&    templ       = map_templ_handling::templ(MapTemplId::egypt);
    const P             templ_dims  = templ.dims();
    const char          stair_ch    = '1' + rnd::range(0, 1);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const MapTemplCell& templ_cell = templ.cell(p);

            if (templ_cell.feature_id != FeatureId::END)
            {
                if (templ_cell.ch == stair_ch)
                {
                    map::put(new Stairs(p));
                }
                else //Not stair pos
                {
                    const auto& d = feature_data::data(templ_cell.feature_id);
                    map::put(static_cast<Rigid*>(d.mk_obj(p)));
                }
            }

            if (templ_cell.actor_id != ActorId::END)
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

            if (f->id() == FeatureId::wall)
            {
                static_cast<Wall*>(f)->type_ = WallType::egypt;
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

    const MapTempl&    templ       = map_templ_handling::templ(MapTemplId::leng);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const MapTemplCell&   templ_cell  = templ.cell(p);
            const FeatureId        f_id        = templ_cell.feature_id;

            if (f_id != FeatureId::END)
            {
                const auto& d = feature_data::data(f_id);
                auto* const f = map::put(static_cast<Rigid*>(d.mk_obj(p)));

                if (f_id == FeatureId::grass)
                {
                    if (rnd::one_in(50))
                    {
                        map::put(new Bush(p));
                    }
                }
                else if (f_id == FeatureId::wall)
                {
                    auto* const wall = static_cast<Wall*>(f);

                    if (templ_cell.ch == '%')
                    {
                        wall->type_ = WallType::cliff;
                    }
                    else if (templ_cell.ch == '#' || templ_cell.ch == '2')
                    {
                        wall->type_ = WallType::leng_monestary;
                    }
                }
            }

            if (templ_cell.actor_id != ActorId::END)
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
                mimic->type_  = WallType::leng_monestary;
                map::put(new Door(p, mimic, DoorSpawnState::closed));
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

    const MapTempl&    templ       = map_templ_handling::templ(MapTemplId::rats_in_the_walls);
    const P             templ_dims  = templ.dims();

    const int       rat_thing_one_in_n_rat = 6;
    const Fraction  bones_one_in_n(1, 2);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const MapTemplCell&   templ_cell  = templ.cell(p);
            const FeatureId        f_id        = templ_cell.feature_id;

            if (f_id != FeatureId::END)
            {
                const auto& d = feature_data::data(f_id);
                auto* const f = map::put(static_cast<Rigid*>(d.mk_obj(p)));

                if (f->id() == FeatureId::wall)
                {
                    if (templ_cell.ch == 'x') //Constructed walls
                    {
                        if (rnd::one_in(2))
                        {
                            map::put(new RubbleLow(p));
                        }
                        else if (rnd::one_in(5))
                        {
                            map::put(new RubbleHigh(p));
                        }
                        else
                        {
                            static_cast<Wall*>(f)->type_ = WallType::cmn;
                        }
                    }
                    else //Cave walls
                    {
                        static_cast<Wall*>(f)->type_ = WallType::cave;
                    }
                }
                else if (f->id() == FeatureId::floor)
                {
                    if (
                        (templ_cell.ch == ',' || templ_cell.ch == 'r') &&
                        bones_one_in_n.roll())
                    {
                        map::put(new Bones(p));
                    }
                    else //Not bones
                    {
                        static_cast<Floor*>(f)->type_ = FloorType::cave;
                    }
                }

                if (templ_cell.actor_id == ActorId::rat)
                {
                    if (rnd::one_in(rat_thing_one_in_n_rat))
                    {
                        actor_factory::mk(ActorId::rat_thing, p);
                    }
                    else
                    {
                        actor_factory::mk(ActorId::rat, p);
                    }
                }
            }

            switch (templ_cell.ch)
            {
            case '@':
                map::player->pos = p;
                break;

            case '1':
                game_time::add_mob(new EventRatsInTheWallsDiscovery(p));
                break;

            default:
                break;
            }
        }
    }

    bool blocked[map_w][map_h];
    map_parse::run(cell_check::BlocksMoveCmn(true), blocked);

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

                if (rnd::one_in(rat_thing_one_in_n_rat))
                {
                    actor_factory::mk(ActorId::rat_thing, p);
                }
                else
                {
                    actor_factory::mk(ActorId::rat, p);
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

    const MapTempl&    templ       = map_templ_handling::templ(MapTemplId::boss_level);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const MapTemplCell&   templ_cell  = templ.cell(p);
            const FeatureId        f_id        = templ_cell.feature_id;

            if (f_id != FeatureId::END)
            {
                const auto& d = feature_data::data(f_id);
                map::put(static_cast<Rigid*>(d.mk_obj(p)));
            }

            if (templ_cell.actor_id != ActorId::END)
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

            if (f->id() == FeatureId::wall)
            {
                static_cast<Wall*>(f)->type_ = WallType::egypt;
            }
        }
    }

    return true;
}

//------------------------------------------------------------------- TRAPEZOHEDRON
bool mk_trapez_lvl()
{
    map::reset_map();

    const MapTempl&    templ       = map_templ_handling::templ(MapTemplId::trapez_level);
    const P             templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            map::cells[x][y].is_dark = true;

            const P p(x, y);

            const MapTemplCell&   templ_cell  = templ.cell(p);
            const FeatureId        f_id        = templ_cell.feature_id;
            const ItemId           item_id     = templ_cell.item_id;

            if (f_id != FeatureId::END)
            {
                const auto& d = feature_data::data(f_id);
                map::put(static_cast<Rigid*>(d.mk_obj(p)));
            }

            if (item_id != ItemId::END)
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

            if (f->id() == FeatureId::wall)
            {
                static_cast<Wall*>(f)->type_ = WallType::egypt;
            }
        }
    }

    return true;
}

} //mapgen

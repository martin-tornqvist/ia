#include "mapgen.hpp"

#include <vector>

#include "actor_player.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "actor_factory.hpp"
#include "feature_rigid.hpp"
#include "feature_door.hpp"
#include "feature_event.hpp"
#include "feature_monolith.hpp"
#include "map_parsing.hpp"
#include "highscore.hpp"
#include "fov.hpp"
#include "text_format.hpp"
#include "populate_monsters.hpp"
#include "populate_items.hpp"
#include "item_factory.hpp"
#include "game_time.hpp"
#include "init.hpp"
#include "rl_utils.hpp"
#include "property.hpp"

namespace mapgen
{

// -----------------------------------------------------------------------------
// Forest
// -----------------------------------------------------------------------------
bool make_intro_lvl()
{
    map::reset_map();

    const auto& templ =
        map_templates::level_templ(
            LevelTemplId::intro_forest);

    const P templ_dims = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const char c = templ(p);

            switch (c)
            {
            case '@':
            case '=':
            {
                Floor* const floor = new Floor(p);

                floor->type_ = FloorType::stone_path;

                map::put(floor);

                if (c == '@')
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
                else // Normal stone floor
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
                    const char adj_c = templ(p + d);

                    if (adj_c == '+')
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

            case '&':
            case ',':
            {
                if (rnd::one_in(12))
                {
                    map::put(new Bush(p));
                }
                else
                {
                    map::put(new Grass(p));
                }
            }
            break;

            case '~':
            {
                auto liquid = new LiquidShallow(p);

                liquid->type_ = LiquidType::water;

                map::put(liquid);
            }
            break;

            case 't':
            {
                map::put(new Tree(p));
            }
            break;

            case 'v':
            {
                map::put(new Brazier(p));
            }
            break;

            case '[':
            {
                map::put(new ChurchBench(p));
            }
            break;

            case '-':
            {
                map::put(new Altar(p));
            }
            break;

            case '*':
            {
                map::put(new Carpet(p));
            }
            break;

            case '>':
            {
                map::put(new Stairs(p));
            }
            break;

            case '+':
            {
                Door* const door =
                    new Door(p,
                             new Wall(p),
                             DoorType::wood,
                             DoorSpawnState::closed);

                map::put(door);
            }
            break;

            default:
            {
                ASSERT(false);
            }
            break;
            }
        }
    }

    // Place graves
    std::vector<HighscoreEntry> entries =
        highscore::entries_sorted();

    const int nr_non_win =
        count_if(begin(entries),
                 end(entries),
                 [](const HighscoreEntry & e)
    {
        return (e.is_win() == IsWin::no);
    });

    if (nr_non_win > 0)
    {
        size_t entry_idx = 0;

        int nr_placed = 0;

        bool is_done = false;

        for (int x = (templ_dims.x - 1); x >= 0; --x)
        {
            for (int y = 0; y < templ_dims.y; ++y)
            {
                const P p(x, y);

                const char c = templ(p);

                if (c == '&')
                {
                    GraveStone* grave = new GraveStone(p);

                    HighscoreEntry entry = entries[entry_idx];

                    // Skip winning entries
                    while (entry.is_win() == IsWin::yes)
                    {
                        ++entry_idx;
                        entry = entries[entry_idx];
                    }

                    const std::string name = entry.name();

                    const std::string date_str = entry.date();

                    const std::string score_str = std::to_string(entry.score());

                    const std::string class_str =
                        player_bon::bg_title(entry.bg());

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

    return true;
}

// -----------------------------------------------------------------------------
// Egypt
// -----------------------------------------------------------------------------
bool make_egypt_lvl()
{
    map::reset_map();

    const auto& templ =
        map_templates::level_templ(
            LevelTemplId::egypt);

    const P templ_dims = templ.dims();

    const char stair_c = '1' + rnd::range(0, 3);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const char c = templ(p);

            switch (c)
            {
            case '.':
            case '@':
            case 'P':
            case 'M':
            case 'C':
            case '1':
            case '2':
            case '3':
            case '4':
            {
                if (c == '@')
                {
                    map::player->pos = p;
                }

                if (c == stair_c)
                {
                    map::put(new Stairs(p));
                }
                else
                {
                    map::put(new Floor(p));
                }

                ActorId actor_id = ActorId::END;

                switch (c)
                {
                case 'P':
                    actor_id = ActorId::khephren;
                    break;

                case 'M':
                    actor_id = ActorId::mummy;
                    break;

                case 'C':
                    actor_id = ActorId::croc_head_mummy;
                    break;
                }

                if (actor_id != ActorId::END)
                {
                    Actor* const actor =
                        actor_factory::make(actor_id, p);

                    static_cast<Mon*>(actor)->is_roaming_allowed_ =
                        MonRoamingAllowed::no;
                }
            }
            break;

            case '#':
            {
                Wall* wall = new Wall(p);

                wall->type_ = WallType::egypt;

                map::put(wall);
            }
            break;

            case 'v':
            {
                map::put(new Brazier(p));
            }
            break;

            case 'S':
            {
                map::put(new Statue(p));
            }
            break;

            case '+':
            {
                Door* const door =
                    new Door(p,
                             new Wall(p),
                             DoorType::wood,
                             DoorSpawnState::closed);

                map::put(door);
            }
            break;

            case '~':
            {
                auto liquid = new LiquidShallow(p);

                liquid->type_ = LiquidType::water;

                map::put(liquid);
            }
            break;
            }
        }
    }

    populate_items::make_items_on_floor();

    return true;
}

// -----------------------------------------------------------------------------
// Leng
// -----------------------------------------------------------------------------
bool make_leng_lvl()
{
    map::reset_map();

    const auto& templ =
        map_templates::level_templ(
            LevelTemplId::leng);

    const P templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const char c = templ(p);

            switch (c)
            {
            case '@':
            case 'S':
            case '-':
            {
                map::put(new Grass(p));

                if (c == '@')
                {
                    map::player->pos = p;
                }
                else if (c == 'S')
                {
                    actor_factory::make(ActorId::leng_spider, p);
                }
                else
                {
                    if (rnd::one_in(50))
                    {
                        map::put(new Bush(p));
                    }
                }
            }
            break;

            case '%':
            {
                Wall* const wall = new Wall(p);

                wall->type_ = WallType::cliff;

                map::put(wall);
            }
            break;

            case '#':
            case '2':
            {
                Wall* wall = new Wall(p);

                wall->type_ = WallType::leng_monestary;

                map::put(wall);
            }
            break;

            case '.':
            case 'E':
            case '1':
            {
                map::put(new Floor(p));

                map::dark[p.x][p.y] = true;

                if (c == 'E')
                {
                    actor_factory::make(ActorId::leng_elder, p);
                }
            }
            break;

            case '$':
            {
            }
            break;

            case '+':
            {
                Wall* mimic = new Wall(p);

                mimic->type_ = WallType::leng_monestary;

                map::put(new Door(p,
                                  mimic,
                                  DoorType::wood,
                                  DoorSpawnState::closed));
            }
            break;
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
// Rat cave
// -----------------------------------------------------------------------------
bool make_rat_cave_level()
{
    map::reset_map();

    const auto& templ =
        map_templates::level_templ(
            LevelTemplId::rat_cave);

    const P templ_dims = templ.dims();

    const int rat_thing_one_in_n_rat = 6;

    const Fraction bones_one_in_n(1, 2);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const char c = templ(p);

            switch (c)
            {
            case '@':
            case '.':
            case ',':
            case '&':
            case 'r':
            case '1':
            {
                if (c == '&' ||
                    (bones_one_in_n.roll() && (c == ',' || c == 'r')))
                {
                    map::put(new Bones(p));
                }
                else
                {
                    map::put(new Floor(p));
                }

                if (c == '@')
                {
                    map::player->pos = p;
                }
                else if (c == '1')
                {
                    game_time::add_mob(
                        new EventRatsInTheWallsDiscovery(p));
                }
                else if (c == 'r')
                {
                    Actor* actor = nullptr;

                    if (rnd::one_in(rat_thing_one_in_n_rat))
                    {
                        actor = actor_factory::make(ActorId::rat_thing, p);
                    }
                    else
                    {
                        actor = actor_factory::make(ActorId::rat, p);
                    }

                    auto prop = new PropFrenzied();

                    prop->set_indefinite();

                    actor->apply_prop(prop);
                }
            }
            break;

            case '#':
            {
                auto* wall = new Wall(p);

                wall->type_ = WallType::cave;

                map::put(wall);
            }
            break;

            case 'x':
            {
                if (rnd::one_in(3))
                {
                    map::put(new RubbleLow(p));
                }
                else if (rnd::one_in(5))
                {
                    map::put(new RubbleHigh(p));
                }
                else
                {
                    auto* wall = new Wall(p);

                    wall->type_ = WallType::common;

                    map::put(wall);
                }
            }
            break;

            case '>':
            {
                map::put(new Stairs(p));
            }
            break;

            case '|':
            {
                map::put(new Monolith(p));
            }
            break;

            case ':':
            {
                map::put(new Stalagmite(p));
            }
            break;
            }
        }
    }

    // Set all actors to non-roaming (the event sets them to roaming)
    for (Actor* const actor : game_time::actors)
    {
        if (!actor->is_player())
        {
            static_cast<Mon*>(actor)->is_roaming_allowed_ =
                MonRoamingAllowed::no;
        }
    }

    populate_items::make_items_on_floor();

    return true;
}

// -----------------------------------------------------------------------------
// Boss level
// -----------------------------------------------------------------------------
bool make_boss_lvl()
{
    map::reset_map();

    const auto& templ =
        map_templates::level_templ(
            LevelTemplId::boss_level);

    const P templ_dims = templ.dims();

    Actor* high_priest = nullptr;

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const P p(x, y);

            const char c = templ(p);

            switch (c)
            {
            case '@':
            case 'P':
            case 'W':
            case 'R':
            case 'G':
            case '.':
            {
                map::put(new Floor(p));

                if (c == '@')
                {
                    map::player->pos = p;
                }
                else if (c == 'P')
                {
                    high_priest =
                        actor_factory::make(ActorId::the_high_priest, p);
                }
                else if (c == 'W')
                {
                    actor_factory::make(ActorId::high_priest_guard_war_vet, p);
                }
                else if (c == 'R')
                {
                    actor_factory::make(ActorId::high_priest_guard_rogue, p);
                }
                else if (c == 'G')
                {
                    actor_factory::make(ActorId::high_priest_guard_ghoul, p);
                }
            }
            break;

            case '#':
            {
                Wall* const wall = new Wall(p);

                wall->type_ = WallType::egypt;

                map::put(wall);
            }
            break;

            case 'v':
            {
                map::put(new Brazier(p));
            }
            break;

            case '>':
            {
                map::put(new Stairs(p));
            }
            break;
            }
        }
    }

    ASSERT(high_priest);

    // Make the High Priest leader of all other monsters
    for (Actor* const actor : game_time::actors)
    {
        if (!actor->is_player() &&
            (actor != high_priest))
        {
            static_cast<Mon*>(actor)->leader_ = high_priest;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
// Trapezohedron level
// -----------------------------------------------------------------------------
bool make_trapez_lvl()
{
    map::reset_map();

    const auto& templ =
        map_templates::level_templ(
            LevelTemplId::trapez_level);

    const P templ_dims  = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            map::dark[x][y] = true;

            const P p(x, y);

            const char c = templ(p);

            switch (c)
            {
            case '@':
            case '.':
            case 'o':
            {
                map::put(new Floor(p));

                if (c == '@')
                {
                    map::player->pos = p;
                }
                else if (c == 'o')
                {
                    item_factory::make_item_on_floor(ItemId::trapez, p);
                }
            }
            break;

            case '#':
            {
                Wall* const wall = new Wall(p);

                wall->type_ = WallType::egypt;

                map::put(wall);
            }
            break;

            case 'v':
            {
                map::put(new Brazier(p));
            }
            break;
            }
        }
    }

    return true;
}

} // mapgen

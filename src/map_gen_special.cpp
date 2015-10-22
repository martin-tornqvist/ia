#include "map_gen.hpp"

#include <vector>

#include "actor_player.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "map.hpp"
#include "utils.hpp"
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

namespace map_gen
{

//------------------------------------------------------------------- FOREST
namespace
{

void mk_forest_limit()
{
    auto put_tree = [](const int X, const int Y) {map::put(new Tree(P(X, Y)));};

    for (int y = 0; y < MAP_H; ++y)
    {
        put_tree(0, y);
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        put_tree(x, 0);
    }

    for (int y = 0; y < MAP_H; ++y)
    {
        put_tree(MAP_W - 1, y);
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        put_tree(x, MAP_H - 1);
    }
}

void mk_forest_outer_treeline()
{
    const int MAX_LEN = 2;

    for (int y = 0; y < MAP_H; ++y)
    {
        for (int x = 0; x <= MAX_LEN; ++x)
        {
            if (rnd::range(1, 4) > 1 || x == 0)
            {
                map::put(new Tree(P(x, y)));
            }
            else
            {
                break;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAX_LEN; ++y)
        {
            if (rnd::range(1, 4) > 1 || y == 0)
            {
                map::put(new Tree(P(x, y)));
            }
            else
            {
                break;
            }
        }
    }

    for (int y = 0; y < MAP_H; ++y)
    {
        for (int x = MAP_W - 1; x >= MAP_W - MAX_LEN; x--)
        {
            if (rnd::range(1, 4) > 1 || x == MAP_W - 1)
            {
                map::put(new Tree(P(x, y)));
            }
            else
            {
                break;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = MAP_H - 1; y >= MAP_H - MAX_LEN; y--)
        {
            if (rnd::range(1, 4) > 1 || y == MAP_H - 1)
            {
                map::put(new Tree(P(x, y)));
            }
            else
            {
                break;
            }
        }
    }
}

void mk_forest_tree_patch()
{
    const int NR_TREES_TO_PUT = rnd::range(5, 17);

    P cur_pos(rnd::range(1, MAP_W - 2), rnd::range(1, MAP_H - 2));

    int nr_trees_created = 0;

    while (nr_trees_created < NR_TREES_TO_PUT)
    {
        if (
            !utils::is_pos_inside_map(cur_pos) ||
            utils::king_dist(cur_pos, map::player->pos) <= 1)
        {
            return;
        }

        map::put(new Tree(cur_pos));

        ++nr_trees_created;

        //Find next pos
        while (map::cells[cur_pos.x][cur_pos.y].rigid->id() == Feature_id::tree ||
                utils::king_dist(cur_pos, map::player->pos) <= 2)
        {
            if (rnd::coin_toss())
            {
                cur_pos.x += rnd::coin_toss() ? -1 : 1;
            }
            else
            {
                cur_pos.y += rnd::coin_toss() ? -1 : 1;
            }

            if (!utils::is_pos_inside_map(cur_pos))
            {
                return;
            }
        }
    }
}

void mk_forest_trees()
{
    map_gen_utils::backup_map();

    const P church_pos(MAP_W - 33, 2);

    int nr_forest_patches = rnd::range(40, 55);

    std::vector<P> path;

    bool proceed = false;

    while (!proceed)
    {
        for (int i = 0; i < nr_forest_patches; ++i) {mk_forest_tree_patch();}

        const map_templ& templ     = map_templ_handling::templ(Map_templ_id::church);
        const P       templ_dims = templ.dims();

        for (int x = 0; x < templ_dims.x; ++x)
        {
            for (int y = 0; y < templ_dims.y; ++y)
            {
                const auto& templ_cell = templ.cell(x, y);
                const auto  f_id       = templ_cell.feature_id;

                const P p(church_pos + P(x, y));

                if (f_id != Feature_id::END)
                {
                    Rigid* const f =
                        map::put(static_cast<Rigid*>(feature_data::data(f_id).mk_obj(p)));

                    if (f_id == Feature_id::grass)
                    {
                        //All grass around the church is withered
                        static_cast<Grass*>(f)->type_ = Grass_type::withered;
                    }
                }

                if (templ_cell.val == 1)
                {
                    map::put(new Door(p, new Wall(p), Door_spawn_state::closed));
                }
            }
        }

        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_move_cmn(false), blocked);

        P stairs_pos;

        for (int x = 0; x < MAP_W; ++x)
        {
            for (int y = 0; y < MAP_H; ++y)
            {
                const auto id = map::cells[x][y].rigid->id();

                if (id == Feature_id::stairs)
                {
                    stairs_pos.set(x, y);
                    blocked[x][y] = false;
                }
                else if (id == Feature_id::door)
                {
                    blocked[x][y] = false;
                }
            }
        }

        path_find::run(map::player->pos, stairs_pos, blocked, path);

        size_t min_path_length = 1;
        size_t max_path_length = 999;

        if (path.size() >= min_path_length && path.size() <= max_path_length)
        {
            proceed = true;
        }
        else
        {
            map_gen_utils::restore_map();
        }

        max_path_length++;
    }

    //Build path
    for (const P& path_pos : path)
    {
        for (int dx = -1; dx < 1; ++dx)
        {
            for (int dy = -1; dy < 1; ++dy)
            {
                const P p(path_pos + P(dx, dy));

                if (map::cells[p.x][p.y].rigid->can_have_rigid() && utils::is_pos_inside_map(p))
                {
                    Floor* const floor = new Floor(p);
                    floor->type_ = Floor_type::stone_path;
                    map::put(floor);
                }
            }
        }
    }

    //Place graves
    std::vector<Highscore_entry> entries = highscore::entries_sorted();

    const int NR_NON_WIN =
        count_if(begin(entries), end(entries), [](const Highscore_entry & e)
    {
        return !e.is_win();
    });

    const int MAX_NR_TO_PLACE     = 7;
    const int NR_GRAVES_TO_PLACE  = std::min(MAX_NR_TO_PLACE, NR_NON_WIN);

    if (NR_GRAVES_TO_PLACE > 0)
    {
        bool blocked[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_move_cmn(true), blocked);

        Los_result fov[MAP_W][MAP_H];

        const int SEARCH_RADI               = FOV_STD_RADI_INT - 2;
        const int TRY_PLACE_EVERY_N_STEP    = 2;

        std::vector<P> grave_cells;

        int path_walk_count = 0;

        for (size_t i = 0; i < path.size(); ++i)
        {
            if (path_walk_count == TRY_PLACE_EVERY_N_STEP)
            {
                fov::run(path[i], blocked, fov);

                for (int dy = -SEARCH_RADI; dy <= SEARCH_RADI; ++dy)
                {
                    for (int dx = -SEARCH_RADI; dx <= SEARCH_RADI; ++dx)
                    {
                        const int X = path[i].x + dx;
                        const int Y = path[i].y + dy;

                        const bool IS_LEFT_OF_CHURCH =
                            X < church_pos.x - (SEARCH_RADI) + 2;

                        const bool IS_ON_STONE_PATH =
                            map::cells[X][Y].rigid->id() == Feature_id::floor;

                        bool is_left_of_prev = true;

                        if (!grave_cells.empty())
                        {
                            is_left_of_prev = X < grave_cells.back().x;
                        }

                        bool is_pos_ok = !fov[X][Y].is_blocked_hard &&
                                         IS_LEFT_OF_CHURCH          &&
                                         !IS_ON_STONE_PATH          &&
                                         is_left_of_prev;

                        if (is_pos_ok)
                        {
                            for (int dxx = -1; dxx <= 1; dxx++)
                            {
                                for (int dyy = -1; dyy <= 1; dyy++)
                                {
                                    if (blocked[X + dxx][Y + dyy])
                                    {
                                        is_pos_ok = false;
                                    }
                                }
                            }

                            if (is_pos_ok)
                            {
                                grave_cells.push_back(P(X, Y));
                                blocked[X][Y] = true;

                                if (int(grave_cells.size()) == NR_GRAVES_TO_PLACE)
                                {
                                    i = 9999;
                                }

                                dy = 99999;
                                dx = 99999;
                            }
                        }
                    }
                }

                path_walk_count = 0;
            }

            path_walk_count++;
        }

        size_t entry_idx = 0;

        for (size_t grave_pos_idx = 0; grave_pos_idx < grave_cells.size(); ++grave_pos_idx)
        {
            Grave_stone*     grave = new Grave_stone(grave_cells[grave_pos_idx]);
            Highscore_entry  entry = entries[entry_idx];

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

            grave->set_inscription("RIP " + name + ", " + class_str + ", " + date_str
                                   + ", Score: " + score_str);

            map::put(grave);
            ++entry_idx;
        }
    }
}

} //namespace

bool mk_intro_lvl()
{
    map::reset_map();

    for (int x = 1; x < MAP_W - 1; ++x)
    {
        for (int y = 1; y < MAP_H - 1; ++y)
        {
            const P p(x, y);

            if (rnd::one_in(6))
            {
                map::put(new Bush(p));
            }
            else
            {
                map::put(new Grass(p));
            }
        }
    }

    mk_forest_outer_treeline();
    mk_forest_trees();
    mk_forest_limit();

    populate_mon::populate_intro_lvl();

    return true;
}

//------------------------------------------------------------------- EGYPT
bool mk_egypt_lvl()
{
    map::reset_map();

    const map_templ& templ      = map_templ_handling::templ(Map_templ_id::egypt);
    const P       templ_dims  = templ.dims();
    const int       STAIR_VAL   = rnd::range(2, 3);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const auto& templ_cell = templ.cell(x, y);
            const P p(x, y);

            if (templ_cell.feature_id != Feature_id::END)
            {
                if (templ_cell.val == STAIR_VAL)
                {
                    map::put(new Stairs(p));
                }
                else
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

            if (templ_cell.val == 1)
            {
                map::player->pos = p;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
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

    const map_templ& templ     = map_templ_handling::templ(Map_templ_id::leng);
    const P       templ_dims = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const auto& templ_cell = templ.cell(x, y);
            const auto  f_id       = templ_cell.feature_id;
            const P p(x, y);

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

                    if (templ_cell.val == 2)
                    {
                        wall->type_ = Wall_type::cliff;
                    }
                    else if (templ_cell.val == 3 || templ_cell.val == 5)
                    {
                        wall->type_ = Wall_type::leng_monestary;
                    }
                }
            }

            if (templ_cell.actor_id != Actor_id::END)
            {
                actor_factory::mk(templ_cell.actor_id, p);
            }

            switch (templ_cell.val)
            {
            case 1:
                map::player->pos = p;
                break;

            case 3:
                map::cells[x][y].is_dark = true;
                break;

            case 6:
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

    const map_templ& templ     = map_templ_handling::templ(Map_templ_id::rats_in_the_walls);
    const P       templ_dims = templ.dims();

    const int       RAT_THING_ONE_IN_N_RAT = 3;
    const Fraction  bones_one_in_n(1, 2);

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            const auto& templ_cell = templ.cell(x, y);
            const auto  f_id       = templ_cell.feature_id;
            const P p(x, y);

            if (f_id != Feature_id::END)
            {
                const auto& d = feature_data::data(f_id);
                auto* const f = map::put(static_cast<Rigid*>(d.mk_obj(p)));

                if (f->id() == Feature_id::wall)
                {
                    if (templ_cell.val == 2) //Constructed walls
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
                    if (templ_cell.val == 4 && rnd::fraction(bones_one_in_n))
                    {
                        map::put(new Bones(p));
                    }
                    else
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

            switch (templ_cell.val)
            {
            case 1:
                map::player->pos = p;
                break;

            case 3:
                game_time::add_mob(new Event_rats_in_the_walls_discovery(p));
                break;

            default:
                break;
            }
        }
    }

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_move_cmn(true), blocked);

    //Spawn extra rats in the rightmost part of the map
    for (int x = (MAP_W * 7) / 8; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (!blocked[x][y])
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

    const map_templ& templ     = map_templ_handling::templ(Map_templ_id::boss_level);
    const P       templ_dims = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
//            map::cells[x][y].is_dark = true;

            const auto& templ_cell = templ.cell(x, y);
            const auto  f_id       = templ_cell.feature_id;

            const P p(x, y);

            if (f_id != Feature_id::END)
            {
                const auto& d = feature_data::data(f_id);
                map::put(static_cast<Rigid*>(d.mk_obj(p)));
            }

            if (templ_cell.actor_id != Actor_id::END)
            {
                actor_factory::mk(templ_cell.actor_id, P(x, y));
            }

            switch (templ_cell.val)
            {
            case 1:
                map::player->pos = p;
                break;

            default:
                break;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
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
bool mk_trapezohedron_lvl()
{
    map::reset_map();

    const map_templ& templ =
        map_templ_handling::templ(Map_templ_id::trapezohedron_level);

    const P templ_dims = templ.dims();

    for (int x = 0; x < templ_dims.x; ++x)
    {
        for (int y = 0; y < templ_dims.y; ++y)
        {
            map::cells[x][y].is_dark = true;

            const auto& templ_cell   = templ.cell(x, y);
            const auto  f_id         = templ_cell.feature_id;
            const auto  item_id      = templ_cell.item_id;

            const P p(x, y);

            if (f_id != Feature_id::END)
            {
                const auto& d = feature_data::data(f_id);
                map::put(static_cast<Rigid*>(d.mk_obj(p)));
            }

            if (item_id != Item_id::END)
            {
                item_factory::mk_item_on_floor(item_id, p);
            }

            switch (templ_cell.val)
            {
            case 1:
                map::player->pos = p;
                break;

            default:
                break;
            }
        }
    }

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
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

} //map_gen

#include "map_parsing.hpp"

#include <algorithm>

#include "map.hpp"
#include "actor_player.hpp"
#include "game_time.hpp"
#include "utils.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"

//------------------------------------------------------------ CELL CHECKS
namespace cell_check
{

bool Blocks_los::check(const Cell& c)  const
{
    return !utils::is_pos_inside_map(c.pos, false) || !c.rigid->is_los_passable();
}

bool Blocks_los::check(const Mob& f) const
{
    return !f.is_los_passable();
}

bool Blocks_move_cmn::check(const Cell& c) const
{
    return !utils::is_pos_inside_map(c.pos, false) || !c.rigid->can_move_cmn();
}

bool Blocks_move_cmn::check(const Mob& f) const
{
    return !f.can_move_cmn();
}

bool Blocks_move_cmn::check(const Actor& a) const
{
    return a.is_alive();
}

bool Blocks_actor::check(const Cell& c) const
{
    return !utils::is_pos_inside_map(c.pos, false) || !c.rigid->can_move(actor_);
}

bool Blocks_actor::check(const Mob& f) const
{
    return !f.can_move(actor_);
}

bool Blocks_actor::check(const Actor& a) const
{
    return a.is_alive();
}

bool Blocks_projectiles::check(const Cell& c)  const
{
    return !utils::is_pos_inside_map(c.pos, false) || !c.rigid->is_projectile_passable();
}

bool Blocks_projectiles::check(const Mob& f)  const
{
    return !f.is_projectile_passable();
}

bool Living_actors_adj_to_pos::check(const Actor& a) const
{
    if (!a.is_alive())
    {
        return false;
    }

    return utils::is_pos_adj(pos_, a.pos, true);
}

bool Blocks_items::check(const Cell& c)  const
{
    return !utils::is_pos_inside_map(c.pos, false) || !c.rigid->can_have_item();
}

bool Blocks_items::check(const Mob& f) const
{
    return !f.can_have_item();
}

bool Is_feature::check(const Cell& c) const
{
    return c.rigid->id() == feature_;
}

bool Is_any_of_features::check(const Cell& c) const
{
    for (auto f : features_)
    {
        if (f == c.rigid->id())
        {
            return true;
        }
    }

    return false;
}

bool All_adj_is_feature::check(const Cell& c) const
{
    const int X = c.pos.x;
    const int Y = c.pos.y;

    if (!utils::is_pos_inside_map(c.pos, false))
    {
        return false;
    }

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (map::cells[X + dx][Y + dy].rigid->id() != feature_)
            {
                return false;
            }
        }
    }

    return true;
}

bool All_adj_is_any_of_features::check(const Cell& c) const
{
    const int X = c.pos.x;
    const int Y = c.pos.y;

    if (X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1)
    {
        return false;
    }

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const auto cur_id = map::cells[X + dx][Y + dy].rigid->id();

            bool is_match = false;

            for (auto f : features_)
            {
                if (f == cur_id)
                {
                    is_match = true;
                    break;
                }
            }

            if (!is_match)
            {
                return false;
            }
        }
    }

    return true;
}

bool All_adj_is_not_feature::check(const Cell& c) const
{
    const int X = c.pos.x;
    const int Y = c.pos.y;

    if (X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1)
    {
        return false;
    }

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (map::cells[X + dx][Y + dy].rigid->id() == feature_)
            {
                return false;
            }
        }
    }

    return true;
}

bool All_adj_is_none_of_features::check(const Cell& c) const
{
    const int X = c.pos.x;
    const int Y = c.pos.y;

    if (X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1)
    {
        return false;
    }

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const auto cur_id = map::cells[X + dx][Y + dy].rigid->id();

            for (auto f : features_)
            {
                if (f == cur_id)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

} //cell_check

//------------------------------------------------------------ MAP PARSE
namespace map_parse
{

void run(const  cell_check::Check& method,
         bool   out[MAP_W][MAP_H],
         const  Map_parse_mode write_rule,
         const  Rect& area_to_check_cells)
{
    IA_ASSERT(method.is_checking_cells()    ||
           method.is_checking_mobs()        ||
           method.is_checking_actors());

    const bool ALLOW_WRITE_FALSE = write_rule == Map_parse_mode::overwrite;

    if (method.is_checking_cells())
    {
        for (int x = area_to_check_cells.p0.x; x <= area_to_check_cells.p1.x; ++x)
        {
            for (int y = area_to_check_cells.p0.y; y <= area_to_check_cells.p1.y; ++y)
            {
                const auto& c         = map::cells[x][y];
                const bool  IS_MATCH  = method.check(c);

                if (IS_MATCH || ALLOW_WRITE_FALSE)
                {
                    out[x][y] = IS_MATCH;
                }
            }
        }
    }

    if (method.is_checking_mobs())
    {
        for (Mob* mob : game_time::mobs)
        {
            const P& p = mob->pos();

            if (utils::is_pos_inside(p, area_to_check_cells))
            {
                const bool IS_MATCH = method.check(*mob);

                if (IS_MATCH || ALLOW_WRITE_FALSE)
                {
                    bool& v = out[p.x][p.y];

                    if (!v)
                    {
                        v = IS_MATCH;
                    }
                }
            }
        }
    }

    if (method.is_checking_actors())
    {
        for (Actor* actor : game_time::actors)
        {
            const P& p = actor->pos;

            if (utils::is_pos_inside(p, area_to_check_cells))
            {
                const bool IS_MATCH = method.check(*actor);

                if (IS_MATCH || ALLOW_WRITE_FALSE)
                {
                    bool& v = out[p.x][p.y];

                    if (!v)
                    {
                        v = IS_MATCH;
                    }
                }
            }
        }
    }
}

bool cell(const cell_check::Check& method, const P& p)
{
    IA_ASSERT(method.is_checking_cells()  ||
           method.is_checking_mobs()   ||
           method.is_checking_actors());

    bool r = false;

    if (method.is_checking_cells())
    {
        const auto& c         = map::cells[p.x][p.y];
        const bool  IS_MATCH  = method.check(c);

        if (IS_MATCH)
        {
            r = true;
        }
    }

    if (method.is_checking_mobs())
    {
        for (Mob* mob : game_time::mobs)
        {
            const P& mob_p = mob->pos();

            if (mob_p == p)
            {
                const bool IS_MATCH = method.check(*mob);

                if (IS_MATCH)
                {
                    r = true;
                    break;
                }
            }
        }
    }

    if (method.is_checking_actors())
    {
        for (Actor* actor : game_time::actors)
        {
            const P& actor_p = actor->pos;

            if (actor_p == p)
            {
                const bool IS_MATCH = method.check(*actor);

                if (IS_MATCH)
                {
                    r = true;
                    break;
                }
            }
        }
    }

    return r;
}

void cells_within_dist_of_others(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
                                 const Range& dist_interval)
{
    IA_ASSERT(in != out);

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            out[x][y] = false;
        }
    }

    for (int y_outer = 0; y_outer < MAP_H; y_outer++)
    {
        for (int x_outer = 0; x_outer < MAP_W; x_outer++)
        {
            if (!out[x_outer][y_outer])
            {
                for (int d = dist_interval.min; d <= dist_interval.max; d++)
                {
                    P p0(std::max(0,         x_outer - d), std::max(0,         y_outer - d));
                    P p1(std::min(MAP_W - 1, x_outer + d), std::min(MAP_H - 1, y_outer + d));

                    for (int x = p0.x; x <= p1.x; ++x)
                    {
                        if (in[x][p0.y] || in[x][p1.y])
                        {
                            out[x_outer][y_outer] = true;
                            break;
                        }
                    }

                    for (int y = p0.y; y <= p1.y; ++y)
                    {
                        if (in[p0.x][y] || in[p1.x][y])
                        {
                            out[x_outer][y_outer] = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool is_val_in_area(const Rect& area, const bool in[MAP_W][MAP_H],
                    const bool VAL)
{
    IA_ASSERT(utils::is_area_inside_map(area));

    for (int y = area.p0.y; y <= area.p1.y; ++y)
    {
        for (int x = area.p0.x; x <= area.p1.x; ++x)
        {
            if (in[x][y] == VAL)
            {
                return true;
            }
        }
    }

    return false;
}

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H])
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (append[x][y]) {base[x][y] = true;}
        }
    }
}

void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
            const Rect& area_allowed_to_modify)
{
    int cmp_x0 = 0;
    int cmp_y0 = 0;
    int cmp_x1 = 0;
    int cmp_y1 = 0;

    const int X0 = std::max(0,          area_allowed_to_modify.p0.x);
    const int Y0 = std::max(0,          area_allowed_to_modify.p0.y);
    const int X1 = std::min(MAP_W - 1,  area_allowed_to_modify.p1.x);
    const int Y1 = std::min(MAP_H - 1,  area_allowed_to_modify.p1.y);

    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            out[x][y] = false;

            cmp_x0 = x == 0 ? 0 : (x - 1);
            cmp_y0 = y == 0 ? 0 : (y - 1);
            cmp_x1 = x == (MAP_W - 1) ? x : (x + 1);
            cmp_y1 = y == (MAP_H - 1) ? y : (y + 1);

            for (int cmp_y = cmp_y0; cmp_y <= cmp_y1; ++cmp_y)
            {
                bool is_found = false;

                for (int cmp_x = cmp_x0; cmp_x <= cmp_x1; ++cmp_x)
                {
                    if (in[cmp_x][cmp_y])
                    {
                        is_found = out[x][y] = true;
                        break;
                    }
                }

                if (is_found)
                {
                    break;
                }
            }
        }
    }
}

void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H], const int DIST)
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            out[x][y] = false;

            const int X0 = x - DIST;
            const int Y0 = y - DIST;
            const int X1 = x + DIST;
            const int Y1 = y + DIST;

            const int CMP_X0 = X0 < 0 ? 0 : X0;
            const int CMP_Y0 = Y0 < 0 ? 0 : Y0;
            const int CMP_X1 = X1 > MAP_W - 1 ? MAP_W - 1 : X1;
            const int CMP_Y1 = Y1 > MAP_H - 1 ? MAP_H - 1 : Y1;

            for (int cmp_y = CMP_Y0; cmp_y <= CMP_Y1; ++cmp_y)
            {
                bool is_found = false;

                for (int cmp_x = CMP_X0; cmp_x <= CMP_X1; ++cmp_x)
                {
                    if (in[cmp_x][cmp_y])
                    {
                        is_found = out[x][y] = true;
                        break;
                    }
                }

                if (is_found)
                {
                    break;
                }
            }
        }
    }
}

bool is_map_connected(const bool blocked[MAP_W][MAP_H])
{
    P origin(-1, -1);

    for (int x = 1; x < MAP_W - 1; ++x)
    {
        for (int y = 1; y < MAP_H - 1; ++y)
        {
            if (!blocked[x][y])
            {
                origin.set(x, y);
                break;
            }
        }

        if (origin.x != -1)
        {
            break;
        }
    }

    IA_ASSERT(utils::is_pos_inside_map(origin, false));

    int flood_fill[MAP_W][MAP_H];
    flood_fill::run(origin, blocked, flood_fill, INT_MAX, P(-1, -1), true);

    //NOTE: We can skip to origin.x immediately, since this is guaranteed to be the
    //leftmost non-blocked cell.
    for (int x = origin.x; x < MAP_W - 1; ++x)
    {
        for (int y = 1; y < MAP_H - 1; ++y)
        {
            if (flood_fill[x][y] == 0 && !blocked[x][y] && P(x, y) != origin)
            {
                return false;
            }
        }
    }

    return true;
}

} //map_parse

//------------------------------------------------------------ IS CLOSER TO POS
bool Is_closer_to_pos::operator()(const P& p1, const P& p2)
{
    const int king_dist1 = utils::king_dist(p_.x, p_.y, p1.x, p1.y);
    const int king_dist2 = utils::king_dist(p_.x, p_.y, p2.x, p2.y);
    return king_dist1 < king_dist2;
}

//------------------------------------------------------------ FLOOD FILL
namespace flood_fill
{

void run(const P& p0,
         const bool blocked[MAP_W][MAP_H],
         int out[MAP_W][MAP_H],
         int travel_lmt,
         const P& p1,
         const bool ALLOW_DIAGONAL)
{
    utils::reset_array(out);

    std::vector<P> positions;
    positions.clear();

    unsigned int    nr_elements_to_skip = 0;
    int             cur_val             = 0;
    bool            path_exists         = true;
    bool            is_at_tgt           = false;
    bool            is_stopping_at_p1   = p1.x != -1;

    const Rect bounds(P(1, 1), P(MAP_W - 2, MAP_H - 2));

    P cur_pos(p0);

    std::vector<P> dirs
    {
        P( 0, -1),
        P(-1,  0),
        P( 0,  1),
        P( 1,  0)
    };

    if (ALLOW_DIAGONAL)
    {
        dirs.push_back(P(-1, -1));
        dirs.push_back(P(-1, 1));
        dirs.push_back(P(1, -1));
        dirs.push_back(P(1, 1));
    }

    bool done = false;

    while (!done)
    {

        for (const P& d : dirs)
        {
            if ((d != 0))
            {
                const P new_pos(cur_pos + d);

                if (
                    !blocked[new_pos.x][new_pos.y]          &&
                    utils::is_pos_inside(new_pos, bounds)   &&
                    out[new_pos.x][new_pos.y] == 0          &&
                    new_pos != p0)
                {
                    cur_val = out[cur_pos.x][cur_pos.y];

                    if (cur_val < travel_lmt)
                    {
                        out[new_pos.x][new_pos.y] = cur_val + 1;
                    }

                    if (is_stopping_at_p1 && cur_pos == p1 - d)
                    {
                        is_at_tgt = true;
                        break;
                    }

                    if (!is_stopping_at_p1 || !is_at_tgt)
                    {
                        positions.push_back(new_pos);
                    }
                }
            }
        }

        if (is_stopping_at_p1)
        {
            if (positions.size() == nr_elements_to_skip)
            {
                path_exists = false;
            }

            if (is_at_tgt || !path_exists)
            {
                done = true;
            }
        }
        else if (positions.size() == nr_elements_to_skip)
        {
            done = true;
        }

        if (cur_val == travel_lmt)
        {
            done = true;
        }

        if (!is_stopping_at_p1 || !is_at_tgt)
        {
            if (positions.size() == nr_elements_to_skip)
            {
                path_exists = false;
            }
            else
            {
                cur_pos = positions[nr_elements_to_skip];
                nr_elements_to_skip++;
            }
        }
    }
}

} //Flood_fill

//------------------------------------------------------------ PATHFINDER
namespace path_find
{

void run(const P& p0, const P& p1, bool blocked[MAP_W][MAP_H], std::vector<P>& out,
         const bool ALLOW_DIAGONAL, const bool RANDOMIZE_STEP_CHOICES)
{
    out.clear();

    if (p0 == p1)
    {
        //Origin and target is same cell
        return;
    }

    int flood[MAP_W][MAP_H];
    flood_fill::run(p0, blocked, flood, 10000, p1, ALLOW_DIAGONAL);

    if (flood[p1.x][p1.y] == 0)
    {
        //No path exists
        return;
    }

    const std::vector<P>& dirs =  ALLOW_DIAGONAL ?
                                  dir_utils::dir_list :
                                  dir_utils::cardinal_list;

    const size_t NR_DIRS = dirs.size();

    std::vector<bool> valid_offsets(NR_DIRS, false); //Corresponds to the elements in "dirs"

    //The path length will be equal to the flood value at the target cell, so we can
    //reserve that many elements.
    out.reserve(flood[p1.x][p1.y]);

    P cur_pos(p1);
    out.push_back(cur_pos);

    while (true)
    {
        P adj_pos;

        //Find valid offsets, and check if origin is reached
        for (size_t i = 0; i < NR_DIRS; ++i)
        {
            const P& d(dirs[i]);

            adj_pos = cur_pos + d;

            if (adj_pos == p0)
            {
                //Origin reached
                return;
            }

            const bool IS_INSIDE_MAP = utils::is_pos_inside_map(adj_pos);

            const int VAL_AT_ADJ = IS_INSIDE_MAP ? flood[adj_pos.x][adj_pos.y] : 0;

            const int VAL_AT_CUR = flood[cur_pos.x][cur_pos.y];

            valid_offsets[i] = VAL_AT_ADJ < VAL_AT_CUR && VAL_AT_ADJ != 0;
        }

        //Set the adjacent position to one of the valid offset
        //Either pick one of the valid offsets at random, or iterate over an offset list
        if (RANDOMIZE_STEP_CHOICES)
        {
            std::vector<P> adj_pos_bucket;

            for (size_t i = 0; i < NR_DIRS; ++i)
            {
                if (valid_offsets[i])
                {
                    adj_pos_bucket.push_back(cur_pos + dirs[i]);
                }
            }

            IA_ASSERT(!adj_pos_bucket.empty());

            adj_pos = adj_pos_bucket[rnd::range(0, adj_pos_bucket.size() - 1)];
        }
        else //Do not randomize step choices - iterate over offset list
        {
            for (size_t i = 0; i < NR_DIRS; ++i)
            {
                if (valid_offsets[i])
                {
                    adj_pos = {cur_pos + dirs[i]};
                    break;
                }
            }
        }

        out.push_back(adj_pos);

        cur_pos = adj_pos;

    } //while
}

} //path_find


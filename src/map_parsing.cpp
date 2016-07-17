#include "map_parsing.hpp"

#include <algorithm>
#include <climits>

#include "init.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "game_time.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"

//------------------------------------------------------------ CELL CHECKS
namespace cell_check
{

bool Blocks_los::check(const Cell& c)  const
{
    return !map::is_pos_inside_map(c.pos, false) || !c.rigid->is_los_passable();
}

bool Blocks_los::check(const Mob& f) const
{
    return !f.is_los_passable();
}

bool Blocks_move_cmn::check(const Cell& c) const
{
    return !map::is_pos_inside_map(c.pos, false) || !c.rigid->can_move_cmn();
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
    return !map::is_pos_inside_map(c.pos, false) || !c.rigid->can_move(actor_);
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
    return !map::is_pos_inside_map(c.pos, false) || !c.rigid->is_projectile_passable();
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

    return is_pos_adj(pos_, a.pos, true);
}

bool Blocks_items::check(const Cell& c)  const
{
    return !map::is_pos_inside_map(c.pos, false) || !c.rigid->can_have_item();
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

    if (!map::is_pos_inside_map(c.pos, false))
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

    if (X <= 0 || X >= map_w - 1 || Y <= 0 || Y >= map_h - 1)
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

    if (X <= 0 || X >= map_w - 1 || Y <= 0 || Y >= map_h - 1)
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

    if (X <= 0 || X >= map_w - 1 || Y <= 0 || Y >= map_h - 1)
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
         bool   out[map_w][map_h],
         const  Map_parse_mode write_rule,
         const  R& area_to_check_cells)
{
    ASSERT(method.is_checking_cells()    ||
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

            if (is_pos_inside(p, area_to_check_cells))
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

            if (is_pos_inside(p, area_to_check_cells))
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
    ASSERT(method.is_checking_cells()  ||
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

void cells_within_dist_of_others(const bool in[map_w][map_h], bool out[map_w][map_h],
                                 const Range& dist_interval)
{
    ASSERT(in != out);

    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            out[x][y] = false;
        }
    }

    for (int y_outer = 0; y_outer < map_h; y_outer++)
    {
        for (int x_outer = 0; x_outer < map_w; x_outer++)
        {
            if (!out[x_outer][y_outer])
            {
                for (int d = dist_interval.min; d <= dist_interval.max; d++)
                {
                    P p0(std::max(0,         x_outer - d), std::max(0,         y_outer - d));
                    P p1(std::min(map_w - 1, x_outer + d), std::min(map_h - 1, y_outer + d));

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

void append(bool base[map_w][map_h], const bool append[map_w][map_h])
{
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            if (append[x][y]) {base[x][y] = true;}
        }
    }
}

void expand(const bool in[map_w][map_h],
            bool out[map_w][map_h],
            const R& area_allowed_to_modify)
{
    const int X0 = std::max(0,          area_allowed_to_modify.p0.x);
    const int Y0 = std::max(0,          area_allowed_to_modify.p0.y);
    const int X1 = std::min(map_w - 1,  area_allowed_to_modify.p1.x);
    const int Y1 = std::min(map_h - 1,  area_allowed_to_modify.p1.y);

    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            out[x][y] = false;

            //Search all cells adjacent to the current position for any cell
            //which is "true" in the input arry.
            const int CMP_X0 = std::max(x - 1, 0);
            const int CMP_Y0 = std::max(y - 1, 0);
            const int CMP_X1 = std::min(x + 1, map_w - 1);
            const int CMP_Y1 = std::min(y + 1, map_h - 1);

            for (int cmp_x = CMP_X0; cmp_x <= CMP_X1; ++cmp_x)
            {
                bool is_found = false;

                for (int cmp_y = CMP_Y0; cmp_y <= CMP_Y1; ++cmp_y)
                {
                    if (in[cmp_x][cmp_y])
                    {
                        out[x][y] = is_found = true;
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

void expand(const bool in[map_w][map_h],
            bool out[map_w][map_h],
            const int DIST)
{
    for (int x = 0; x < map_w; ++x)
    {
        for (int y = 0; y < map_h; ++y)
        {
            out[x][y] = false;

            const int X0 = x - DIST;
            const int Y0 = y - DIST;
            const int X1 = x + DIST;
            const int Y1 = y + DIST;

            const int CMP_X0 = X0 < 0 ? 0 : X0;
            const int CMP_Y0 = Y0 < 0 ? 0 : Y0;
            const int CMP_X1 = X1 > map_w - 1 ? map_w - 1 : X1;
            const int CMP_Y1 = Y1 > map_h - 1 ? map_h - 1 : Y1;

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

bool is_map_connected(const bool blocked[map_w][map_h])
{
    P origin(-1, -1);

    for (int x = 1; x < map_w - 1; ++x)
    {
        for (int y = 1; y < map_h - 1; ++y)
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

    ASSERT(map::is_pos_inside_map(origin, false));

    int floodfill[map_w][map_h];

    floodfill::run(origin,
                    blocked,
                    floodfill,
                    INT_MAX,
                    P(-1, -1),
                    true);

    //NOTE: We can skip to origin.x immediately, since this is guaranteed to be
    //the leftmost non-blocked cell.
    for (int x = origin.x; x < map_w - 1; ++x)
    {
        for (int y = 1; y < map_h - 1; ++y)
        {
            if (
                floodfill[x][y] == 0   &&
                !blocked[x][y]          &&
                P(x, y) != origin)
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
    const int king_dist1 = king_dist(p_.x, p_.y, p1.x, p1.y);
    const int king_dist2 = king_dist(p_.x, p_.y, p2.x, p2.y);
    return king_dist1 < king_dist2;
}

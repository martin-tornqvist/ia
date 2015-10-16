#include "utils.hpp"

#include <algorithm>
#include <vector>
#include <cassert>

#include "converters.hpp"
#include "game_time.hpp"
#include "mersenne_twister.hpp"
#include "actor.hpp"
#include "feature_mob.hpp"

namespace rnd
{

namespace
{

MTRand mt_rand;

int roll(const int ROLLS, const int SIDES)
{
    if (SIDES <= 0)
    {
        return 0;
    }

    if (SIDES == 1)
    {
        return ROLLS * SIDES;
    }

    int result = 0;

    for (int i = 0; i < ROLLS; ++i)
    {
        result += mt_rand.randInt(SIDES - 1) + 1;
    }

    return result;
}

} //Namespace

void seed(const unsigned long val)
{
    mt_rand = MTRand(val);
}

int dice(const int ROLLS, const int SIDES)
{
    return roll(ROLLS, SIDES);
}

int dice(const Dice_param& p)
{
    return roll(p.rolls, p.sides);
}

bool coin_toss()
{
    return roll(1, 2) == 2;
}

bool fraction(const int NUMERATOR, const int DENOMINATOR)
{
    //This function should never be called with a denominator less than one, because it's unclear
    //what it means that something should happen "N times in 0".
    assert(DENOMINATOR >= 1);

    //If numerator is bigger than denominator, it's most likely a bug (should something occur e.g.
    //5 times in 3 ???) - don't allow this.
    assert(NUMERATOR <= DENOMINATOR);

    assert(NUMERATOR >= 0);

    //If any of the rules above are broken on a release build (and was never caught in debug mode),
    //try to perform the action that was *probably* intended.

    if ((NUMERATOR <= 0) || (DENOMINATOR <= 0))
    {
        return false;
    }

    if ((NUMERATOR >= DENOMINATOR) || (DENOMINATOR == 1))
    {
        return true;
    }

    return roll(1, DENOMINATOR) <= NUMERATOR;
}

bool fraction(const Fraction& f)
{
    return fraction(f.numerator, f.denominator);
}

bool one_in(const int N)
{
    return fraction(1, N);
}

int range(const int MIN, const int MAX)
{
    return MIN + roll(1, MAX - MIN + 1) - 1;
}

int range(const Range& value_range)
{
    return range(value_range.min, value_range.max);
}

int percent()
{
    return roll(1, 100);
}

bool percent(const int PCT_CHANCE)
{
    return PCT_CHANCE >= roll(1, 100);
}

int weighted_choice(const std::vector<int> weights)
{
#ifndef NDEBUG
    for (const int WEIGHT : weights)
    {
        assert(WEIGHT > 0);
    }
#endif // NDEBUG

    const int SUM = std::accumulate(begin(weights), end(weights), 0);

    int rnd = rnd::range(0, SUM - 1);

    for (size_t i = 0; i < weights.size(); ++i)
    {
        const int WEIGHT = weights[i];

        if (rnd < WEIGHT)
        {
            return i;
        }

        rnd -= WEIGHT;
    }

    //This point should never be reached
    assert(false);

    return 0;
}

} //rnd

namespace utils
{

void set_constr_in_range(const int MIN, int& val, const int MAX)
{
    if (MAX >= MIN)
    {
        val = std::min(MAX, std::max(val, MIN));
    }
}

void set_constr_in_range(const double MIN, double& val, const double MAX)
{
    if (MAX > MIN)
    {
        val = std::min(MAX, std::max(val, MIN));
    }
}

int constr_in_range(const int MIN, const int VAL, const int MAX)
{
    if (MAX < MIN)
    {
        return -1;
    }

    return std::min(MAX, std::max(VAL, MIN));
}

int constr_in_range(const double MIN, const double VAL, const double MAX)
{
    if (MAX < MIN)
    {
        return -1;
    }

    return std::min(MAX, std::max(VAL, MIN));
}

bool is_clr_eq(const Clr& clr1, const Clr& clr2)
{
    return clr1.r == clr2.r && clr1.g == clr2.g && clr1.b == clr2.b;
}

void reset_array(int a[MAP_W][MAP_H])
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            a[x][y] = 0;
        }
    }
}

void reset_array(bool a[MAP_W][MAP_H], const bool value)
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            a[x][y] = value;
        }
    }
}

void reverse_bool_array(bool array[MAP_W][MAP_H])
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            array[x][y] = !array[x][y];
        }
    }
}

void copy_bool_array(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H])
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            out[x][y] = in[x][y];
        }
    }
}

void mk_vector_from_bool_map(const bool VALUE_TO_STORE, const bool a[MAP_W][MAP_H],
                             std::vector<P>& out)
{
    out.clear();

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (a[x][y] == VALUE_TO_STORE)
            {
                out.push_back(P(x, y));
            }
        }
    }
}

void mk_bool_map_from_vector(const std::vector<P>& positions, bool out[MAP_W][MAP_H])
{
    reset_array(out, false);

    for (const P& p : positions)
    {
        out[p.x][p.y] = true;
    }
}

void actor_cells(const std::vector<Actor*>& actors, std::vector<P>& out)
{
    out.clear();

    for (const auto* const a : actors)
    {
        out.push_back(a->pos);
    }
}

Actor* actor_at_pos(const P& pos, Actor_state state)
{
    for (auto* const actor : game_time::actors_)
    {
        if (actor->pos == pos && actor->state() == state)
        {
            return actor;
        }
    }

    return nullptr;
}

Mob* first_mob_at_pos(const P& pos)
{
    for (auto* const mob : game_time::mobs_)
    {
        if (mob->pos() == pos)
        {
            return mob;
        }
    }

    return nullptr;
}

void mk_actor_array(Actor* a[MAP_W][MAP_H])
{
    reset_array(a);

    for (Actor* actor : game_time::actors_)
    {
        const P& p = actor->pos;
        a[p.x][p.y] = actor;
    }
}

bool is_pos_inside_map(const P& pos, const bool COUNT_EDGE_AS_INSIDE)
{
    if (COUNT_EDGE_AS_INSIDE)
    {
        return pos.x >= 0 && pos.y >= 0 && pos.x < MAP_W && pos.y < MAP_H;
    }
    else
    {
        return pos.x > 0 && pos.y > 0 && pos.x < MAP_W - 1 && pos.y < MAP_H - 1;
    }
}

bool is_pos_inside(const P& pos, const Rect& area)
{
    return
        pos.x >= area.p0.x &&
        pos.x <= area.p1.x &&
        pos.y >= area.p0.y &&
        pos.y <= area.p1.y;
}

bool is_area_inside_other(const Rect& inner, const Rect& outer,
                          const bool COUNT_EQUAL_AS_INSIDE)
{
    if (COUNT_EQUAL_AS_INSIDE)
    {
        return inner.p0.x >= outer.p0.x &&
               inner.p1.x <= outer.p1.x &&
               inner.p0.y >= outer.p0.y &&
               inner.p1.y <= outer.p1.y;
    }
    else
    {
        return inner.p0.x > outer.p0.x &&
               inner.p1.x < outer.p1.x &&
               inner.p0.y > outer.p0.y &&
               inner.p1.y < outer.p1.y;
    }
}

bool is_area_inside_map(const Rect& area)
{
    return is_pos_inside_map(area.p0) && is_pos_inside_map(area.p1);
}

int king_dist(const int X0, const int Y0, const int X1, const int Y1)
{
    return std::max(abs(X1 - X0), abs(Y1 - Y0));
}

int king_dist(const P& p0, const P& p1)
{
    return std::max(abs(p1.x - p0.x), abs(p1.y - p0.y));
}

int taxicab_dist(const P& p0, const P& p1)
{
    return abs(p1.x - p0.x) + abs(p1.y - p0.y);
}

P closest_pos(const P& p, const std::vector<P>& positions)
{
    int dist_to_nearest = INT_MAX;
    P closest_pos;

    for (P p_cmp : positions)
    {
        const int CUR_DIST = king_dist(p, p_cmp);

        if (CUR_DIST < dist_to_nearest)
        {
            dist_to_nearest = CUR_DIST;
            closest_pos    = p_cmp;
        }
    }

    return closest_pos;
}

Actor* random_closest_actor(const P& c, const std::vector<Actor*>& actors)
{
    if (actors.empty())
    {
        return nullptr;
    }

    if (actors.size() == 1)
    {
        return actors[0];
    }

    //Find distance to nearest actor(s)
    int dist_to_nearest = INT_MAX;

    for (Actor* actor : actors)
    {
        const int CUR_DIST = king_dist(c, actor->pos);

        if (CUR_DIST < dist_to_nearest)
        {
            dist_to_nearest = CUR_DIST;
        }
    }

    assert(dist_to_nearest != INT_MAX);

    //Store all actors with distance equal to the nearest distance
    std::vector<Actor*> closest_actors;

    for (Actor* actor : actors)
    {
        if (king_dist(c, actor->pos) == dist_to_nearest)
        {
            closest_actors.push_back(actor);
        }
    }

    assert(!closest_actors.empty());

    const int ELEMENT = rnd::range(0, closest_actors.size() - 1);

    return closest_actors[ELEMENT];
}

bool is_pos_adj(const P& pos1, const P& pos2, const bool COUNT_SAME_CELL_AS_ADJ)
{
    if (
        pos1.x < pos2.x - 1 ||
        pos1.x > pos2.x + 1 ||
        pos1.y < pos2.y - 1 ||
        pos1.y > pos2.y + 1)
    {
        return false;
    }
    else if (pos1.x == pos2.x && pos1.y == pos2.y)
    {
        return COUNT_SAME_CELL_AS_ADJ;
    }

    return true;
}

bool is_val_in_range(const int V, const Range range)
{
    return range.is_in_range(V);
}

Time_data cur_time()
{
    time_t      t   = time(nullptr);
    struct tm*  now = localtime(&t);
    return Time_data(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                     now->tm_hour, now->tm_min, now->tm_sec);
}

} //utils

//------------------------------------------------------ TIME DATA
std::string Time_data::time_str(const Time_type lowest, const bool ADD_SEPARATORS) const
{
    std::string ret = to_str(year_);

    const std::string month_str   = (month_   < 10 ? "0" : "") + to_str(month_);
    const std::string day_str     = (day_     < 10 ? "0" : "") + to_str(day_);
    const std::string hour_str    = (hour_    < 10 ? "0" : "") + to_str(hour_);
    const std::string minute_str  = (minute_  < 10 ? "0" : "") + to_str(minute_);
    const std::string second_str  = (second_  < 10 ? "0" : "") + to_str(second_);

    if (lowest >= Time_type::month)
    {
        ret += "-" + month_str;
    }

    if (lowest >= Time_type::day)
    {
        ret += "-" + day_str;
    }

    if (lowest >= Time_type::hour)
    {
        ret += (ADD_SEPARATORS ? " " : "_") + hour_str;
    }

    if (lowest >= Time_type::minute)
    {
        ret += (ADD_SEPARATORS ? ":" : "-") + minute_str;
    }

    if (lowest >= Time_type::second)
    {
        ret += (ADD_SEPARATORS ? ":" : "-") + second_str;
    }

    return ret;
}

namespace dir_utils
{

namespace
{

const std::string compass_dir_names[3][3] =
{
    {"NW", "N", "NE"},
    {"W",  "",  "E",},
    {"SW", "S", "SE"}
};

const double PI_DB            = 3.14159265;
const double ANGLE_45_DB      = 2 * PI_DB / 8;
const double ANGLE_45_HALF_DB = ANGLE_45_DB / 2.0;

const double edge[4] =
{
    ANGLE_45_HALF_DB + (ANGLE_45_DB * 0),
    ANGLE_45_HALF_DB + (ANGLE_45_DB * 1),
    ANGLE_45_HALF_DB + (ANGLE_45_DB * 2),
    ANGLE_45_HALF_DB + (ANGLE_45_DB * 3)
};

} //namespace

Dir dir(const P& offset)
{
    assert(offset.x >= -1 && offset.y >= -1 && offset.x <= 1 && offset.y <= 1);

    if (offset.y == -1)
    {
        return offset.x == -1 ? Dir::up_left :
               offset.x ==  0 ? Dir::up :
               offset.x ==  1 ? Dir::up_right :
               Dir::END;
    }

    if (offset.y == 0)
    {
        return offset.x == -1 ? Dir::left :
               offset.x ==  0 ? Dir::center :
               offset.x ==  1 ? Dir::right :
               Dir::END;
    }

    if (offset.y == 1)
    {
        return offset.x == -1 ? Dir::down_left :
               offset.x ==  0 ? Dir::down :
               offset.x ==  1 ? Dir::down_right :
               Dir::END;
    }

    return Dir::END;
}

P offset(const Dir dir)
{
    assert(dir != Dir::END);

    switch (dir)
    {
    case Dir::down_left:
        return P(-1, 1);

    case Dir::down:
        return P(0, 1);

    case Dir::down_right:
        return P(1, 1);

    case Dir::left:
        return P(-1, 0);

    case Dir::center:
        return P(0, 0);

    case Dir::right:
        return P(1, 0);

    case Dir::up_left:
        return P(-1, -1);

    case Dir::up:
        return P(0, -1);

    case Dir::up_right:
        return P(1, -1);

    case Dir::END:
        return P(0, 0);
    }

    return P(0, 0);
}

P rnd_adj_pos(const P& origin, const bool IS_ORIGIN_ALLOWED)
{
    if (IS_ORIGIN_ALLOWED)
    {
        const int ELEMENT = rnd::range(0, dir_list.size()); //Treat origin as extra element
        return ELEMENT == int(dir_list.size()) ? origin : (origin + dir_list[ELEMENT]);
    }
    else
    {
        return origin + dir_list[rnd::range(0, dir_list.size() - 1)];
    }
}

void compass_dir_name(const P& from_pos, const P& to_pos, std::string& dst)
{

    dst = "";

    const P offset(to_pos - from_pos);
    const double ANGLE_DB = atan2(-offset.y, offset.x);

    if (ANGLE_DB        <  -edge[2] && ANGLE_DB >  -edge[3])
    {
        dst = "SW";
    }
    else if (ANGLE_DB <= -edge[1] && ANGLE_DB >= -edge[2])
    {
        dst = "S";
    }
    else if (ANGLE_DB <  -edge[0] && ANGLE_DB >  -edge[1])
    {
        dst = "SE";
    }
    else if (ANGLE_DB >= -edge[0] && ANGLE_DB <=  edge[0])
    {
        dst = "E";
    }
    else if (ANGLE_DB >   edge[0] && ANGLE_DB <   edge[1])
    {
        dst = "NE";
    }
    else if (ANGLE_DB >=  edge[1] && ANGLE_DB <=  edge[2])
    {
        dst = "N";
    }
    else if (ANGLE_DB >   edge[2] && ANGLE_DB <   edge[3])
    {
        dst = "NW";
    }
    else
    {
        dst = "W";
    }
}

void compass_dir_name(const Dir dir, std::string& dst)
{

    const P& o = offset(dir);
    dst = compass_dir_names[o.x + 1][o.y + 1];
}

void compass_dir_name(const P& offs, std::string& dst)
{
    dst = compass_dir_names[offs.x + 1][offs.y + 1];
}

bool is_cardinal(const P& d)
{
    assert(d.x >= -1 && d.x <= 1 && d.y >= -1 && d.y <= 1);
    return d != 0 && (d.x == 0 || d.y == 0);
}

} //dir_utils

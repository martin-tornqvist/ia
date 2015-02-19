#include "utils.hpp"

#include <algorithm>
#include <vector>
#include <assert.h>

#include "converters.hpp"
#include "game_time.hpp"
#include "mersenne_twister.hpp"
#include "actor.hpp"
#include "feature_mob.hpp"

using namespace std;

namespace Rnd
{

namespace
{

MTRand mt_rand;

int roll(const int ROLLS, const int SIDES)
{
    if (SIDES <= 0) {return 0;}
    if (SIDES == 1) {return ROLLS * SIDES;}

    int result = 0;
    for (int i = 0; i < ROLLS; ++i)
    {
        result += mt_rand.rand_int(SIDES - 1) + 1;
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
    if (NUMERATOR <= 0)
    {
        return false;
    }
    return roll(1, DENOMINATOR) <= NUMERATOR;
}

bool fraction(const Fraction& fraction_vals)
{
    return fraction(fraction_vals.numerator, fraction_vals.denominator);
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
    return range(value_range.lower, value_range.upper);
}

int percent()
{
    return roll(1, 100);
}

bool percent(const int PCT_CHANCE)
{
    return PCT_CHANCE >= roll(1, 100);
}

} //Rnd

namespace Utils
{

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
                         vector<Pos>& out)
{
    out.clear();
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            if (a[x][y] == VALUE_TO_STORE)
            {
                out.push_back(Pos(x, y));
            }
        }
    }
}

void mk_bool_map_from_vector(const vector<Pos>& positions, bool out[MAP_W][MAP_H])
{
    reset_array(out, false);
    for (const Pos& p : positions) {out[p.x][p.y] = true;}
}

void get_actor_cells(const vector<Actor*>& actors, vector<Pos>& out)
{
    out.clear();
    for (const auto* const a : actors) {out.push_back(a->pos);}
}

Actor* get_actor_at_pos(const Pos& pos, Actor_state state)
{
    for (auto* const actor : Game_time::actors_)
    {
        if (actor->pos == pos && actor->get_state() == state)
        {
            return actor;
        }
    }
    return nullptr;
}

Mob* get_first_mob_at_pos(const Pos& pos)
{
    for (auto* const mob : Game_time::mobs_)
    {
        if (mob->get_pos() == pos) {return mob;}
    }
    return nullptr;
}

void mk_actor_array(Actor* a[MAP_W][MAP_H])
{
    reset_array(a);

    for (Actor* actor : Game_time::actors_)
    {
        const Pos& p = actor->pos;
        a[p.x][p.y] = actor;
    }
}

bool is_pos_inside_map(const Pos& pos, const bool COUNT_EDGE_AS_INSIDE)
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

bool is_pos_inside(const Pos& pos, const Rect& area)
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
    return max(abs(X1 - X0), abs(Y1 - Y0));
}

int king_dist(const Pos& p0, const Pos& p1)
{
    return max(abs(p1.x - p0.x), abs(p1.y - p0.y));
}

int taxicab_dist(const Pos& p0, const Pos& p1)
{
    return abs(p1.x - p0.x) + abs(p1.y - p0.y);
}

Pos get_closest_pos(const Pos& p, const vector<Pos>& positions)
{
    int dist_to_nearest = INT_MAX;
    Pos closest_pos;
    for (Pos p_cmp : positions)
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

Actor* get_random_closest_actor(const Pos& c, const vector<Actor*>& actors)
{
    if (actors.empty())      {return nullptr;}
    if (actors.size() == 1)  {return actors[0];}

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
    vector<Actor*> closest_actors;
    for (Actor* actor : actors)
    {
        if (king_dist(c, actor->pos) == dist_to_nearest)
        {
            closest_actors.push_back(actor);
        }
    }

    assert(!closest_actors.empty());

    const int ELEMENT = Rnd::range(0, closest_actors.size() - 1);

    return closest_actors[ELEMENT];
}

bool is_pos_adj(const Pos& pos1, const Pos& pos2, const bool COUNT_SAME_CELL_AS_ADJ)
{
    if (pos1.x < pos2.x - 1)                   {return false;}
    if (pos1.x > pos2.x + 1)                   {return false;}
    if (pos1.y < pos2.y - 1)                   {return false;}
    if (pos1.y > pos2.y + 1)                   {return false;}
    if (pos1.x == pos2.x && pos1.y == pos2.y)  {return COUNT_SAME_CELL_AS_ADJ;}
    return true;
}

Time_data get_cur_time()
{
    time_t      t   = time(nullptr);
    struct tm*  now = localtime(&t);
    return Time_data(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                    now->tm_hour, now->tm_min, now->tm_sec);
}

bool is_val_in_range(const int VAL, const Range& range)
{
    assert(range.upper >= range.lower);
    return (unsigned int)(VAL - range.lower) <= (unsigned int)(range.upper - range.lower);
}

} //Utils

//------------------------------------------------------ TIME DATA
string Time_data::get_time_str(const Time_type lowest,
                            const bool ADD_SEPARATORS) const
{
    string ret = to_str(year_);

    string month_str   = (month_   < 10 ? "0" : "") + to_str(month_);
    string day_str     = (day_     < 10 ? "0" : "") + to_str(day_);
    string hour_str    = (hour_    < 10 ? "0" : "") + to_str(hour_);
    string minute_str  = (minute_  < 10 ? "0" : "") + to_str(minute_);
    string second_str  = (second_  < 10 ? "0" : "") + to_str(second_);

    if (lowest >= Time_type::month)  ret += (ADD_SEPARATORS ? "-" : "-") + month_str;
    if (lowest >= Time_type::day)    ret += (ADD_SEPARATORS ? "-" : "-") + day_str;
    if (lowest >= Time_type::hour)   ret += (ADD_SEPARATORS ? " " : "_") + hour_str;
    if (lowest >= Time_type::minute) ret += (ADD_SEPARATORS ? ":" : "-") + minute_str;
    if (lowest >= Time_type::second) ret += (ADD_SEPARATORS ? ":" : "-") + second_str;

    return ret;
}

namespace Dir_utils
{

namespace
{

const string compass_dir_names[3][3] =
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

Dir get_dir(const Pos& offset)
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

Pos get_offset(const Dir dir)
{
    assert(dir != Dir::END);

    switch (dir)
    {
    case Dir::down_left:   return Pos(-1, 1);
    case Dir::down:       return Pos(0, 1);
    case Dir::down_right:  return Pos(1, 1);
    case Dir::left:       return Pos(-1, 0);
    case Dir::center:     return Pos(0, 0);
    case Dir::right:      return Pos(1, 0);
    case Dir::up_left:     return Pos(-1, -1);
    case Dir::up:         return Pos(0, -1);
    case Dir::up_right:    return Pos(1, -1);
    case Dir::END:        return Pos(0, 0);
    }
    return Pos(0, 0);
}

Pos get_rnd_adj_pos(const Pos& origin, const bool IS_ORIGIN_ALLOWED)
{
    if (IS_ORIGIN_ALLOWED)
    {
        const int ELEMENT = Rnd::range(0, dir_list.size()); //Treat origin as extra element
        return ELEMENT == int(dir_list.size()) ? origin : (origin + dir_list[ELEMENT]);
    }
    else
    {
        return origin + dir_list[Rnd::range(0, dir_list.size() - 1)];
    }
}

void get_compass_dir_name(const Pos& from_pos, const Pos& to_pos, string& str_ref)
{

    str_ref = "";

    const Pos offset(to_pos - from_pos);
    const double ANGLE_DB = atan2(-offset.y, offset.x);

    if (ANGLE_DB        <  -edge[2] && ANGLE_DB >  -edge[3])
    {
        str_ref = "SW";
    }
    else if (ANGLE_DB <= -edge[1] && ANGLE_DB >= -edge[2])
    {
        str_ref = "S";
    }
    else if (ANGLE_DB <  -edge[0] && ANGLE_DB >  -edge[1])
    {
        str_ref = "SE";
    }
    else if (ANGLE_DB >= -edge[0] && ANGLE_DB <=  edge[0])
    {
        str_ref = "E";
    }
    else if (ANGLE_DB >   edge[0] && ANGLE_DB <   edge[1])
    {
        str_ref = "NE";
    }
    else if (ANGLE_DB >=  edge[1] && ANGLE_DB <=  edge[2])
    {
        str_ref = "N";
    }
    else if (ANGLE_DB >   edge[2] && ANGLE_DB <   edge[3])
    {
        str_ref = "NW";
    }
    else
    {
        str_ref = "W";
    }
}

void get_compass_dir_name(const Dir dir, string& str_ref)
{

    const Pos& offset = get_offset(dir);
    str_ref = compass_dir_names[offset.x + 1][offset.y + 1];
}

void get_compass_dir_name(const Pos& offset, string& str_ref)
{
    str_ref = compass_dir_names[offset.x + 1][offset.y + 1];
}

bool is_cardinal(const Pos& d)
{
    assert(d.x >= -1 && d.x <= 1 && d.y >= -1 && d.y <= 1);
    return d != 0 && (d.x == 0 || d.y == 0);
}

} //Dir_utils

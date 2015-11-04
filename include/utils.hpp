#ifndef UTILS_H
#define UTILS_H

#include <vector>

#include "cmn_data.hpp"
#include "cmn_types.hpp"
#include "mersenne_twister.hpp"

class Actor;
class Mob;

namespace rnd
{

//NOTE: If no parameters are passed to the MTRand constructor, it will be seeded with current time.
//Seeding it manually is only necessary if seed should be controlled.
void seed(const unsigned long val);

//If not called with a positive non-zero number of sides, this will always return zero.
int dice(const int ROLLS, const int SIDES);

int dice(const Dice_param& p);

bool coin_toss();

bool fraction(const int NUMERATOR, const int DENOMINATOR);

bool fraction(const Fraction& f);

bool one_in(const int N);

//Can be called with any range (positive or negative), V2 does *not* have to be bigger than V1.
int range(const int V1, const int V2);

int range(const Range& value_range);

int percent();

bool percent(const int PCT_CHANCE);

int weighted_choice(const std::vector<int> weights);

} //rnd

enum class Time_type
{
    year,
    month,
    day,
    hour,
    minute,
    second
};

struct Time_data
{
    Time_data() :
        year_(0),
        month_(0),
        day_(0),
        hour_(0),
        minute_(0),
        second_(0) {}

    Time_data(int year, int month, int day, int hour, int minute, int second) :
        year_(year),
        month_(month),
        day_(day),
        hour_(hour),
        minute_(minute),
        second_(second) {}

    std::string time_str(const Time_type lowest, const bool ADD_SEPARATORS) const;

    int year_, month_, day_, hour_, minute_, second_;
};

namespace utils
{

//The following functions will first raise val to at least MIN, then lower val to at most MAX
void set_constr_in_range(const int MIN, int& val, const int MAX);
void set_constr_in_range(const double MIN, double& val, const double MAX);
int constr_in_range(const int MIN, const int VAL, const int MAX);
int constr_in_range(const double MIN, const double VAL, const double MAX);

bool is_clr_eq(const Clr& clr1, const Clr& clr2);

void reset_array(int a[MAP_W][MAP_H]);
void reset_array(bool a[MAP_W][MAP_H], const bool value);

template<typename T> void reset_array(T* a[MAP_W][MAP_H])
{
    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            a[x][y] = nullptr;
        }
    }
}

void reverse_bool_array(bool array[MAP_W][MAP_H]);

void copy_bool_array(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H]);

void mk_vector_from_bool_map(const bool VALUE_TO_STORE, const bool a[MAP_W][MAP_H],
                             std::vector<P>& out);

void mk_bool_map_from_vector(const std::vector<P>& positions, bool out[MAP_W][MAP_H]);

Actor* actor_at_pos(const P& pos, Actor_state state = Actor_state::alive);

Mob* first_mob_at_pos(const P& pos);

void actor_cells(const std::vector<Actor*>& actors, std::vector<P>& out);

void mk_actor_array(Actor* a[MAP_W][MAP_H]);

bool is_pos_inside_map(const P& pos, const bool COUNT_EDGE_AS_INSIDE = true);

bool is_pos_inside(const P& pos, const Rect& area);

bool is_area_inside_other(const Rect& inner, const Rect& outer,
                          const bool COUNT_EQUAL_AS_INSIDE);

bool is_area_inside_map(const Rect& area);

bool is_pos_adj(const P& pos1, const P& pos2, const bool COUNT_SAME_CELL_AS_ADJ);

P closest_pos(const P& p, const std::vector<P>& positions);

Actor* random_closest_actor(const P& c, const std::vector<Actor*>& actors);

int king_dist(const int X0, const int Y0, const int X1, const int Y1);
int king_dist(const P& p0, const P& p1);
int taxicab_dist(const P& p0, const P& p1);

bool is_val_in_range(const int V, const Range range);

Time_data cur_time();

} //utils

namespace dir_utils
{

//Useful lists to iterate over (e.g. in map generation)
const std::vector<P> cardinal_list
{
    P(-1,  0),
    P( 1,  0),
    P( 0, -1),
    P( 0,  1)
};

const std::vector<P> dir_list
{
    P(-1,  0),
    P( 1,  0),
    P( 0, -1),
    P( 0,  1),
    P(-1, -1),
    P(-1,  1),
    P( 1, -1),
    P( 1,  1)
};

const std::vector<P> dir_list_w_center
{
    P( 0,  0),
    P(-1,  0),
    P( 1,  0),
    P( 0, -1),
    P( 0,  1),
    P(-1, -1),
    P(-1,  1),
    P( 1, -1),
    P( 1,  1)
};

Dir dir(const P& offset_values);

P offset(const Dir dir);

P rnd_adj_pos(const P& origin, const bool IS_ORIGIN_ALLOWED);

void compass_dir_name(const P& from_pos, const P& to_pos, std::string& dst);

void compass_dir_name(const Dir dir, std::string& dst);

void compass_dir_name(const P& offs, std::string& dst);

bool is_cardinal(const P& d);

} //dir_utils

#endif

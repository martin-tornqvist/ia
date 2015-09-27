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

//NOTE: If MTRand is not provided any parameters to the constructor, it will be seeded
//with current time. So seeding it manually is not necessary for normal gameplay
//purposes - only if seed should be controlled.
void seed(const unsigned long val);

int dice(const int ROLLS, const int SIDES);

int dice(const Dice_param& p);

bool coin_toss();

bool fraction(const int NUMERATOR, const int DENOMINATOR);

bool fraction(const Fraction& fraction_vals);

bool one_in(const int N);

int range(const int MIN, const int MAX);

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
                             std::vector<Pos>& out);

void mk_bool_map_from_vector(const std::vector<Pos>& positions, bool out[MAP_W][MAP_H]);

Actor* actor_at_pos(const Pos& pos, Actor_state state = Actor_state::alive);

Mob* first_mob_at_pos(const Pos& pos);

void actor_cells(const std::vector<Actor*>& actors, std::vector<Pos>& out);

void mk_actor_array(Actor* a[MAP_W][MAP_H]);

bool is_pos_inside_map(const Pos& pos, const bool COUNT_EDGE_AS_INSIDE = true);

bool is_pos_inside(const Pos& pos, const Rect& area);

bool is_area_inside_other(const Rect& inner, const Rect& outer,
                          const bool COUNT_EQUAL_AS_INSIDE);

bool is_area_inside_map(const Rect& area);

bool is_pos_adj(const Pos& pos1, const Pos& pos2, const bool COUNT_SAME_CELL_AS_ADJ);

Pos closest_pos(const Pos& p, const std::vector<Pos>& positions);

Actor* random_closest_actor(const Pos& c, const std::vector<Actor*>& actors);

int king_dist(const int X0, const int Y0, const int X1, const int Y1);
int king_dist(const Pos& p0, const Pos& p1);
int taxicab_dist(const Pos& p0, const Pos& p1);

bool is_val_in_range(const int V, const Range range);

Time_data cur_time();

} //utils

namespace dir_utils
{

//Useful lists to iterate over (e.g. in map generation)
const std::vector<Pos> cardinal_list
{
    Pos(-1, 0), Pos(1, 0), Pos(0, -1), Pos(0, 1)
};

const std::vector<Pos> dir_list
{
    Pos(-1,  0), Pos(1, 0), Pos(0, -1), Pos(0, 1),
    Pos(-1, -1), Pos(-1, 1), Pos(1, -1), Pos(1, 1)
};

Dir dir(const Pos& offset_values);

Pos offset(const Dir dir);

Pos rnd_adj_pos(const Pos& origin, const bool IS_ORIGIN_ALLOWED);

void compass_dir_name(const Pos& from_pos, const Pos& to_pos, std::string& dst);

void compass_dir_name(const Dir dir, std::string& dst);

void compass_dir_name(const Pos& offs, std::string& dst);

bool is_cardinal(const Pos& d);

} //dir_utils

#endif

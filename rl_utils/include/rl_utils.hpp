#ifndef RL_UTILS_H
#define RL_UTILS_H

#include <vector>
#include <string>

//-----------------------------------------------------------------------------
// Geometry
//-----------------------------------------------------------------------------
class P
{
public:
    P() :
        x(0),
        y(0) {}

    P(const int X, const int Y) :
        x(X),
        y(Y) {}

    P(const P& p) :
        x(p.x),
        y(p.y) {}

    P(const int V) :
        x(V),
        y(V) {}

    P& operator=(const P& p)
    {
        x = p.x;
        y = p.y;
        return *this;
    }

    P& operator/=(const int V)
    {
        x /= V;
        y /= V;
        return *this;
    }

    P& operator+=(const P& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    P& operator-=(const P& o)
    {
        x -= o.x;
        y -= o.y;
        return *this;
    }

    P operator+(const P& p) const
    {
        return P(x + p.x, y + p.y);
    }

    P operator+(const int  V) const
    {
        return P(x + V, y + V);
    }

    P operator-(const P& p) const
    {
        return P(x - p.x, y - p.y);
    }

    P operator-(const int V) const
    {
        return P(x - V, y - V);
    }

    P operator/(const int  V) const
    {
        return P(x / V, y / V);
    }

    P operator/(const P& p) const
    {
        return P(x / p.x, y / p.y);
    }

    P operator*(const int V) const
    {
        return P(x * V, y * V);
    }

    P operator*(const P& p) const
    {
        return P(x * p.x, y * p.y);
    }

    bool operator==(const P& p) const
    {
        return x == p.x && y == p.y;
    }

    bool operator!=(const P& p) const
    {
        return x != p.x || y != p.y;
    }

    bool operator!=(const int V) const
    {
        return x != V || y != V;
    }

    bool operator>(const P& p) const
    {
        return x >  p.x && y > p.y;
    }

    bool operator>(const int V) const
    {
        return x >  V && y > V;
    }

    bool operator<(const P& p) const
    {
        return x <  p.x && y < p.y;
    }

    bool operator<(const int V) const
    {
        return x < V && y < V;
    }

    bool operator>=(const P& p) const
    {
        return x >= p.x && y >= p.y;
    }

    bool operator>=(const int V) const
    {
        return x >= V && y >= V;
    }

    bool operator<=(const P& p) const
    {
        return x <= p.x && y <= p.y;
    }

    bool operator<=(const int  V) const
    {
        return x <= V && y <= V;
    }

    P signs() const
    {
        return P(x == 0 ? 0 : x > 0 ? 1 : -1,
                 y == 0 ? 0 : y > 0 ? 1 : -1);
    }

    void set(const int x_, const int y_)
    {
        x = x_;
        y = y_;
    }

    void set(const P& p)
    {
        x = p.x;
        y = p.y;
    }

    void swap(P& p)
    {
        P tmp(p);
        p = *this; set(tmp);
    }

    int x, y;
};

class Rect
{
public:
    Rect() :
        p0(P()),
        p1(P()) {}

    Rect(const P& p0, const P& p1) :
        p0(p0),
        p1(p1) {}

    Rect(const int X0, const int Y0, const int X1, const int Y1) :
        p0(P(X0, Y0)),
        p1(P(X1, Y1)) {}

    Rect(const Rect& r) :
        p0(r.p0),
        p1(r.p1) {}

    int w() const
    {
        return p1.x - p0.x + 1;
    }

    int h() const
    {
        return p1.y - p0.y + 1;
    }

    P dims() const
    {
        return P(w(), h());
    }

    int min_dim() const
    {
        return std::min(w(), h());
    }

    int max_dim() const
    {
        return std::max(w(), h());
    }

    P center() const
    {
        return ((p1 + p0) / 2);
    }

    P p0;
    P p1;
};

struct Pos_val
{
    Pos_val() :
        pos(P()),
        val(-1) {}

    Pos_val(const P& pos_, const int val_) :
        pos     (pos_),
        val     (val_) {}

    Pos_val(const Pos_val& o) :
        pos     (o.pos),
        val     (o.val) {}

    P pos;
    int val;
};

//-----------------------------------------------------------------------------
// Utility functionality for working with directions and offsets
//-----------------------------------------------------------------------------
enum class Dir
{
    down_left   = 1,
    down        = 2,
    down_right  = 3,
    left        = 4,
    center      = 5,
    right       = 6,
    up_left     = 7,
    up          = 8,
    up_right    = 9,
    END
};

namespace dir_utils
{

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

P rnd_adj_pos(const P& origin, const bool IS_CENTER_ALLOWED);

void compass_dir_name(const P& from_pos,
                      const P& to_pos,
                      std::string& dst);

void compass_dir_name(const Dir dir, std::string& dst);

void compass_dir_name(const P& offs, std::string& dst);

} //dir_utils

//-----------------------------------------------------------------------------
// Types for random number generation
//-----------------------------------------------------------------------------
struct Dice_param
{
    Dice_param() :
        rolls   (0),
        sides   (0),
        plus    (0) {}

    Dice_param(const int ROLLS, const int SIDES, const int PLUS = 0) :
        rolls   (ROLLS),
        sides   (SIDES),
        plus    (PLUS) {}

    Dice_param(const Dice_param& other) :
        rolls   (other.rolls),
        sides   (other.sides),
        plus    (other.plus) {}

    Dice_param& operator=(const Dice_param& other)
    {
        rolls = other.rolls;
        sides = other.sides;
        plus  = other.plus;
        return *this;
    }

    int max() const
    {
        return (rolls * sides) + plus;
    }

    int min() const
    {
        return (rolls + plus);
    }

    int roll() const;

    int rolls, sides, plus;
};

struct Range
{
    Range() :
        min(-1),
        max(-1) {}

    Range(const int MIN, const int MAX) :
        min(MIN),
        max(MAX) {}

    Range(const Range& other) :
        Range(other.min, other.max) {}

    int len() const
    {
        return max - min + 1;
    }

    bool is_in_range(const int V) const
    {
        return V >= min && V <= max;
    }

    void set(const int MIN, const int MAX)
    {
        min = MIN;
        max = MAX;
    }

    Range& operator/=(const int V)
    {
        min /= V;
        max /= V;
        return *this;
    }

    int roll() const;

    int min, max;
};

struct Fraction
{
    Fraction() :
        num(-1),
        den(-1) {}

    Fraction(const int NUM, const int DEN) :
        num(NUM),
        den(DEN) {}

    void set(const int NUM, const int DEN)
    {
        num = NUM;
        den = DEN;
    }

    bool roll() const;

    int num, den;
};

//-----------------------------------------------------------------------------
// Random number generation
//-----------------------------------------------------------------------------
namespace rnd
{

//NOTE: If no parameters are passed to the MTRand constructor, it will be seeded with current time.
//Seeding it manually is only necessary if seed should be controlled.
void seed(const unsigned long val);

//If not called with a positive non-zero number of sides, this will always return zero.
int dice(const int ROLLS, const int SIDES);

bool coin_toss();

bool fraction(const int NUMER, const int DENOM);

bool one_in(const int N);

//Can be called with any range (positive or negative), V2 does *not* have to be bigger than V1.
int range(const int V1, const int V2);

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
        year_   (0),
        month_  (0),
        day_    (0),
        hour_   (0),
        minute_ (0),
        second_ (0) {}

    Time_data(int year,
              int month,
              int day,
              int hour,
              int minute,
              int second) :
        year_   (year),
        month_  (month),
        day_    (day),
        hour_   (hour),
        minute_ (minute),
        second_ (second) {}

    std::string time_str(const Time_type lowest,
                         const bool ADD_SEPARATORS) const;

    int year_, month_, day_, hour_, minute_, second_;
};

//-----------------------------------------------------------------------------
// Misc utils
//-----------------------------------------------------------------------------
void set_constr_in_range(const int MIN,
                         int& val,
                         const int MAX);

void set_constr_in_range(const double MIN,
                         double& val,
                         const double MAX);

int constr_in_range(const int MIN,
                    const int VAL,
                    const int MAX);

int constr_in_range(const double MIN,
                    const double VAL,
                    const double MAX);

//Takes a boolean 2d array of given size, and populates a vector with positions matching the value
//to store (true/false). This can for example be useful if you have a parsed map of blocked and
//free cells, and you want a list of free cells to (randomly) select from.
void to_vec(const bool* array2,
            const bool VALUE_TO_STORE,
            const int W,
            const int H,
            std::vector<P>& out);

bool is_pos_inside(const P& pos, const Rect& area);

bool is_area_inside(const Rect& inner,
                    const Rect& outer,
                    const bool COUNT_EQUAL_AS_INSIDE);

bool is_pos_adj(const P& pos1,
                const P& pos2,
                const bool COUNT_SAME_CELL_AS_ADJ);

P closest_pos(const P& p, const std::vector<P>& positions);

//Distance as the king moves in chess
//The distance between (x0, y0) and (x1, y1) is defined as max(|x1 - x0|, |y1 - y0|).
//This is typically the model used for movement in roguelikes.
int king_dist(const int X0,
              const int Y0,
              const int X1,
              const int Y1);

int king_dist(const P& p0, const P& p1);

//Taxicab distance - also called "rectilinear distance", "Manhattan distance", etc.
//The distance between (x0, y0) and (x1, y1) is defined as |x1 - x0| + |y1 - y0|.
int taxi_dist(const P& p0, const P& p1);

bool is_val_in_range(const int V, const Range range);

Time_data cur_time();

std::string to_str(const int IN);

int to_int(const std::string& in);

#endif //RL_UTILS_H

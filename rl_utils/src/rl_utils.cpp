#include "rl_utils.hpp"

#include <algorithm>
#include <vector>
#include <cassert>
#include <sstream>

#include "mersenne_twister.hpp"

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
    assert(offset.x >= -1 &&
           offset.y >= -1 &&
           offset.x <=  1 &&
           offset.y <=  1);

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

P rnd_adj_pos(const P& origin, const bool IS_CENTER_ALLOWED)
{
    const std::vector<P>* vec = nullptr;

    if (IS_CENTER_ALLOWED)
    {
        vec = &dir_list_w_center;
    }
    else //Center not allowed
    {
        vec = &dir_list;
    }

    const size_t IDX = rnd::range(0, vec->size() - 1);

    return origin + vec->at(IDX);
}

void compass_dir_name(const P& from_pos,
                      const P& to_pos,
                      std::string& dst)
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

    const P& o  = offset(dir);
    dst         = compass_dir_names[o.x + 1][o.y + 1];
}

void compass_dir_name(const P& offs, std::string& dst)
{
    dst = compass_dir_names[offs.x + 1][offs.y + 1];
}

} //dir_utils

int Dice_param::roll() const
{
    return rnd::dice(rolls, sides) + plus;
}

int Range::roll() const
{
    return rnd::range(min, max);
}

bool Fraction::roll() const
{
    return rnd::fraction(num, den);
}

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

} //namespace

void seed(const unsigned long val)
{
    mt_rand = MTRand(val);
}

int dice(const int ROLLS, const int SIDES)
{
    return roll(ROLLS, SIDES);
}

bool coin_toss()
{
    return roll(1, 2) == 2;
}

bool fraction(const int NUMER, const int DENOM)
{
    //This function should never be called with a denominator less than one, since it's unclear
    //what it means that something should happen e.g. "N times in 0", or "N times in -1".
    assert(DENOM >= 1);

    //If numerator is bigger than denominator, it's most likely a bug (should something occur e.g.
    //5 times in 3 ???) - don't allow this...
    assert(NUMER <= DENOM);

    //A negative numerator is of course nonsense
    assert(NUMER >= 0);

    //If any of the rules above are broken on a release build, try to perform the action that was
    //*probably* intended.

    //NOTE: A numerator of 0 is always allowed (it simply means "no chance")

    if ((NUMER <= 0) || (DENOM <= 0))
    {
        return false;
    }

    if ((NUMER >= DENOM) || (DENOM == 1))
    {
        return true;
    }

    return roll(1, DENOM) <= NUMER;
}

bool one_in(const int N)
{
    return fraction(1, N);
}

int range(const int V1, const int V2)
{
    const int MIN = std::min(V1, V2);
    const int MAX = std::max(V1, V2);

    return MIN + roll(1, MAX - MIN + 1) - 1;
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

void to_vec(const bool* array2,
            const bool VALUE_TO_STORE,
            const int W,
            const int H,
            std::vector<P>& out)
{
    out.clear();

    for (int x = 0; x < W; ++x)
    {
        for (int y = 0; y < H; ++y)
        {
            const bool V = *(array2 + (x * H) + y);

            if (V == VALUE_TO_STORE)
            {
                out.push_back(P(x, y));
            }
        }
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

bool is_area_inside(const Rect& inner,
                    const Rect& outer,
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

int king_dist(const int X0, const int Y0, const int X1, const int Y1)
{
    return std::max(abs(X1 - X0), abs(Y1 - Y0));
}

int king_dist(const P& p0, const P& p1)
{
    return std::max(abs(p1.x - p0.x), abs(p1.y - p0.y));
}

int taxi_dist(const P& p0, const P& p1)
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
            closest_pos     = p_cmp;
        }
    }

    return closest_pos;
}

bool is_pos_adj(const P& pos1,
                const P& pos2,
                const bool COUNT_SAME_CELL_AS_ADJ)
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
    time_t t        = time(nullptr);
    struct tm* now  = localtime(&t);

    return Time_data(now->tm_year + 1900,
                     now->tm_mon + 1,
                     now->tm_mday,
                     now->tm_hour,
                     now->tm_min,
                     now->tm_sec);
}

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

std::string to_str(const int IN)
{
    std::ostringstream buffer;
    buffer << IN;
    return buffer.str();
}

int to_int(const std::string& in)
{
    int _nr;
    std::istringstream buffer(in);
    buffer >> _nr;
    return _nr;
}

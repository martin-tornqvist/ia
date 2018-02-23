#ifndef RL_UTILS_RANDOM_HPP
#define RL_UTILS_RANDOM_HPP

#include <random>
#include <algorithm>
#include <string>
#include <iomanip>
#include <sstream>

struct Dice
{
    Dice() :
        rolls(0),
        sides(0),
        plus(0) {}

    Dice(const int rolls, const int sides, const int plus = 0) :
        rolls(rolls),
        sides(sides),
        plus(plus) {}

    Dice(const Dice& other) :
        rolls(other.rolls),
        sides(other.sides),
        plus(other.plus) {}

    Dice& operator=(const Dice& other)
    {
        rolls = other.rolls;
        sides = other.sides;
        plus  = other.plus;
        return *this;
    }

    bool operator==(const Dice& other) const
    {
        return
            (rolls == other.rolls) &&
            (sides == other.sides) &&
            (plus == other.plus);
    }

    bool operator!=(const Dice& other) const
    {
        return !(*this == other);
    }

    int max() const
    {
        return (rolls * sides) + plus;
    }

    int min() const
    {
        return (rolls + plus);
    }

    double avg() const
    {
        const double roll_avg = ((double)sides + 1.0) / 2.0;

        const double roll_avg_tot = roll_avg * (double)rolls;

        return roll_avg_tot + (double)plus;
    }

    std::string str() const
    {
        const std::string rolls_str = std::to_string(rolls);

        const std::string sides_str = std::to_string(sides);

        const std::string plus_str = str_plus();

        return rolls_str + "d" + sides_str + plus_str;
    }

    std::string str_plus() const
    {
        return
            (plus == 0) ? "" :
            (plus > 0) ?
            ("+" + std::to_string(plus)) :
            ("-" + std::to_string(plus));
    }

    std::string str_avg() const
    {
        const double val = avg();

        double rounded = roundf(val * 100.0) / 100.0;

        std::ostringstream ss;

        ss << std::fixed << std::setprecision(1) << rounded;

        return ss.str();
    }

    int roll() const;

    int rolls, sides, plus;
};

struct Range
{
    Range() :
        min(-1),
        max(-1) {}

    Range(const int min, const int max) :
        min(min),
        max(max) {}

    Range(const Range& other) :
        Range(other.min, other.max) {}

    int len() const
    {
        return max - min + 1;
    }

    bool is_in_range(const int v) const
    {
        return
            (v >= min) &&
            (v <= max);
    }

    void set(const int min_val, const int max_val)
    {
        min = min_val;
        max = max_val;
    }

    Range& operator/=(const int v)
    {
        min /= v;
        max /= v;
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

    Fraction(const int num, const int den) :
        num(num),
        den(den) {}

    void set(const int num, const int den)
    {
        this->num = num;
        this->den = den;
    }

    Fraction& operator=(const Fraction& other)
    {
        num = other.num;
        den = other.den;

        return *this;
    }

    bool roll() const;

    int num, den;
};

//------------------------------------------------------------------------------
// Random number generation
//------------------------------------------------------------------------------
namespace rnd
{

extern std::mt19937 rng;

void seed();

void seed(uint32_t seed);

// NOTE: If not called with a positive non-zero number of sides, this will
// always return zero.
int dice(const int rolls, const int sides);

bool coin_toss();

bool fraction(const int num, const int den);

bool one_in(const int N);

// Can be called with any range (positive or negative), V2 does *not* have to be
// bigger than V1.
int range(const int v1, const int v2);

// NOTE: "p" shall be within [0.0, 1.0]
int range_binom(const int v1, const int v2, const double p);

bool percent(const int pct_chance);

int weighted_choice(const std::vector<int> weights);

template <typename T>
T element(const std::vector<T>& v)
{
    const size_t idx = range(0, v.size() - 1);

    return v[idx];
}

template <typename T>
size_t idx(const std::vector<T>& v)
{
    return range(0, v.size() - 1);
}

template <typename T>
void shuffle(std::vector<T>& v)
{
    std::shuffle(begin(v), end(v), rng);
}

} // rnd

#endif // RL_UTILS_RANDOM_HPP

#include "cmn_types.hpp"
#include "utils.hpp"

//---------------------------------------------------------- Dice
int Dice_param::roll() const
{
    return rnd::dice(rolls, sides) + plus;
}

//---------------------------------------------------------- Range
int Range::roll() const
{
    return rnd::range(min, max);
}

//---------------------------------------------------------- Fraction
bool Fraction::roll() const
{
    return rnd::fraction(numerator, denominator);
}

#include "colors.hpp"

bool is_clr_eq(const Clr& clr1, const Clr& clr2)
{
    return clr1.r == clr2.r &&
           clr1.g == clr2.g &&
           clr1.b == clr2.b;
}

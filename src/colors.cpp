#include "colors.hpp"

bool is_clr_equal(const Clr& clr1, const Clr& clr2)
{
    return clr1.r == clr2.r &&
           clr1.g == clr2.g &&
           clr1.b == clr2.b;
}

void div_clr(Clr& clr, const double div)
{
    clr.r = (Uint8)((double)clr.r / div);
    clr.g = (Uint8)((double)clr.g / div);
    clr.b = (Uint8)((double)clr.b / div);
}

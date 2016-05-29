#include "colors.hpp"

bool is_clr_equal(const Clr& clr1, const Clr& clr2)
{
    return clr1.r == clr2.r &&
           clr1.g == clr2.g &&
           clr1.b == clr2.b;
}

void div_clr(Clr& clr, const double DIV)
{
    clr.r = double(clr.r) / DIV;
    clr.g = double(clr.g) / DIV;
    clr.b = double(clr.b) / DIV;
}

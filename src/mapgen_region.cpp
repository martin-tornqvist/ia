#include "mapgen.hpp"

R Region::rnd_room_rect() const
{
    //
    // Set random size
    //

    // Minimum possible size
    const P min_size(2, 2);

    // Maximum possible size (cover the whole region)
    const P max_size(r.p1 - r.p0 + 1);

    // Set random width and height using binomial distribution
    const bool big = rnd::one_in(6);

    const double w_p = big ? 0.60 : 0.30;
    const double h_p = big ? 0.80 : 0.45;

    const int w = rnd::range_binom(min_size.x,
                                   max_size.x,
                                   w_p);

    const int h = rnd::range_binom(min_size.y,
                                   max_size.y,
                                   h_p);

    //
    // Set random position
    //

    const P p0(r.p0.x + rnd::range(0, max_size.x - w),
               r.p0.y + rnd::range(0, max_size.y - h));

    const P p1(p0.x + w - 1,
               p0.y + h - 1);

    return R(p0, p1);

} // rnd_room_rect

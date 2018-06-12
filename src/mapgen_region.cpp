#include "mapgen.hpp"

R Region::rnd_room_rect() const
{
    // Set random size
    const Range w_range(2, r.p1.x - r.p0.x + 1);
    const Range h_range(2, r.p1.y - r.p0.y + 1);

    // Set random width and height using binomial distribution
    // const double p = 0.25;

    // const int w = rnd::range_binom(
    //         min_size.x,
    //         max_size.x,
    //         p);

    // const int h = rnd::range_binom(
    //         min_size.y,
    //         max_size.y,
    //         p);

    const int w = w_range.roll();
    const int h = h_range.roll();

    // Set random position
    const P p0(r.p0.x + rnd::range(0, w_range.max - w),
               r.p0.y + rnd::range(0, h_range.max - h));

    const P p1(p0.x + w - 1,
               p0.y + h - 1);

    return R(p0, p1);

} // rnd_room_rect

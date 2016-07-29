#include "mapgen.hpp"

R Region::rnd_room_rect() const
{
    const bool allow_tiny_w = rnd::coin_toss();

    const P min_size_lmt(allow_tiny_w ? 2 : 4, allow_tiny_w ? 4 : 2);
    const P max_size_lmt(r.p1 - r.p0 + 1);

    const int   h           = rnd::range(min_size_lmt.y, max_size_lmt.y);
    const int   w_max_small = min_size_lmt.x + ((max_size_lmt.x - min_size_lmt.x) / 5);
    const int   w_max_big   = max_size_lmt.x;
    const bool  allow_big_w = h > (max_size_lmt.y * 5) / 6;
    const int   w_max       = allow_big_w ? w_max_big : w_max_small;
    const int   w           = rnd::range(min_size_lmt.x, w_max);

    const P p0(r.p0.x + rnd::range(0, max_size_lmt.x - w),
               r.p0.y + rnd::range(0, max_size_lmt.y - h));

    const P p1(p0.x + w - 1, p0.y + h - 1);

    return R(p0, p1);
}

#ifndef FOV_HPP
#define FOV_HPP

#include "rl_utils.hpp"
#include "global.hpp"

struct LosResult
{
        LosResult() :
                is_blocked_hard(false),
                is_blocked_by_drk(false) {}

        bool is_blocked_hard;
        bool is_blocked_by_drk;
};

namespace fov
{

R get_fov_rect(const P& p);

bool is_in_fov_range(const P& p0, const P& p1);

LosResult check_cell(
        const P& p0,
        const P& p1,
        const Array2<bool>& hard_blocked);

Array2<LosResult> run(const P& p0, const Array2<bool>& hard_blocked);

} // fov

#endif // FOV_HPP

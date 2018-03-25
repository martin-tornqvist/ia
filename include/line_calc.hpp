#ifndef LINE_CALC_HPP
#define LINE_CALC_HPP

#include <vector>

#include "rl_utils.hpp"

namespace line_calc
{

void init();

std::vector<P> calc_new_line(
        const P& origin,
        const P& target,
        const bool should_stop_at_target,
        const int king_dist_limit,
        const bool allow_outside_map);

const std::vector<P>* fov_delta_line(
        const P& delta,
        const double& max_dist_abs);

} // line_calc

#endif // LINE_CALC_HPP

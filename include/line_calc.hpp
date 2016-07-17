#ifndef LINE_CALC_HPP
#define LINE_CALC_HPP

#include <vector>

#include "rl_utils.hpp"

namespace line_calc
{

void init();

void calc_new_line(const P& origin, const P& tgt,
                   const bool should_stop_at_target,
                   const int cheb_travel_limit,
                   const bool allow_outside_map,
                   std::vector<P>& line_ref);

const std::vector<P>* fov_delta_line(const P& delta,
                                     const double& max_dist_abs);

} //line_calc

#endif

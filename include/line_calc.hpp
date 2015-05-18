#ifndef LINE_CALC_H
#define LINE_CALC_H

#include <vector>

#include "cmn_data.hpp"
#include "cmn_types.hpp"

namespace line_calc
{

void init();

void calc_new_line(const Pos& origin, const Pos& tgt,
                   const bool SHOULD_STOP_AT_TARGET, const int CHEB_TRAVEL_LIMIT,
                   const bool ALLOW_OUTSIDE_MAP, std::vector<Pos>& line_ref);

const std::vector<Pos>* fov_delta_line(const Pos& delta,
                                       const double& MAX_DIST_ABS);

} //line_calc

#endif

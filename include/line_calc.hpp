#ifndef LINE_CALC_H
#define LINE_CALC_H

#include <vector>

#include "cmn_data.h"
#include "cmn_types.h"

namespace Line_calc
{

void init();

void calc_new_line(const Pos& origin, const Pos& target,
                 const bool SHOULD_STOP_AT_TARGET, const int CHEB_TRAVEL_LIMIT,
                 const bool ALLOW_OUTSIDE_MAP, std::vector<Pos>& line_ref);

const std::vector<Pos>* get_fov_delta_line(const Pos& delta,
                                        const double& MAX_DIST_ABS);

} //Line_calc

#endif

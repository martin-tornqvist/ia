#ifndef LINE_CALC_H
#define LINE_CALC_H

#include <vector>
#include <iostream>

#include "CmnData.h"
#include "CmnTypes.h"

namespace LineCalc {

void init();

void calcNewLine(const Pos& origin, const Pos& target,
                 const bool SHOULD_STOP_AT_TARGET, const int CHEB_TRAVEL_LIMIT,
                 const bool ALLOW_OUTSIDE_MAP, std::vector<Pos>& lineRef);

const std::vector<Pos>* getFovDeltaLine(const Pos& delta,
                                   const double& MAX_DIST_ABS);

} //LineCalc

#endif

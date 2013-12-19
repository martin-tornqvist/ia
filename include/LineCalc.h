#ifndef LINE_CALC_H
#define LINE_CALC_H

#include <vector>
#include <iostream>

#include "CommonData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;

class LineCalc {
public:
  LineCalc(Engine& engine) :
    eng(engine) {
    calcFovAbsDistances();
    calcFovDeltaLines();
  }

  void calcNewLine(const Pos& origin, const Pos& target,
                   const bool SHOULD_STOP_AT_TARGET,
                   const int CHEB_TRAVEL_LIMIT,
                   const bool ALLOW_OUTSIDE_MAP,
                   vector<Pos>& lineRef);

  inline const vector<Pos>* getFovDeltaLine(
    const Pos& delta, const double& MAX_DIST_ABS) const {
    const int X = delta.x + FOV_MAX_RADI_INT;
    const int Y = delta.y + FOV_MAX_RADI_INT;
    if(X >= 0 && Y >= 0 && X < FOV_MAX_W_INT && Y < FOV_MAX_W_INT) {
      if(fovAbsDistances[X][Y] <= MAX_DIST_ABS) {
        return &(fovDeltaLines[X][Y]);
      }
    }
    return NULL;
  }

private:
  void calcFovAbsDistances();
  double fovAbsDistances[FOV_MAX_W_INT][FOV_MAX_W_INT];

  void calcFovDeltaLines();
  vector<Pos> fovDeltaLines[FOV_MAX_W_INT][FOV_MAX_W_INT];

  Engine& eng;
};

#endif

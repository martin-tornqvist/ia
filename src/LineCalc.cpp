#include "LineCalc.h"

#include <math.h>

#include "Engine.h"
#include "Utils.h"

void LineCalc::calcNewLine(const Pos& origin, const Pos& target,
                           const bool SHOULD_STOP_AT_TARGET,
                           const int CHEB_TRAVEL_LIMIT,
                           const bool ALLOW_OUTSIDE_MAP,
                           vector<Pos>& lineRef) {
  lineRef.resize(0);

  if(target == origin) {
    lineRef.push_back(origin);
    return;
  }

  const double DELTA_X_DB = double(target.x - origin.x);
  const double DELTA_Y_DB = double(target.y - origin.y);

  const double HYPOT_DB =
    sqrt((DELTA_X_DB * DELTA_X_DB) + (DELTA_Y_DB * DELTA_Y_DB));

  const double X_INCR_DB = (DELTA_X_DB / HYPOT_DB);
  const double Y_INCR_DB = (DELTA_Y_DB / HYPOT_DB);

  double curX_db = double(origin.x) + 0.5;
  double curY_db = double(origin.y) + 0.5;

  Pos curPos = Pos(int(curX_db), int(curY_db));

  const double STEP_SIZE_DB = 0.04;

  for(double i = 0.0; i <= 9999.0; i += STEP_SIZE_DB) {
    curX_db += X_INCR_DB * STEP_SIZE_DB;
    curY_db += Y_INCR_DB * STEP_SIZE_DB;

    curPos.set(floor(curX_db), floor(curY_db));

    if(ALLOW_OUTSIDE_MAP == false) {
      if(Utils::isPosInsideMap(curPos) == false) {
        return;
      }
    }

    bool isPosOkToAdd = false;
    if(lineRef.empty()) {
      isPosOkToAdd = true;
    } else {
      isPosOkToAdd = lineRef.back() != curPos;
    }
    if(isPosOkToAdd) {
      lineRef.push_back(curPos);
    }

    //Check distance limits
    if(SHOULD_STOP_AT_TARGET && (curPos == target)) {
      return;
    }
    const int DISTANCE_TRAVELED =
      Utils::chebyshevDist(
        origin.x, origin.y, curPos.x, curPos.y);
    if(DISTANCE_TRAVELED >= CHEB_TRAVEL_LIMIT) {
      return;
    }
  }
}

void LineCalc::calcFovDeltaLines() {
  const int R_INT = FOV_MAX_RADI_INT;

  for(int deltaX = -R_INT; deltaX <= R_INT; deltaX++) {
    for(int deltaY = -R_INT; deltaY <= R_INT; deltaY++) {
      const Pos origin(0, 0);
      const Pos target(Pos(deltaX, deltaY));
      vector<Pos> curLine;
      calcNewLine(origin, target, true, 999, true, curLine);
      fovDeltaLines[deltaX + R_INT][deltaY + R_INT] = curLine;
    }
  }
}

void LineCalc::calcFovAbsDistances() {
  for(int y = 0; y < FOV_MAX_W_INT; y++) {
    for(int x = 0; x < FOV_MAX_W_INT; x++) {
      fovAbsDistances[x][y] = 0;
    }
  }

  double deltaX, deltaY, hypot;

  //deltaX corresponds to i - FOV_MAX_W_INT, and vice versa for deltaY and j.
  //So:
  //i = 0                 => deltaX = -FOV_MAX_W_INT
  //i = FOV_MAX_W_INT * 2 => deltaX =  FOV_MAX_W_INT

  const int R_INT = FOV_MAX_RADI_INT;

  for(int x = 0; x <= R_INT * 2; x++) {
    for(int y = 0; y <= R_INT * 2; y++) {
      deltaX = double(x);
      deltaX -= FOV_MAX_RADI_DB;
      deltaY = double(y);
      deltaY -= FOV_MAX_RADI_DB;
      hypot = sqrt((deltaX * deltaX) + (deltaY * deltaY));
      fovAbsDistances[x][y] = floor(hypot);
    }
  }
}


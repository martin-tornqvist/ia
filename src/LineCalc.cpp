#include "LineCalc.h"

#include "Engine.h"

void LineCalc::calcNewLine(const Pos& origin, const Pos& target,
                           const bool SHOULD_STOP_AT_TARGET,
                           const int CHEB_TRAVEL_LIMIT,
                           const bool ALLOW_OUTSIDE_MAP,
                           vector<Pos>& lineToFill) {
  lineToFill.resize(0);

  if(target == origin) {
    lineToFill.push_back(origin);
    return;
  }

  double deltaX = (double(target.x) - double(origin.x));
  double deltaY = (double(target.y) - double(origin.y));

  double hypot = sqrt((deltaX * deltaX) + (deltaY * deltaY));

  double xIncr = (deltaX / hypot);
  double yIncr = (deltaY / hypot);

  double curX_prec = double(origin.x) + 0.5;
  double curY_prec = double(origin.y) + 0.5;

  Pos curPos = Pos(int(curX_prec), int(curY_prec));

  for(double i = 0; i <= 9999.0; i += 0.04) {
    curX_prec += xIncr * 0.04;
    curY_prec += yIncr * 0.04;

    curPos.set(int(curX_prec), int(curY_prec));

    if(ALLOW_OUTSIDE_MAP == false) {
      if(eng->basicUtils->isPosInsideMap(curPos) == false) {
        return;
      }
    }

    bool isPosOkToAdd = false;
    if(lineToFill.empty()) {
      isPosOkToAdd = true;
    } else {
      isPosOkToAdd = lineToFill.back() != curPos;
    }
    if(isPosOkToAdd) {
      lineToFill.push_back(curPos);
    }

    //Check distance limits
    if(stopAtTarget && (curPos == target)) {
      return;
    }
    const int DISTANCE_TRAVELED =
      eng->basicUtils->chebyshevDist(
        origin.x, origin.y, curPos.x, curPos.y);
    if(DISTANCE_TRAVELED >= chebTravelLimit) {
      return;
    }
  }
}

void LineCalc::calcFovDeltaLines() {
  for(int y = 0; y < FOV_MAX_W_INT; y++) {
    for(int x = 0; x < FOV_MAX_W_INT; x++) {
      fovLines[x][y] = 0.0;
    }
  }

  const int R_INT = FOV_MAX_RADI_INT;

  for(int deltaX = -R_INT; deltaX <= R_INT; deltaX++) {
    for(int deltaY = -R_INT; deltaY <= R_INT; deltaY++) {
      const Pos origin(0, 0);
      const Pos target(origin + Pos(deltaX, deltaY));
      vector<Pos> curLine;
      calcNewLine(origin, target, true, 999, true, curLine);
      lineTravelVectors[deltaX + R_INT][deltaY + R_INT] = currentPath;
    }
  }
}

void LineCalc::calcFovAbsDistances() {
  for(int y = 0; y < FOV_MAX_W_INT; y++) {
    for(int x = 0; x < FOV_MAX_W_INT; x++) {
      absDistances[x][y] = 0;
    }
  }

  double deltaX, deltaY, hypot, xIncr, yIncr;

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
      absDistances[x][y] = floor(hypot);
    }
  }
}


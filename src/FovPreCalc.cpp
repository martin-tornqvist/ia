#include "FovPreCalc.h"

//#include <math.h>

#include "Engine.h"

void FovPreCalc::calcLineTravelVectors() {
  calcLineAnglesAndAbsDistances();

  int deltaX = -FOV_MAX_RADI_INT;
  int deltaY = -FOV_MAX_RADI_INT;

  double xStep, yStep, incrInX, incrInY;

  const double originX = 0.5;
  const double originY = 0.5;

  coord currentDelta;

  vector<coord> currentPath;

  pair<double, double> angles;

  while(deltaX <= FOV_MAX_RADI_INT) {

    deltaY = -FOV_MAX_RADI_INT;

    while(deltaY <= FOV_MAX_RADI_INT) {
      currentPath.resize(0);
      incrInX = originX;
      incrInY = originY;
      getLineAngles(deltaX, deltaY, angles);
      xStep = angles.first * 0.04;
      yStep = angles.second * 0.04;
      for(double i = 0.0; i <= FOV_MAX_RADI_DB; i += 0.04) {
        currentDelta.set(int(floor(incrInX)), int(floor(incrInY)));
        //Add position to current vector if vector is empty, or if pos not equal to previous.
        bool isPositionGoodToAdd = currentPath.empty();
        if(isPositionGoodToAdd == false) {
          isPositionGoodToAdd = currentDelta != currentPath.back();
        }
        if(isPositionGoodToAdd == true) {
          currentPath.push_back(currentDelta);
        }
        if(currentDelta == coord(deltaX, deltaY)) {
          i = 999999;
        }
        incrInX += xStep;
        incrInY += yStep;
      }
      lineTravelVectors[deltaX + FOV_MAX_RADI_INT][deltaY + FOV_MAX_RADI_INT] = currentPath;
      deltaY++;
    }
    deltaX++;
  }
}

void FovPreCalc::calcLineAnglesAndAbsDistances() {
  double deltaX, deltaY, hypot, xIncr, yIncr;

  // deltaX corresponds to i - 16, and
  // vice versa for deltaY and j.

  // So:
  // i = 0  corresponds to delta_x = -16
  // i = 32       -||-             =  16

  for(unsigned int x = 0; x <= FOV_MAX_RADI_INT * 2; x++) {
    for(unsigned int y = 0; y <= FOV_MAX_RADI_INT * 2; y++) {
      deltaX = double(x);
      deltaX -= FOV_MAX_RADI_DB;
      deltaY = double(y);
      deltaY -= FOV_MAX_RADI_DB;
      hypot = sqrt((deltaX * deltaX) + (deltaY * deltaY));
      absDistances[x][y] = floor(hypot);
      xIncr = deltaX / hypot;
      yIncr = deltaY / hypot;
      vectorAnglesX[x][y] = xIncr;
      vectorAnglesY[x][y] = yIncr;
    }
  }
}



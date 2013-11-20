#ifndef FOV_PRECALC_H
#define FOV_PRECALC_H

#include <vector>
#include <iostream>

#include "CommonData.h"
#include "CommonTypes.h"

using namespace std;

class Engine;

class FovPreCalc {
public:
  FovPreCalc(Engine* engine) :
    eng(engine) {
    calcLineTravelVectors();
  }

  inline const vector<Pos>* getLineTravelVector(
    const Pos& delta, const double& MAX_DIST_ABS) const {
    const int X = delta.x + FOV_MAX_RADI_INT;
    const int Y = delta.y + FOV_MAX_RADI_INT;
    if(
      X < FOV_MAX_WIDTH_INT && Y < FOV_MAX_WIDTH_INT &&
      absDistances[X][Y] <= MAX_DIST_ABS) {
      return &(lineTravelVectors[X][Y]);
    }
    return NULL;
  }

private:
  void getLineAngles(const int deltaX, const int deltaY,
                     pair<double, double>& angles) const {
    const int X = deltaX + FOV_MAX_RADI_INT;
    const int Y = deltaY + FOV_MAX_RADI_INT;
    angles.first  = vectorAnglesX[X][Y];
    angles.second = vectorAnglesY[X][Y];
  }

  void calcLineAnglesAndAbsDistances();
  double vectorAnglesX[FOV_MAX_WIDTH_INT][FOV_MAX_WIDTH_INT];
  double vectorAnglesY[FOV_MAX_WIDTH_INT][FOV_MAX_WIDTH_INT];
  double absDistances[FOV_MAX_WIDTH_INT][FOV_MAX_WIDTH_INT];

  void calcLineTravelVectors();
  vector<Pos> lineTravelVectors[FOV_MAX_WIDTH_INT][FOV_MAX_WIDTH_INT];

  Engine* eng;
};

#endif

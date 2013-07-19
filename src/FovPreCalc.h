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

  void getLineTravelVector(vector<Pos>& vectorUsed, const Pos& delta, const double MAX_DIST_ABS) const {
    if(absDistances[delta.x + FOV_MAX_RADI_INT][delta.y +  FOV_MAX_RADI_INT] <= MAX_DIST_ABS) {
      vectorUsed = lineTravelVectors[delta.x + FOV_MAX_RADI_INT][delta.y + FOV_MAX_RADI_INT];
    }
  }

private:
  void getLineAngles(const int deltaX, const int deltaY, pair<double, double>& angles) const {
    angles.first = vectorAnglesX[deltaX + FOV_MAX_RADI_INT][deltaY + FOV_MAX_RADI_INT];
    angles.second = vectorAnglesY[deltaX + FOV_MAX_RADI_INT][deltaY + FOV_MAX_RADI_INT];
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

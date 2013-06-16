#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

#include <vector>

#include "CommonTypes.h"
#include "CommonSettings.h"

using namespace std;

class Engine;

enum PlacementRule_t {
  placementRule_nextToWalls,
  placementRule_awayFromWalls,
  placementRule_nextToWallsOrAwayFromWalls
};

class MapPatterns {
public:
  MapPatterns(Engine* engine) :
    eng(engine) {
  }

  void setPositionsInArea(const Rect& area, vector<coord>& nextToWalls, vector<coord>& awayFromWalls);

private:
  int getWalkBlockersInDirection(const Directions_t dir, const coord pos);

  Engine* eng;

};

#endif

#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

using namespace std;

#include <vector>

#include "ConstTypes.h"
#include "ConstDungeonSettings.h"

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

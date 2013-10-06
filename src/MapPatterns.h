#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

#include <vector>

#include "CommonData.h"
#include "CommonTypes.h"

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

  void setPositionsInArea(const Rect& area, vector<Pos>& nextToWalls,
                          vector<Pos>& awayFromWalls);

private:
  int getWalkBlockersInDirection(const Direction_t dir, const Pos pos);

  Engine* eng;

};

#endif

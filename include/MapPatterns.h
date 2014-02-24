#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

#include <vector>

#include "CommonData.h"
#include "CommonTypes.h"

//TODO This should be merged into MapParsing
//Also, most/all things in here really needs clearer purpose/naming

using namespace std;

class Engine;

enum PlacementRule {
  placementRule_nextToWalls,
  placementRule_awayFromWalls,
  placementRule_nextToWallsOrAwayFromWalls
};

class MapPatterns {
public:
  MapPatterns(Engine& engine) : eng(engine) {}

  void setPositionsInArea(const Rect& area, vector<Pos>& nextToWalls,
                          vector<Pos>& awayFromWalls);

  int getWalkBlockersInDir(const Dir dir, const Pos& pos);

private:
  Engine& eng;
};

#endif

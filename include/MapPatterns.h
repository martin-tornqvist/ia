#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

#include <vector>

#include "CmnData.h"
#include "CmnTypes.h"

//TODO This should be merged into MapParsing
//Also, most/all things in here really needs clearer purpose/naming

enum PlacementRule {
  placementRule_nextToWalls,
  placementRule_awayFromWalls,
  placementRule_nextToWallsOrAwayFromWalls
};

class MapPatterns {
public:
  MapPatterns() {}

  void setPositionsInArea(const Rect& area, std::vector<Pos>& nextToWalls,
                          std::vector<Pos>& awayFromWalls);

  int getWalkBlockersInDir(const Dir dir, const Pos& pos);

private:

};

#endif

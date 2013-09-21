#include "MapPatterns.h"

#include "Engine.h"
#include "Map.h"

void MapPatterns::setPositionsInArea(const Rect& area, vector<Pos>& nextToWalls, vector<Pos>& awayFromWalls) {
  trace << "MapPatterns::setPositionsInArea()..." << endl;
  vector<Pos> positionCandidates;
  positionCandidates.resize(0);

  for(int y = area.x0y0.y; y <= area.x1y1.y; y++) {
    for(int x = area.x0y0.x; x <= area.x1y1.x; x++) {
      FeatureStatic* const f = eng->map->featuresStatic[x][y];
      if(f->isBodyTypePassable(actorBodyType_normal) && f->canHaveStaticFeature()) {
        positionCandidates.push_back(Pos(x, y));
      }
    }
  }

  nextToWalls.resize(0);
  awayFromWalls.resize(0);

  for(unsigned int i = 0; i < positionCandidates.size(); i++) {
    const Pos pos = positionCandidates.at(i);

    const int BLOCKERS_RIGHT = getWalkBlockersInDirection(direction_right, pos);
    const int BLOCKERS_DOWN = getWalkBlockersInDirection(direction_down, pos);
    const int BLOCKERS_LEFT = getWalkBlockersInDirection(direction_left, pos);
    const int BLOCKERS_UP = getWalkBlockersInDirection(direction_up, pos);

    const bool IS_ALL_BLOCKERS_ZERO = BLOCKERS_RIGHT == 0 && BLOCKERS_DOWN == 0 && BLOCKERS_LEFT == 0 && BLOCKERS_UP == 0;

    if(IS_ALL_BLOCKERS_ZERO) {
      awayFromWalls.push_back(pos);
      continue;
    }

    bool isDoorAdjacent = false;
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        if(eng->map->featuresStatic[pos.x + dx][pos.y + dy]->getId() == feature_door) {
          isDoorAdjacent = true;
        }
      }
    }

    if(isDoorAdjacent) {
      continue;
    }

    if(
      (BLOCKERS_RIGHT == 3 && BLOCKERS_UP == 1 && BLOCKERS_DOWN == 1 && BLOCKERS_LEFT == 0) ||
      (BLOCKERS_RIGHT == 1 && BLOCKERS_UP == 3 && BLOCKERS_DOWN == 0 && BLOCKERS_LEFT == 1) ||
      (BLOCKERS_RIGHT == 1 && BLOCKERS_UP == 0 && BLOCKERS_DOWN == 3 && BLOCKERS_LEFT == 1) ||
      (BLOCKERS_RIGHT == 0 && BLOCKERS_UP == 1 && BLOCKERS_DOWN == 1 && BLOCKERS_LEFT == 3)) {
      nextToWalls.push_back(pos);
      continue;
    }

  }

  trace << "MapPatterns::setPositionsInArea() [DONE]" << endl;
}

int MapPatterns::getWalkBlockersInDirection(const Directions_t dir, const Pos pos) {
  int nrBlockers = 0;
  switch(dir) {
  case direction_right: {
    for(int dy = -1; dy <= 1; dy++) {
      if(eng->map->featuresStatic[pos.x + 1][pos.y + dy]->isBodyTypePassable(actorBodyType_normal) == false) {
        nrBlockers += 1;
      }
    }
  }
  break;
  case direction_down: {
    for(int dx = -1; dx <= 1; dx++) {
      if(eng->map->featuresStatic[pos.x + dx][pos.y + 1]->isBodyTypePassable(actorBodyType_normal) == false) {
        nrBlockers += 1;
      }
    }
  }
  break;
  case direction_left: {
    for(int dy = -1; dy <= 1; dy++) {
      if(eng->map->featuresStatic[pos.x - 1][pos.y + dy]->isBodyTypePassable(actorBodyType_normal) == false) {
        nrBlockers += 1;
      }
    }
  }
  break;
  case direction_up: {
    for(int dx = -1; dx <= 1; dx++) {
      if(eng->map->featuresStatic[pos.x + dx][pos.y - 1]->isBodyTypePassable(actorBodyType_normal) == false) {
        nrBlockers += 1;
      }
    }
  }
  break;
  }
  return nrBlockers;
}

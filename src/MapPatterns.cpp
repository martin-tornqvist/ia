#include "MapPatterns.h"

#include "Map.h"

void MapPatterns::setPositionsInArea(
  const Rect& area, vector<Pos>& nextToWalls, vector<Pos>& awayFromWalls) {

  trace << "MapPatterns::setPositionsInArea()..." << endl;
  vector<Pos> positionCandidates;
  positionCandidates.resize(0);

  for(int y = area.x0y0.y; y <= area.x1y1.y; y++) {
    for(int x = area.x0y0.x; x <= area.x1y1.x; x++) {
      FeatureStatic* const f = Map::cells[x][y].featureStatic;
      if(f->canMoveCmn() && f->canHaveStaticFeature()) {
        positionCandidates.push_back(Pos(x, y));
      }
    }
  }

  nextToWalls.resize(0);
  awayFromWalls.resize(0);

  for(Pos & pos : positionCandidates) {
    const int NR_BLK_R = getWalkBlockersInDir(Dir::right, pos);
    const int NR_BLK_D = getWalkBlockersInDir(Dir::down, pos);
    const int NR_BLK_L = getWalkBlockersInDir(Dir::left, pos);
    const int NR_BLK_U = getWalkBlockersInDir(Dir::up, pos);

    const bool IS_ZERO_BLK_ALL_DIR =
      NR_BLK_R == 0 && NR_BLK_D == 0 && NR_BLK_L == 0 && NR_BLK_U == 0;

    if(IS_ZERO_BLK_ALL_DIR) {
      awayFromWalls.push_back(pos);
      continue;
    }

    bool isDoorAdjacent = false;
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        const FeatureStatic* const f =
          Map::cells[pos.x + dx][pos.y + dy].featureStatic;
        if(f->getId() == FeatureId::door) {
          isDoorAdjacent = true;
        }
      }
    }

    if(isDoorAdjacent) {
      continue;
    }

    if(
      (NR_BLK_R == 3 && NR_BLK_U == 1 && NR_BLK_D == 1 && NR_BLK_L == 0) ||
      (NR_BLK_R == 1 && NR_BLK_U == 3 && NR_BLK_D == 0 && NR_BLK_L == 1) ||
      (NR_BLK_R == 1 && NR_BLK_U == 0 && NR_BLK_D == 3 && NR_BLK_L == 1) ||
      (NR_BLK_R == 0 && NR_BLK_U == 1 && NR_BLK_D == 1 && NR_BLK_L == 3)) {
      nextToWalls.push_back(pos);
      continue;
    }

  }

  trace << "MapPatterns::setPositionsInArea() [DONE]" << endl;
}

int MapPatterns::getWalkBlockersInDir(const Dir dir, const Pos& pos) {
  int nrBlockers = 0;
  switch(dir) {
    case Dir::right: {
      for(int dy = -1; dy <= 1; dy++) {
        const FeatureStatic* const f =
          Map::cells[pos.x + 1][pos.y + dy].featureStatic;
        if(f->canMoveCmn() == false) {
          nrBlockers += 1;
        }
      }
    } break;

    case Dir::down: {
      for(int dx = -1; dx <= 1; dx++) {
        const FeatureStatic* const f =
          Map::cells[pos.x + dx][pos.y + 1].featureStatic;
        if(f->canMoveCmn() == false) {
          nrBlockers += 1;
        }
      }
    } break;

    case Dir::left: {
      for(int dy = -1; dy <= 1; dy++) {
        const FeatureStatic* const f =
          Map::cells[pos.x - 1][pos.y + dy].featureStatic;
        if(f->canMoveCmn() == false) {
          nrBlockers += 1;
        }
      }
    } break;

    case Dir::up: {
      for(int dx = -1; dx <= 1; dx++) {
        const FeatureStatic* const f =
          Map::cells[pos.x + dx][pos.y - 1].featureStatic;
        if(f->canMoveCmn() == false) {
          nrBlockers += 1;
        }
      }
    } break;

    case Dir::downLeft:
    case Dir::downRight:
    case Dir::upLeft:
    case Dir::upRight:
    case Dir::center:
    case Dir::endOfDirs: {} break;
  }
  return nrBlockers;
}

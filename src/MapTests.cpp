#include "MapTests.h"

#include <cassert>

#include "Engine.h"
#include "Map.h"
#include "ActorPlayer.h"

bool IsCloserToOrigin::operator()(const coord& c1, const coord& c2) {
  const int chebDist1 = eng->basicUtils->chebyshevDistance(c_.x, c_.y, c1.x, c1.y);
  const int chebDist2 = eng->basicUtils->chebyshevDistance(c_.x, c_.y, c2.x, c2.y);
  return chebDist1 < chebDist2;
}

coord MapTests::getClosestPos(const coord c, const vector<coord>& positions) const {
  int distToNearest = 99999;
  int closestElement = 0;
  for(unsigned int i = 0; i < positions.size(); i++) {
    const int CUR_DIST = eng->basicUtils->chebyshevDistance(c, positions.at(i));
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
      closestElement = i;
    }
  }

  return positions.at(closestElement);
}

Actor* MapTests::getClosestActor(const coord c, const vector<Actor*>& actors) const {
  if(actors.size() == 0) return NULL;

  int distToNearest = 99999;
  int closestElement = 0;
  for(unsigned int i = 0; i < actors.size(); i++) {
    const int CUR_DIST = eng->basicUtils->chebyshevDistance(c, actors.at(i)->pos);
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
      closestElement = i;
    }
  }

  return actors.at(closestElement);
}

void MapTests::makeVisionBlockerArray(const coord& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS], const int MAX_VISION_RANMGE) {
  const int X0 = max(0, origin.x - MAX_VISION_RANMGE - 1);
  const int Y0 = max(0, origin.y - MAX_VISION_RANMGE - 1);
  const int X1 = min(MAP_X_CELLS - 1, origin.x + MAX_VISION_RANMGE + 1);
  const int Y1 = min(MAP_Y_CELLS - 1, origin.y + MAX_VISION_RANMGE + 1);

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      arrayToFill[x][y] = eng->map->featuresStatic[x][y]->isVisionPassable() == false;
    }
  }

  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
  int x = 0;
  int y = 0;
  FeatureMob* f = NULL;
  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    f = eng->gameTime->getFeatureMobAt(i);
    x = f->getX();
    y = f->getY();
    if(arrayToFill[x][y] == false) {
      arrayToFill[x][y] = f->isVisionPassable() == false;
    }
  }
}

void MapTests::makeMoveBlockerArray(const Actor* const actorMoving, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  makeMoveBlockerArrayForMoveType(actorMoving->getMoveType(), arrayToFill);
}

void MapTests::makeMoveBlockerArrayFeaturesOnly(const Actor* const actorMoving, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  makeMoveBlockerArrayForMoveTypeFeaturesOnly(actorMoving->getMoveType(), arrayToFill);
}

void MapTests::makeWalkBlockingArrayFeaturesOnly(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, arrayToFill);
}

void MapTests::makeMoveBlockerArrayForMoveTypeFeaturesOnly(const MoveType_t moveType, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      arrayToFill[x][y] = !(eng->map->featuresStatic[x][y]->isMoveTypePassable(moveType));
    }
  }
  FeatureMob* f = NULL;
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    f = eng->gameTime->getFeatureMobAt(i);
    if(arrayToFill[f->getX()][f->getY()] == false) {
      arrayToFill[f->getX()][f->getY()] = !(f->isMoveTypePassable(moveType));
    }
  }
}

void MapTests::makeShootBlockerFeaturesArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      arrayToFill[x][y] = eng->map->featuresStatic[x][y]->isShootPassable() == false;
    }
  }
  FeatureMob* f = NULL;
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    f = eng->gameTime->getFeatureMobAt(i);
    if(arrayToFill[f->getX()][f->getY()] == false) {
      arrayToFill[f->getX()][f->getY()] = f->isShootPassable() == false;
    }
  }
}

void MapTests::addItemsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->items[x][y] != NULL) {
        arrayToFill[x][y] = true;
      }
    }
  }
}

void MapTests::makeMoveBlockerArrayForMoveType(const MoveType_t moveType, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      arrayToFill[x][y] = !(eng->map->featuresStatic[x][y]->isMoveTypePassable(moveType));
    }
  }
  FeatureMob* f = NULL;
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    f = eng->gameTime->getFeatureMobAt(i);
    if(arrayToFill[f->getX()][f->getY()] == false) {
      arrayToFill[f->getX()][f->getY()] = !(f->isMoveTypePassable(moveType));
    }
  }
  addLivingActorsToBlockerArray(arrayToFill);
}

void MapTests::makeItemBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int y = MAP_Y_CELLS - 1; y >= 0; y--) {
    for(int x = MAP_X_CELLS - 1; x >= 0; x--) {
      arrayToFill[x][y] = !eng->map->featuresStatic[x][y]->canHaveItem();
    }
  }
  FeatureMob* f = NULL;
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    f = eng->gameTime->getFeatureMobAt(i);
    if(arrayToFill[f->getX()][f->getY()] == false) {
      arrayToFill[f->getX()][f->getY()] = !f->canHaveItem();
    }
  }
  //addActorsToBlockerArray(arrayToFill); //Why?
}

void MapTests::addLivingActorsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  Actor* a = NULL;
  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    a = eng->gameTime->getActorAt(i);
    if(a->deadState == actorDeadState_alive) {
      if(arrayToFill[a->pos.x][a->pos.y] == false) {
        arrayToFill[a->pos.x][a->pos.y] = true;
      }
    }
  }
}

void MapTests::addAllActorsToBlockerArray(bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  Actor* a = NULL;
  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    a = eng->gameTime->getActorAt(i);
    if(arrayToFill[a->pos.x][a->pos.y] == false) {
      arrayToFill[a->pos.x][a->pos.y] = true;
    }
  }
}

void MapTests::addAdjacentLivingActorsToBlockerArray(const coord origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
  Actor* a = NULL;
  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    a = eng->gameTime->getActorAt(i);
    if(a->deadState == actorDeadState_alive) {
      if(arrayToFill[a->pos.x][a->pos.y] == false && a->pos != origin) {
        if(isCellsNeighbours(origin, a->pos, false)) {
          arrayToFill[a->pos.x][a->pos.y] = true;
        }
      }
    }
  }
}

void MapTests::makeMapVectorFromArray(bool a[MAP_X_CELLS][MAP_Y_CELLS], vector<coord>& vectorToFill) {
  vectorToFill.resize(0);
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(a[x][y]) {
        vectorToFill.push_back(coord(x, y));
      }
    }
  }
}

void MapTests::makeFloodFill(const coord origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
                             int valueArray[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit,
                             const coord target) {
  eng->basicUtils->resetArray(valueArray);

  vector<coord> coordinates;
  coord c;

  int currentX = origin.x;
  int currentY = origin.y;

  int currentValue = 0;

  bool pathExists = true;
  bool atTarget = false;

  bool stopAtTarget = target.x != -1;

  bool done = false;
  while(done == false) {
    for(int dx = -1; dx <= 1; dx++) {
      for(int dy = -1; dy <= 1; dy++) {
        if((dx != 0 || dy != 0)) {
          //TODO Improve readability
          if(
            blockers[currentX + dx][currentY + dy] == false &&
            (currentX + dx >= 1 && currentY + dy >= 1 &&
             currentX + dx < MAP_X_CELLS - 1 && currentY + dy < MAP_Y_CELLS - 1) &&
            (valueArray[currentX + dx][currentY + dy] == 0)) {
            currentValue = valueArray[currentX][currentY];

            if(currentValue < travelLimit) {
              valueArray[currentX + dx][currentY + dy] = currentValue + 1;
            }

            if(stopAtTarget == true) {
              if(currentX == target.x - dx && currentY == target.y - dy) {
                atTarget = true;
                dx = 9999;
                dy = 9999;
              }
            }

            if(stopAtTarget == false || atTarget == false) {
              c.x = currentX + dx;
              c.y = currentY + dy;
              coordinates.push_back(c);
            }
          }
        }
      }
    }

    if(stopAtTarget == true) {
      if(coordinates.size() == 0) {
        pathExists = false;
      }
      if(atTarget == true || pathExists == false) {
        done = true;
      }
    } else if(coordinates.size() == 0) {
      done = true;
    }

    if(currentValue == travelLimit) {
      done = true;
    }

    if(stopAtTarget == false || atTarget == false) {
      if(coordinates.size() == 0) {
        pathExists = false;
      } else {
        c = coordinates.front();
        currentX = c.x;
        currentY = c.y;
        coordinates.erase(coordinates.begin());
      }
    }
  }
}

bool MapTests::isCellsNeighbours(const int x1, const int y1, const int x2, const int y2, const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {
  if(x1 == x2 && y1 == y2) {
    return COUNT_SAME_CELL_AS_NEIGHBOUR;
  }
  if(x1 < x2 - 1) {
    return false;
  }
  if(x1 > x2 + 1) {
    return false;
  }
  if(y1 < y2 - 1) {
    return false;
  }
  if(y1 > y2 + 1) {
    return false;
  }
  return true;
}

bool MapTests::isCellsNeighbours(const coord c1, const coord c2, const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {
  return isCellsNeighbours(c1.x, c1.y, c2.x, c2.y, COUNT_SAME_CELL_AS_NEIGHBOUR);
}

bool MapTests::isCellNextToPlayer(const int x, const int y, const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {
  return isCellsNeighbours(x, y, eng->player->pos.x, eng->player->pos.y, COUNT_SAME_CELL_AS_NEIGHBOUR);
}

vector<coord> MapTests::getLine(int originX, int originY, int targetX, int targetY, bool stopAtTarget, int chebTravelLimit) {
  double deltaX = (double(targetX) - double(originX));
  double deltaY = (double(targetY) - double(originY));

  double hypot = sqrt((deltaX * deltaX) + (deltaY * deltaY));

  double xIncr = (deltaX / hypot);
  double yIncr = (deltaY / hypot);

  double curX_prec = double(originX) + 0.5;
  double curY_prec = double(originY) + 0.5;

  int curX = int(curX_prec);
  int curY = int(curY_prec);

  vector<coord> line;
  line.resize(0);
  coord c;

  for(double i = 0; i <= 9999.0; i += 0.04) {
    curX_prec += xIncr * 0.04;
    curY_prec += yIncr * 0.04;

    curX = int(curX_prec);
    curY = int(curY_prec);

    if(eng->mapTests->isCellInsideMainScreen(curX, curY) == false) {
      return line;
    }

    if(line.size() == 0) {
      c.x = curX;
      c.y = curY;
      line.push_back(c);
    } else {
      if(line.back().x != curX || line.back().y != curY) {
        c.x = curX;
        c.y = curY;
        line.push_back(c);
      }
    }

    //Check distance limits
    if(stopAtTarget == true && (curX == targetX && curY == targetY)) {
      return line;
    }
    if(eng->basicUtils->chebyshevDistance(originX, originY, curX, curY) >= chebTravelLimit) {
      return line;
    }
  }

  return line;
}

Actor* MapTests::getActorAtPos(const coord pos) const {
  const unsigned int LOOP_SIZE = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    Actor* actor = eng->gameTime->getActorAt(i);
    if(actor->pos == pos && actor->deadState == actorDeadState_alive) {
      return actor;
    }
  }
  return NULL;
}


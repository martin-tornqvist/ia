#include "MapTests.h"

#include <cassert>

#include "Engine.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "GameTime.h"

bool IsCloserToOrigin::operator()(const Pos& c1, const Pos& c2) {
  const int chebDist1 = eng->basicUtils->chebyshevDist(c_.x, c_.y, c1.x, c1.y);
  const int chebDist2 = eng->basicUtils->chebyshevDist(c_.x, c_.y, c2.x, c2.y);
  return chebDist1 < chebDist2;
}

Pos MapTests::getClosestPos(const Pos c, const vector<Pos>& positions) const {
  int distToNearest = 99999;
  int closestElement = 0;
  for(unsigned int i = 0; i < positions.size(); i++) {
    const int CUR_DIST = eng->basicUtils->chebyshevDist(c, positions.at(i));
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
      closestElement = i;
    }
  }

  return positions.at(closestElement);
}

void MapTests::getActorsPositions(const vector<Actor*>& actors,
                                  vector<Pos>& vectorToFill) {
  vectorToFill.resize(0);
  const unsigned int NR_ACTORS = actors.size();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    vectorToFill.push_back(actors.at(i)->pos);
  }
}

Actor* MapTests::getClosestActor(const Pos c,
                                 const vector<Actor*>& actors) const {
  if(actors.size() == 0) return NULL;

  int distToNearest = 99999;
  int closestElement = 0;
  for(unsigned int i = 0; i < actors.size(); i++) {
    const int CUR_DIST = eng->basicUtils->chebyshevDist(c, actors.at(i)->pos);
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
      closestElement = i;
    }
  }

  return actors.at(closestElement);
}

void MapTests::makeVisionBlockerArray(
  const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS],
  const int MAX_VISION_RANGE) {

  const int X0 = max(0, origin.x - MAX_VISION_RANGE - 1);
  const int Y0 = max(0, origin.y - MAX_VISION_RANGE - 1);
  const int X1 = min(MAP_X_CELLS - 1, origin.x + MAX_VISION_RANGE + 1);
  const int Y1 = min(MAP_Y_CELLS - 1, origin.y + MAX_VISION_RANGE + 1);

  const Map* const map = eng->map;

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      arrayToFill[x][y] =
        map->featuresStatic[x][y]->isVisionPassable() == false;
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

void MapTests::makeMoveBlockerArray(
  const Actor* const actorMoving,
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  makeMoveBlockerArrayForBodyType(actorMoving->getBodyType(), arrayToFill);
}

void MapTests::makeMoveBlockerArrayFeaturesOnly(
  const Actor* const actorMoving,
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  makeMoveBlockerArrayForBodyTypeFeaturesOnly(
    actorMoving->getBodyType(), arrayToFill);
}

void MapTests::makeWalkBlockingArrayFeaturesOnly(
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  makeMoveBlockerArrayForBodyTypeFeaturesOnly(
    actorBodyType_normal, arrayToFill);
}

void MapTests::makeMoveBlockerArrayForBodyTypeFeaturesOnly(
  const ActorBodyType_t bodyType,
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  const Map* const map = eng->map;

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      arrayToFill[x][y] =
        map->featuresStatic[x][y]->isBodyTypePassable(bodyType) == false;
    }
  }
  GameTime* const gameTime = eng->gameTime;
  FeatureMob* f = NULL;
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    f = gameTime->getFeatureMobAt(i);
    const Pos& pos = f->getPos();
    if(arrayToFill[pos.x][pos.y] == false) {
      arrayToFill[pos.x][pos.y] = f->isBodyTypePassable(bodyType) == false;
    }
  }
}

void MapTests::makeShootBlockerFeaturesArray(
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

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

void MapTests::addItemsToBlockerArray(
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->items[x][y] != NULL) {
        arrayToFill[x][y] = true;
      }
    }
  }
}

void MapTests::makeMoveBlockerArrayForBodyType(
  const ActorBodyType_t bodyType,
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      arrayToFill[x][y] =
        eng->map->featuresStatic[x][y]->isBodyTypePassable(bodyType) == false;
    }
  }
  FeatureMob* f = NULL;
  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
    f = eng->gameTime->getFeatureMobAt(i);
    const Pos& pos = f->getPos();
    if(arrayToFill[pos.x][pos.y] == false) {
      arrayToFill[pos.x][pos.y] = f->isBodyTypePassable(bodyType) == false;
    }
  }
  addLivingActorsToBlockerArray(arrayToFill);
}

void MapTests::makeItemBlockerArray(
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

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

void MapTests::addLivingActorsToBlockerArray(
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

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

void MapTests::addAllActorsToBlockerArray(
  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  Actor* a = NULL;
  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    a = eng->gameTime->getActorAt(i);
    if(arrayToFill[a->pos.x][a->pos.y] == false) {
      arrayToFill[a->pos.x][a->pos.y] = true;
    }
  }
}

void MapTests::addAdjLivingActorsToBlockerArray(
  const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {

  Actor* a = NULL;
  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    a = eng->gameTime->getActorAt(i);
    if(a->deadState == actorDeadState_alive) {
      if(arrayToFill[a->pos.x][a->pos.y] == false && a->pos != origin) {
        if(isCellsAdj(origin, a->pos, false)) {
          arrayToFill[a->pos.x][a->pos.y] = true;
        }
      }
    }
  }
}

bool MapTests::isCellNextToPlayer(
  const Pos& pos,
  const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {

  return isCellsAdj(pos, eng->player->pos, COUNT_SAME_CELL_AS_NEIGHBOUR);
}

void MapTests::makeBoolVectorFromMapArray(
  bool a[MAP_X_CELLS][MAP_Y_CELLS], vector<Pos>& vectorToFill) {

  vectorToFill.resize(0);
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      if(a[x][y]) {
        vectorToFill.push_back(Pos(x, y));
      }
    }
  }
}

void MapTests::floodFill(
  const Pos& origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
  int values[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit, const Pos& target) {

  eng->basicUtils->resetArray(values);

  vector<Pos> positions;
  positions.resize(0);
  unsigned int nrElementsToSkip = 0;
  Pos c;

  int currentX = origin.x;
  int currentY = origin.y;

  int currentValue = 0;

  bool pathExists = true;
  bool isAtTarget = false;

  bool isStoppingAtTarget = target.x != -1;

  const Rect bounds(Pos(1, 1), Pos(MAP_X_CELLS - 2, MAP_Y_CELLS - 2));

  bool done = false;
  while(done == false) {
    for(int dx = -1; dx <= 1; dx++) {
      for(int dy = -1; dy <= 1; dy++) {
        if((dx != 0 || dy != 0)) {
          const Pos newPos(currentX + dx, currentY + dy);
          if(
            blockers[newPos.x][newPos.y] == false           &&
            isCellInside(Pos(newPos.x, newPos.y), bounds) &&
            values[newPos.x][newPos.y] == 0) {
            currentValue = values[currentX][currentY];

            if(currentValue < travelLimit) {
              values[newPos.x][newPos.y] = currentValue + 1;
            }

            if(isStoppingAtTarget) {
              if(currentX == target.x - dx && currentY == target.y - dy) {
                isAtTarget = true;
                dx = 9999;
                dy = 9999;
              }
            }

            if(isStoppingAtTarget == false || isAtTarget == false) {
              positions.push_back(newPos);
            }
          }
        }
      }
    }

    if(isStoppingAtTarget) {
      if(positions.size() == nrElementsToSkip) {
        pathExists = false;
      }
      if(isAtTarget || pathExists == false) {
        done = true;
      }
    } else if(positions.size() == nrElementsToSkip) {
      done = true;
    }

    if(currentValue == travelLimit) {
      done = true;
    }

    if(isStoppingAtTarget == false || isAtTarget == false) {
      if(positions.size() == nrElementsToSkip) {
        pathExists = false;
      } else {
        c = positions.at(nrElementsToSkip);
        currentX = c.x;
        currentY = c.y;
        //positions.erase(positions.begin()); //Slow!
        nrElementsToSkip++;
      }
    }
  }
}

bool MapTests::isCellsAdj(
  const Pos& pos1, const Pos& pos2,
  const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {

  if(pos1.x == pos2.x && pos1.y == pos2.y) {
    return COUNT_SAME_CELL_AS_NEIGHBOUR;
  }
  if(pos1.x < pos2.x - 1) {
    return false;
  }
  if(pos1.x > pos2.x + 1) {
    return false;
  }
  if(pos1.y < pos2.y - 1) {
    return false;
  }
  if(pos1.y > pos2.y + 1) {
    return false;
  }
  return true;
}

void MapTests::getLine(const Pos& origin, const Pos& target,
                       bool stopAtTarget, int chebTravelLimit,
                       vector<Pos>& posList) {
  posList.resize(0);

  if(target == origin) {
    posList.push_back(origin);
    return;
  }

  double deltaX = (double(target.x) - double(origin.x));
  double deltaY = (double(target.y) - double(origin.y));

  double hypot = sqrt((deltaX * deltaX) + (deltaY * deltaY));

  double xIncr = (deltaX / hypot);
  double yIncr = (deltaY / hypot);

  double curX_prec = double(origin.x) + 0.5;
  double curY_prec = double(origin.y) + 0.5;

  Pos curPos = Pos(int(curX_prec), int(curY_prec));

  for(double i = 0; i <= 9999.0; i += 0.04) {
    curX_prec += xIncr * 0.04;
    curY_prec += yIncr * 0.04;

    curPos.set(int(curX_prec), int(curY_prec));

    if(eng->mapTests->isCellInsideMap(curPos) == false) {
      return;
    }

    bool isPosOkToAdd = false;
    if(posList.size() == 0) {
      isPosOkToAdd = true;
    } else {
      isPosOkToAdd = posList.back() != curPos;
    }
    if(isPosOkToAdd) {
      posList.push_back(curPos);
    }

    //Check distance limits
    if(stopAtTarget && (curPos == target)) {
      return;
    }
    const int DISTANCE_TRAVELED =
      eng->basicUtils->chebyshevDist(
        origin.x, origin.y, curPos.x, curPos.y);
    if(DISTANCE_TRAVELED >= chebTravelLimit) {
      return;
    }
  }
}

Actor* MapTests::getActorAtPos(const Pos pos) const {
  const unsigned int LOOP_SIZE = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < LOOP_SIZE; i++) {
    Actor* actor = eng->gameTime->getActorAt(i);
    if(actor->pos == pos && actor->deadState == actorDeadState_alive) {
      return actor;
    }
  }
  return NULL;
}


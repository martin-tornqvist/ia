#include "MapParsing.h"

#include "Engine.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "GameTime.h"

//------------------------------------------------------------ CELL PREDICATES
bool CellPredBlocksVision::check(const Cell& c)  const {
  return c.featureStatic->isVisionPassable() == false;
}

bool CellPredBlocksVision::check(const FeatureMob& f) const {
  return f.isVisionPassable() == false;
}

bool CellPredBlocksBodyType::check(const FeatureMob& f) const {
  return f.isBodyTypePassable(bodyType_) == false;
}

bool CellPredBlocksBodyType::check(const Actor& a) const {
  return a.deadState == actorDeadState_alive;
}

bool CellPredBlocksProjectiles::check(const Cell& c)  const {
  return c.featureStatic->isProjectilesPassable() == false;
}

bool CellPredBlocksProjectiles::check(const FeatureMob& f)  const {
  return f.isProjectilesPassable() == false;
}

bool CellPredLivingActorsAdjToPos::check(const Actor& a) const {
  if(a.deadState != actorDeadState_alive) {
    return false;
  }
  return eng->basicUtils->isPosAdj(pos_, a.pos, true);
}

//------------------------------------------------------------ MAP PARSER
void MapParser::parse(
  const CellPred& predicate, bool arrayOut[MAP_X_CELLS][MAP_Y_CELLS],
  const MapParseWriteRule writeRule) {

  if(
    predicate.isCheckingCells()       == false &&
    predicate.isCheckingMobFeatures() == false &&
    predicate.isCheckingActors()      == false) {
    throw runtime_error("Predicate not checking anything");
  }

  const Engine* const eng = predicate.eng;

  const bool WRITE_T = writeRule == mapParseWriteAlways ||
                       writeRule == mapParseWriteOnlyTrue;

  const bool WRITE_F = writeRule == mapParseWriteAlways ||
                       writeRule == mapParseWriteOnlyFalse;

  if(predicate.isCheckingCells()) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
        const Cell& c = eng->map->cells[x][y];
        const bool IS_MATCH = predicate.check(c);
        if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
          arrayOut[x][y] = IS_MATCH;
        }
      }
    }
  }

  if(predicate.isCheckingMobFeatures()) {
    const int NR_MOB_FEATURES = eng->gameTime->getNrFeatureMobs();
    for(int i = 0; i < NR_MOB_FEATURES; i++) {
      const FeatureMob& f = eng->gameTime->getFeatureMobAtElement(i);
      const Pos& p = f.getPos();
      const bool IS_MATCH = predicate.check(f);
      if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
        arrayOut[p.x][p.y] = IS_MATCH;
      }
    }
  }

  if(predicate.isCheckingActors()) {
    const int NR_ACTORS = eng->gameTime->getNrActors();
    for(int i = 0; i < NR_ACTORS; i++) {
      const Actor& a = eng->gameTime->getActorAtElement(i);
      const Pos& p = a.pos;
      const bool IS_MATCH = predicate.check(a);
      if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
        arrayOut[p.x][p.y] = IS_MATCH;
      }
    }
  }
}


bool IsCloserToOrigin::operator()(const Pos& c1, const Pos& c2) {
  const int chebDist1 = eng->basicUtils->chebyshevDist(c_.x, c_.y, c1.x, c1.y);
  const int chebDist2 = eng->basicUtils->chebyshevDist(c_.x, c_.y, c2.x, c2.y);
  return chebDist1 < chebDist2;
}

//------------------------------------------------------------ FLOOD FILL
void FloodFill::run(
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
            blockers[newPos.x][newPos.y] == false &&
            eng->basicUtils->isPosInside(Pos(newPos.x, newPos.y), bounds) &&
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
        nrElementsToSkip++;
      }
    }
  }
}

//------------------------------------------------------------ PATHFINDER
void PathFinder::run(const Pos& origin, const Pos& target,
                     bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
                     vector<Pos>& vectorToFill) const {

  vectorToFill.resize(0);

  int floodValues[MAP_X_CELLS][MAP_Y_CELLS];
  eng->floodFill->run(origin, blockers, floodValues, 1000, target);

  bool pathExists = floodValues[target.x][target.y] != 0;

  if(pathExists == true) {
    vector<Pos> positions;
    Pos c;

    int currentX = target.x;
    int currentY = target.y;

    bool done = false;
    while(done == false) {
      //TODO use for-loop instead
      //starts from 0 instead of -1 so that cardinal directions are tried first
      int xOffset = 0;
      while(xOffset <= 1) {
        int yOffset = 0;
        while(yOffset <= 1) {
          if(xOffset != 0 || yOffset != 0) {
            //TODO increase readability
            if(currentX + xOffset >= 0 && currentY + yOffset >= 0) {
              if((floodValues[currentX + xOffset][currentY + yOffset] == floodValues[currentX][currentY] - 1 && floodValues[currentX
                  + xOffset][currentY + yOffset] != 0) || (currentX + xOffset == origin.x && currentY + yOffset == origin.y)) {
                c.x = currentX;
                c.y = currentY;
                vectorToFill.push_back(c);

                currentX = currentX + xOffset;
                currentY = currentY + yOffset;

                if(currentX == origin.x && currentY == origin.y) {
                  done = true;
                }

                xOffset = 99;
                yOffset = 99;
              }
            }
          }
          yOffset = yOffset == 1 ? 2 : yOffset == -1 ? 1 : yOffset == 0 ? -1 : yOffset;
        }
        xOffset = xOffset == 1 ? 2 : xOffset == -1 ? 1 : xOffset == 0 ? -1 : xOffset;
      }
    }
  }
}


//void MapTests::getActorsPositions(const vector<Actor*>& actors,
//                                  vector<Pos>& vectorToFill) {
//  vectorToFill.resize(0);
//  const unsigned int NR_ACTORS = actors.size();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    vectorToFill.push_back(actors.at(i)->pos);
//  }
//}
//
//
//void MapTests::makeVisionBlockerArray(
//  const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS],
//  const int MAX_VISION_RANGE) {
//
//  const int X0 = max(0, origin.x - MAX_VISION_RANGE - 1);
//  const int Y0 = max(0, origin.y - MAX_VISION_RANGE - 1);
//  const int X1 = min(MAP_X_CELLS - 1, origin.x + MAX_VISION_RANGE + 1);
//  const int Y1 = min(MAP_Y_CELLS - 1, origin.y + MAX_VISION_RANGE + 1);
//
//  const Map* const map = eng->map;
//
//  for(int y = Y0; y <= Y1; y++) {
//    for(int x = X0; x <= X1; x++) {
//      arrayToFill[x][y] =
//        map->featuresStatic[x][y]->isVisionPassable() == false;
//    }
//  }
//
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  int x = 0;
//  int y = 0;
//  FeatureMob* f = NULL;
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    x = f->getX();
//    y = f->getY();
//    if(map->featuresStatic[x][y]->isVisionPassable()) {
//      arrayToFill[x][y] = f->isVisionPassable() == false;
//    }
//  }
//}
//
//void MapTests::makeMoveBlockerArray(
//  const Actor* const actorMoving,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  makeMoveBlockerArrayForBodyType(actorMoving->getBodyType(), arrayToFill);
//}
//
//void MapTests::makeMoveBlockerArrayFeaturesOnly(
//  const Actor* const actorMoving,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  makeMoveBlockerArrayForBodyTypeFeaturesOnly(
//    actorMoving->getBodyType(), arrayToFill);
//}
//
//void MapTests::makeWalkBlockingArrayFeaturesOnly(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  makeMoveBlockerArrayForBodyTypeFeaturesOnly(
//    bodyType_normal, arrayToFill);
//}
//
//void MapTests::makeMoveBlockerArrayForBodyTypeFeaturesOnly(
//  const BodyType_t bodyType,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  const Map* const map = eng->map;
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      arrayToFill[x][y] =
//        map->featuresStatic[x][y]->isBodyTypePassable(bodyType) == false;
//    }
//  }
//  GameTime* const gameTime = eng->gameTime;
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = gameTime->getFeatureMobAt(i);
//    const Pos& pos = f->getPos();
//    if(arrayToFill[pos.x][pos.y] == false) {
//      arrayToFill[pos.x][pos.y] = f->isBodyTypePassable(bodyType) == false;
//    }
//  }
//}
//
//void MapTests::makeShootBlockerFeaturesArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      arrayToFill[x][y] = eng->map->featuresStatic[x][y]->isProjectilesPassable() == false;
//    }
//  }
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    if(arrayToFill[f->getX()][f->getY()] == false) {
//      arrayToFill[f->getX()][f->getY()] = f->isProjectilesPassable() == false;
//    }
//  }
//}
//
//void MapTests::addItemsToBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      if(eng->map->items[x][y] != NULL) {
//        arrayToFill[x][y] = true;
//      }
//    }
//  }
//}
//
//void MapTests::makeMoveBlockerArrayForBodyType(
//  const BodyType_t bodyType,
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = 0; y < MAP_Y_CELLS; y++) {
//    for(int x = 0; x < MAP_X_CELLS; x++) {
//      arrayToFill[x][y] =
//        eng->map->featuresStatic[x][y]->isBodyTypePassable(bodyType) == false;
//    }
//  }
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    const Pos& pos = f->getPos();
//    if(arrayToFill[pos.x][pos.y] == false) {
//      arrayToFill[pos.x][pos.y] = f->isBodyTypePassable(bodyType) == false;
//    }
//  }
//  addLivingActorsToBlockerArray(arrayToFill);
//}
//
//void MapTests::makeItemBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  for(int y = MAP_Y_CELLS - 1; y >= 0; y--) {
//    for(int x = MAP_X_CELLS - 1; x >= 0; x--) {
//      arrayToFill[x][y] = !eng->map->featuresStatic[x][y]->canHaveItem();
//    }
//  }
//  FeatureMob* f = NULL;
//  const unsigned int FEATURE_MOBS_SIZE = eng->gameTime->getFeatureMobsSize();
//  for(unsigned int i = 0; i < FEATURE_MOBS_SIZE; i++) {
//    f = eng->gameTime->getFeatureMobAt(i);
//    if(arrayToFill[f->getX()][f->getY()] == false) {
//      arrayToFill[f->getX()][f->getY()] = !f->canHaveItem();
//    }
//  }
//  //addActorsToBlockerArray(arrayToFill); //Why?
//}
//
//void MapTests::addLivingActorsToBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  Actor* a = NULL;
//  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    a = eng->gameTime->getActorAtElement(i);
//    if(a->deadState == actorDeadState_alive) {
//      if(arrayToFill[a->pos.x][a->pos.y] == false) {
//        arrayToFill[a->pos.x][a->pos.y] = true;
//      }
//    }
//  }
//}
//
//void MapTests::addAllActorsToBlockerArray(
//  bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  Actor* a = NULL;
//  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    a = eng->gameTime->getActorAtElement(i);
//    if(arrayToFill[a->pos.x][a->pos.y] == false) {
//      arrayToFill[a->pos.x][a->pos.y] = true;
//    }
//  }
//}
//
//void MapTests::addAdjLivingActorsToBlockerArray(
//  const Pos& origin, bool arrayToFill[MAP_X_CELLS][MAP_Y_CELLS]) {
//
//  Actor* a = NULL;
//  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
//  for(unsigned int i = 0; i < NR_ACTORS; i++) {
//    a = eng->gameTime->getActorAtElement(i);
//    if(a->deadState == actorDeadState_alive) {
//      if(arrayToFill[a->pos.x][a->pos.y] == false && a->pos != origin) {
//        if(isCellsAdj(origin, a->pos, false)) {
//          arrayToFill[a->pos.x][a->pos.y] = true;
//        }
//      }
//    }
//  }
//}
//
//bool MapTests::isCellNextToPlayer(
//  const Pos& pos,
//  const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {
//
//  return isCellsAdj(pos, eng->player->pos, COUNT_SAME_CELL_AS_NEIGHBOUR);
//}

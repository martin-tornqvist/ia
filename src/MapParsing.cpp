#include "MapParsing.h"

#include <assert.h>

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

bool CellPredBlocksBodyType::check(const Cell& c) const {
  return c.featureStatic->isBodyTypePassable(bodyType_) == false;
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
  return eng.basicUtils->isPosAdj(pos_, a.pos, true);
}

bool CellPredBlocksItems::check(const Cell& c)  const {
  return c.featureStatic->canHaveItem() == false;
}

bool CellPredBlocksItems::check(const FeatureMob& f) const {
  return f.canHaveItem() == false;
}

//------------------------------------------------------------ MAP PARSER
void MapParser::parse(
  const CellPred& predicate, bool arrayOut[MAP_X_CELLS][MAP_Y_CELLS],
  const MapParseWriteRule writeRule) {

  assert(predicate.isCheckingCells()       == true ||
         predicate.isCheckingMobFeatures() == true ||
         predicate.isCheckingActors()      == true);

  const Engine& eng = predicate.eng;

  const bool WRITE_T = writeRule == mapParseWriteAlways ||
                       writeRule == mapParseWriteOnlyTrue;

  const bool WRITE_F = writeRule == mapParseWriteAlways ||
                       writeRule == mapParseWriteOnlyFalse;

  if(predicate.isCheckingCells()) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
        const Cell& c = eng.map->cells[x][y];
        const bool IS_MATCH = predicate.check(c);
        if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
          arrayOut[x][y] = IS_MATCH;
        }
      }
    }
  }

  if(predicate.isCheckingMobFeatures()) {
    const int NR_MOB_FEATURES = eng.gameTime->getNrFeatureMobs();
    for(int i = 0; i < NR_MOB_FEATURES; i++) {
      const FeatureMob& f = eng.gameTime->getFeatureMobAtElement(i);
      const Pos& p = f.getPos();
      const bool IS_MATCH = predicate.check(f);
      if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
        arrayOut[p.x][p.y] = IS_MATCH;
      }
    }
  }

  if(predicate.isCheckingActors()) {
    const int NR_ACTORS = eng.gameTime->getNrActors();
    for(int i = 0; i < NR_ACTORS; i++) {
      const Actor& a = eng.gameTime->getActorAtElement(i);
      const Pos& p = a.pos;
      const bool IS_MATCH = predicate.check(a);
      if((IS_MATCH && WRITE_T) || (IS_MATCH == false && WRITE_F)) {
        arrayOut[p.x][p.y] = IS_MATCH;
      }
    }
  }
}

bool IsCloserToOrigin::operator()(const Pos& c1, const Pos& c2) {
  const int chebDist1 = eng.basicUtils->chebyshevDist(c_.x, c_.y, c1.x, c1.y);
  const int chebDist2 = eng.basicUtils->chebyshevDist(c_.x, c_.y, c2.x, c2.y);
  return chebDist1 < chebDist2;
}

//------------------------------------------------------------ FLOOD FILL
void FloodFill::run(
  const Pos& origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
  int values[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit, const Pos& target) {

  eng.basicUtils->resetArray(values);

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
            eng.basicUtils->isPosInside(Pos(newPos.x, newPos.y), bounds) &&
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
                     vector<Pos>& vectorRef) const {

  vectorRef.resize(0);

  int floodValues[MAP_X_CELLS][MAP_Y_CELLS];
  eng.floodFill->run(origin, blockers, floodValues, 1000, target);

  bool pathExists = floodValues[target.x][target.y] != 0;

  if(pathExists == true) {
    vector<Pos> positions;
    Pos c;

    int currentX = target.x;
    int currentY = target.y;

    bool done = false;
    while(done == false) {
      //TODO use for-loop instead
      //Starts from 0 instead of -1 so that cardinal directions are tried first
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
                vectorRef.push_back(c);

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


#include "MapParsing.h"

#include <assert.h>

#include "Engine.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "GameTime.h"

//------------------------------------------------------------ CELL PREDICATES
namespace CellPred {

//bool CellPred::canWalkAt(const int X, const int Y) const {
//  return eng.map->cells[X][Y].featureStatic-> canMoveCmn();
//}

bool BlocksVision::check(const Cell& c)  const {
  return c.featureStatic->isVisionPassable() == false;
}

bool BlocksVision::check(const FeatureMob& f) const {
  return f.isVisionPassable() == false;
}

bool BlocksMoveCmn::check(const Cell& c) const {
  return c.featureStatic->canMoveCmn() == false;
}

bool BlocksMoveCmn::check(const FeatureMob& f) const {
  return f.canMoveCmn() == false;
}

bool BlocksMoveCmn::check(const Actor& a) const {
  return a.deadState == actorDeadState_alive;
}

BlocksActor::BlocksActor(Actor& actor, bool isActorsBlocking, Engine& engine) :
  CellPred(engine), IS_ACTORS_BLOCKING_(isActorsBlocking) {
  actor.getPropHandler().getAllActivePropIds(actorsProps_);
}

bool BlocksActor::check(const Cell& c) const {
  return c.featureStatic->canMove(actorsProps_) == false;
}

bool BlocksActor::check(const FeatureMob& f) const {
  return f.canMove(actorsProps_) == false;
}

bool BlocksActor::check(const Actor& a) const {
  return a.deadState == actorDeadState_alive;
}

bool BlocksProjectiles::check(const Cell& c)  const {
  return c.featureStatic->isProjectilePassable() == false;
}

bool BlocksProjectiles::check(const FeatureMob& f)  const {
  return f.isProjectilePassable() == false;
}

bool LivingActorsAdjToPos::check(const Actor& a) const {
  if(a.deadState != actorDeadState_alive) {
    return false;
  }
  return eng.basicUtils->isPosAdj(pos_, a.pos, true);
}

bool BlocksItems::check(const Cell& c)  const {
  return c.featureStatic->canHaveItem() == false;
}

bool BlocksItems::check(const FeatureMob& f) const {
  return f.canHaveItem() == false;
}

//bool Corridor::check(const Cell& c) const {
//  const int X = c.pos.x;
//  const int Y = c.pos.y;
//
//  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}
//
//  const bool IS_HOR_COR = canWalkAt(X,     Y - 1) == false &&
//                          canWalkAt(X,     Y)              &&
//                          canWalkAt(X,     Y + 1) == false &&
//                          canWalkAt(X - 1, Y)              &&
//                          canWalkAt(X + 1, Y);
//
//  const bool IS_VER_COR = canWalkAt(X - 1, Y) == false &&
//                          canWalkAt(X,     Y)          &&
//                          canWalkAt(X + 1, Y) == false &&
//                          canWalkAt(X,     Y - 1)      &&
//                          canWalkAt(X,     Y + 1);
//
//  return IS_HOR_COR != IS_VER_COR;
//}

//bool Nook::check(const Cell& c) const {
//  const int X = c.pos.x;
//  const int Y = c.pos.y;
//
//  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}
//
//  const bool C  = canWalkAt(X    , Y);
//  const bool E  = canWalkAt(X + 1, Y);
//  const bool W  = canWalkAt(X - 1, Y);
//  const bool N  = canWalkAt(X,     Y - 1);
//  const bool S  = canWalkAt(X,     Y + 1);
//  const bool NE = canWalkAt(X + 1, Y - 1);
//  const bool NW = canWalkAt(X - 1, Y - 1);
//  const bool SE = canWalkAt(X + 1, Y + 1);
//  const bool SW = canWalkAt(X - 1, Y + 1);
//
//  if(C == false) {return false;}
//
//  //Horizontal nook
//  if(N == false && S == false) {
//    // ##
//    // #..
//    // ##
//    if(NW == false && W == false && SW == false && E) {return true;}
//
//    //  ##
//    // ..#
//    //  ##
//    if(NE == false && E == false && SE == false && W) {return true;}
//  }
//
//  //Vertical nook
//  if(E == false && W == false) {
//    // ###
//    // #.#
//    //  .
//    if(NW == false && N == false && NE == false && S) {return true;}
//
//    //  .
//    // #.#
//    // ###
//    if(SW == false && S == false && SE == false && N) {return true;}
//  }
//  return false;
//}

bool IsAnyOfFeatures::check(const Cell& c) const {
  for(Feature_t f : features_) {if(f == c.featureStatic->getId()) return true;}
  return false;
}

bool AllAdjIsAnyOfFeatures::check(const Cell& c) const {
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const Feature_t curId =
        eng.map->cells[X + dx][Y + dy].featureStatic->getId();

      bool isMatch = false;
      for(Feature_t f : features_) {
        if(f == curId) {
          isMatch = true;
          break;
        }
      }

      if(isMatch == false) return false;
    }
  }

  return true;
}

} //CellPred

//------------------------------------------------------------ MAP PARSE
namespace MapParse {

void parse(const CellPred::CellPred& predicate, bool arrayOut[MAP_W][MAP_H],
           const MapParseWriteRule writeRule) {

  assert(predicate.isCheckingCells()       == true ||
         predicate.isCheckingMobFeatures() == true ||
         predicate.isCheckingActors()      == true);

  const Engine& eng = predicate.eng;

  const bool ALLOW_WRITE_FALSE = writeRule == mapParseWriteAlways;

  if(predicate.isCheckingCells()) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        const Cell& c = eng.map->cells[x][y];
        const bool IS_MATCH = predicate.check(c);
        if(IS_MATCH || ALLOW_WRITE_FALSE) {
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
      if(IS_MATCH || ALLOW_WRITE_FALSE) {
        bool& v = arrayOut[p.x][p.y];
        if(v == false) {v = IS_MATCH;}
      }
    }
  }

  if(predicate.isCheckingActors()) {
    const int NR_ACTORS = eng.gameTime->getNrActors();
    for(int i = 0; i < NR_ACTORS; i++) {
      const Actor& a  = eng.gameTime->getActorAtElement(i);
      const Pos& p    = a.pos;
      const bool IS_MATCH = predicate.check(a);
      if(IS_MATCH || ALLOW_WRITE_FALSE) {
        bool& v = arrayOut[p.x][p.y];
        if(v == false) {v = IS_MATCH;}
      }
    }
  }
}

void getCellsWithinDistOfOthers(const bool in[MAP_W][MAP_H],
                                bool out[MAP_W][MAP_H],
                                const Range& distIntervall) {

  assert(in != out);

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      out[x][y] = false;
    }
  }

  for(int yOuter = 0; yOuter < MAP_H; yOuter++) {
    for(int xOuter = 0; xOuter < MAP_W; xOuter++) {
      if(out[xOuter][yOuter] == false) {
        for(int d = distIntervall.lower; d <= distIntervall.upper; d++) {
          Pos x0y0(max(0,         xOuter - d), max(0,         yOuter - d));
          Pos x1y1(min(MAP_W - 1, xOuter + d), min(MAP_H - 1, yOuter + d));

          for(int x = x0y0.x; x <= x1y1.x; x++) {
            if(in[x][x0y0.y] || in[x][x1y1.y]) {
              out[xOuter][yOuter] = true;
              break;
            }
          }
          for(int y = x0y0.y; y <= x1y1.y; y++) {
            if(in[x0y0.x][y] || in[x1y1.x][y]) {
              out[xOuter][yOuter] = true;
              break;
            }
          }
        }
      }
    }
  }
}

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H]) {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(append[x][y]) base[x][y] = true;
    }
  }
}

} //MapParse

//------------------------------------------------------------ FUNCT OBJECT
bool IsCloserToOrigin::operator()(const Pos& c1, const Pos& c2) {
  const int chebDist1 = eng.basicUtils->chebyshevDist(c_.x, c_.y, c1.x, c1.y);
  const int chebDist2 = eng.basicUtils->chebyshevDist(c_.x, c_.y, c2.x, c2.y);
  return chebDist1 < chebDist2;
}

//------------------------------------------------------------ FLOOD FILL
namespace FloodFill {

void run(const Pos& origin, bool blockers[MAP_W][MAP_H],
         int values[MAP_W][MAP_H], int travelLimit, const Pos& target,
         Engine& eng) {

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

  const Rect bounds(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

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

} //FloodFill

//------------------------------------------------------------ PATHFINDER
namespace PathFind {
void run(const Pos& origin, const Pos& target, bool blockers[MAP_W][MAP_H],
         vector<Pos>& vectorRef, Engine& eng) {

  vectorRef.resize(0);

  int floodValues[MAP_W][MAP_H];
  FloodFill::run(origin, blockers, floodValues, 1000, target, eng);

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

} //PathFind


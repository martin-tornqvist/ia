#include "MapParsing.h"

#include <assert.h>
#include <algorithm>

#include "Map.h"
#include "ActorPlayer.h"
#include "GameTime.h"
#include "Utils.h"

using namespace std;

//------------------------------------------------------------ CELL PREDICATES
namespace CellPred {

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
  return a.deadState == ActorDeadState::alive;
}

BlocksActor::BlocksActor(Actor& actor, bool isActorsBlocking) :
  Pred(), IS_ACTORS_BLOCKING_(isActorsBlocking) {
  actor.getPropHandler().getAllActivePropIds(actorsProps_);
}

bool BlocksActor::check(const Cell& c) const {
  return c.featureStatic->canMove(actorsProps_) == false;
}

bool BlocksActor::check(const FeatureMob& f) const {
  return f.canMove(actorsProps_) == false;
}

bool BlocksActor::check(const Actor& a) const {
  return a.deadState == ActorDeadState::alive;
}

bool BlocksProjectiles::check(const Cell& c)  const {
  return c.featureStatic->isProjectilePassable() == false;
}

bool BlocksProjectiles::check(const FeatureMob& f)  const {
  return f.isProjectilePassable() == false;
}

bool LivingActorsAdjToPos::check(const Actor& a) const {
  if(a.deadState != ActorDeadState::alive) {
    return false;
  }
  return Utils::isPosAdj(pos_, a.pos, true);
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
  for(FeatureId f : features_) {if(f == c.featureStatic->getId()) return true;}
  return false;
}

bool AllAdjIsAnyOfFeatures::check(const Cell& c) const {
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const FeatureId curId =
        Map::cells[X + dx][Y + dy].featureStatic->getId();

      bool isMatch = false;
      for(FeatureId f : features_) {
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

void parse(const CellPred::Pred& pred, bool out[MAP_W][MAP_H],
           const MapParseWriteRule writeRule) {

  assert(pred.isCheckingCells()       == true ||
         pred.isCheckingMobFeatures() == true ||
         pred.isCheckingActors()      == true);

  const bool ALLOW_WRITE_FALSE = writeRule == MapParseWriteRule::always;

  if(pred.isCheckingCells()) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        const Cell& c = Map::cells[x][y];
        const bool IS_MATCH = pred.check(c);
        if(IS_MATCH || ALLOW_WRITE_FALSE) {
          out[x][y] = IS_MATCH;
        }
      }
    }
  }

  if(pred.isCheckingMobFeatures()) {
    for(FeatureMob * mob : GameTime::featureMobs_) {
      const Pos& p = mob->getPos();
      const bool IS_MATCH = pred.check(*mob);
      if(IS_MATCH || ALLOW_WRITE_FALSE) {
        bool& v = out[p.x][p.y];
        if(v == false) {v = IS_MATCH;}
      }
    }
  }

  if(pred.isCheckingActors()) {
    for(Actor * actor : GameTime::actors_) {
      const Pos& p = actor->pos;
      const bool IS_MATCH = pred.check(*actor);
      if(IS_MATCH || ALLOW_WRITE_FALSE) {
        bool& v = out[p.x][p.y];
        if(v == false) {v = IS_MATCH;}
      }
    }
  }
}

void getCellsWithinDistOfOthers(const bool in[MAP_W][MAP_H],
                                bool out[MAP_W][MAP_H],
                                const Range& distInterval) {
  assert(in != out);

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      out[x][y] = false;
    }
  }

  for(int yOuter = 0; yOuter < MAP_H; yOuter++) {
    for(int xOuter = 0; xOuter < MAP_W; xOuter++) {
      if(out[xOuter][yOuter] == false) {
        for(int d = distInterval.lower; d <= distInterval.upper; d++) {
          Pos p0(max(0,         xOuter - d), max(0,         yOuter - d));
          Pos p1(min(MAP_W - 1, xOuter + d), min(MAP_H - 1, yOuter + d));

          for(int x = p0.x; x <= p1.x; x++) {
            if(in[x][p0.y] || in[x][p1.y]) {
              out[xOuter][yOuter] = true;
              break;
            }
          }
          for(int y = p0.y; y <= p1.y; y++) {
            if(in[p0.x][y] || in[p1.x][y]) {
              out[xOuter][yOuter] = true;
              break;
            }
          }
        }
      }
    }
  }
}

int getNrAdjCellsWithFeature(const Pos& p,
                             const CellPred::IsAnyOfFeatures& pred) {
  int ret = 0;

  const int X0 = getConstrInRange(0, p.x - 1, MAP_W - 1);
  const int Y0 = getConstrInRange(0, p.y - 1, MAP_W - 1);
  const int X1 = getConstrInRange(0, p.x + 1, MAP_W - 1);
  const int Y1 = getConstrInRange(0, p.y + 1, MAP_W - 1);

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(pred.check(Map::cells[x][y])) {ret++;}
    }
  }

  return ret;
}

bool isValInArea(const Rect& area, const bool in[MAP_W][MAP_H],
                 const bool VAL) {
  assert(Utils::isAreaInsideMap(area));

  for(int y = area.p0.y; y <= area.p1.y; y++) {
    for(int x = area.p0.x; x <= area.p1.x; x++) {
      if(in[x][y] == VAL) {return false;}
    }
  }
  return true;
}

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H]) {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(append[x][y]) {base[x][y] = true;}
    }
  }
}

void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H], const int DIST,
            const bool VAL_TO_EXPAND) {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {

      out[x][y] = !VAL_TO_EXPAND;

      const int CHECK_X0 = max(x - DIST, 0);
      const int CHECK_Y0 = max(y - DIST, 0);
      const int CHECK_X1 = min(x + DIST, MAP_W - 1);
      const int CHECK_Y1 = min(y + DIST, MAP_H - 1);

      for(int checkY = CHECK_Y0; checkY <= CHECK_Y1; checkY++) {

        bool isFound = false;

        for(int checkX = CHECK_X0; checkX <= CHECK_X1; checkX++) {
          if(in[checkX][checkY] == VAL_TO_EXPAND) {
            out[x][y] = VAL_TO_EXPAND;
            isFound = true;
            break;
          }
          if(isFound) {break;}
        }
      }
    }
  }
}

} //MapParse

//------------------------------------------------------------ FUNCT OBJECT
bool IsCloserToOrigin::operator()(const Pos& c1, const Pos& c2) {
  const int chebDist1 = Utils::kingDist(c_.x, c_.y, c1.x, c1.y);
  const int chebDist2 = Utils::kingDist(c_.x, c_.y, c2.x, c2.y);
  return chebDist1 < chebDist2;
}

//------------------------------------------------------------ FLOOD FILL
namespace FloodFill {

void run(const Pos& origin, bool blocked[MAP_W][MAP_H],
         int values[MAP_W][MAP_H], int travelLimit, const Pos& target) {

  Utils::resetArray(values);

  vector<Pos> positions;
  positions.resize(0);
  unsigned int nrElementsToSkip = 0;
  Pos c;

  int curX = origin.x;
  int curY = origin.y;

  int curValue = 0;

  bool pathExists = true;
  bool isAtTarget = false;

  bool isStoppingAtTarget = target.x != -1;

  const Rect bounds(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

  bool done = false;
  while(done == false) {
    for(int dx = -1; dx <= 1; dx++) {
      for(int dy = -1; dy <= 1; dy++) {
        if((dx != 0 || dy != 0)) {
          const Pos newPos(curX + dx, curY + dy);
          if(
            blocked[newPos.x][newPos.y] == false &&
            Utils::isPosInside(newPos, bounds) &&
            values[newPos.x][newPos.y] == 0) {
            curValue = values[curX][curY];

            if(curValue < travelLimit) {
              values[newPos.x][newPos.y] = curValue + 1;
            }

            if(isStoppingAtTarget) {
              if(curX == target.x - dx && curY == target.y - dy) {
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

    if(curValue == travelLimit) {
      done = true;
    }

    if(isStoppingAtTarget == false || isAtTarget == false) {
      if(positions.size() == nrElementsToSkip) {
        pathExists = false;
      } else {
        c = positions.at(nrElementsToSkip);
        curX = c.x;
        curY = c.y;
        nrElementsToSkip++;
      }
    }
  }
}

} //FloodFill

//------------------------------------------------------------ PATHFINDER
namespace PathFind {

void run(const Pos& origin, const Pos& target, bool blocked[MAP_W][MAP_H],
         vector<Pos>& vectorRef) {

  vectorRef.resize(0);

  int vals[MAP_W][MAP_H];
  FloodFill::run(origin, blocked, vals, 1000, target);

  bool pathExists = vals[target.x][target.y] != 0;

  if(pathExists == true) {
    Pos c;

    int curX = target.x;
    int curY = target.y;

    bool done = false;
    while(done == false) {
      //TODO use for-loop instead
      //Starts from 0 instead of -1 so that cardinal directions are tried first
      int dX = 0;
      while(dX <= 1) {
        int dY = 0;
        while(dY <= 1) {
          if(dX != 0 || dY != 0) {
            const Pos newPos(curX + dX, curY + dY);

            if(newPos.x >= 0 && newPos.y >= 0) {
              const int VAL_AT_NEW = vals[newPos.x][newPos.y];
              const int VAL_AT_CUR = vals[curX][curY];
              if(
                (VAL_AT_NEW == VAL_AT_CUR - 1 && VAL_AT_NEW != 0) ||
                (newPos == origin)) {

                c.x = curX;
                c.y = curY;
                vectorRef.push_back(c);

                curX = curX + dX;
                curY = curY + dY;

                if(curX == origin.x && curY == origin.y) {
                  done = true;
                }

                dX = 99;
                dY = 99;
              }
            }
          }
          dY = dY == 1 ? 2 : dY == -1 ? 1 : dY == 0 ? -1 : dY;
        }
        dX = dX == 1 ? 2 : dX == -1 ? 1 : dX == 0 ? -1 : dX;
      }
    }
  }
}

} //PathFind


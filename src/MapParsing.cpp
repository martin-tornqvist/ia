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
  return !c.featureStatic->isVisionPassable();
}

bool BlocksVision::check(const FeatureMob& f) const {
  return !f.isVisionPassable();
}

bool BlocksMoveCmn::check(const Cell& c) const {
  return !c.featureStatic->canMoveCmn();
}

bool BlocksMoveCmn::check(const FeatureMob& f) const {
  return !f.canMoveCmn();
}

bool BlocksMoveCmn::check(const Actor& a) const {
  return a.deadState == ActorDeadState::alive;
}

BlocksActor::BlocksActor(Actor& actor, bool isActorsBlocking) :
  Pred(), IS_ACTORS_BLOCKING_(isActorsBlocking) {
  actor.getPropHandler().getAllActivePropIds(actorsProps_);
}

bool BlocksActor::check(const Cell& c) const {
  return !c.featureStatic->canMove(actorsProps_);
}

bool BlocksActor::check(const FeatureMob& f) const {
  return !f.canMove(actorsProps_);
}

bool BlocksActor::check(const Actor& a) const {
  return a.deadState == ActorDeadState::alive;
}

bool BlocksProjectiles::check(const Cell& c)  const {
  return !c.featureStatic->isProjectilePassable();
}

bool BlocksProjectiles::check(const FeatureMob& f)  const {
  return !f.isProjectilePassable();
}

bool LivingActorsAdjToPos::check(const Actor& a) const {
  if(a.deadState != ActorDeadState::alive) {
    return false;
  }
  return Utils::isPosAdj(pos_, a.pos, true);
}

bool BlocksItems::check(const Cell& c)  const {
  return !c.featureStatic->canHaveItem();
}

bool BlocksItems::check(const FeatureMob& f) const {
  return !f.canHaveItem();
}

bool IsFeature::check(const Cell& c) const {
  return c.featureStatic->getId() == feature_;
}

bool IsAnyOfFeatures::check(const Cell& c) const {
  for(auto f : features_) {if(f == c.featureStatic->getId()) return true;}
  return false;
}

bool AllAdjIsFeature::check(const Cell& c) const {
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      if(Map::cells[X + dx][Y + dy].featureStatic->getId() != feature_) {
        return false;
      }
    }
  }

  return true;
}

bool AllAdjIsAnyOfFeatures::check(const Cell& c) const {
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const auto curId = Map::cells[X + dx][Y + dy].featureStatic->getId();

      bool isMatch = false;
      for(auto f : features_) {if(f == curId) {isMatch = true; break;}}
      if(!isMatch) return false;
    }
  }

  return true;
}

bool AllAdjIsNotFeature::check(const Cell& c) const {
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      if(Map::cells[X + dx][Y + dy].featureStatic->getId() == feature_) {
        return false;
      }
    }
  }

  return true;
}

bool AllAdjIsNoneOfFeatures::check(const Cell& c) const {
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if(X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const auto curId = Map::cells[X + dx][Y + dy].featureStatic->getId();
      for(auto f : features_) {if(f == curId) {return false;}}
    }
  }

  return true;
}

} //CellPred

//------------------------------------------------------------ MAP PARSE
namespace MapParse {

void parse(const CellPred::Pred& pred, bool out[MAP_W][MAP_H],
           const MapParseWriteRule writeRule) {

  assert(pred.isCheckingCells()       ||
         pred.isCheckingMobFeatures() ||
         pred.isCheckingActors());

  const bool ALLOW_WRITE_FALSE = writeRule == MapParseWriteRule::always;

  if(pred.isCheckingCells()) {
    for(int y = 0; y < MAP_H; ++y) {
      for(int x = 0; x < MAP_W; ++x) {
        const auto& c = Map::cells[x][y];
        const bool IS_MATCH = pred.check(c);
        if(IS_MATCH || ALLOW_WRITE_FALSE) {out[x][y] = IS_MATCH;}
      }
    }
  }

  if(pred.isCheckingMobFeatures()) {
    for(FeatureMob* mob : GameTime::featureMobs_) {
      const Pos& p = mob->getPos();
      const bool IS_MATCH = pred.check(*mob);
      if(IS_MATCH || ALLOW_WRITE_FALSE) {
        bool& v = out[p.x][p.y];
        if(!v) {v = IS_MATCH;}
      }
    }
  }

  if(pred.isCheckingActors()) {
    for(Actor* actor : GameTime::actors_) {
      const Pos& p = actor->pos;
      const bool IS_MATCH = pred.check(*actor);
      if(IS_MATCH || ALLOW_WRITE_FALSE) {
        bool& v = out[p.x][p.y];
        if(!v) {v = IS_MATCH;}
      }
    }
  }
}

void getCellsWithinDistOfOthers(const bool in[MAP_W][MAP_H],
                                bool out[MAP_W][MAP_H],
                                const Range& distInterval) {
  assert(in != out);

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      out[x][y] = false;
    }
  }

  for(int yOuter = 0; yOuter < MAP_H; yOuter++) {
    for(int xOuter = 0; xOuter < MAP_W; xOuter++) {
      if(!out[xOuter][yOuter]) {
        for(int d = distInterval.lower; d <= distInterval.upper; d++) {
          Pos p0(max(0,         xOuter - d), max(0,         yOuter - d));
          Pos p1(min(MAP_W - 1, xOuter + d), min(MAP_H - 1, yOuter + d));

          for(int x = p0.x; x <= p1.x; ++x) {
            if(in[x][p0.y] || in[x][p1.y]) {
              out[xOuter][yOuter] = true;
              break;
            }
          }
          for(int y = p0.y; y <= p1.y; ++y) {
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

bool isValInArea(const Rect& area, const bool in[MAP_W][MAP_H],
                 const bool VAL) {
  assert(Utils::isAreaInsideMap(area));

  for(int y = area.p0.y; y <= area.p1.y; ++y) {
    for(int x = area.p0.x; x <= area.p1.x; ++x) {
      if(in[x][y] == VAL) {return false;}
    }
  }
  return true;
}

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H]) {
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      if(append[x][y]) {base[x][y] = true;}
    }
  }
}

void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H], const int DIST,
            const bool VAL_TO_EXPAND) {
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {

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
bool IsCloserToPos::operator()(const Pos& p1, const Pos& p2) {
  const int kingDist1 = Utils::kingDist(p_.x, p_.y, p1.x, p1.y);
  const int kingDist2 = Utils::kingDist(p_.x, p_.y, p2.x, p2.y);
  return kingDist1 < kingDist2;
}

//------------------------------------------------------------ FLOOD FILL
namespace FloodFill {

void run(const Pos& p0, bool blocked[MAP_W][MAP_H],
         int out[MAP_W][MAP_H], int travelLimit, const Pos& p1,
         const bool ALLOW_DIAGONAL) {

  Utils::resetArray(out);

  vector<Pos> positions;
  positions.resize(0);

  unsigned int  nrElementsToSkip  = 0;
  int           curVal            = 0;
  bool          pathExists        = true;
  bool          isAtTarget        = false;
  bool          isStoppingAtP1    = p1.x != -1;
  const         Rect bounds(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));
  Pos           curPos(p0);

  vector<Pos> dirs {Pos(0, -1), Pos(-1, 0), Pos(0, 1), Pos(1, 0)};
  if(ALLOW_DIAGONAL) {
    dirs.push_back(Pos(-1, -1));
    dirs.push_back(Pos(-1, 1));
    dirs.push_back(Pos(1, -1));
    dirs.push_back(Pos(1, 1));
  }

  bool done = false;
  while(!done) {

    for(const Pos& d : dirs) {
      if((d != 0)) {
        const Pos newPos(curPos + d);
        if(
          !blocked[newPos.x][newPos.y]        &&
          Utils::isPosInside(newPos, bounds)  &&
          out[newPos.x][newPos.y] == 0        &&
          newPos != p0) {
          curVal = out[curPos.x][curPos.y];

          if(curVal < travelLimit) {out[newPos.x][newPos.y] = curVal + 1;}

          if(isStoppingAtP1 && curPos == p1 - d) {
            isAtTarget = true;
            break;
          }
          if(!isStoppingAtP1 || !isAtTarget) {positions.push_back(newPos);}
        }
      }
    }

    if(isStoppingAtP1) {
      if(positions.size() == nrElementsToSkip)  {pathExists = false;}
      if(isAtTarget || !pathExists)             {done = true;}
    } else if(positions.size() == nrElementsToSkip) {
      done = true;
    }

    if(curVal == travelLimit) {done = true;}

    if(!isStoppingAtP1 || !isAtTarget) {
      if(positions.size() == nrElementsToSkip) {
        pathExists = false;
      } else {
        curPos = positions.at(nrElementsToSkip);
        nrElementsToSkip++;
      }
    }
  }
}

} //FloodFill

//------------------------------------------------------------ PATHFINDER
namespace PathFind {

void run(const Pos& p0, const Pos& p1, bool blocked[MAP_W][MAP_H],
         vector<Pos>& out, const bool ALLOW_DIAGONAL) {

  out.resize(0);

  int flood[MAP_W][MAP_H];
  FloodFill::run(p0, blocked, flood, 1000, p1, ALLOW_DIAGONAL);

  if(flood[p1.x][p1.y] == 0) {return;} //No path exists

  Pos curPos(p1);
  out.push_back(curPos);

  vector<Pos> dirs {Pos(0, -1), Pos(-1, 0), Pos(0, 1), Pos(1, 0)};
  if(ALLOW_DIAGONAL) {
    dirs.push_back(Pos(-1, -1));
    dirs.push_back(Pos(-1, 1));
    dirs.push_back(Pos(1, -1));
    dirs.push_back(Pos(1, 1));
  }

  while(true) {
    for(const Pos& d : dirs) {

      const Pos adjPos(curPos + d);

      if(Utils::isPosInsideMap(adjPos)) {
        const int VAL_AT_ADJ = flood[adjPos.x][adjPos.y];
        const int VAL_AT_CUR = flood[curPos.x][curPos.y];
        if(
          (VAL_AT_ADJ < VAL_AT_CUR && VAL_AT_ADJ != 0) ||
          (adjPos == p0)) {

          if(adjPos == p0) {return;} //Origin reached

          out.push_back(adjPos);

          curPos = adjPos;

          break;
        }
      }
    }
  }
}

} //PathFind


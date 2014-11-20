#include "MapParsing.h"

#include <assert.h>
#include <algorithm>

#include "Map.h"
#include "ActorPlayer.h"
#include "GameTime.h"
#include "Utils.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"

using namespace std;

//------------------------------------------------------------ CELL CHECKS
namespace CellCheck
{

bool BlocksLos::check(const Cell& c)  const
{
  return !c.rigid->isLosPassable();
}

bool BlocksLos::check(const Mob& f) const
{
  return !f.isLosPassable();
}

bool BlocksMoveCmn::check(const Cell& c) const
{
  return !c.rigid->canMoveCmn();
}

bool BlocksMoveCmn::check(const Mob& f) const
{
  return !f.canMoveCmn();
}

bool BlocksMoveCmn::check(const Actor& a) const
{
  return a.isAlive();
}

BlocksActor::BlocksActor(Actor& actor, bool isActorsBlocking) :
  Check(), IS_ACTORS_BLOCKING_(isActorsBlocking)
{
  actor.getPropHandler().getAllActivePropIds(actorsProps_);
}

bool BlocksActor::check(const Cell& c) const
{
  return !c.rigid->canMove(actorsProps_);
}

bool BlocksActor::check(const Mob& f) const
{
  return !f.canMove(actorsProps_);
}

bool BlocksActor::check(const Actor& a) const
{
  return a.isAlive();
}

bool BlocksProjectiles::check(const Cell& c)  const
{
  return !c.rigid->isProjectilePassable();
}

bool BlocksProjectiles::check(const Mob& f)  const
{
  return !f.isProjectilePassable();
}

bool LivingActorsAdjToPos::check(const Actor& a) const
{
  if (!a.isAlive())
  {
    return false;
  }
  return Utils::isPosAdj(pos_, a.pos, true);
}

bool BlocksItems::check(const Cell& c)  const
{
  return !c.rigid->canHaveItem();
}

bool BlocksItems::check(const Mob& f) const
{
  return !f.canHaveItem();
}

bool IsFeature::check(const Cell& c) const
{
  return c.rigid->getId() == feature_;
}

bool IsAnyOfFeatures::check(const Cell& c) const
{
  for (auto f : features_) {if (f == c.rigid->getId()) return true;}
  return false;
}

bool AllAdjIsFeature::check(const Cell& c) const
{
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if (X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for (int dx = -1; dx <= 1; ++dx)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      if (Map::cells[X + dx][Y + dy].rigid->getId() != feature_)
      {
        return false;
      }
    }
  }

  return true;
}

bool AllAdjIsAnyOfFeatures::check(const Cell& c) const
{
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if (X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for (int dx = -1; dx <= 1; ++dx)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      const auto curId = Map::cells[X + dx][Y + dy].rigid->getId();

      bool isMatch = false;
      for (auto f : features_) {if (f == curId) {isMatch = true; break;}}
      if (!isMatch) return false;
    }
  }

  return true;
}

bool AllAdjIsNotFeature::check(const Cell& c) const
{
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if (X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for (int dx = -1; dx <= 1; ++dx)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      if (Map::cells[X + dx][Y + dy].rigid->getId() == feature_)
      {
        return false;
      }
    }
  }

  return true;
}

bool AllAdjIsNoneOfFeatures::check(const Cell& c) const
{
  const int X = c.pos.x;
  const int Y = c.pos.y;

  if (X <= 0 || X >= MAP_W - 1 || Y <= 0 || Y >= MAP_H - 1) {return false;}

  for (int dx = -1; dx <= 1; ++dx)
  {
    for (int dy = -1; dy <= 1; ++dy)
    {
      const auto curId = Map::cells[X + dx][Y + dy].rigid->getId();
      for (auto f : features_) {if (f == curId) {return false;}}
    }
  }

  return true;
}

} //CellCheck

//------------------------------------------------------------ MAP PARSE
namespace MapParse
{

const Rect mapRect(0, 0, MAP_W - 1, MAP_H - 1);

void parse(const CellCheck::Check& check, bool out[MAP_W][MAP_H],
           const MapParseWriteRule writeRule, const Rect& areaToCheckCells)
{
  assert(check.isCheckingCells()  ||
         check.isCheckingMobs()   ||
         check.isCheckingActors());

  const bool ALLOW_WRITE_FALSE = writeRule == MapParseWriteRule::always;

  if (check.isCheckingCells())
  {
    for (int x = areaToCheckCells.p0.x; x <= areaToCheckCells.p1.x; ++x)
    {
      for (int y = areaToCheckCells.p0.y; y <= areaToCheckCells.p1.y; ++y)
      {
        const auto& c         = Map::cells[x][y];
        const bool  IS_MATCH  = check.check(c);
        if (IS_MATCH || ALLOW_WRITE_FALSE)
        {
          out[x][y] = IS_MATCH;
        }
      }
    }
  }

  if (check.isCheckingMobs())
  {
    for (Mob* mob : GameTime::mobs_)
    {
      const Pos& p = mob->getPos();
      const bool IS_MATCH = check.check(*mob);
      if (IS_MATCH || ALLOW_WRITE_FALSE)
      {
        bool& v = out[p.x][p.y];
        if (!v) {v = IS_MATCH;}
      }
    }
  }

  if (check.isCheckingActors())
  {
    for (Actor* actor : GameTime::actors_)
    {
      const Pos& p = actor->pos;
      const bool IS_MATCH = check.check(*actor);
      if (IS_MATCH || ALLOW_WRITE_FALSE)
      {
        bool& v = out[p.x][p.y];
        if (!v) {v = IS_MATCH;}
      }
    }
  }
}

void getCellsWithinDistOfOthers(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
                                const Range& distInterval)
{
  assert(in != out);

  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      out[x][y] = false;
    }
  }

  for (int yOuter = 0; yOuter < MAP_H; yOuter++)
  {
    for (int xOuter = 0; xOuter < MAP_W; xOuter++)
    {
      if (!out[xOuter][yOuter])
      {
        for (int d = distInterval.lower; d <= distInterval.upper; d++)
        {
          Pos p0(max(0,         xOuter - d), max(0,         yOuter - d));
          Pos p1(min(MAP_W - 1, xOuter + d), min(MAP_H - 1, yOuter + d));

          for (int x = p0.x; x <= p1.x; ++x)
          {
            if (in[x][p0.y] || in[x][p1.y])
            {
              out[xOuter][yOuter] = true;
              break;
            }
          }
          for (int y = p0.y; y <= p1.y; ++y)
          {
            if (in[p0.x][y] || in[p1.x][y])
            {
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
                 const bool VAL)
{
  assert(Utils::isAreaInsideMap(area));

  for (int y = area.p0.y; y <= area.p1.y; ++y)
  {
    for (int x = area.p0.x; x <= area.p1.x; ++x)
    {
      if (in[x][y] == VAL) {return true;}
    }
  }
  return false;
}

void append(bool base[MAP_W][MAP_H], const bool append[MAP_W][MAP_H])
{
  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      if (append[x][y]) {base[x][y] = true;}
    }
  }
}

void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H],
            const Rect& areaAllowedToModify)
{
  int cmpX0 = 0;
  int cmpY0 = 0;
  int cmpX1 = 0;
  int cmpY1 = 0;

  const int X0 = max(0,     areaAllowedToModify.p0.x);
  const int Y0 = max(0,     areaAllowedToModify.p0.y);
  const int X1 = min(MAP_W, areaAllowedToModify.p1.x);
  const int Y1 = min(MAP_H, areaAllowedToModify.p1.y);

  for (int x = X0; x <= X1; ++x)
  {
    for (int y = Y0; y <= Y1; ++y)
    {
      out[x][y] = false;

      cmpX0 = x == 0 ? 0 : (x - 1);
      cmpY0 = y == 0 ? 0 : (y - 1);
      cmpX1 = x == (MAP_W - 1) ? x : (x + 1);
      cmpY1 = y == (MAP_H - 1) ? y : (y + 1);

      for (int cmpY = cmpY0; cmpY <= cmpY1; ++cmpY)
      {
        bool isFound = false;

        for (int cmpX = cmpX0; cmpX <= cmpX1; ++cmpX)
        {
          if (in[cmpX][cmpY])
          {
            isFound = out[x][y] = true;
            break;
          }
          if (isFound)
          {
            break;
          }
        }
      }
    }
  }
}

void expand(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H], const int DIST)
{
  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {

      out[x][y] = false;

      const int X0 = x - DIST;
      const int Y0 = y - DIST;
      const int X1 = x + DIST;
      const int Y1 = y + DIST;

      const int CMP_X0 = X0 < 0 ? 0 : X0;
      const int CMP_Y0 = Y0 < 0 ? 0 : Y0;
      const int CMP_X1 = X1 > MAP_W - 1 ? MAP_W - 1 : X1;
      const int CMP_Y1 = Y1 > MAP_H - 1 ? MAP_H - 1 : Y1;

      for (int cmpY = CMP_Y0; cmpY <= CMP_Y1; ++cmpY)
      {

        bool isFound = false;

        for (int cmpX = CMP_X0; cmpX <= CMP_X1; ++cmpX)
        {
          if (in[cmpX][cmpY])
          {
            isFound = out[x][y] = true;
            break;
          }
          if (isFound) {break;}
        }
      }
    }
  }
}

bool isMapConnected(const bool blocked[MAP_W][MAP_H])
{
  Pos origin(-1, -1);
  for (int x = 1; x < MAP_W - 1; ++x)
  {
    for (int y = 1; y < MAP_H - 1; ++y)
    {
      if (!blocked[x][y])
      {
        origin.set(x, y);
        break;
      }
    }
    if (origin.x != -1)
    {
      break;
    }
  }

  assert(Utils::isPosInsideMap(origin, false));

  int floodFill[MAP_W][MAP_H];
  FloodFill::run(origin, blocked, floodFill, INT_MAX, Pos(-1, -1), true);

  //Note: We can skip to origin.x immediately, since this is guaranteed to be the
  //leftmost non-blocked cell.
  for (int x = origin.x; x < MAP_W - 1; ++x)
  {
    for (int y = 1; y < MAP_H - 1; ++y)
    {
      if (floodFill[x][y] == 0 && !blocked[x][y] && Pos(x, y) != origin)
      {
        return false;
      }
    }
  }
  return true;
}

} //MapParse

//------------------------------------------------------------ IS CLOSER TO POS
bool IsCloserToPos::operator()(const Pos& p1, const Pos& p2)
{
  const int kingDist1 = Utils::kingDist(p_.x, p_.y, p1.x, p1.y);
  const int kingDist2 = Utils::kingDist(p_.x, p_.y, p2.x, p2.y);
  return kingDist1 < kingDist2;
}

//------------------------------------------------------------ FLOOD FILL
namespace FloodFill
{

void run(const Pos& p0, const bool blocked[MAP_W][MAP_H], int out[MAP_W][MAP_H],
         int travelLmt, const Pos& p1, const bool ALLOW_DIAGONAL)
{
  Utils::resetArray(out);

  vector<Pos> positions;
  positions.clear();

  unsigned int  nrElementsToSkip  = 0;
  int           curVal            = 0;
  bool          pathExists        = true;
  bool          isAtTarget        = false;
  bool          isStoppingAtP1    = p1.x != -1;
  const         Rect bounds(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));
  Pos           curPos(p0);

  vector<Pos> dirs {Pos(0, -1), Pos(-1, 0), Pos(0, 1), Pos(1, 0)};
  if (ALLOW_DIAGONAL)
  {
    dirs.push_back(Pos(-1, -1));
    dirs.push_back(Pos(-1, 1));
    dirs.push_back(Pos(1, -1));
    dirs.push_back(Pos(1, 1));
  }

  bool done = false;
  while (!done)
  {

    for (const Pos& d : dirs)
    {
      if ((d != 0))
      {
        const Pos newPos(curPos + d);
        if (!blocked[newPos.x][newPos.y]        &&
            Utils::isPosInside(newPos, bounds)  &&
            out[newPos.x][newPos.y] == 0        &&
            newPos != p0)
        {
          curVal = out[curPos.x][curPos.y];

          if (curVal < travelLmt) {out[newPos.x][newPos.y] = curVal + 1;}

          if (isStoppingAtP1 && curPos == p1 - d)
          {
            isAtTarget = true;
            break;
          }
          if (!isStoppingAtP1 || !isAtTarget) {positions.push_back(newPos);}
        }
      }
    }

    if (isStoppingAtP1)
    {
      if (positions.size() == nrElementsToSkip)  {pathExists = false;}
      if (isAtTarget || !pathExists)             {done = true;}
    }
    else if (positions.size() == nrElementsToSkip)
    {
      done = true;
    }

    if (curVal == travelLmt) {done = true;}

    if (!isStoppingAtP1 || !isAtTarget)
    {
      if (positions.size() == nrElementsToSkip)
      {
        pathExists = false;
      }
      else
      {
        curPos = positions[nrElementsToSkip];
        nrElementsToSkip++;
      }
    }
  }
}

} //FloodFill

//------------------------------------------------------------ PATHFINDER
namespace PathFind
{

void run(const Pos& p0, const Pos& p1, bool blocked[MAP_W][MAP_H],
         vector<Pos>& out, const bool ALLOW_DIAGONAL)
{

  out.clear();

  int flood[MAP_W][MAP_H];
  FloodFill::run(p0, blocked, flood, 1000, p1, ALLOW_DIAGONAL);

  if (flood[p1.x][p1.y] == 0) {return;} //No path exists

  Pos curPos(p1);
  out.push_back(curPos);

  vector<Pos> dirs {Pos(0, -1), Pos(-1, 0), Pos(0, 1), Pos(1, 0)};
  if (ALLOW_DIAGONAL)
  {
    dirs.push_back(Pos(-1, -1));
    dirs.push_back(Pos(-1, 1));
    dirs.push_back(Pos(1, -1));
    dirs.push_back(Pos(1, 1));
  }

  while (true)
  {
    for (const Pos& d : dirs)
    {

      const Pos adjPos(curPos + d);

      if (Utils::isPosInsideMap(adjPos))
      {
        const int VAL_AT_ADJ = flood[adjPos.x][adjPos.y];
        const int VAL_AT_CUR = flood[curPos.x][curPos.y];
        if ((VAL_AT_ADJ < VAL_AT_CUR && VAL_AT_ADJ != 0) || (adjPos == p0))
        {
          if (adjPos == p0)
          {
            //Origin reached
            return;
          }

          out.push_back(adjPos);

          curPos = adjPos;

          break;
        }
      }
    }
  }
}

} //PathFind


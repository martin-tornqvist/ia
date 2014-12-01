#include "MapPatterns.h"

#include <vector>
#include <iostream>

#include "Init.h"
#include "Map.h"
#include "FeatureRigid.h"

using namespace std;

namespace MapPatterns
{

void getCellsInRoom(const Room& room, std::vector<Pos>& adjToWalls,
                    std::vector<Pos>& awayFromWalls)
{
  TRACE_FUNC_BEGIN_VERBOSE;
  vector<Pos> posBucket;
  posBucket.clear();

  const Rect& r = room.r_;

  for (int x = r.p0.x; x <= r.p1.x; ++x)
  {
    for (int y = r.p0.y; y <= r.p1.y; ++y)
    {
      if (Map::roomMap[x][y] == &room)
      {
        auto* const f = Map::cells[x][y].rigid;

        if (f->canMoveCmn() && f->canHaveRigid())
        {
          posBucket.push_back(Pos(x, y));
        }
      }
    }
  }

  adjToWalls.clear();
  awayFromWalls.clear();

  for (Pos& pos : posBucket)
  {
    const int NR_BLK_R = getWalkBlockersInDir(Dir::right, pos);
    const int NR_BLK_D = getWalkBlockersInDir(Dir::down, pos);
    const int NR_BLK_L = getWalkBlockersInDir(Dir::left, pos);
    const int NR_BLK_U = getWalkBlockersInDir(Dir::up, pos);

    const bool IS_ZERO_BLK_ALL_DIR =
      NR_BLK_R == 0 && NR_BLK_D == 0 && NR_BLK_L == 0 && NR_BLK_U == 0;

    if (IS_ZERO_BLK_ALL_DIR)
    {
      awayFromWalls.push_back(pos);
      continue;
    }

    bool isDoorAdjacent = false;
    for (int dx = -1; dx <= 1; ++dx)
    {
      for (int dy = -1; dy <= 1; ++dy)
      {
        const auto* const f = Map::cells[pos.x + dx][pos.y + dy].rigid;
        if (f->getId() == FeatureId::door) {isDoorAdjacent = true;}
      }
    }

    if (isDoorAdjacent) {continue;}

    if ((NR_BLK_R == 3 && NR_BLK_U == 1 && NR_BLK_D == 1 && NR_BLK_L == 0) ||
        (NR_BLK_R == 1 && NR_BLK_U == 3 && NR_BLK_D == 0 && NR_BLK_L == 1) ||
        (NR_BLK_R == 1 && NR_BLK_U == 0 && NR_BLK_D == 3 && NR_BLK_L == 1) ||
        (NR_BLK_R == 0 && NR_BLK_U == 1 && NR_BLK_D == 1 && NR_BLK_L == 3))
    {
      adjToWalls.push_back(pos);
      continue;
    }

  }

  TRACE_FUNC_END_VERBOSE;
}

int getWalkBlockersInDir(const Dir dir, const Pos& pos)
{
  int nrBlockers = 0;
  switch (dir)
  {
    case Dir::right:
    {
      for (int dy = -1; dy <= 1; ++dy)
      {
        const auto* const f = Map::cells[pos.x + 1][pos.y + dy].rigid;
        if (!f->canMoveCmn()) {nrBlockers += 1;}
      }
    } break;

    case Dir::down:
    {
      for (int dx = -1; dx <= 1; ++dx)
      {
        const auto* const f = Map::cells[pos.x + dx][pos.y + 1].rigid;
        if (!f->canMoveCmn()) {nrBlockers += 1;}
      }
    } break;

    case Dir::left:
    {
      for (int dy = -1; dy <= 1; ++dy)
      {
        const auto* const f = Map::cells[pos.x - 1][pos.y + dy].rigid;
        if (!f->canMoveCmn()) {nrBlockers += 1;}
      }
    } break;

    case Dir::up:
    {
      for (int dx = -1; dx <= 1; ++dx)
      {
        const auto* const f = Map::cells[pos.x + dx][pos.y - 1].rigid;
        if (!f->canMoveCmn()) {nrBlockers += 1;}
      }
    } break;

    case Dir::downLeft:
    case Dir::downRight:
    case Dir::upLeft:
    case Dir::upRight:
    case Dir::center:
    case Dir::END: {} break;
  }
  return nrBlockers;
}

} //MapPatterns

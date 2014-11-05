#include "Fov.h"

#include <math.h>
#include <iostream>
#include <vector>

#include "CmnTypes.h"
#include "LineCalc.h"
#include "Map.h"
#include "Utils.h"

using namespace std;

namespace Fov {

namespace {

void checkOneCellOfMany(const bool obstructions[MAP_W][MAP_H],
                        const Pos& cellToCheck,
                        const Pos& origin, bool values[MAP_W][MAP_H],
                        const bool IS_AFFECTED_BY_DARKNESS) {

  const Pos deltaToTarget(cellToCheck.x - origin.x, cellToCheck.y - origin.y);
  const vector<Pos>* pathDeltas =
    LineCalc::getFovDeltaLine(deltaToTarget, FOV_STD_RADI_DB);

  if(!pathDeltas) {return;}

  const bool TGT_IS_LGT = Map::cells[cellToCheck.x][cellToCheck.y].isLit;

  Pos curPos;
  Pos prevPos;
  const unsigned int PATH_SIZE = pathDeltas->size();

  for(unsigned int i = 0; i < PATH_SIZE; ++i) {
    curPos.set(origin + pathDeltas->at(i));
    if(i > 1) {
      prevPos.set(origin + pathDeltas->at(i - 1));
      const bool PRE_CELL_IS_DRK = Map::cells[prevPos.x][prevPos.y].isDark;
      const bool CUR_CELL_IS_DRK = Map::cells[curPos.x][curPos.y].isDark;
      const bool CUR_CELL_IS_LGT = Map::cells[curPos.x][curPos.y].isLit;
      if(
        !CUR_CELL_IS_LGT && !TGT_IS_LGT &&
        (PRE_CELL_IS_DRK || CUR_CELL_IS_DRK) && IS_AFFECTED_BY_DARKNESS) {
        return;
      }
    }
    if(curPos == cellToCheck) {
      values[cellToCheck.x][cellToCheck.y] = true;
      return;
    }
    if(i > 0) {
      if(obstructions[curPos.x][curPos.y]) {
        return;
      }
    }
  }
}

} //namespace

bool checkCell(const bool obstructions[MAP_W][MAP_H], const Pos& cellToCheck,
               const Pos& origin, const bool IS_AFFECTED_BY_DARKNESS) {

  if(!Utils::isPosInsideMap(cellToCheck)) {return false;}

  if(Utils::kingDist(origin, cellToCheck) > FOV_STD_RADI_INT) {return false;}

  const Pos deltaToTarget(cellToCheck - origin);
  const vector<Pos>* pathDeltas =
    LineCalc::getFovDeltaLine(deltaToTarget, FOV_STD_RADI_DB);

  if(!pathDeltas) {return false;}

  const bool TGT_IS_LGT = Map::cells[cellToCheck.x][cellToCheck.y].isLit;

  Pos curPos;
  Pos prevPos;
  const int PATH_SIZE = pathDeltas->size();

  for(int i = 0; i < PATH_SIZE; ++i) {
    curPos.set(origin + pathDeltas->at(i));
    if(i > 1) {
      prevPos.set(origin + pathDeltas->at(i - 1));
      const bool PRE_CELL_IS_DRK = Map::cells[prevPos.x][prevPos.y].isDark;
      const bool CUR_CELL_IS_DRK = Map::cells[curPos.x][curPos.y].isDark;
      const bool CUR_CELL_IS_LGT = Map::cells[curPos.x][curPos.y].isLit;
      if(
        !CUR_CELL_IS_LGT && !TGT_IS_LGT &&
        (PRE_CELL_IS_DRK || CUR_CELL_IS_DRK) && IS_AFFECTED_BY_DARKNESS) {
        return false;
      }
    }
    if(curPos == cellToCheck) {return true;}
    if(i > 0 && obstructions[curPos.x][curPos.y]) {return false;}
  }
  return false;
}

void runFovOnArray(const bool obstructions[MAP_W][MAP_H], const Pos& origin,
                   bool values[MAP_W][MAP_H],
                   const bool IS_AFFECTED_BY_DARKNESS) {

  Utils::resetArray(values, false);

  values[origin.x][origin.y] = true;

  const int checkX_end = min(MAP_W - 1, origin.x + FOV_STD_RADI_INT);
  const int checkY_end = min(MAP_H - 1, origin.y + FOV_STD_RADI_INT);

  int checkX = max(0, origin.x - FOV_STD_RADI_INT);

  while(checkX <= checkX_end) {
    int checkY = max(0, origin.y - FOV_STD_RADI_INT);

    while(checkY <= checkY_end) {
      checkOneCellOfMany(obstructions, Pos(checkX, checkY), origin, values,
                         IS_AFFECTED_BY_DARKNESS);
      checkY++;
    }
    checkX++;
  }
}

void runPlayerFov(const bool obstructions[MAP_W][MAP_H], const Pos& origin) {
  bool fovTmp[MAP_W][MAP_H];

  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      Map::cells[x][y].isSeenByPlayer = false;
      fovTmp[x][y]                    = false;
    }
  }

  Map::cells[origin.x][origin.y].isSeenByPlayer = true;
  fovTmp[origin.x][origin.y] = true;

  const int R = FOV_STD_RADI_INT;
  const int X0 = getConstrInRange(0, origin.x - R, MAP_W - 1);
  const int Y0 = getConstrInRange(0, origin.y - R, MAP_H - 1);
  const int X1 = getConstrInRange(0, origin.x + R, MAP_W - 1);
  const int Y1 = getConstrInRange(0, origin.y + R, MAP_H - 1);

  for(int y = Y0; y <= Y1; ++y) {
    for(int x = X0; x <= X1; ++x) {
      checkOneCellOfMany(obstructions, Pos(x, y), origin, fovTmp, true);
      Map::cells[x][y].isSeenByPlayer = fovTmp[x][y];
    }
  }
}

} //Fov

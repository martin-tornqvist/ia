#include "Fov.h"

#include <math.h>
#include <iostream>

#include "Engine.h"
#include "FovPreCalc.h"
#include "ConstDungeonSettings.h"

void Fov::allUnseen(bool array[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      array[x][y] = false;
    }
  }
}

void Fov::performCheck(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                       const coord& cellToCheck, const coord& origin,
                       bool array[MAP_X_CELLS][MAP_Y_CELLS],
                       const bool IS_AFFECTED_BY_DARKNESS) {
  const coord deltaToTarget(cellToCheck.x - origin.x, cellToCheck.y - origin.y);
  vector<coord> pathDeltas;
  eng->fovPreCalc->getLineTravelVector(pathDeltas, deltaToTarget, FOV_STANDARD_RADI_DB);

  const bool TARGET_IS_LIGHT = eng->map->light[cellToCheck.x][cellToCheck.y];

  coord curPos;
  coord prevPos;
  const unsigned int PATH_SIZE = pathDeltas.size();

  for(unsigned int i = 0; i < PATH_SIZE; i++) {
    curPos.set(origin + pathDeltas.at(i));
    if(i > 1) {
      prevPos.set(origin + pathDeltas.at(i - 1));
      const bool PREV_CELL_IS_DARK = eng->map->darkness[prevPos.x][prevPos.y];
      const bool CURRENT_CELL_IS_DARK = eng->map->darkness[curPos.x][curPos.y];
      const bool CURRENT_CELL_IS_LIGHT = eng->map->light[curPos.x][curPos.y];
      if(
        CURRENT_CELL_IS_LIGHT == false &&
        TARGET_IS_LIGHT == false &&
        (PREV_CELL_IS_DARK || CURRENT_CELL_IS_DARK) &&
        IS_AFFECTED_BY_DARKNESS) {
        return;
      }
    }
    if(curPos == cellToCheck) {
      array[cellToCheck.x][cellToCheck.y] = true;
      return;
    }
    if(i > 0) {
      if(obstructions[curPos.x][curPos.y]) {
        return;
      }
    }
  }
}

bool Fov::checkOneCell(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                       const coord& cellToCheck,
                       const coord& origin,
                       const bool IS_AFFECTED_BY_DARKNESS) {
  if(
    eng->basicUtils->chebyshevDistance(
      origin.x, origin.y,
      cellToCheck.x, cellToCheck.y) > FOV_STANDARD_RADI_INT) {
    return false;
  }

  const coord deltaToTarget(cellToCheck - origin);
  vector<coord> pathDeltas;
  eng->fovPreCalc->getLineTravelVector(pathDeltas, deltaToTarget, FOV_STANDARD_RADI_DB);

  const bool TARGET_IS_LIGHT = eng->map->light[cellToCheck.x][cellToCheck.y];

  coord curPos;
  coord prevPos;
  const unsigned int PATH_SIZE = pathDeltas.size();

  for(unsigned int i = 0; i < PATH_SIZE; i++) {
    curPos.set(origin + pathDeltas.at(i));
    if(i > 1) {
      prevPos.set(origin + pathDeltas.at(i - 1));
      const bool PREV_CELL_IS_DARK = eng->map->darkness[prevPos.x][prevPos.y];
      const bool CURRENT_CELL_IS_DARK = eng->map->darkness[curPos.x][curPos.y];
      const bool CURRENT_CELL_IS_LIGHT = eng->map->light[curPos.x][curPos.y];
      if(
        CURRENT_CELL_IS_LIGHT == false &&
        TARGET_IS_LIGHT == false &&
        (PREV_CELL_IS_DARK || CURRENT_CELL_IS_DARK) &&
        IS_AFFECTED_BY_DARKNESS) {
        return false;
      }
    }
    if(curPos == cellToCheck) {
      return true;
    }
    if(i > 0) {
      if(obstructions[curPos.x][curPos.y]) {
        return false;
      }
    }
  }
  return false;
}

void Fov::runFovOnArray(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                        const coord& origin,
                        bool array[MAP_X_CELLS][MAP_Y_CELLS],
                        const bool IS_AFFECTED_BY_DARKNESS) {
  int checkX, checkY;

  allUnseen(array);

  array[origin.x][origin.y] = true;

  const int checkX_end = origin.x + FOV_STANDARD_RADI_INT;
  const int checkY_end = origin.y + FOV_STANDARD_RADI_INT;

  checkX = origin.x - FOV_STANDARD_RADI_INT;

  while(checkX <= checkX_end) {
    checkY = origin.y - FOV_STANDARD_RADI_INT;

    while(checkY <= checkY_end) {
      performCheck(obstructions, coord(checkX, checkY),
                   origin, array, IS_AFFECTED_BY_DARKNESS);
      checkY++;
    }
    checkX++;
  }
}

void Fov::runPlayerFov(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                       const coord& origin) {
  int checkX, checkY;

  allUnseen(eng->map->playerVision);

  eng->map->playerVision[origin.x][origin.y] = true;

  const int checkX_end = origin.x + FOV_STANDARD_RADI_INT;
  const int checkY_end = origin.y + FOV_STANDARD_RADI_INT;

  checkX = origin.x - FOV_STANDARD_RADI_INT;

  while(checkX <= checkX_end) {
    checkY = origin.y - FOV_STANDARD_RADI_INT;

    while(checkY <= checkY_end) {
      performCheck(obstructions, coord(checkX, checkY),
                   origin, eng->map->playerVision, true);
      checkY++;
    }
    checkX++;
  }
}


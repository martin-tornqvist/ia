#include "Fov.h"

#include <math.h>
#include <iostream>

#include "Engine.h"
#include "FovPreCalc.h"
#include "CommonTypes.h"

void Fov::allUnseen(bool values[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      values[x][y] = false;
    }
  }
}

void Fov::checkOneCellOfMany(
  const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const Pos& cellToCheck,
  const Pos& origin, bool values[MAP_X_CELLS][MAP_Y_CELLS],
  const bool IS_AFFECTED_BY_DARKNESS) {

  const Pos deltaToTarget(cellToCheck.x - origin.x, cellToCheck.y - origin.y);
  const vector<Pos>* pathDeltas =
    eng->fovPreCalc->getLineTravelVector(deltaToTarget, FOV_STANDARD_RADI_DB);

  if(pathDeltas == NULL) {
    return;
  }

  const bool TARGET_IS_LIGHT = eng->map->light[cellToCheck.x][cellToCheck.y];

  Pos curPos;
  Pos prevPos;
  const unsigned int PATH_SIZE = pathDeltas->size();

  for(unsigned int i = 0; i < PATH_SIZE; i++) {
    curPos.set(origin + pathDeltas->at(i));
    if(i > 1) {
      prevPos.set(origin + pathDeltas->at(i - 1));
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

bool Fov::checkCell(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                    const Pos& cellToCheck,
                    const Pos& origin,
                    const bool IS_AFFECTED_BY_DARKNESS) {

  if(eng->mapTests->isPosInsideMap(cellToCheck) == false) {
    return false;
  }

  if(
    eng->basicUtils->chebyshevDist(origin, cellToCheck) >
    FOV_STANDARD_RADI_INT) {
    return false;
  }

  const Pos deltaToTarget(cellToCheck - origin);
  const vector<Pos>* pathDeltas =
    eng->fovPreCalc->getLineTravelVector(deltaToTarget, FOV_STANDARD_RADI_DB);

  if(pathDeltas == NULL) {
    return false;
  }

  const bool TARGET_IS_LIGHT = eng->map->light[cellToCheck.x][cellToCheck.y];

  Pos curPos;
  Pos prevPos;
  const unsigned int PATH_SIZE = pathDeltas->size();

  for(unsigned int i = 0; i < PATH_SIZE; i++) {
    curPos.set(origin + pathDeltas->at(i));
    if(i > 1) {
      prevPos.set(origin + pathDeltas->at(i - 1));
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
                        const Pos& origin,
                        bool values[MAP_X_CELLS][MAP_Y_CELLS],
                        const bool IS_AFFECTED_BY_DARKNESS) {
  int checkX, checkY;

  allUnseen(values);

  values[origin.x][origin.y] = true;

  const int checkX_end = min(MAP_X_CELLS - 1, origin.x + FOV_STANDARD_RADI_INT);
  const int checkY_end = min(MAP_Y_CELLS - 1, origin.y + FOV_STANDARD_RADI_INT);

  checkX = max(0, origin.x - FOV_STANDARD_RADI_INT);

  while(checkX <= checkX_end) {
    checkY = max(0, origin.y - FOV_STANDARD_RADI_INT);

    while(checkY <= checkY_end) {
      checkOneCellOfMany(obstructions, Pos(checkX, checkY), origin, values,
                         IS_AFFECTED_BY_DARKNESS);
      checkY++;
    }
    checkX++;
  }
}

void Fov::runPlayerFov(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                       const Pos& origin) {
  int checkX, checkY;

  allUnseen(eng->map->playerVision);

  eng->map->playerVision[origin.x][origin.y] = true;

  const int checkX_end = min(MAP_X_CELLS - 1, origin.x + FOV_STANDARD_RADI_INT);
  const int checkY_end = min(MAP_Y_CELLS - 1, origin.y + FOV_STANDARD_RADI_INT);

  checkX = max(0, origin.x - FOV_STANDARD_RADI_INT);

  while(checkX <= checkX_end) {
    checkY = max(0, origin.y - FOV_STANDARD_RADI_INT);

    while(checkY <= checkY_end) {
      checkOneCellOfMany(obstructions, Pos(checkX, checkY), origin,
                         eng->map->playerVision, true);
      checkY++;
    }
    checkX++;
  }
}


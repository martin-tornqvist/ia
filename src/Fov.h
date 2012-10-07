#ifndef FOV_H
#define FOV_H

#include "Map.h"

#include "ConstTypes.h"

class Actor;

class Fov {
public:
  Fov(Engine* engine) {
    eng = engine;
  }

  bool checkOneCell(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const coord& cellToCheck,
                    const coord& origin, const bool IS_AFFECTED_BY_DARKNESS);

  void runPlayerFov(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const coord& origin);

  void runFovOnArray(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const coord& origin,
                     bool array[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_AFFECTED_BY_DARKNESS);

private:
  void allUnseen(bool array[MAP_X_CELLS][MAP_Y_CELLS]);

  void performCheck(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const coord& cellToCheck, const coord& origin,
                    bool array[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_AFFECTED_BY_DARKNESS);

  Engine* eng;
};

#endif

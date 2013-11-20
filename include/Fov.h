#ifndef FOV_H
#define FOV_H

#include "Map.h"

#include "CommonData.h"

class Actor;

class Fov {
public:
  Fov(Engine* engine) {
    eng = engine;
  }

  bool checkCell(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                 const Pos& cellToCheck,
                 const Pos& origin, const bool IS_AFFECTED_BY_DARKNESS);

  void runPlayerFov(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                    const Pos& origin);

  void runFovOnArray(const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS],
                     const Pos& origin, bool values[MAP_X_CELLS][MAP_Y_CELLS],
                     const bool IS_AFFECTED_BY_DARKNESS);

private:
  void allUnseen(bool values[MAP_X_CELLS][MAP_Y_CELLS]);

  void checkOneCellOfMany(
    const bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const Pos& cellToCheck,
    const Pos& origin, bool values[MAP_X_CELLS][MAP_Y_CELLS],
    const bool IS_AFFECTED_BY_DARKNESS);

  Engine* eng;
};

#endif

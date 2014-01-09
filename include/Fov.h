#ifndef FOV_H
#define FOV_H

#include "CommonData.h"
#include "CommonTypes.h"

class Actor;
class Engine;

class Fov {
public:
  Fov(Engine& engine) : eng(engine) {}

  bool checkCell(const bool obstructions[MAP_W][MAP_H],
                 const Pos& cellToCheck,
                 const Pos& origin, const bool IS_AFFECTED_BY_DARKNESS);

  void runPlayerFov(const bool obstructions[MAP_W][MAP_H], const Pos& origin);

  void runFovOnArray(const bool obstructions[MAP_W][MAP_H], const Pos& origin,
                     bool values[MAP_W][MAP_H],
                     const bool IS_AFFECTED_BY_DARKNESS);

private:
  void checkOneCellOfMany(
    const bool obstructions[MAP_W][MAP_H], const Pos& cellToCheck,
    const Pos& origin, bool values[MAP_W][MAP_H],
    const bool IS_AFFECTED_BY_DARKNESS);

  Engine& eng;
};

#endif

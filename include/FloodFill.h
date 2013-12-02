#ifndef FLOODFILL_H
#define FLOODFILL_H

#include "CommonTypes.h"
#include "CommonData.h"

class Engine;

class FloodFill {
public:
  FloodFill(Engine* engine) : eng(engine) {}

  void run(
    const Pos& origin, bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
    int values[MAP_X_CELLS][MAP_Y_CELLS], int travelLimit,
    const Pos& target);

private:
  Engine* const eng;
};

#endif

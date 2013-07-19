#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <queue>

#include "CommonTypes.h"
#include "Config.h"

class Engine;

class Pathfinder {
public:
  Pathfinder(Engine* engine) : eng(engine) {}

  //TODO Let the pathfinder fill a referenced parameter position vector instead of returning a vector
  vector<Pos> findPath(const Pos origin,
                         bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS],
                         const Pos target);
private:
  Engine* eng;
};

#endif


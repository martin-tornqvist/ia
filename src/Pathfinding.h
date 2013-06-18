#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <queue>

#include "CommonTypes.h"
#include "Config.h"

class Engine;

class Pathfinder {
public:
	Pathfinder(Engine* engine) :
		eng(engine) {
	}

	//TODO Let the pathfinder fill a referenced parameter coordinate vector instead of returning a vector
	vector<coord> findPath(const coord origin,
                        bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS],
                        const coord target);

private:
	Engine* eng;
};

#endif


#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

using namespace std;

#include <vector>

#include "ConstTypes.h"
#include "ConstDungeonSettings.h"

class Engine;

class MapPatterns {
public:
	MapPatterns(Engine* engine) :
		eng(engine) {
	}

	/*
	 * Assumptions: No isolated pillars
	 */
	vector<coord> getAllCellsExceptEdge(Rectangle bounds, const int EDGE_DIST);

private:
	Engine* eng;

};

#endif

#include "Pathfinding.h"

#include <iostream>

#include "Engine.h"

using namespace std;

vector<coord> Pathfinder::findPath(const coord origin, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS], const coord target) {
	vector<coord> path;

	int floodValues[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeFloodFill(origin, blockingCells, floodValues, 1000, target);

	bool pathExists = floodValues[target.x][target.y] != 0;

	if(pathExists == true) {
		vector<coord> coordinates;
		coord c;

		int currentX = target.x;
		int currentY = target.y;

		bool done = false;
		while(done == false) {
		    //TODO use for-loop instead
			int xOffset = 0; // starts from 0 instead of -1 so that cardinal directions is tried first
			while(xOffset <= 1) {
				int yOffset = 0; // ------------------------- | | -------------------------
				while(yOffset <= 1) {
					if(xOffset != 0 || yOffset != 0) {
					    //TODO increase readability
						if(currentX + xOffset >= 0 && currentY + yOffset >= 0) {
							if((floodValues[currentX + xOffset][currentY + yOffset] == floodValues[currentX][currentY] - 1 && floodValues[currentX
									+ xOffset][currentY + yOffset] != 0) || (currentX + xOffset == origin.x && currentY + yOffset == origin.y)) {
								c.x = currentX;
								c.y = currentY;
								path.push_back(c);

								currentX = currentX + xOffset;
								currentY = currentY + yOffset;

								if(currentX == origin.x && currentY == origin.y) {
									done = true;
								}

								xOffset = 99;
								yOffset = 99;
							}
						}
					}
					yOffset = yOffset == 1 ? 2 : yOffset == -1 ? 1 : yOffset == 0 ? -1 : yOffset;
				}
				xOffset = xOffset == 1 ? 2 : xOffset == -1 ? 1 : xOffset == 0 ? -1 : xOffset;
			}
		}
	}

	return path;
}


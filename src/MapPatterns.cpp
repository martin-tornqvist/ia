#include "MapPatterns.h"

#include "Engine.h"
#include "Map.h"

vector<coord> MapPatterns::getAllCellsExceptEdge(Rectangle bounds, const int EDGE_DIST) {

	//TODO MapAeas are now supposed to store their edge coords, see if it works and can be used here

	vector<coord> returnVector;

	for(int x = bounds.x0y0.x; x <= bounds.x1y1.x; x++) {
		for(int y = bounds.x0y0.y; y <= bounds.x1y1.y; y++) {
			bool posOk = true;
			for(int dx = -1 - EDGE_DIST; dx <= 1 + EDGE_DIST; dx++) {
				for(int dy = -1 - EDGE_DIST; dy <= 1 + EDGE_DIST; dy++) {
					if(dx != 0 || dy != 0) {
						const coord c(x + dx, y + dy);
						if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneWall) {
							posOk = false;
							dx = 9999;
							dy = 9999;
						}
					}
				}
			}
			if(posOk) {
				returnVector.push_back(coord(x, y));
			}
		}
	}
	return returnVector;
}

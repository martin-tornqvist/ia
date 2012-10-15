#ifndef MAPBUILD_BSP_H
#define MAPBUILD_BSP_H

#include <vector>

#include "ConstTypes.h"

#include "FeatureData.h"
#include "MapArea.h"

using namespace std;

enum HorizontalVertical_t {
	horizontal, vertical
};

class Engine;
struct Region;

class MapBuildBSP {
public:
	MapBuildBSP(Engine* engine) :
		eng(engine) {
	}
	~MapBuildBSP() {
		for(unsigned int i = 0; i < mapAreas_.size(); i++) {
			delete mapAreas_.at(i);
		}
	}
	void run();

	void coverAreaWithFeature(const Rect area, const Feature_t feature);

	int getNrStepsInDirectionUntilWallFound(coord c, const Directions_t dir) const;

private:
	bool isAllRoomsConnected();

	MapArea* buildRoom(const Rect roomCoords);

	bool roomCells[MAP_X_CELLS][MAP_Y_CELLS]; //Used for helping to build the map
   bool regionsToBuildCave[3][3];

   void makeCrumbleRoom(const Rect roomAreaIncludingWalls, const coord proxEventPos);

	void connectRegions(Region* regions[3][3]);
	void buildAuxRooms(Region* regions[3][3]);
	bool tryPlaceAuxRoom(const int X0, const int Y0, const int W, const int H, bool blockers[MAP_X_CELLS][MAP_Y_CELLS], const coord doorPos);

	void buildMergedRegion(Region* regions[3][3], const int SPLIT_X1, const int SPLIT_X2, const int SPLIT_Y1, const int SPLIT_Y2);

   void buildCaves(Region* regions[3][3]);

	void buildCorridorBetweenRooms(const Region& region1, const Region& region2);
	void placeDoorAtPosIfSuitable(const coord pos);

	void reshapeRoom(MapArea& area);

	void buildRoomsInRooms();

	int getTotalNrOfConnections(Region* regions[3][3]) const;

	bool doorPositionCandidates[MAP_X_CELLS][MAP_Y_CELLS];

	void findEdgesOfRoom(const Rect roomCoords, vector<coord>& vectorToFill);

	bool isRegionFoundInCardinalDirection(const coord pos, bool region[MAP_X_CELLS][MAP_Y_CELLS]) const;

	bool isAreaFree(const Rect& area, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]);
	bool isAreaAndBorderFree(const Rect& areaWithBorder, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]);
	bool isAreaFree(const int X0, const int Y0, const int X1, const int Y1, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]);

	void decorateWalls();

	coord placeStairs();

  void makeLeverPuzzle();

	void revealAllDoorsBetweenPlayerAndStairs(const coord& stairsCoord);

//	void buildNaturalArea(Region* regions[3][3]);
//	void makeRiver(Region* regions[3][3]);

	vector<MapArea*> mapAreas_;

	bool forbiddenStairCellsGlobal[MAP_X_CELLS][MAP_Y_CELLS];

	Engine* eng;
};

struct ConnectionPointsAndDistance {
public:
	ConnectionPointsAndDistance(coord c1_, coord c2_, int dist_) :
		c1(c1_), c2(c2_), dist(dist_) {
	}
	ConnectionPointsAndDistance() {
	}

	coord c1;
	coord c2;
	int dist;
};

struct Region {
public:
	Region(coord x0y0, coord x1y1);
	Region();
	~Region();

	Rect getRandomCoordsForRoom(Engine* eng) const;
	Rect getRegionCoords() const {
		return Rect(x0y0_, x1y1_);
	}

	bool isRegionNeighbour(const Region& other, Engine* const engine);

	coord getCenterCoord() const {
		return (x1y1_ + x0y0_) / 2;
	}
	coord getX0Y0() const {
		return x0y0_;
	}
	coord getX1Y1() const {
		return x1y1_;
	}

	int getNrOfConnections();

	bool regionsConnectedTo[3][3];

	MapArea* mapArea;
	bool isConnected;

private:
	coord x0y0_, x1y1_;
};

#endif

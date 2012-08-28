#ifndef MAP_AREA_H
#define MAP_AREA_H

#include <vector>

#include "ConstTypes.h"
#include "Map.h"

using namespace std;

class BasicUtils;
class MapArea;
class Engine;

struct MapJunction {
	MapJunction() :
		hasTarget(false), coordinate(coord(0, 0)), expansionPossible(true), nrOfTriesToBuild(0) {
	}

	~MapJunction() {
	}

	bool hasTarget;
	coord coordinate;
	bool expansionPossible;
	int nrOfTriesToBuild;

	//For example if a room makes a junction two steps right of its rightmost cell,
	//"relativeOrigin" would be (-2, 0).
	//This is used to connect areas at their junctions.
	coord relativeOrigin;
};

enum RoomReshape_t {
	roomReshape_trimCorners, roomReshape_pillarsRandom //roomReshape_pillarsFollowEdge
};

enum MapAreaType_t {
	mapArea_none, mapArea_corridor, mapArea_room, mapArea_template, mapArea_connection
};

class MapArea {
public:
	MapArea() : m_areaType(mapArea_none), x0y0(coord(0,0)), x1y1(coord(0,0)), isSpecialRoomAllowed(true), isCave(false) {
		m_junctions.resize(0);
	}
	MapArea(MapAreaType_t areaType, Rectangle coords) :
		m_areaType(areaType), x0y0(coords.x0y0), x1y1(coords.x1y1), isSpecialRoomAllowed(true), isCave(false) {
		m_junctions.resize(0);
	}

	bool isPosOnEdge(const coord& pos) {
		for(unsigned int i = 0; i < roomEdges.size(); i++) {
			if(roomEdges.at(i) == pos) {
				return true;
			}
		}
		return false;
	}
	vector<coord> roomEdges;

	~MapArea() {
		m_junctions.clear();
		m_junctions.resize(0);
	}

	MapJunction* getRandomExpandableJunction(Engine* const engine);

	void setArea(int x0, int y0, int x1, int y1, MapAreaType_t areaType);

	void addJunction(const int x, const int y, const coord relativeOrigin, Engine* engine);

	MapJunction* getJunctionAt(int x, int y);

	vector<MapJunction> m_junctions;
	MapAreaType_t m_areaType;
	coord x0y0, x1y1;

	bool isSpecialRoomAllowed;
   bool isCave;
private:
};

#endif

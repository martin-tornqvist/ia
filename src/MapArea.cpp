#include "MapArea.h"

#include "BasicUtils.h"

#include "MapBuild.h"
#include "BasicUtils.h"
#include "Engine.h"

MapJunction* MapArea::getRandomExpandableJunction(Engine* const engine) {
	const unsigned NR_OF_JUNCTIONS = m_junctions.size();

	if(NR_OF_JUNCTIONS == 0)
		return NULL;

	vector<MapJunction*> expandableJunctions;
	MapJunction* currentJunction;

	for(unsigned int i = 0; i < NR_OF_JUNCTIONS; i++) {
		currentJunction = &(m_junctions.at(i));

		if(currentJunction->expansionPossible == true) {
			expandableJunctions.push_back(currentJunction);
		}
	}

	if(expandableJunctions.size() == 0) {
		return NULL;
	}

	return expandableJunctions.at(engine->dice(1, expandableJunctions.size()) - 1);
}

void MapArea::addJunction(const int x, const int y, const coord relativeOrigin, Engine* engine) {
	if(engine->mapTests->isCellInsideMainScreen(x, y) == true) {
		MapJunction junction;

		junction.coordinate.set(x, y);

		junction.relativeOrigin.set(relativeOrigin);

		m_junctions.push_back(junction);
	}
}

void MapArea::setArea(int x0, int y0, int x1, int y1, MapAreaType_t areaType) {
	m_areaType = areaType;
	x0y0 = coord(x0, y0);
	x1y1 = coord(x1, y1);
}

MapJunction* MapArea::getJunctionAt(int x, int y) {
	MapJunction* junction;

	for(unsigned int i = 0; i < m_junctions.size(); i++) {
		junction = &(m_junctions.at(i));
		if(junction->coordinate.x == x && junction->coordinate.y == y) {
			return junction;
		}
	}

	return NULL;
}


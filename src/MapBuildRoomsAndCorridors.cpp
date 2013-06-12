//#include "MapBuild.h"
//
////#include <iostream>
////#include <vector>
//
//#include "Engine.h"
//
////#include "FeatureDoor.h"
////#include "SpecialRoom.h"
//
//void MapBuild::buildDungeonLevel() {
////	const unsigned int MIN_NUMBER_OF_CELLS = 500;
////	unsigned int nrOfCells = 0;
////	while(nrOfCells <= MIN_NUMBER_OF_CELLS) {
////		m_mapAreas.clear();
////		m_mapAreas.resize(0);
////
////		eng->map->clearDungeon();
////		buildCorridorsAndRooms();
////		buildRoomsAtCorridorEnds();
////		connectCorridorsAndRooms();
////		removeCorridorStumps();
////		placeDoors();
////		bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
////		eng->mapTests->makeWalkBlockingArrayFeaturesOnly(blockers);
////		eng->basicUtils->reverseBoolArray(blockers);
////		vector<coord> freeCells;
////		eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
////		nrOfCells = freeCells.size();
////	}
////
//////	eng->specialRoomHandler->makeSpecialRooms(m_mapAreas);
////
////	placeStairs();
//}
//
//void MapBuild::buildCorridorsAndRooms() {
////	const bool varyCorridorLength = eng->dice.percentile() < 85;
////	const int forcedCorridorLength = varyCorridorLength == true ? -1 : getRandomCorridorLength();
////
////	const int diceValueCeilRoom = eng->dice(1, 20) + 35;
////	const int templateProbability = 10;
////	const int diceValueCeilCorr = 100 - templateProbability;
////
////	m_mapAreas.resize(0);
////	for(int y = 0; y < MAP_Y_CELLS; y++) {
////		for(int x = 0; x < MAP_X_CELLS; x++) {
////			forbiddenStairCells[x][y] = true;
////		}
////	}
////
////	const coord playerCoord(eng->player->pos);
////
////	const int initRoomWidth = getRandomRoomSize();
////	const int initRoomHeight = getRandomRoomSize();
////	const int initRoomOffsetX = eng->dice(1, initRoomWidth) - 1;
////	const int initRoomOffsetY = eng->dice(1, initRoomHeight) - 1;
////	const int initRoomX = playerCoord.x - initRoomOffsetX;
////	const int initRoomY = playerCoord.y - initRoomOffsetY;
////
////	buildStartRoom(initRoomX, initRoomY, initRoomX + initRoomWidth - 1, initRoomY + initRoomHeight - 1);
////
////	MapJunction* junction = NULL;
////
////	int mapAreaIndex = getRandomExpandableArea();
////	while(mapAreaIndex != -1) {
////		junction = m_mapAreas.at(mapAreaIndex).getRandomExpandableJunction(eng);
////
////		if(junction != NULL) {
////			const int rnd = eng->dice.percentile();
////			const MapAreaType_t areaType = rnd < diceValueCeilRoom ? mapArea_room : (rnd < diceValueCeilCorr ? mapArea_corridor : mapArea_template);
////
////			if(areaType == mapArea_template) {
////				buildGeneralAreaFromTemplate(junction);
////			} else {
////				if(buildRectangularArea(junction, areaType, forcedCorridorLength) == false) {
////
////					junction->nrOfTriesToBuild += 1;
////
////					if(junction->nrOfTriesToBuild == NUMBER_OF_TRIES_TO_BUILD_FROM_JUNCTIONS) {
////						junction->expansionPossible = false;
////					}
////				}
////			}
////		}
////		mapAreaIndex = getRandomExpandableArea();
////	}
//}
//
//int MapBuild::getRandomExpandableArea() {
////	const unsigned NR_OF_AREAS = m_mapAreas.size();
////
////	if(NR_OF_AREAS == 0) {
////
////		return -1;
////	}
////
////	vector<unsigned int> expandableAreas;
////	MapArea* currentArea;
////
////	for(unsigned int i = 0; i < NR_OF_AREAS; i++) {
////		currentArea = &(m_mapAreas.at(i));
////
////		if(currentArea->getRandomExpandableJunction(eng) != NULL) {
////			expandableAreas.push_back(i);
////		}
////	}
////
////	if(expandableAreas.size() == 0) {
////		return -1;
////	}
////
////	return expandableAreas.at(eng->dice(1, expandableAreas.size()) - 1);
//    return 0;
//}
//
//coord getAreaOffset(const int direction, const int width, const int height) {
//	coord offset;
//
//	switch (direction) {
//	case direction_right:
//		offset.set(0, -(height - 1) / 2);
//	break;
//
//	case direction_up:
//		offset.set(-(width - 1) / 2, -(height - 1));
//	break;
//
//	case direction_left:
//		offset.set(-(width - 1), -(height - 1) / 2);
//	break;
//
//	case direction_down:
//		offset.set(-(width - 1) / 2, 0);
//	break;
//
//	default: {}
//	break;
//	}
//
//	return offset;
//}
//
//Directions_t getOppositeDirection(const Directions_t direction) {
//	if(direction == direction_right)
//		return direction_left;
//	if(direction == direction_up)
//		return direction_down;
//	if(direction == direction_left)
//		return direction_right;
//	if(direction == direction_down)
//		return direction_up;
//
//	return direction_right;
//}
//
//Directions_t rotateDirectionCCW(const Directions_t direction) {
//	if(direction == direction_right)
//		return direction_up;
//	if(direction == direction_up)
//		return direction_left;
//	if(direction == direction_left)
//		return direction_down;
//	if(direction == direction_down)
//		return direction_right;
//
//	return direction_right;
//}
//
//bool MapBuild::buildRectangularArea(MapJunction* const junction, const MapAreaType_t areaType, const int forcedCorridorLength) {
//	const Directions_t startDirection = getRandomDirection();
//
//	Directions_t currentDirection = startDirection;
//
//	bool okToBuildHere = true;
//
//	bool done = false;
//	while(done == false) {
//		const coord dimensions = getRectangularAreaDimensions(currentDirection, areaType, forcedCorridorLength);
//		coord upperLeft(junction->coordinate);
//		const coord offset = getAreaOffset(currentDirection, dimensions.x + 1, dimensions.y + 1);
//		upperLeft += offset;
//		coord lowerRight(upperLeft + dimensions);
//		okToBuildHere = isAreaFree(upperLeft.x - 1, upperLeft.y - 1, lowerRight.x + 1, lowerRight.y + 1);
//
//		if(okToBuildHere) {
//			coverAreaWithFeaturecoverAreaWithFeature(Rect(upperLeft, lowerRight), feature_stoneFloor);
//
//			junction->hasTarget = true;
//
//			MapArea area;
//			area.setArea(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y, areaType);
//
//			m_mapAreas.push_back(area);
//			addJunctionsToArea(m_mapAreas.back(), upperLeft, lowerRight, dimensions.x + 1, dimensions.y + 1, currentDirection);
//
//			done = true;
//		} else {
//			currentDirection = rotateDirectionCCW(currentDirection);
//			if(currentDirection == startDirection) {
//				done = true;
//			}
//		}
//	}
//	return okToBuildHere;
//}
//
//Directions_t MapBuild::getRandomDirection() const {
//	return static_cast<Directions_t> (eng->dice(1, 4) * 2);
//}
//
//int MapBuild::getRandomCorridorLength() const {
//	return eng->dice(1, 8) + 1;
//}
//
//int MapBuild::getRandomRoomSize() const {
//	return eng->dice(1, 7) * 2 + 1;
//}
//
//int MapBuild::getRandomRoomSizeSmall() const {
//	return eng->dice(1, 3) * 2 + 1;
//}
//
//coord MapBuild::getRectangularAreaDimensions(const Directions_t direction, const MapAreaType_t areaType, const int forcedCorridorLength) {
//	coord dimensions;
//
//	switch (areaType) {
//	case mapArea_room: {
//		if(eng->dice(1, 3) == 1) {
//			dimensions.x = getRandomRoomSizeSmall() - 1;
//			dimensions.y = getRandomRoomSizeSmall() - 1;
//
//		} else {
//			dimensions.x = getRandomRoomSize() - 1;
//			dimensions.y = getRandomRoomSize() - 1;
//		}
//	}
//	break;
//
//	case mapArea_corridor: {
//		const int forced = forcedCorridorLength;
//
//		if(direction == direction_right || direction == direction_left) {
//			dimensions.x = forced == -1 ? getRandomCorridorLength() - 1 : forced - 1;
//			dimensions.y = 0;
//		} else {
//			dimensions.x = 0;
//			dimensions.y = forced == -1 ? getRandomCorridorLength() - 1 : forced - 1;
//		}
//	}
//	break;
//
//	default: {}
//	break;
//	}
//
//	return dimensions;
//}
//
//void MapBuild::addJunctionsToArea(MapArea& area, coord upperLeft, coord lowerRight, const int width, const int height,
//		const Directions_t areaDirection) {
//	switch (area.m_areaType) {
//	case mapArea_corridor: {
//		addJunction(area, upperLeft, lowerRight, width, height, areaDirection);
//
//		Directions_t junctionDirection = getRandomDirection();
//		if(junctionDirection != getOppositeDirection(areaDirection) && junctionDirection != areaDirection) {
//			addJunction(area, upperLeft, lowerRight, width, height, junctionDirection);
//		}
//	}
//	break;
//
//	case mapArea_room:
//	case mapArea_template: {
//		Directions_t junctionDirection;
//		for(unsigned int i = 2; i <= 8; i += 2) {
//			junctionDirection = static_cast<Directions_t> (i);
//			if(junctionDirection != getOppositeDirection(areaDirection)) {
//				addJunction(area, upperLeft, lowerRight, width, height, junctionDirection);
//			}
//		}
//	}
//	break;
//
//	default: {}
//	break;
//	}
//}
//
//void MapBuild::addJunction(MapArea& area, const coord upperLeft, const coord lowerRight, const int width, const int height,
//		const Directions_t junctionDirection) {
//	coord newJunction;
//	coord relativeOrigin;
//
//	switch (junctionDirection) {
//	case direction_right: {
//		newJunction.set(lowerRight.x + 2, upperLeft.y + (height - 1) / 2);
//		relativeOrigin.set(-2, 0);
//	}
//	break;
//
//	case direction_up: {
//		newJunction.set(upperLeft.x + (width - 1) / 2, upperLeft.y - 2);
//		relativeOrigin.set(0, 2);
//	}
//	break;
//
//	case direction_left: {
//		newJunction.set(upperLeft.x - 2, upperLeft.y + (height - 1) / 2);
//		relativeOrigin.set(2, 0);
//	}
//	break;
//
//	case direction_down: {
//		newJunction.set(upperLeft.x + (width - 1) / 2, lowerRight.y + 2);
//		relativeOrigin.set(0, -2);
//	}
//	break;
//
//	default: {
//	}
//	break;
//	}
//
//	area.addJunction(newJunction.x, newJunction.y, relativeOrigin, eng);
//}
//
//bool MapBuild::buildGeneralAreaFromTemplate(MapJunction* junction) {
//	bool areaIsFree = false;
//
//	MapTemplate* t = eng->mapTemplateHandler->getRandomTemplateForMapBuilder();
//	coord jT;
//	const coord jP = junction->coordinate;
//
//	int x0, y0, x1, y1;
//
//	for(unsigned int i = 0; i < t->junctionPositions.size(); i++) {
//
//		jT = t->junctionPositions.at(i);
//		x0 = jP.x - jT.x;
//		x0 += jT.x == -2 ? -2 : (jT.x == t->width + 1 ? 2 : 0);
//		y0 = jP.y - jT.y;
//		y0 += jT.y == -2 ? -2 : (jT.y == t->height + 1 ? 2 : 0);
//		x1 = x0 + t->width - 1;
//		y1 = y0 + t->height - 1;
//
//		if(isAreaFree(x0 - 1, y0 - 1, x1 + 1, y1 + 1)) {
//			buildFromTemplate(coord(x0, y0), t);
//
//			areaIsFree = true;
//			i = 99999;
//		}
//	}
//
//	return areaIsFree;
//}
//
//void MapBuild::buildRoomsAtCorridorEnds() {
//	bool built = false;
//
//	for(int r = 0; r < 1; r++) {
//		for(int i = 0; i < int (m_mapAreas.size()); i++) {
//
//			if(m_mapAreas.at(i).m_areaType == mapArea_corridor) {
//
//				for(unsigned int q = 0; q < m_mapAreas.at(i).m_junctions.size(); q++) {
//
//					if(m_mapAreas.at(i).m_junctions.at(q).expansionPossible == true) {
//
//						if(m_mapAreas.at(i).m_junctions.at(q).hasTarget == false) {
//
//							built = false;
//
//							for(int ii = 0; ii < 5; ii++) {
//								if(buildRectangularArea(&(m_mapAreas.at(i).m_junctions.at(q)), mapArea_room) == true) {
//									r--;
//									built = true;
//									break;
//								}
//							}
//
////							if(built == false) {
////								for(int ii = 0; ii < 5; ii++) {
////									if(buildRectangularArea(&(m_mapAreas.at(i).m_junctions.at(q)), mapArea_corridor, -1) == true) {
////										r--;
////										built = true;
////										break;
////									}
////								}
////							}
//
//							if(built == false) {
//								m_mapAreas.at(i).m_junctions.at(q).nrOfTriesToBuild += 1;
//								if(m_mapAreas.at(i).m_junctions.at(q).nrOfTriesToBuild == NUMBER_OF_TRIES_TO_BUILD_FROM_JUNCTIONS) {
//									m_mapAreas.at(i).m_junctions.at(q).expansionPossible = false;
//								}
//							}
//						}
//					}
//				}
//				if(built == true)
//					break;
//			}
//			if(built == true)
//				break;
//		}
//	}
//}
//
//void MapBuild::connectCorridorsAndRooms() {
//	MapArea* area = NULL;
//	MapJunction* junction = NULL;
//	coord c;
//
//	for(unsigned int i = 0; i < m_mapAreas.size(); i++) {
//		area = &(m_mapAreas.at(i));
//
//		for(unsigned int ii = 0; ii < area->m_junctions.size(); ii++) {
//			junction = &(area->m_junctions.at(ii));
//			c = junction->coordinate;
//
//			tryConnectPoints(c, junction->relativeOrigin);
//		}
//	}
//}
//
//bool MapBuild::tryConnectPoints(const coord& c, const coord& offset) {
////	if(eng->mapTests->isCellInsideMap(c) == true && eng->mapTests->isCellInsideMap(c + offset) == true) {
////		if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor && eng->map->featuresStatic[(c + offset).x][(c + offset).y]->getId()
////				== feature_stoneFloor) {
////			const coord connectPoint = c + offset / 2;
////			eng->featureFactory->spawnFeatureAt(feature_stoneFloor, connectPoint);
////			return true;
////		}
////	}
//	return false;
//}
//
//void MapBuild::removeCorridorStumps() {
////	for(unsigned int x = 1; x < MAP_X_CELLS - 1; x++) {
////		for(unsigned int y = 1; y < MAP_Y_CELLS - 1; y++) {
////			const coord c = coord(x, y);
////			if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
////				int adjacentFloorCardinal = 0;
////				int adjacentFloorDiagonal = 0;
////				for(int dx = -1; dx <= 1; dx++) {
////					for(int dy = -1; dy <= 1; dy++) {
////						if(dx != 0 || dy != 0) {
////							const coord adjCell = c + coord(dx, dy);
////							if(eng->map->featuresStatic[adjCell.x][adjCell.y]->getId() == feature_stoneFloor) {
////								if(dx == 0 || dy == 0) {
////									adjacentFloorCardinal++;
////								} else {
////									adjacentFloorDiagonal++;
////								}
////							}
////						}
////					}
////				}
////
////				if(adjacentFloorCardinal == 1 && adjacentFloorDiagonal <= 1) {
////					eng->featureFactory->spawnFeatureAt(feature_stoneWall, c);
////					x = 1;
////					y = 1;
////				}
////			}
////		}
////	}
//}
//
//void MapBuild::placeDoors() {
//	const int CHANCE_FOR_DOOR = 75;
//
//	coord x0y0;
//	coord x1y1;
//	MapArea* area = NULL;
//	vector<coord> doorCandidatesLeft;
//	vector<coord> doorCandidatesDown;
//	vector<coord> doorCandidatesRight;
//	vector<coord> doorCandidatesUp;
//
//	for(int y = 0; y < MAP_Y_CELLS; y++) {
//		for(int x = 0; x < MAP_X_CELLS; x++) {
//			doors[x][y] = false;
//		}
//	}
//
//	coord c;
//
//	for(unsigned int i = 0; i < m_mapAreas.size(); i++) {
//		area = &(m_mapAreas.at(i));
//
//		if(area->m_areaType == mapArea_room || area->m_areaType == mapArea_template) {
//			x0y0.set(area->x0y0 + coord(-1, -1));
//			x1y1.set(area->x1y1 + coord(+1, +1));
//
//			c.set(x0y0);
//
//			while(c.y != x1y1.y) {
//				if(eng->dice.percentile() < CHANCE_FOR_DOOR) {
//					if(isPointGoodForDoor(c) == true) {
//						doorCandidatesLeft.push_back(c);
//					}
//				}
//				c.y += 1;
//			}
//			while(c.x != x1y1.x) {
//				if(eng->dice.percentile() < CHANCE_FOR_DOOR) {
//					if(isPointGoodForDoor(c) == true) {
//						doorCandidatesDown.push_back(c);
//					}
//				}
//				c.x += 1;
//			}
//			while(c.y != x0y0.y) {
//				if(eng->dice.percentile() < CHANCE_FOR_DOOR) {
//					if(isPointGoodForDoor(c) == true) {
//						doorCandidatesRight.push_back(c);
//					}
//				}
//				c.y -= 1;
//			}
//			while(c.x != x0y0.x) {
//				if(eng->dice.percentile() < CHANCE_FOR_DOOR) {
//					if(isPointGoodForDoor(c) == true) {
//						doorCandidatesUp.push_back(c);
//					}
//				}
//				c.x -= 1;
//			}
//
//			placeDoorIfSingleEntrance(doorCandidatesRight);
//			placeDoorIfSingleEntrance(doorCandidatesUp);
//			placeDoorIfSingleEntrance(doorCandidatesLeft);
//			placeDoorIfSingleEntrance(doorCandidatesDown);
//
//			doorCandidatesLeft.resize(0);
//			doorCandidatesDown.resize(0);
//			doorCandidatesRight.resize(0);
//			doorCandidatesUp.resize(0);
//		}
//	}
//}
//
//bool MapBuild::isPointGoodForDoor(coord c) {
//	bool goodVertical = true;
//	bool goodHorizontal = true;
//
//	for(int i = -1; i <= 1; i++) {
//
//		if(isPointPassable(c.x + i, c.y) == false)
//			goodHorizontal = false;
//
//		if(isPointPassable(c.x, c.y + i) == false)
//			goodVertical = false;
//
//		if(i != 0) {
//			if(isPointPassable(c.x, c.y + i) == true)
//				goodHorizontal = false;
//
//			if(isPointPassable(c.x + i, c.y) == true)
//				goodVertical = false;
//		}
//	}
//
//	return goodHorizontal || goodVertical;
//}
//
//bool MapBuild::isPointPassable(const int x, const int y) {
////	if(eng->mapTests->isCellInsideMap(x, y) == true) {
////		return eng->map->featuresStatic[x][y]->isMovePassable(eng->player);
////	}
//	return false;
//}
//
//void MapBuild::placeDoorIfSingleEntrance(const vector<coord>& doorCandidates) {
////	if(doorCandidates.size() == 1) {
////		coord c(doorCandidates.back());
////		if(eng->mapTests->isCellInsideMap(c) == true) {
////			if(doors[c.x][c.y] == false) {
////				eng->featureFactory->spawnFeatureAt(feature_door, c, new DoorSpawnData(eng->featureData->getFeatureDef(feature_stoneWall)));
////				doors[c.x][c.y] = true;
////			}
////		}
////	}
//}
//
//bool MapBuild::buildStartRoom(int x0, int y0, int x1, int y1) {
//	bool areaIsFree = true;
//
//	while(x0 <= 1) {
//		x0++;
//		x1++;
//	}
//	while(x1 >= MAP_X_CELLS - 1) {
//		x0--;
//		x1--;
//	}
//	while(y0 <= 1) {
//		y0++;
//		y1++;
//	}
//	while(y1 >= MAP_Y_CELLS - 1) {
//		y0--;
//		y1--;
//	}
//
//	if(x0 <= 1 || x1 >= MAP_X_CELLS - 1 || y0 <= 1 || y1 >= MAP_Y_CELLS - 1) {
//		areaIsFree = false;
//	}
//
//	if(areaIsFree == true) {
//		for(int x = x0; x <= x1; x++) {
//			for(int y = y0; y <= y1; y++) {
//				eng->featureFactory->spawnFeatureAt(feature_stoneFloor, coord(x, y));
//			}
//		}
//
//		MapArea area;
//		area.setArea(x0, y0, x1, y1, mapArea_room);
//
//		coord j;
//
//		j.x = x1 + 2;
//		j.y = y0 + (y1 - y0) / 2;
//		area.addJunction(j.x, j.y, coord(-2, 0), eng);
//
//		j.x = x0 + (x1 - x0) / 2;
//		j.y = y0 - 2;
//		area.addJunction(j.x, j.y, coord(0, 2), eng);
//
//		j.x = x0 - 2;
//		j.y = y0 + (y1 - y0) / 2;
//		area.addJunction(j.x, j.y, coord(2, 0), eng);
//
//		j.x = x0 + (x1 - x0) / 2;
//		j.y = y1 + 2;
//		area.addJunction(j.x, j.y, coord(0, -2), eng);
//
//		m_mapAreas.push_back(area);
//	}
//
//	return areaIsFree;
//}
//
//void MapBuild::placeStairs() {
//	for(unsigned int i = 0; i < m_mapAreas.size(); i++) {
//		const MapArea* const currentArea = &(m_mapAreas.at(i));
//
//		if(currentArea->m_areaType == mapArea_room || currentArea->m_areaType == mapArea_template) {
//			const coord x0y0 = currentArea->x0y0;
//			const coord x1y1 = currentArea->x1y1;
//
//			for(int y = x0y0.y; y <= x1y1.y; y++) {
//				for(int x = x0y0.x; x <= x1y1.x; x++) {
//					forbiddenStairCells[x][y] = false;
//				}
//			}
//		}
//	}
//
////	const unsigned int NR_OF_SPECIAL_ROOMS = eng->specialRoomHandler->getNrOfRooms();
////	for(unsigned int i = 0; i < NR_OF_SPECIAL_ROOMS; i++) {
////		const SpecialRoom* const specialRoom = eng->specialRoomHandler->getRoomAtIndex(i);
////		if(specialRoom->isTreasureRoom() == true) {
////			const coord x0y0 = specialRoom->getX0Y0();
////			const coord x1y1 = specialRoom->getX1Y1();
////			for(int x = x0y0.x - 1; x <= x1y1.x + 1; x++) {
////				for(int y = x0y0.y - 1; y <= x1y1.y + 1; y++) {
////					forbiddenStairCells[x][y] = true;
////				}
////			}
////		}
////	}
//
//	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//	eng->mapTests->makeMoveBlockerArray(eng->player, blockers);
//
//	for(int y = 0; y < MAP_Y_CELLS; y++) {
//		for(int x = 0; x < MAP_X_CELLS; x++) {
//			if(forbiddenStairCells[x][y]) {
//				blockers[x][y] = true;
//			}
//		}
//	}
//
//	eng->basicUtils->reverseBoolArray(blockers);
//	vector<coord> freeCells;
//	eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
//
//	//Make sure the stairs are placed at least somewhat away from the player
//	const unsigned int FREE_STAIR_CELLS_DIV = 4;
//	if(freeCells.size() > FREE_STAIR_CELLS_DIV) {
//		IsCloserToOrigin isCloserToOrigin(eng->player->pos, eng);
//		sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);
//		reverse(freeCells.begin(), freeCells.end());
//		freeCells.resize(freeCells.size() / FREE_STAIR_CELLS_DIV);
//	}
//
//	const int STAIR_COORD_INDEX = eng->dice(1, freeCells.size()) - 1;
//
//	const coord stairsCoord(freeCells.at(STAIR_COORD_INDEX));
//
//	Feature* f = eng->featureFactory->spawnFeatureAt(feature_stairsDown, stairsCoord);
//	f->setHasBlood(false);
//
//	eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
//	for(int y = 0; y < MAP_Y_CELLS; y++) {
//		for(int x = 0; x < MAP_X_CELLS; x++) {
//			if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
//				blockers[x][y] = false;
//			}
//		}
//	}
//
//	vector<coord> path = eng->pathfinder->findPath(eng->player->pos, blockers, stairsCoord);
//
//	const unsigned int PATH_SIZE = path.size();
//	for(unsigned int i = 0; i < PATH_SIZE; i++) {
//		if(eng->map->featuresStatic[path.at(i).x][path.at(i).y]->getId() == feature_door) {
//			dynamic_cast<Door*>(eng->map->featuresStatic[path.at(i).x][path.at(i).y])->reveal(false);
//		}
//	}
//}
//
///*
// * The parameter rectangle does not have to go up-left to bottom-right, the method adjusts the order
// */
//void MapBuild::coverAreaWithFeaturecoverAreaWithFeature(const Rect area, const Feature_t feature) {
//	const coord x0y0 = coord(min(area.x0y0.x, area.x1y1.x), min(area.x0y0.y, area.x1y1.y));
//	const coord x1y1 = coord(max(area.x0y0.x, area.x1y1.x), max(area.x0y0.y, area.x1y1.y));
//
//	for(int x = x0y0.x; x <= x1y1.x; x++) {
//		for(int y = x0y0.y; y <= x1y1.y; y++) {
//			eng->featureFactory->spawnFeatureAt(feature, coord(x, y), NULL);
//		}
//	}
//}
//

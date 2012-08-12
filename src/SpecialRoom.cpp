#include "SpecialRoom.h"

#include "Engine.h"
#include "MapArea.h"
#include "MapPatterns.h"
#include "FeatureDoor.h"
#include "FeatureFactory.h"
#include "Blood.h"
#include "Populate.h"
#include "Popup.h"
#include "MapBuild.h"
#include "ActorPlayer.h"

void RoomDescriptModuleForest::makeRoom() {
	for(int x = x0y0.x; x <= x1y1.x; x++) {
		for(int y = x0y0.y; y <= x1y1.y; y++) {

			int grass = eng->dice(1, 12);
			if(grass == 1)
				eng->featureFactory->spawnFeatureAt(feature_bush, coord(x, y));
			if(grass == 2)
				eng->featureFactory->spawnFeatureAt(feature_bushWithered, coord(x, y));
			if(grass == 3 || grass == 4)
				eng->featureFactory->spawnFeatureAt(feature_grassWithered, coord(x, y));
			if(grass >= 5)
				eng->featureFactory->spawnFeatureAt(feature_grass, coord(x, y));
		}
	}

	const int TREE_DENSITY = 45;
	const coord mid = getRoomMid();

	for(int x = x0y0.x; x <= x1y1.x; x += 2) {
		for(int y = x0y0.y; y <= x1y1.y; y += 2) {
			if(eng->dice(1, 100) <= TREE_DENSITY && x != mid.x && y != mid.y) {
				eng->featureFactory->spawnFeatureAt(feature_tree, coord(x, y));
			}
		}
	}
}

void RoomDescriptModuleStatueGarden::makeRoom() {
	for(int x = x0y0.x + 1; x <= x1y1.x - 1; x += 2) {
		for(int y = x0y0.y + 1; y <= x1y1.y - 1; y += 2) {
			eng->featureFactory->spawnFeatureAt(feature_statue, coord(x, y));
		}
	}
}

void RoomDescriptModuleGhoulStatueGarden::makeRoom() {
	for(int x = x0y0.x + 1; x <= x1y1.x - 1; x += 2) {
		for(int y = x0y0.y + 1; y <= x1y1.y - 1; y += 2) {
			eng->featureFactory->spawnFeatureAt(feature_ghoulStatue, coord(x, y));
		}
	}
}

void RoomDescriptModuleGore::makeRoom() {
	const coord mid = getRoomMid();
	eng->gore->makeBlood(mid);
	eng->gore->makeGore(mid);

	const int GORE_DENSITY = 9;

	for(int x = x0y0.x; x <= x1y1.x; x++) {
		for(int y = x0y0.y; y <= x1y1.y; y++) {

			if(eng->dice(1, 100) <= GORE_DENSITY) {
				eng->gore->makeBlood(coord(x, y));
				eng->gore->makeGore(coord(x, y));
			}
		}
	}
}

void RoomDescriptModuleDarkness::makeRoom() {
	for(int x = x0y0.x; x <= x1y1.x; x++) {
		for(int y = x0y0.y; y <= x1y1.y; y++) {
			eng->map->darkness[x][y] = true;
		}
	}
}

void RoomDescriptModuleTombs::makeRoom() {
	for(int x = x0y0.x + 1; x <= x1y1.x - 1; x += 2) {
		for(int y = x0y0.y + 1; y <= x1y1.y - 1; y += 2) {

			bool isAreaFree = true;
			for(int dy = -1; dy <= 1; dy++) {
				for(int dx = -1; dx <= 1; dx++) {
					if(eng->map->featuresStatic[x + dx][y + dy]->getId() != feature_stoneFloor) {
						isAreaFree = false;
					}
				}
			}
			if(isAreaFree) {
				eng->featureFactory->spawnFeatureAt(feature_tomb, coord(x, y));
			}
		}
	}
}

void RoomDescriptModuleSpiderLair::makeRoom() {
	const int WEB_DENSITY = 20;

	for(int x = x0y0.x; x <= x1y1.x; x++) {
		for(int y = x0y0.y; y <= x1y1.y; y++) {
			if(eng->dice(1, 100) <= WEB_DENSITY) {
				if(eng->dice(1, 100) <= 70) {
					const Feature_t featureIdHere = eng->map->featuresStatic[x][y]->getId();
					const FeatureDef* const featureDefHere = eng->featureData->getFeatureDef(featureIdHere);
					eng->featureFactory->spawnFeatureAt(feature_trap, coord(x, y), new TrapSpawnData(featureDefHere, trap_spiderWeb));
				} else {
					eng ->featureFactory->spawnFeatureAt(feature_cocoon, coord(x, y));
				}
			}
		}
	}
}

void RoomDescriptModuleHighCeil::makeRoom() {
	const int CHANCE_TO_CONTAIN_CREATURES = 50;
	if(eng->dice(1, 100) > CHANCE_TO_CONTAIN_CREATURES) {
		allowCreaturesComeDown = false;
	}
}

void RoomDescriptModuleHighCeil::newTurn() {
	if(allowCreaturesComeDown == true) {
		if(eng->dice(1, 100) <= chanceCreaturesComeDown) {
			creaturesComeDown();
			allowCreaturesComeDown = false;
		}
		chanceCreaturesComeDown += 15;
	}
}

void RoomDescriptModuleHighCeil::creaturesComeDown() {
	bool blockedCells[MAP_X_CELLS][MAP_Y_CELLS];
	eng->basicUtils->resetBoolArray(blockedCells, true);
	for(int x = x0y0.x; x <= x1y1.x; x++) {
		for(int y = x0y0.y; y <= x1y1.y; y++) {
			blockedCells[x][y] = false;
		}
	}

	eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_fly, blockedCells);
	eng->basicUtils->reverseBoolArray(blockedCells);
	vector<coord> freeCells;
	eng->mapTests->makeMapVectorFromArray(blockedCells, freeCells);

	if(eng->populate->spawnGroupOfMonstersAtFreeCells(freeCells, specialRoom_highCeil, true)) {
		eng->popup->showMessage("Something descends from the haze of the domed ceiling.");
	}
}

void RoomDescriptModule::clearGoreAt(const int X, const int Y) {
	eng->map->featuresStatic[X][Y]->clearGore();
}

void RoomDescriptModule::clearGoreAt(const coord c) {
	clearGoreAt(c.x, c.y);
}

//TODO Need more interesting patterns
void RoomDescriptModuleRitualChamber::makeRoom() {
	const coord mid = getRoomMid();
	eng->featureFactory->spawnFeatureAt(feature_altar, mid);

	eng->featureFactory->spawnFeatureAt(feature_brazierGolden, mid + coord(-2, -1));
	eng->featureFactory->spawnFeatureAt(feature_brazierGolden, mid + coord(-2, 1));
	eng->featureFactory->spawnFeatureAt(feature_brazierGolden, mid + coord(2, -1));
	eng->featureFactory->spawnFeatureAt(feature_brazierGolden, mid + coord(2, 1));

	clearGoreAt(mid.x, mid.y);
	clearGoreAt(mid.x - 2, mid.y - 1);
	clearGoreAt(mid.x - 2, mid.y + 1);
	clearGoreAt(mid.x + 2, mid.y - 1);
	clearGoreAt(mid.x + 2, mid.y + 1);
}

void RoomDescriptModulePool::makePool() {
	const Feature_t content = getPoolFeatureId();
	const coord origin = getRoomMid();
	eng->mapBuild->makePathByRandomWalk(origin.x, origin.y, 75, content, false,  false, x0y0, x1y1);

	for(int x = x0y0.x; x <= x1y1.x; x++) {
		for(int y = x0y0.y; y <= x1y1.y; y++) {
			if(eng->map->featuresStatic[x][y]->getId() == content) {
				clearGoreAt(x, y);
			}
		}
	}
}

//string RoomDescriptModuleTreasureRoom::getDescript() {
//	if(hasTraps) {
//		const int PLAYER_SPOT_TRAPS_SKILL = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_searching, true);
//		if(eng->abilityRoll->roll(PLAYER_SPOT_TRAPS_SKILL + 30) >= successSmall) {
//			return "You notice that there is something suspicious about this room.";
//		}
//	}
//
//	return "";
//}
//
//void RoomDescriptModuleTreasureRoom::makeTraps() {
//	hasTraps = true;
//
//	for(int x = x0y0.x + 1; x <= x1y1.x - 1; x++) {
//		for(int y = x0y0.y + 1; y <= x1y1.y - 1; y++) {
//			if(eng->map->items[x][y] == NULL && eng->map->featuresStatic[x][y]->canHaveStaticFeature()) {
//				if(eng->dice(1, 100) < 45) {
//					const Feature_t featureIdHere = eng->map->featuresStatic[x][y]->getId();
//					const FeatureDef* const featureDefHere = eng->featureData->getFeatureDef(featureIdHere);
//					if(eng->dice(1, 100) < 90) {
//						eng->featureFactory->spawnFeatureAt(feature_trap, coord(x, y), new TrapSpawnData(featureDefHere, trap_spear));
//					} else {
//						eng->featureFactory->spawnFeatureAt(feature_trap, coord(x, y), new TrapSpawnData(featureDefHere, trap_blinding));
//					}
//				}
//			}
//		}
//	}
//}
//
//bool RoomDescriptModuleTreasureRoom::isRoomDataAcceptedByModule() {
//	//This room is only allowed if all entrances are covered by doors.
//	//This function also stores a list of the doors on the room border.
//	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//	eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, blockers);
//	for(int x = x0y0.x - 1; x <= x1y1.x + 1; x++) {
//		for(int y = x0y0.y - 1; y <= x1y1.y + 1; y++) {
//			if(x == x0y0.x - 1 || x == x1y1.x + 1 || y == x0y0.y - 1 || y == x1y1.y + 1) {
//				Feature* f = eng->map->featuresStatic[x][y];
//				if(f->getId() == feature_door) {
//					doorsToRoom.push_back(dynamic_cast<Door*> (f));
//					blockers[x][y] = true;
//				}
//			}
//		}
//	}
//	if(doorsToRoom.size() > 0) {
//		const vector<coord> path = eng->pathfinder->findPath(x0y0 + coord(1, 1), blockers, eng->player->pos);
//		if(path.size() == 0) {
//			return true;
//		}
//	}
//
//	return false;
//}
//
//void RoomDescriptModuleTreasureRoom::makeRoom() {
//	treasureRoomSpecificMakeRoom();
//	if(eng->dice.coinToss()) {
//		makeTraps();
//	}
//}
//
//void RoomDescriptModuleArmory::treasureRoomSpecificMakeRoom() {
//	for(unsigned int i = 0; i < doorsToRoom.size(); i++) {
//		doorsToRoom.at(i)->setToSecret();
//	}
//
//	const coord centerCoord((x1y1.x + x0y0.x) / 2, (x1y1.y + x0y0.y) / 2);
//
//	if(eng->dice(1, 2) == 1) {
//		eng->itemFactory->spawnItemOnMap(item_armorIronSuit, centerCoord);
//	} else {
//		eng->itemFactory->spawnItemOnMap(item_armorFlackJacket, centerCoord);
//	}
//
//	if(eng->dice.coinToss()) {
//		Item* item = eng->itemFactory->spawnItemOnMap(item_shotgunShell, centerCoord);
//		item->numberOfItems = eng->dice(1, 15);
//	}
//	if(eng->dice.coinToss()) {
//		eng->itemFactory->spawnItemOnMap(item_drumOfBullets, centerCoord);
//	}
//	if(eng->dice.coinToss()) {
//		eng->itemFactory->spawnItemOnMap(item_axe, centerCoord);
//	}
//}
//
//void RoomDescriptModuleLibrary::treasureRoomSpecificMakeRoom() {
//	for(unsigned int i = 0; i < doorsToRoom.size(); i++) {
//		doorsToRoom.at(i)->setToSecret();
//	}
//
//	const coord centerCoord((x1y1.x + x0y0.x) / 2, (x1y1.y + x0y0.y) / 2);
//
//	for(int i = eng->dice(1, 2) + 2; i > 0; i--) {
//		Item* item = eng->itemFactory->spawnRandomScrollOrPotion(true, true);
//		eng->itemDrop->dropItemOnMap(centerCoord, &item);
//	}
//}

void RoomDescriptModulePoolOfWater::makeRoom() {
	makePool();
}

void RoomDescriptModulePoolOfWater::newTurn() {

}

void RoomDescriptModulePoolOfMud::makeRoom() {
	makePool();
}

void RoomDescriptModulePoolOfBlood::makeRoom() {
	makePool();
}

/*
 void RoomDescriptModulePoolOfLava::makeRoom()
 {
 const LandscapeDevName_t content = getPoolLandscape();
 const coord origin = getRoomMid();
 eng->mapBuild->makePathByRandomWalk(origin.x, origin.y, 75, content, false, x0y0 + coord(2,2), x1y1-coord(2,2));

 for(int x = x0y0.x; x <= x1y1.x; x++) {
 for(int y = x0y0.y; y <= x1y1.y; y++) {
 if(*(eng->map->getLandscape(coord(x,y))) == content) {
 clearGoreAt(x, y);
 }
 }
 }
 }
 */

//void RoomDescriptModulePoolOfAcid::makeRoom() {
//	makePool();
//}

//void RoomDescriptModuleChasm::makeRoom() {
//	coord c(getRoomMid());
//	if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//		//TODO implement better pattern(s) for the chasm
//		//		vector<coord> chasmCoords = eng->mapPatterns->getAllCellsExceptEdge(Rectangle(x0y0, x1y1), 1);
//		//		for(unsigned int i = 0; i < chasmCoords.size(); i++) {
//		//			*(eng->map->getLandscape(c)) = landscapeDevName_chasm;
//		//			clearGoreAt(c);
//		//		}
//
//		const coord chasmX0Y0(x0y0.x + eng->dice(1, 2), x0y0.y + eng->dice(1, 2));
//		const coord chasmX1Y1(x1y1.x - eng->dice(1, 2), x1y1.y - eng->dice(1, 2));
//
//		for(int x = chasmX0Y0.x; x <= chasmX1Y1.x; x++) {
//			for(int y = chasmX0Y0.y; y <= chasmX1Y1.y; y++) {
//				eng->featureFactory->spawnFeatureAt(feature_chasm, coord(x, y));
//				clearGoreAt(x, y);
//			}
//		}
//	}
//}

void RoomDescriptModulePits::makeRoom() {
	const int PIT_DENSITY = 25;

	const coord mid = getRoomMid();

	for(int x = x0y0.x; x <= x1y1.x; x += 2) {
		for(int y = x0y0.y; y <= x1y1.y; y += 2) {
			if(eng->dice(1, 100) <= PIT_DENSITY && x != mid.x && y != mid.y) {
				clearGoreAt(x, y);
				eng->featureFactory->spawnFeatureAt(feature_pit, coord(x, y));
			}
		}
	}
}

void RoomDescriptModulePits::newTurn() {

}

void RoomDescriptModuleTemple::makeRoom() {

}

void SpecialRoomHandler::makeModuleDataList() {
	//FOREST MODULE
	moduleData.push_back(new RoomDescriptModuleForest);
	addAssocToBack(specialRoom_forest, 100, forceDescriptPos_first);
	addAssocToBack(specialRoom_statueGarden, 25);
	addAssocToBack(specialRoom_ghoulStatueGarden, 25);

	//STATUE GARDEN MODULE
	moduleData.push_back(new RoomDescriptModuleStatueGarden);
	addAssocToBack(specialRoom_statueGarden, 100, forceDescriptPos_first);
	addAssocToBack(specialRoom_forest, 25, forceDescriptPos_first);

	//GHOUL STATUE GARDEN MODULE
	moduleData.push_back(new RoomDescriptModuleGhoulStatueGarden);
	addAssocToBack(specialRoom_ghoulStatueGarden, 100, forceDescriptPos_first);

	//GORE MODULE
	moduleData.push_back(new RoomDescriptModuleGore);
	addAssocToBack(specialRoom_monster, 100);
	addAssocToBack(specialRoom_ritualChamber, 60);
	addAssocToBack(specialRoom_poolOfBlood, 80);
//	addAssocToBack(specialRoom_chasm, 20);
	addAssocToBack(specialRoom_highCeil, 20);
	addAssocToBack(specialRoom_pits, 30);

	//DARKNESS MODULE
	moduleData.push_back(new RoomDescriptModuleDarkness);
	addAssocToBack(specialRoom_monster, 75);
	addAssocToBack(specialRoom_statueGarden, 20);
	addAssocToBack(specialRoom_ghoulStatueGarden, 80);
	addAssocToBack(specialRoom_forest, 30);
	addAssocToBack(specialRoom_spiderLair, 75);
	addAssocToBack(specialRoom_tombs, 70);
	addAssocToBack(specialRoom_pits, 75);
	addAssocToBack(specialRoom_shallowMud, 20);
	addAssocToBack(specialRoom_shallowWater, 20);

	//SPIDER LAIR MODULE
	moduleData.push_back(new RoomDescriptModuleSpiderLair);
	addAssocToBack(specialRoom_spiderLair, 100, forceDescriptPos_first);

	//TOMBS MODULE
	moduleData.push_back(new RoomDescriptModuleTombs);
	addAssocToBack(specialRoom_tombs, 100, forceDescriptPos_first);

	//HIGH CEILING MODULE
	moduleData.push_back(new RoomDescriptModuleHighCeil);
	addAssocToBack(specialRoom_highCeil, 100);

	//RITUAL CHAMBER MODULE
	moduleData.push_back(new RoomDescriptModuleRitualChamber);
	addAssocToBack(specialRoom_ritualChamber, 100);

	//POOL OF BLOOD MODULE
	moduleData.push_back(new RoomDescriptModulePoolOfBlood);
	addAssocToBack(specialRoom_poolOfBlood, 100);

	//POOL OF ACID MODULE
	//	moduleData.push_back(new RoomDescriptModulePoolOfAcid);
	//	addAssocToBack(specialRoom_poolOfAcid, 100);

	//POOL OF LAVA MODULE
	/*
	 moduleData.push_back(new RoomDescriptModulePoolOfLava);
	 addAssocToBack(specialRoom_poolOfLava, 100);
	 */

	//POOL OF WATER MODULE
	moduleData.push_back(new RoomDescriptModulePoolOfWater);
	addAssocToBack(specialRoom_shallowWater, 100);

	//POOL OF MUD MODULE
	moduleData.push_back(new RoomDescriptModulePoolOfMud);
	addAssocToBack(specialRoom_shallowMud, 100);

	//CHASM MODULE
//	moduleData.push_back(new RoomDescriptModuleChasm);
//	addAssocToBack(specialRoom_chasm, 100);
//	addAssocToBack(specialRoom_highCeil, 40);

	//PITS MODULE
	moduleData.push_back(new RoomDescriptModulePits);
	addAssocToBack(specialRoom_pits, 100);

	//ARMORY MODULE
//	moduleData.push_back(new RoomDescriptModuleArmory);
//	addAssocToBack(specialRoom_armory, 100);

	//LIBRARY MODULE
//	moduleData.push_back(new RoomDescriptModuleLibrary);
//	addAssocToBack(specialRoom_library, 100);

	//FLOOR PATTERN MODULE
	//moduleData.push_back(new RoomDescriptModuleFloorPattern);

	//TEMPLE MODULE
	/*moduleData.push_back(new RoomDescriptModuleTemple);
	 addAssocToBack(specialRoom_temple, 100);*/
}

void SpecialRoomHandler::makeSpecialRooms(vector<MapArea*>& mapAreas) {

	const int MAX_SPECIAL_ROOMS = eng->dice(1, 3);

	if(MAX_SPECIAL_ROOMS > 0) {
		int specialRoomsCreated = 0;
		for(unsigned int i = 0; i < mapAreas.size(); i++) {
			if(mapAreas.at(i)->m_areaType == mapArea_room && mapAreas.at(i)->isSpecialRoomAllowed) {

				const coord x0y0 = mapAreas.at(i)->x0y0;
				const coord x1y1 = mapAreas.at(i)->x1y1;

				if(eng->mapTests->isCellInside(eng->player->pos, x0y0, x1y1) == false) {

					const int W = x1y1.x - x0y0.x + 1;
					const int H = x1y1.y - x0y0.y + 1;

					if(W >= 6 && H >= 4) {
						const int CHANCE_FOR_SPECIAL_ROOM = 100;
						if(eng->dice(1, 100) <= CHANCE_FOR_SPECIAL_ROOM) {
							//Choose a room type randomly
							const SpecialRoom_t roomType = static_cast<SpecialRoom_t>(eng->dice(1, endOfSpecialRooms) - 1);

							if(attemptMakeSpecialRoom(roomType, *(mapAreas.at(i)))) {
								specialRoomsCreated++;
							}
						}
					}
				}
			}
			if(specialRoomsCreated == MAX_SPECIAL_ROOMS) {
				i = 99999;
			}
		}
	}
}

bool SpecialRoomHandler::attemptMakeSpecialRoom(const SpecialRoom_t roomType, MapArea& mapArea) {
	SpecialRoom* specialRoom = new SpecialRoom(mapArea.x0y0, mapArea.x1y1, roomType, eng);
	addModulesToRoom(specialRoom, mapArea.x0y0, mapArea.x1y1);
	if(specialRoom->getNrOfModulesAdded() > 0) {
		specialRoom->makeRoom();
		specialRoom->spawnRoomMonsters();
		roomsActive.push_back(specialRoom);
		mapArea.isSpecialRoomAllowed = false;
		return true;
	}
	delete specialRoom;
	return false;
}


void SpecialRoomHandler::addModulesToRoom(SpecialRoom* const room, const coord roomX0Y0, const coord roomX1Y1) {
	//Look through a list of module data and build a candidate list for additions to the room.
	vector<RoomDescriptModule*> moduleCandidates;
	for(unsigned int modules_i = 0; modules_i < moduleData.size(); modules_i++) {
		if(moduleData.at(modules_i)->isAllowedInRoom(room->getRoomType()) == true) {
			moduleCandidates.push_back(moduleData.at(modules_i));
		}
	}

	if(moduleCandidates.size() != 0) {
		for(unsigned int modules_i = 0; modules_i < moduleCandidates.size(); modules_i++) {
			const RoomAndDescriptAssoc assoc = moduleCandidates.at(modules_i)->getRoomAssoc(room->getRoomType());
			if(eng->dice(1, 100) <= assoc.chanceToOccur) {
				RoomDescriptModule* moduleCandidate = moduleCandidates.at(modules_i)->copy();
				moduleCandidate->initRoomData(roomX0Y0, roomX1Y1, eng);
				if(moduleCandidate->isRoomDataAcceptedByModule(Rectangle(roomX0Y0, roomX1Y1))) {
					room->addModule(moduleCandidate);
				} else {
					delete moduleCandidate;
				}
			}
		}
	}
}

void SpecialRoomHandler::deleteAllSpecialRooms() {
	for(unsigned int i = 0; i < roomsActive.size(); i++) {
		delete roomsActive.at(i);
	}
	roomsActive.resize(0);
}

SpecialRoomHandler::~SpecialRoomHandler() {
	for(unsigned int i = 0; i < roomsActive.size(); i++) {
		delete roomsActive.at(i);
	}
	for(unsigned int i = 0; i < moduleData.size(); i++) {
		delete moduleData.at(i);
	}
}

void SpecialRoomHandler::runRoomEvents() {
	for(unsigned int i = 0; i < roomsActive.size(); i++) {
		roomsActive.at(i)->attemptRunEvents();
	}
}

void SpecialRoom::attemptRunEvents() {
	if(isCellInside(eng->player->pos) == true) {

		//Description
		if(hasShownDescript == false) {
			string line = "";
			for(unsigned int i = 0; i < modules.size(); i++) {
				const string moduleDescript = modules.at(i)->getDescript();
				if(moduleDescript != "") {
					//Add description lines.
					line += moduleDescript + " ";
				}
			}
			if(line != "") {
				eng->popup->showMessage(line);
			}
			hasShownDescript = true;
		}

		//New turn
		for(unsigned int i = 0; i < modules.size(); i++) {
			modules.at(i)->newTurn();
		}
	}
}

void SpecialRoom::spawnRoomMonsters() {
	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			if(eng->mapTests->isCellInside(coord(x,y), x0y0, x1y1) == false) {
				blockers[x][y] = true;
			}
		}
	}

	eng->basicUtils->reverseBoolArray(blockers);
	vector<coord> freeCells;
	eng->mapTests->makeMapVectorFromArray(blockers, freeCells);

	for(int i = eng->dice(1, 4); i > 0; i--) {
		eng->populate->spawnGroupOfMonstersAtFreeCells(freeCells, roomType, false);
	}
}

#include "MapBuildBSP.h"

#include "algorithm"
#include <stdlib.h>
#include <cassert>

#include "Engine.h"
#include "MapBuild.h"
#include "FeatureProxEvent.h"
#include "ActorPlayer.h"
#include "FeatureDoor.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "ItemDrop.h"
#include "Pathfinding.h"
#include "ItemFactory.h"
#include "Map.h"
#include "FeatureWall.h"

//======================================================================== MAPBUILD-BSP
void MapBuildBSP::run() {
  tracer << "MapBuildBSP::run()..." << endl;

  tracer << "MapBuildBSP: Calling map->clearDungeon()" << endl;
  eng->map->clearDungeon();

  tracer << "MapBuildBSP: Deleting rooms" << endl;
  for(unsigned int i = 0; i < rooms_.size(); i++) {
    delete rooms_.at(i);
  }
  rooms_.resize(0);

  tracer << "MapBuildBSP: Setting all cells to stone walls and resetting helper arrays" << endl;
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      roomCells[x][y] = false;
      eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(x, y));
      doorPositionCandidates[x][y] = false;
      forbiddenStairCellsGlobal[x][y] = false;
    }
  }

  const int SPLIT_X1 = MAP_X_CELLS / 3 + eng->dice.getInRange(-1, 1);
  const int SPLIT_X2 = 2 * (MAP_X_CELLS / 3) + eng->dice.getInRange(-1, 1);
  const int SPLIT_Y1 = MAP_Y_CELLS / 3;
  const int SPLIT_Y2 = 2 * (MAP_Y_CELLS / 3);

  Region* regions[3][3];

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      regions[x][y] = NULL;
      regionsToBuildCave[x][y] = false;
    }
  }

  buildMergedRegionsAndRooms(regions, SPLIT_X1, SPLIT_X2, SPLIT_Y1, SPLIT_Y2);

  const int FIRST_DUNGEON_LEVEL_WITH_CAVES = 5;
  if(eng->map->getDungeonLevel() >= FIRST_DUNGEON_LEVEL_WITH_CAVES) {
    const int CHANCE_FOR_CAVE_AREA = eng->map->getDungeonLevel() * 15;
    if(eng->dice(1, 100) < CHANCE_FOR_CAVE_AREA) {
      const bool IS_TWO_CAVES = eng->dice(1, 100) < CHANCE_FOR_CAVE_AREA / 3;
      for(int nrCaves = IS_TWO_CAVES ? 2 : 1; nrCaves > 0; nrCaves--) {
        int nrTriesToMark = 1000;
        while(nrTriesToMark > 0) {
          coord c(eng->dice.getInRange(0, 2), eng->dice.getInRange(0, 2));
          if(regions[c.x][c.y] == NULL && regionsToBuildCave[c.x][c.y] == false) {
            regionsToBuildCave[c.x][c.y] = true;
            nrTriesToMark = 0;
          }
          nrTriesToMark--;
        }
      }
    }
  }

  tracer << "MapBuildBSP: Making rooms" << endl;
  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      if(regions[x][y] == NULL) {
        const int X0 = x == 0 ? 0 : x == 1 ? SPLIT_X1 : SPLIT_X2;
        const int Y0 = y == 0 ? 0 : y == 1 ? SPLIT_Y1 : SPLIT_Y2;
        const int X1 = x == 0 ? SPLIT_X1 - 1 : x == 1 ? SPLIT_X2 - 1 : MAP_X_CELLS - 1;
        const int Y1 = y == 0 ? SPLIT_Y1 - 1 : y == 1 ? SPLIT_Y2 - 1 : MAP_Y_CELLS - 1;
        Region* region = new Region(coord(X0, Y0), coord(X1, Y1));
        regions[x][y] = region;
        const Rect roomCoords = region->getRandomCoordsForRoom(eng);

        rooms_.push_back(buildRoom(roomCoords));
        regions[x][y]->mainRoom = rooms_.back();

        if(eng->dice(1, 100) < 30) {
          reshapeRoom(*(regions[x][y]->mainRoom));
        }
      }
    }
  }

  connectRegions(regions);

  buildAuxRooms(regions);

  buildCaves(regions);

  buildRoomsInRooms();

  tracer << "MapBuildBSP: Placing doors" << endl;
  const int CHANCE_TO_PLACE_DOOR = 70;
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(doorPositionCandidates[x][y] == true) {
        if(eng->dice(1, 100) < CHANCE_TO_PLACE_DOOR) {
          placeDoorAtPosIfSuitable(coord(x, y));
        }
      }
    }
  }

  tracer << "MapBuildBSP: Moving player to nearest floor cell" << endl;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeWalkBlockingArrayFeaturesOnly(blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
  sort(freeCells.begin(), freeCells.end(), IsCloserToOrigin(eng->player->pos, eng));
  eng->player->pos = freeCells.front();


  tracer << "MapBuildBSP: Calling RoomThemeMaker::run()" << endl;
  eng->roomThemeMaker->run(rooms_);

  const coord stairsCoord = placeStairs();

  const int LAST_LEVEL_TO_REVEAL_STAIRS_PATH = 5;
  if(eng->map->getDungeonLevel() <= LAST_LEVEL_TO_REVEAL_STAIRS_PATH) {
    revealAllDoorsBetweenPlayerAndStairs(stairsCoord);
  }

  makeLevers();

  // Note: This must be run last, everything else depends on all walls being common stone walls
  decorateWalls();

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      delete regions[x][y];
      regions[x][y] = NULL;
    }
  }

  tracer << "MapBuildBSP::run() [DONE]" << endl;
}

void MapBuildBSP::deleteAndRemoveRoomFromList(Room* const room) {
  for(unsigned int i = 0; i < rooms_.size(); i++) {
    if(rooms_.at(i) == room) {
      delete room;
      rooms_.erase(rooms_.begin() + i);
      return;
    }
  }
  tracer << "[WARNING] Tried to remove room that is not in list, in MapBuildBSP::deleteAndRemoveRoomFromList()" << endl;
}

void MapBuildBSP::makeLevers() {
  tracer << "MapBuildBSP::makeLeverPuzzle()..." << endl;

  tracer << "MapBuildBSP: Picking a random door" << endl;
  vector<Door*> doorCandidates;
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      Feature* const feature = eng->map->featuresStatic[x][y];
      if(feature->getId() == feature_door) {
        Door* const door = dynamic_cast<Door*>(feature);
        doorCandidates.push_back(door);
      }
    }
  }
  Door* const doorToLink = doorCandidates.at(eng->dice.getInRange(0, doorCandidates.size() - 1));

  tracer << "MapBuildBSP: Making floodfill and picking pos with lower value than the door" << endl;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, blockers);
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      Feature* const feature = eng->map->featuresStatic[x][y];
      if(feature->getId() == feature_door) {
        blockers[x][y] = false;
      }
    }
  }
  int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeFloodFill(eng->player->pos, blockers, floodFill, 99999, coord(-1, -1));
  const int FLOOD_VALUE_AT_DOOR = floodFill[doorToLink->pos_.x][doorToLink->pos_.y];
  vector<coord> leverPosCandidates;
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(floodFill[x][y] < FLOOD_VALUE_AT_DOOR) {
        if(eng->map->featuresStatic[x][y]->canHaveStaticFeature()) {
          leverPosCandidates.push_back(coord(x, y));
        }
      }
    }
  }

  if(leverPosCandidates.size() > 0) {
    const int ELEMENT = eng->dice.getInRange(0, leverPosCandidates.size() - 1);
    const coord leverPos(leverPosCandidates.at(ELEMENT));
    spawnLeverAdaptAndLinkDoor(leverPos, *doorToLink);
  } else {
    tracer << "[WARNING] Could not find position to place lever, in MapBuildBSP::makeLeverPuzzle()" << endl;
  }
  tracer << "MapBuildBSP::makeLeverPuzzle() [DONE]" << endl;
}

void MapBuildBSP::spawnLeverAdaptAndLinkDoor(const coord& leverPos, Door& door) {
  tracer << "MapBuildBSP: Spawning lever and linking it to the door" << endl;
  eng->featureFactory->spawnFeatureAt(feature_lever, leverPos, new LeverSpawnData(&door));

  tracer << "MapBuildBSP: Changing door properties" << endl;
  door.material_ = doorMaterial_metal;
  door.isOpen_ = false;
  door.isStuck_ = false;
  door.isOpenedAndClosedExternally_ = true;
}

void MapBuildBSP::buildCaves(Region* regions[3][3]) {
  tracer << "MapBuildBSP::buildCaves()..." << endl;
  for(int regY = 0; regY <= 2; regY++) {
    for(int regX = 0; regX <= 2; regX++) {

      if(regionsToBuildCave[regX][regY]) {

        Region* const region = regions[regX][regY];

        // This region no longer has a room, delete it from list
        deleteAndRemoveRoomFromList(region->mainRoom);
        region->mainRoom = NULL;

        bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

        for(int y = 0; y < MAP_Y_CELLS; y++) {
          for(int x = 0; x < MAP_X_CELLS; x++) {

            blockers[x][y] = false;

            if(x == 0 || y == 0 || x == MAP_X_CELLS - 1 || y == MAP_Y_CELLS - 1) {
              blockers[x][y] = true;
            } else {
              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  const Feature_t featureId = eng->map->featuresStatic[x + dx][y + dy]->getId();
                  const bool IS_FLOOR = featureId == feature_stoneFloor || featureId == feature_caveFloor;
                  if(IS_FLOOR && eng->mapTests->isCellInside(coord(x + dx, y + dy), region->getRegionCoords()) == false) {
                    blockers[x][y] = true;
                  }
                }
              }
            }
          }
        }

        const coord origin(region->getX0Y0() + coord(1, 1));
        int floodFillResult[MAP_X_CELLS][MAP_Y_CELLS];

        const int FLOOD_FILL_TRAVEL_LIMIT = 20;

        eng->mapTests->makeFloodFill(origin, blockers, floodFillResult, FLOOD_FILL_TRAVEL_LIMIT, coord(-1, -1));

        for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
          for(int x = 1; x < MAP_X_CELLS - 1; x++) {
            const coord c(x, y);
            if(c == origin || floodFillResult[x][y] > 0) {

              eng->featureFactory->spawnFeatureAt(feature_caveFloor, c);

              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  if(eng->map->featuresStatic[x + dx][y + dy]->getId() == feature_stoneWall) {
                    eng->featureFactory->spawnFeatureAt(feature_stoneWall, c + coord(dx , dy));
                    Wall* const wall = dynamic_cast<Wall*>(eng->map->featuresStatic[x + dx][y + dy]);
                    wall->wallType = wall_cave;
                    wall->setRandomIsSlimy();
                  }
                }
              }
            }
          }
        }

        const int CHANCE_TO_MAKE_CHASM = 25;

        if(eng->dice(1, 100) < CHANCE_TO_MAKE_CHASM) {
          eng->basicUtils->resetBoolArray(blockers, false);

          for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
            for(int x = 1; x < MAP_X_CELLS - 1; x++) {
              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  if(eng->map->featuresStatic[x + dx][y + dy]->getId() == feature_stoneWall) {
                    blockers[x][y] = blockers[x + dx][y + dy] = true;
                  }
                }
              }
            }
          }

          eng->mapTests->makeFloodFill(origin, blockers, floodFillResult, FLOOD_FILL_TRAVEL_LIMIT / 2, coord(-1, -1));

          for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
            for(int x = 1; x < MAP_X_CELLS - 1; x++) {
              const coord c(x, y);
              if(blockers[x][y] == false && (c == origin || floodFillResult[x][y] > 0)) {
                eng->featureFactory->spawnFeatureAt(feature_chasm, c);
              }
            }
          }
        }
      }
    }
  }
  tracer << "MapBuildBSP::buildCaves()[DONE]" << endl;
}

void MapBuildBSP::buildMergedRegionsAndRooms(Region* regions[3][3], const int SPLIT_X1, const int SPLIT_X2, const int SPLIT_Y1, const int SPLIT_Y2) {
  const int NR_OF_MERGED_REGIONS_TO_ATTEMPT = eng->dice.getInRange(0, 2);

  for(int attemptCount = 0; attemptCount < NR_OF_MERGED_REGIONS_TO_ATTEMPT; attemptCount++) {
    coord regIndex1(2, 2);
    while(regIndex1 == coord(2, 2)) {
      regIndex1 = coord(eng->dice.getInRange(0, 2), eng->dice.getInRange(0, 2));
    }

    coord regIndex2(regIndex1);
    int nrTriesToFindRegions = 100;
    bool isGoodRegionsFound = false;
    while(isGoodRegionsFound == false) {
      nrTriesToFindRegions--;
      if(nrTriesToFindRegions <= 0) {
        return;
      }
      regIndex2 = regIndex1 + (eng->dice.coinToss() ? coord(1, 0) : coord(0, 1));
      const bool IS_REGIONS_PLACED_OK = regIndex2.x <= 2 && regIndex2.y <= 2 && (regIndex2 != regIndex1);
      if(IS_REGIONS_PLACED_OK) {
        isGoodRegionsFound = regions[regIndex1.x][regIndex1.y] == NULL && regions[regIndex2.x][regIndex2.y] == NULL;
      }
    }

    const int MERGED_X0 = regIndex1.x == 0 ? 0 : regIndex1.x == 1 ? SPLIT_X1 : SPLIT_X2;
    const int MERGED_Y0 = regIndex1.y == 0 ? 0 : regIndex1.y == 1 ? SPLIT_Y1 : SPLIT_Y2;
    const int MERGED_X1 = regIndex2.x == 0 ? SPLIT_X1 - 1 : regIndex2.x == 1 ? SPLIT_X2 - 1 : MAP_X_CELLS - 1;
    const int MERGED_Y1 = regIndex2.y == 0 ? SPLIT_Y1 - 1 : regIndex2.y == 1 ? SPLIT_Y2 - 1 : MAP_Y_CELLS - 1;

    const int AREA_2_X0 = regIndex2.x == 0 ? 0 : regIndex2.x == 1 ? SPLIT_X1 : SPLIT_X2;
    const int AREA_2_Y0 = regIndex2.y == 0 ? 0 : regIndex2.y == 1 ? SPLIT_Y1 : SPLIT_Y2;
    const int AREA_2_X1 = MERGED_X1;
    const int AREA_2_Y1 = MERGED_Y1;

    const bool AREA_2_IS_BELOW = regIndex2.y > regIndex1.y;

    const int AREA_1_X0 = MERGED_X0;
    const int AREA_1_X1 = AREA_2_IS_BELOW ? MERGED_X1 - 1 : AREA_2_X0 - 1;
    const int AREA_1_Y0 = MERGED_Y0;
    const int AREA_1_Y1 = AREA_2_IS_BELOW ? AREA_2_Y0 - 1 : MERGED_Y1;

    const Rect area1(coord(AREA_1_X0, AREA_1_Y0), coord(AREA_1_X1, AREA_1_Y1));
    const Rect area2(coord(AREA_2_X0, AREA_2_Y0), coord(AREA_2_X1, AREA_2_Y1));

    Region* region1 = new Region(area1.x0y0, area1.x1y1);
    Region* region2 = new Region(area2.x0y0, area2.x1y1);
    regions[regIndex1.x][regIndex1.y] = region1;
    regions[regIndex2.x][regIndex2.y] = region2;

    Rect roomCoords(area1.x0y0 + coord(2, 2), area2.x1y1 - coord(2, 2));
    Room* const room = buildRoom(roomCoords);
    rooms_.push_back(room);

    region1->mainRoom = region2->mainRoom = room;

    region1->regionsConnectedTo[regIndex2.x][regIndex2.y] = true;
    region2->regionsConnectedTo[regIndex1.x][regIndex1.y] = true;

    if(eng->dice(1, 100) < 33) {
      reshapeRoom(*room);
    }
  }
}

void MapBuildBSP::buildRoomsInRooms() {

  const int NR_OF_TRIES = 40;
  const int MAX_NR_INNER_ROOMS = 7;
  const int MIN_DIM_W = 4;
  const int MIN_DIM_H = 4;

  for(unsigned int i = 0; i < rooms_.size(); i++) {

    const coord roomX0Y0 = rooms_.at(i)->getX0Y0();
    const coord roomX1Y1 = rooms_.at(i)->getX1Y1();

    const int ROOM_WI = roomX1Y1.x - roomX0Y0.x + 1;
    const int ROOM_HE = roomX1Y1.y - roomX0Y0.y + 1;

    const bool IS_ROOM_BIG = ROOM_WI > 16 || ROOM_HE > 8;

    if(IS_ROOM_BIG || eng->dice(1, 100) < 30) {
      const int MAX_DIM_W = min(16, ROOM_WI);
      const int MAX_DIM_H = min(16, ROOM_HE);

      if(MAX_DIM_W >= MIN_DIM_W && MAX_DIM_H >= MIN_DIM_H) {

        for(int nrRoomsCount = 0; nrRoomsCount < MAX_NR_INNER_ROOMS; nrRoomsCount++) {
          for(int tryCount = 0; tryCount < NR_OF_TRIES; tryCount++) {

            const int W = eng->dice.getInRange(MIN_DIM_W, MAX_DIM_W);
            const int H = eng->dice.getInRange(MIN_DIM_H, MAX_DIM_H);

            const int X0 = eng->dice.getInRange(roomX0Y0.x - 1, roomX1Y1.x - W + 2);
            const int Y0 = eng->dice.getInRange(roomX0Y0.y - 1, roomX1Y1.y - H + 2);
            const int X1 = X0 + W - 1;
            const int Y1 = Y0 + H - 1;

            bool isSpaceFree = true;

            for(int y = Y0 - 1; y <= Y1 + 1; y++) {
              for(int x = X0 - 1; x <= X1 + 1; x++) {
                if(eng->mapTests->isCellInside(coord(x, y), Rect(roomX0Y0 - coord(1, 1), roomX1Y1 + coord(1, 1)))) {
                  if(x == roomX0Y0.x - 1 || x == roomX1Y1.x + 1 || y == roomX0Y0.y - 1 || y == roomX1Y1.y + 1) {
                    if(eng->map->featuresStatic[x][y]->getId() != feature_stoneWall) {
                      isSpaceFree = false;
                    }
                  } else {
                    if(eng->map->featuresStatic[x][y]->getId() != feature_stoneFloor) {
                      isSpaceFree = false;
                    }
                  }
                }
              }
            }

            if((X0 == roomX0Y0.x && X1 == roomX1Y1.x) || (Y0 == roomX0Y0.y && Y1 == roomX1Y1.y)) {
              isSpaceFree = false;
            }

            if(isSpaceFree) {
              vector<coord> doorCandidates;
              rooms_.push_back(new Room(Rect(coord(X0 + 1, Y0 + 1), coord(X1 - 1, Y1 - 1))));
              for(int y = Y0; y <= Y1; y++) {
                for(int x = X0; x <= X1; x++) {
                  if(x == X0 || x == X1 || y == Y0 || y == Y1) {

                    eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(x, y));

                    if(x != roomX0Y0.x - 1 && x != roomX0Y0.x && x != roomX1Y1.x && x != roomX1Y1.x + 1 &&
                        y != roomX0Y0.y - 1 && y != roomX0Y0.y && y != roomX1Y1.y && y != roomX1Y1.y + 1) {
                      if((x != X0 && x != X1) || (y != Y0 && y != Y1)) {
                        doorCandidates.push_back(coord(x, y));
                      }
                    }
                  }
                }
              }
              if(eng->dice.coinToss() || doorCandidates.size() <= 2) {
                const int DOOR_POS_ELEMENT = eng->dice.getInRange(0, doorCandidates.size() - 1);
                const coord doorPos = doorCandidates.at(DOOR_POS_ELEMENT);
                if(eng->dice.coinToss()) {
                  eng->featureFactory->spawnFeatureAt(feature_door, doorPos, new DoorSpawnData(eng->featureData->getFeatureDef(feature_stoneWall)));
                } else {
                  eng->featureFactory->spawnFeatureAt(feature_stoneFloor, doorPos);
                }
              } else {
                vector<coord> positionsWithDoor;
                const int NR_TRIES = eng->dice.getInRange(1, 10);
                for(int j = 0; j < NR_TRIES; j++) {
                  const int DOOR_POS_ELEMENT = eng->dice.getInRange(0, doorCandidates.size() - 1);
                  const coord doorPos = doorCandidates.at(DOOR_POS_ELEMENT);

                  bool positionOk = true;
                  for(unsigned int n = 0; n < positionsWithDoor.size(); n++) {
                    if(eng->mapTests->isCellsNeighbours(doorPos, positionsWithDoor.at(n), false)) {
                      positionOk = false;
                    }
                  }
                  if(positionOk) {
                    eng->featureFactory->spawnFeatureAt(feature_stoneFloor, doorPos);
                    positionsWithDoor.push_back(doorPos);
                  }
                }
              }
              tryCount = 99999;
            }
          }
        }
      }
    }
  }
}

//void MapBuildBSP::buildNaturalArea(Region* regions[3][3]) {
//	const coord origin(1,1);
//	for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
//		for(int x = 1; x < MAP_X_CELLS - 1; x++) {
//			if(eng->basicUtils->pointDistance(origin.x, origin.y, x, y) < 20) {
//				eng->featureFactory->spawnFeatureAt(feature_deepWater, coord(x,y));
//				for(int yRegion = 0; yRegion < 3; yRegion++) {
//					for(int xRegion = 0; xRegion < 3; xRegion++) {
//						Region* region = regions[xRegion][yRegion];
//						if(eng->mapTests->isCellInside(coord(x,y), region->getX0Y0(), region->getX1Y1())) {
//							region->mapArea.isSpecialRoomAllowed = false;
//						}
//					}
//				}
//			}
//		}
//	}
//	makeRiver(regions);
//}

//void MapBuildBSP::makeRiver(Region* regions[3][3]) {
//	(void)regions;
//
//	const int WIDTH = eng->dice.getInRange(4, 12);
//	const int START_X_OFFSET_MAX = 5;
//	const int X_POS_START = MAP_X_CELLS/2 + eng->dice.getInRange(-START_X_OFFSET_MAX, START_X_OFFSET_MAX);
//
//	coord leftCoord(X_POS_START, 0);
//	while(eng->mapTests->isCellInsideMainScreen(leftCoord) && eng->mapTests->isCellInsideMainScreen(leftCoord + coord(WIDTH,0))) {
//		coverAreaWithFeature(Rect(leftCoord, leftCoord + coord(WIDTH, 0)), feature_deepWater);
//		leftCoord += coord(eng->dice.getInRange(-1,1), 1);
//	}
//}

coord MapBuildBSP::placeStairs() {
  tracer << "MapBuildBSP::placeStairs()..." << endl;
  bool forbiddenStairCells[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(forbiddenStairCells, true);

  tracer << "MapBuildBSP: Setting local forbiddenStairCells from global" << endl;
  for(unsigned int i = 0; i < rooms_.size(); i++) {
    const Room* const room = rooms_.at(i);
    for(int y = room->getY0(); y <= room->getY1(); y++) {
      for(int x = room->getX0(); x <= room->getX1(); x++) {
        if(forbiddenStairCellsGlobal[x][y] == false) {
          forbiddenStairCells[x][y] = false;
        }
      }
    }
  }

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  tracer << "MapBuildBSP: Calling MapTests::makeMoveBlockerArrayFeaturesOnly()" << endl;
  eng->mapTests->makeMoveBlockerArrayFeaturesOnly(eng->player, blockers);

  tracer << "MapBuildBSP: Setting all door cells to non-blocking" << endl;
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
        blockers[x][y] = false;
      }
    }
  }

  int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
  tracer << "MapBuildBSP: Calling MapTests::makeFloodFill()" << endl;
  eng->mapTests->makeFloodFill(eng->player->pos, blockers, floodFill, 99999, coord(-1, -1));

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(forbiddenStairCells[x][y] == true || floodFill[x][y] == 0) {
        blockers[x][y] = true;
      }
    }
  }

  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);

  tracer << "MapBuildBSP: Sorting the free cells vector (size:" << freeCells.size() << "), and removing the furthest cells" << endl;
  const unsigned int FREE_STAIR_CELLS_DIV = 4;
  if(freeCells.size() > FREE_STAIR_CELLS_DIV) {
    IsCloserToOrigin isCloserToOrigin(eng->player->pos, eng);
    sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);
    reverse(freeCells.begin(), freeCells.end());
    freeCells.resize(freeCells.size() / FREE_STAIR_CELLS_DIV);
  }

  tracer << "MapBuildBSP: Picking the furthest cell from the remaining(size:" << freeCells.size() <<  ")" << endl;
  const int STAIR_COORD_INDEX = eng->dice(1, freeCells.size()) - 1;
  const coord stairsCoord(freeCells.at(STAIR_COORD_INDEX));

  tracer << "MapBuildBSP: Spawning down stairs at chosen coord" << endl;
  Feature* f = eng->featureFactory->spawnFeatureAt(feature_stairsDown, stairsCoord);
  f->setHasBlood(false);

  tracer << "MapBuildBSP::placeStairs()[DONE]" << endl;
  return stairsCoord;
}

void MapBuildBSP::revealAllDoorsBetweenPlayerAndStairs(const coord& stairsCoord) {
  tracer << "MapBuildBSP::revealAllDoorsBetweenPlayerAndStairs()..." << endl;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, blockers);

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
        blockers[x][y] = false;
      }
    }
  }

  vector<coord> path = eng->pathfinder->findPath(eng->player->pos, blockers, stairsCoord);

  tracer << "MapBuildBSP: Travelling along coord vector of size " << path.size() << " and revealing all doors" << endl;
  const unsigned int PATH_SIZE = path.size();
  for(unsigned int i = 0; i < PATH_SIZE; i++) {
    if(eng->map->featuresStatic[path.at(i).x][path.at(i).y]->getId() == feature_door) {
      dynamic_cast<Door*>(eng->map->featuresStatic[path.at(i).x][path.at(i).y])->reveal(false);
    }
  }

  tracer << "MapBuildBSP::revealAllDoorsBetweenPlayerAndStairs()[DONE]" << endl;
}

void MapBuildBSP::decorateWalls() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_stoneWall) {
        if(eng->dice(1, 100) < 10) {
          eng->featureFactory->spawnFeatureAt(feature_rubbleHigh, coord(x, y));
          continue;
        }

        Wall* const wall = dynamic_cast<Wall*>(eng->map->featuresStatic[x][y]);
        wall->setRandomIsSlimy();

        bool isNextToFloor = false;
        for(int checkY = max(0, y - 1); checkY <= min(MAP_Y_CELLS - 1, y + 1); checkY++) {
          for(int checkX = max(0, x - 1); checkX <= min(MAP_X_CELLS - 1, x + 1); checkX++) {
            if(checkX != x || checkY != y) {
              if(eng->map->featuresStatic[checkX][checkY]->getId() == feature_stoneFloor) {
                isNextToFloor = true;
                checkY = 999999;
                checkX = 999999;
              }
            }
          }
        }
        if(isNextToFloor) {
          wall->setRandomNormalWall();
        } else {
          dynamic_cast<Wall*>(eng->map->featuresStatic[x][y])->wallType = wall_cave;
        }
      }
    }
  }
}

void MapBuildBSP::connectRegions(Region* regions[3][3]) {
  tracer << "MapBuildBSP::connectRegions()..." << endl;
  const int MIN_NR_OF_CON_LIMIT = 16;
  const int MAX_NR_OF_CON_LIMIT = 22;
  const int MIN_NR_OF_CON = eng->dice.getInRange(MIN_NR_OF_CON_LIMIT, MAX_NR_OF_CON_LIMIT);
  int totalNrConnections = 0;
  bool isAllConnected = false;
  while(isAllConnected == false || totalNrConnections < MIN_NR_OF_CON) {
    totalNrConnections = getTotalNrOfConnections(regions);
    isAllConnected = isAllRoomsConnected();

    coord c1(eng->dice(1, 3) - 1, eng->dice(1, 3) - 1);
    Region* r1 = regions[c1.x][c1.y];
    c1.set(eng->dice(1, 3) - 1, eng->dice(1, 3) - 1);
    r1 = regions[c1.x][c1.y];

    coord delta(0, 0);
    bool deltaOk = false;
    while(deltaOk == false) {
      delta.set(eng->dice.getInRange(-1, 1), eng->dice.getInRange(-1, 1));
      coord c2(c1 + delta);
      const bool IS_INSIDE_BOUNDS = c2.x >= 0 && c2.y >= 0 && c2.x <= 2 && c2.y <= 2;
      const bool IS_ZERO = delta.x == 0 && delta.y == 0;
      const bool IS_DIAGONAL = delta.x != 0 && delta.y != 0;
      deltaOk = IS_ZERO == false && IS_DIAGONAL == false && IS_INSIDE_BOUNDS == true;
    }
    coord c2(c1 + delta);

    Region* const r2 = regions[c2.x][c2.y];

    if(r1->regionsConnectedTo[c2.x][c2.y] == false) {
      buildCorridorBetweenRooms(*r1, *r2);
      r1->regionsConnectedTo[c2.x][c2.y] = true;
      r2->regionsConnectedTo[c1.x][c1.y] = true;
    }
    if(totalNrConnections >= MAX_NR_OF_CON_LIMIT) {
      break;
    }
  }
  tracer << "MapBuildBSP::connectRegions()[DONE]" << endl;
}

bool MapBuildBSP::isAllRoomsConnected() {
  coord c;
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      c.set(x, y);
      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
        x = 999;
        y = 999;
      }
    }
  }

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeWalkBlockingArrayFeaturesOnly(blockers);
  int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeFloodFill(c, blockers, floodFill, 99999, coord(-1, -1));
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_stoneFloor) {
        if((x != c.x || y != c.y) && floodFill[x][y] == 0) {
          return false;
        }
      }
    }
  }
  return true;
}

int MapBuildBSP::getTotalNrOfConnections(Region* regions[3][3]) const {
  int nrOfConnections = 0;
  for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
      nrOfConnections += regions[x][y]->getNrOfConnections();
    }
  }
  return nrOfConnections;
}

bool MapBuildBSP::isRegionFoundInCardinalDirection(const coord pos, bool region[MAP_X_CELLS][MAP_Y_CELLS]) const {
  for(int dy = -1; dy <= 1; dy++) {
    for(int dx = -1; dx <= 1; dx++) {
      if(dx == 0 || dy == 0) {
        if(region[pos.x + dx][pos.y + dy]) {
          return true;
        }
      }
    }
  }
  return false;
}

void MapBuildBSP::buildCorridorBetweenRooms(const Region& region1, const Region& region2) {
  //Find all floor in both regions
  vector<coord> floorInR1Vector;
  bool floorInR1Grid[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(floorInR1Grid, false);
  for(int y = region1.mainRoom->getY0(); y <= region1.mainRoom->getY1(); y++) {
    for(int x = region1.mainRoom->getX0(); x <= region1.mainRoom->getX1(); x++) {
      const coord c = coord(x, y);
      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
        floorInR1Vector.push_back(c);
        floorInR1Grid[x][y] = true;
      }
    }
  }

  const coord regionDelta = region2.getX0Y0() - region1.getX0Y0();
  const coord regionDeltaAbs(abs(regionDelta.x), abs(regionDelta.y));
  const coord regionDeltaSign = regionDelta.getSigns();

  vector<coord> coordsInR1closeToR2;

  vector<coord> floorInR2Vector;
  bool floorInR2Grid[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(floorInR2Grid, false);
  for(int y = region2.mainRoom->getY0(); y <= region2.mainRoom->getY1(); y++) {
    for(int x = region2.mainRoom->getX0(); x <= region2.mainRoom->getX1(); x++) {
      coord c = coord(x, y);
      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
        floorInR2Vector.push_back(c);
        floorInR2Grid[x][y] = true;

        // Check if the two rooms are close by travelling to the other room in a straight line.
        bool doneTravelling = false;
        int nrOfJumps = 0;
        while(doneTravelling == false) {
          c += regionDeltaSign * -1;
          nrOfJumps++;
          if(floorInR1Grid[c.x][c.y] == true) {
            //floor -> wall -> (wall) -> floor
            if(nrOfJumps <= 3) {
              coordsInR1closeToR2.push_back(c);
            }
          }
          if(eng->mapTests->isCellInsideMainScreen(c) == false) {
            doneTravelling = true;
          }
        }
      }
    }
  }

  if(coordsInR1closeToR2.size() > 0) {
    coord c = coordsInR1closeToR2.at(eng->dice(1, coordsInR1closeToR2.size()) - 1);
    while(floorInR2Grid[c.x][c.y] == false) {
      c += regionDeltaSign;
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
      doorPositionCandidates[c.x][c.y] = true;
    }
  } else {

    /*
    * Description of the following algorithm:
    * ==============================================================================================
    * What has been done so far is to pick one coordinate in each room with floor in it (c1 and c2).
    * There is also a vector of all floor in in c1 and one of all floor in c2 (floorInR1Vector,
    * floorInR2Vector), as well as bool arrays (floorInR1Grid, floorInR2Grid).
    *
    * First, two points that has only one or two wall cells between them was attempted to be found,
    * so a short, straight connection could be made. Given that this point was reached, no two such
    * coordinates were found.
    *
    * A constant value is stored that is used heavily for corridor navigation: "regionDeltaSign".
    * It tells the direction from region 1 to region 2 (e.g. 1,0 means R2 lies to the east of R1).
    *
    * (1) One coordinate in each room is picked randomly (c1 and c2).
    *
    * (2) While still in the x0y0,x1y1-boundaries of room 1, move straight out from room 1.
    * "Straight out" means in the direction of regionDeltaSign. When we stand in the first
    * cell outside room 1, we turn that into floor, and store that as a suggestion for a door.
    * Then we move yet another step straight away from the room.
    *
    * (3) A secondary corridor is tunnelled backwards from where we stood after (2), until floor
    * is reached in room 1.
    *
    * (4) We stand once again in the cell where (2) finished. Now a new constant value is stored
    * called "deltaCurSign". It tells the direction from the coordinate we stand in at the start
    * of this phase to the coordinate we are aiming for in room 2 (c2). If regionDeltaSign is
    * subtracted from deltaCurSign, a coordinate is had which tells the direction 90 degrees from
    * the tunnel dug in (2). Example: Region 1 lies east of region 2, regionDeltaSign = (1,0).
    * c2 lies to the east and below the current coordinate, deltaCurSign = (1,1). So
    * deltaCurSign - regionDeltaSign = (1,1) - (1,0) = (0,1).
    * We now tunnel in this direction. First make floor, then move, until we hit either the
    * x- or y-axis of c2 --or-- if we stand on floor in room 2.
    *
    * (5) Now we move again in the direction of regionDeltaSign (straight east in the example).
    * First make floor, then move, until we stand on floor in room 2. Afterwards we move one
    * step backwards and suggest that for door position.
    *
    *
    * #######
    * ...33324  ######
    * ....###4  #.....
    * ......#4  #.....
    * ......#45555....
    * #######   #.....
    *           ######
    */

    // (1)
    const coord c1 = floorInR1Vector.at(eng->dice(1, floorInR1Vector.size()) - 1);
    const coord c2 = floorInR2Vector.at(eng->dice(1, floorInR2Vector.size()) - 1);

    coord c = c1;

    // (2)
    while(floorInR1Grid[c.x][c.y] == true) {
      c += regionDeltaSign;
    }
    eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
    doorPositionCandidates[c.x][c.y] = true;
    c += regionDeltaSign;

    // (3)
    coord cTemp(c - regionDeltaSign);
    while(floorInR1Grid[cTemp.x][cTemp.y] == false) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, cTemp, NULL);
      cTemp -= regionDeltaSign;
    }

    const coord deltaCur(c2 - c);
    const coord deltaCurAbs(abs(deltaCur.x), abs(deltaCur.y));
    const coord deltaCurSign(deltaCurAbs.x == 0 ? 0 : deltaCur.x / (deltaCurAbs.x), deltaCurAbs.y == 0 ? 0 : deltaCur.y / (deltaCurAbs.y));

    // (4)
    while(c.x != c2.x && c.y != c2.y && floorInR2Grid[c.x][c.y] == false) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
      c += deltaCurSign - regionDeltaSign;
    }

    // (5)
    while(c != c2 && floorInR2Grid[c.x][c.y] == false) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
      c += regionDeltaSign;
    }
    c -= regionDeltaSign;
    if(floorInR2Vector.size() > 1) {
      doorPositionCandidates[c.x][c.y] = true;
    }
  }
}

void MapBuildBSP::placeDoorAtPosIfSuitable(const coord pos) {
  for(int dx = -2; dx <= 2; dx++) {
    for(int dy = -2; dy <= 2; dy++) {
      if(dx != 0 || dy != 0) {
        if(eng->mapTests->isCellInsideMainScreen(pos + coord(dx, dy))) {
          if(eng->map->featuresStatic[pos.x + dx][pos.y + dy]->getId() == feature_door) {
            return;
          }
        }
      }
    }
  }

  bool goodVertical = true;
  bool goodHorizontal = true;

  for(int d = -1; d <= 1; d++) {
    if(eng->map->featuresStatic[pos.x + d][pos.y]->getId() == feature_stoneWall) {
      goodHorizontal = false;
    }

    if(eng->map->featuresStatic[pos.x][pos.y + d]->getId() == feature_stoneWall) {
      goodVertical = false;
    }

    if(d != 0) {
      if(eng->map->featuresStatic[pos.x][pos.y + d]->getId() != feature_stoneWall) {
        goodHorizontal = false;
      }

      if(eng->map->featuresStatic[pos.x + d][pos.y]->getId() != feature_stoneWall) {
        goodVertical = false;
      }
    }
  }

  if(goodHorizontal || goodVertical) {
    const FeatureDef* const mimicFeature = eng->featureData->getFeatureDef(feature_stoneWall);
    eng->featureFactory->spawnFeatureAt(feature_door, pos, new DoorSpawnData(mimicFeature));
  }
}

Room* MapBuildBSP::buildRoom(const Rect& roomCoords) {
  coverAreaWithFeature(roomCoords, feature_stoneFloor);
  for(int y = roomCoords.x0y0.y; y <= roomCoords.x1y1.y; y++) {
    for(int x = roomCoords.x0y0.x; x <= roomCoords.x1y1.x; x++) {
      roomCells[x][y] = true;
    }
  }
  return new Room(roomCoords);
}

//void MapBuildBSP::findEdgesOfRoom(const Rect roomCoords, vector<coord>& vectorToFill) {
//  bool coordsToAdd[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->basicUtils->resetBoolArray(coordsToAdd, false);
//
//  coord c;
//
//  //Top to bottom
//  for(c.x = roomCoords.x0y0.x; c.x <= roomCoords.x1y1.x; c.x++) {
//    for(c.y = roomCoords.x0y0.y; c.y <= roomCoords.x1y1.y; c.y++) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        coordsToAdd[c.x][c.y] = true;
//        c.y = 9999;
//      }
//    }
//  }
//  //Left to right
//  for(c.y = roomCoords.x0y0.y; c.y <= roomCoords.x1y1.y; c.y++) {
//    for(c.x = roomCoords.x0y0.x; c.x <= roomCoords.x1y1.x; c.x++) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        coordsToAdd[c.x][c.y] = true;
//        c.x = 99999;
//      }
//    }
//  }
//  //Bottom to top
//  for(c.x = roomCoords.x0y0.x; c.x <= roomCoords.x1y1.x; c.x++) {
//    for(c.y = roomCoords.x1y1.y; c.y >= roomCoords.x0y0.y; c.y--) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        coordsToAdd[c.x][c.y] = true;
//        c.y = -9999;
//      }
//    }
//  }
//  //Right to left
//  for(c.y = roomCoords.x0y0.y; c.y <= roomCoords.x1y1.y; c.y++) {
//    for(c.x = roomCoords.x1y1.x; c.x >= roomCoords.x0y0.x; c.x--) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        coordsToAdd[c.x][c.y] = true;
//        c.x = -9999;
//      }
//    }
//  }
//
//  for(c.x = roomCoords.x0y0.x; c.x <= roomCoords.x1y1.x; c.x++) {
//    for(c.y = roomCoords.x0y0.y; c.y <= roomCoords.x1y1.y; c.y++) {
//      if(coordsToAdd[c.x][c.y] == true) {
//        vectorToFill.push_back(c);
//      }
//    }
//  }
//}

// The parameter rectangle does not have to go up-left to bottom-right, the method adjusts the order
void MapBuildBSP::coverAreaWithFeature(const Rect& area, const Feature_t feature) {
  const coord x0y0 = coord(min(area.x0y0.x, area.x1y1.x), min(area.x0y0.y, area.x1y1.y));
  const coord x1y1 = coord(max(area.x0y0.x, area.x1y1.x), max(area.x0y0.y, area.x1y1.y));

  for(int x = x0y0.x; x <= x1y1.x; x++) {
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      eng->featureFactory->spawnFeatureAt(feature, coord(x, y), NULL);
    }
  }
}

void MapBuildBSP::reshapeRoom(const Room& room) {
  const int ROOM_W = room.getX1() - room.getX0() + 1;
  const int ROOM_H = room.getY1() - room.getY0() + 1;

  if(ROOM_W >= 4 && ROOM_H >= 4) {

    vector<RoomReshape_t> reshapesToPerform;
    if(eng->dice(1, 100) < 75) {
      reshapesToPerform.push_back(roomReshape_trimCorners);
    }
    if(eng->dice(1, 100) < 75) {
      reshapesToPerform.push_back(roomReshape_pillarsRandom);
    }

    for(unsigned int i = 0; i < reshapesToPerform.size(); i++) {
      switch(reshapesToPerform.at(i)) {
      case roomReshape_trimCorners: {
        const int W_DIV = 3 + (eng->dice.coinToss() ? eng->dice(1, 2) - 1 : 0);
        const int H_DIV = 3 + (eng->dice.coinToss() ? eng->dice(1, 2) - 1 : 0);

        const int W = max(1, ROOM_W / W_DIV);
        const int H = max(1, ROOM_H / H_DIV);

        const bool TRIM_ALL = false;

        if(TRIM_ALL || eng->dice.coinToss()) {
          const coord upLeft(room.getX0() + W - 1, room.getY0() + H - 1);
          MapBuildBSP::coverAreaWithFeature(Rect(room.getX0Y0(), upLeft), feature_stoneWall);
        }

        if(TRIM_ALL || eng->dice.coinToss()) {
          const coord upRight(room.getX1() - W + 1, room.getY0() + H - 1);
          MapBuildBSP::coverAreaWithFeature(Rect(coord(room.getX0() + ROOM_W - 1, room.getY0()), upRight), feature_stoneWall);
        }

        if(TRIM_ALL || eng->dice.coinToss()) {
          const coord downLeft(room.getX0() + W - 1, room.getY1() - H + 1);
          MapBuildBSP::coverAreaWithFeature(Rect(coord(room.getX0(), room.getY0() + ROOM_H - 1), downLeft), feature_stoneWall);
        }

        if(TRIM_ALL || eng->dice.coinToss()) {
          const coord downRight(room.getX1() - W + 1, room.getY1() - H + 1);
          MapBuildBSP::coverAreaWithFeature(Rect(room.getX1Y1(), downRight), feature_stoneWall);
        }
      }
      break;

      case roomReshape_pillarsRandom: {
        for(int x = room.getX0() + 1; x <= room.getX1() - 1; x++) {
          for(int y = room.getY0() + 1; y <= room.getY1() - 1; y++) {
            coord c(x + eng->dice(1, 3) - 2, y + eng->dice(1, 3) - 2);
            bool isNextToWall = false;
            for(int dxCheck = -1; dxCheck <= 1; dxCheck++) {
              for(int dyCheck = -1; dyCheck <= 1; dyCheck++) {
                if(eng->map->featuresStatic[c.x + dxCheck][c.y + dyCheck]->getId() == feature_stoneWall) {
                  isNextToWall = true;
                }
              }
            }
            if(isNextToWall == false) {
              if(eng->dice(1, 100) < 20) {
                eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(c.x, c.y));
              }
            }
          }
        }
      }
      break;
      }
    }
  }
}

int MapBuildBSP::getNrStepsInDirectionUntilWallFound(coord c, const Directions_t dir) const {
  int stepsTaken = 0;
  bool done = false;
  while(done == false) {
    if(eng->mapTests->isCellInsideMainScreen(c.x, c.y) == false) {
      return -1;
    }
    if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneWall) {
      return stepsTaken;
    }
    c += dir == direction_right ? coord(1, 0) : dir == direction_up ? coord(0, -1) : dir == direction_left ? coord(-1, 0) : coord(0, 1);
    stepsTaken++;
  }
  return -1;
}

bool MapBuildBSP::isAreaFree(const Rect& area, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]) {
  return isAreaFree(area.x0y0.x, area.x0y0.y, area.x1y1.x, area.x1y1.y, blockingCells);
}

bool MapBuildBSP::isAreaFree(const int X0, const int Y0, const int X1, const int Y1, bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(blockingCells[x][y]) {
        return false;
      }
    }
  }
  return true;
}

void MapBuildBSP::buildAuxRooms(Region* regions[3][3]) {
  const int NR_TRIES_PER_SIDE = 10;

  for(int regionY = 0; regionY < 3; regionY++) {
    for(int regionX = 0; regionX < 3; regionX++) {

      if(eng->dice.coinToss()) {

        const Region* const region = regions[regionX][regionY];
        const Room* const mainRoom = region->mainRoom;

        if(mainRoom != NULL) {

          bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS];
          eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, blockingCells);
          eng->basicUtils->reverseBoolArray(blockingCells);

          int connectX, connectY, auxRoomW, auxRoomH, auxRoomX, auxRoomY;

          //Right
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = mainRoom->getX1() + 1;
            connectY = eng->dice.getInRange(mainRoom->getY0() + 1, mainRoom->getY1() - 1);
            auxRoomW = eng->dice(1, 5) + 2;
            auxRoomH = eng->dice(1, 5) + 2;
            auxRoomX = connectX + 1;
            auxRoomY = eng->dice.getInRange(connectY - auxRoomH + 1, connectY);
            coord c(connectX, connectY);
            if(tryPlaceAuxRoom(auxRoomX, auxRoomY, auxRoomW, auxRoomH, blockingCells, c)) {
              i = 99999;
            }
          }

          //Up
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = eng->dice.getInRange(mainRoom->getX0() + 1, mainRoom->getX1() - 1);
            connectY = mainRoom->getY0() - 1;
            auxRoomW = eng->dice(1, 5) + 2;
            auxRoomH = eng->dice(1, 5) + 2;
            auxRoomX = eng->dice.getInRange(connectX - auxRoomW + 1, connectX);
            auxRoomY = connectY - auxRoomH;
            coord c(connectX, connectY);
            if(tryPlaceAuxRoom(auxRoomX, auxRoomY, auxRoomW, auxRoomH, blockingCells, c)) {
              i = 99999;
            }
          }

          //Left
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = mainRoom->getX0() - 1;
            connectY = eng->dice.getInRange(mainRoom->getY0() + 1, mainRoom->getY1() - 1);
            auxRoomW = eng->dice(1, 5) + 2;
            auxRoomH = eng->dice(1, 5) + 2;
            auxRoomX = connectX - auxRoomW;
            auxRoomY = eng->dice.getInRange(connectY - auxRoomH + 1, connectY);
            coord c(connectX, connectY);
            if(tryPlaceAuxRoom(auxRoomX, auxRoomY, auxRoomW, auxRoomH, blockingCells, c)) {
              i = 99999;
            }
          }

          //Down
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = eng->dice.getInRange(mainRoom->getX0() + 1, mainRoom->getX1() - 1);
            connectY = mainRoom->getY1() + 1;
            auxRoomW = eng->dice(1, 5) + 2;
            auxRoomH = eng->dice(1, 5) + 2;
            auxRoomX = eng->dice.getInRange(connectX - auxRoomW + 1, connectX);
            auxRoomY = connectY + 1;
            coord c(connectX, connectY);
            if(tryPlaceAuxRoom(auxRoomX, auxRoomY, auxRoomW, auxRoomH, blockingCells, c)) {
              i = 99999;
            }
          }
        }
      }
    }
  }
}

bool MapBuildBSP::tryPlaceAuxRoom(const int X0, const int Y0, const int W, const int H,
                                  bool blockers[MAP_X_CELLS][MAP_Y_CELLS], const coord doorPos) {
  Rect auxArea, auxAreaWithWalls;
  auxArea.x0y0.set(X0, Y0);
  auxArea.x1y1.set(X0 + W - 1, Y0 + H - 1);
  auxAreaWithWalls.x0y0.set(auxArea.x0y0 - coord(1, 1));
  auxAreaWithWalls.x1y1.set(auxArea.x1y1 + coord(1, 1));
  if(isAreaFree(auxAreaWithWalls, blockers) && eng->mapTests->isAreaInsideMainScreen(auxAreaWithWalls)) {
    Room* room = buildRoom(auxArea);
    rooms_.push_back(room);
    for(int y = auxArea.x0y0.y; y <= auxArea.x1y1.y; y++) {
      for(int x = auxArea.x0y0.x; x <= auxArea.x1y1.x; x++) {
        blockers[x][y] = true;
      }
    }

    const int CHANCE_FOR_CRUMLE_ROOM = 30;

    if(eng->dice.getInRange(1, 100) < CHANCE_FOR_CRUMLE_ROOM) {
      makeCrumbleRoom(auxAreaWithWalls, doorPos);
      // If we're making a "crumble room" we don't want to keep it for applying a theme and such
      deleteAndRemoveRoomFromList(room);
      room = NULL;
    } else {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, doorPos);
      doorPositionCandidates[doorPos.x][doorPos.y] = true;
    }

    return true;
  }
  return false;
}

void MapBuildBSP::makeCrumbleRoom(const Rect roomAreaIncludingWalls, const coord proxEventPos) {
  vector<coord> wallCells;
  vector<coord> innerCells;

  const Rect a(roomAreaIncludingWalls);

  for(int y = a.x0y0.y; y <= a.x1y1.y; y++) {
    for(int x = a.x0y0.x; x <= a.x1y1.x; x++) {
      if(x == a.x0y0.x || x == a.x1y1.x || y == a.x0y0.y || y == a.x1y1.y) {
        wallCells.push_back(coord(x, y));
      } else {
        innerCells.push_back(coord(x, y));
      }
      eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(x, y));
    }
  }

  ProxEventWallCrumbleSpawnData* const spawnData = new ProxEventWallCrumbleSpawnData(wallCells, innerCells);
  eng->featureFactory->spawnFeatureAt(feature_proxEventWallCrumble, proxEventPos, spawnData);
}


//======================================================================== REGION
Region::Region(coord x0y0, coord x1y1) :
  mainRoom(NULL), isConnected(false), x0y0_(x0y0), x1y1_(x1y1) {
  for(int x = 0; x <= 2; x++) {
    for(int y = 0; y <= 2; y++) {
      regionsConnectedTo[x][y] = false;
    }
  }
}

Region::Region() :
  mainRoom(NULL), isConnected(false), x0y0_(coord(-1, -1)), x1y1_(coord(-1, -1)) {
  for(int x = 0; x <= 2; x++) {
    for(int y = 0; y <= 2; y++) {
      regionsConnectedTo[x][y] = false;
    }
  }
}

Region::~Region() {
}

int Region::getNrOfConnections() {
  int connections = 0;
  for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
      if(regionsConnectedTo[x][y]) {
        connections++;
      }
    }
  }
  return connections;
}

bool Region::isRegionNeighbour(const Region& other, Engine* const engine) {
  for(int x = x0y0_.x; x <= x1y1_.x; x++) {
    for(int y = x0y0_.y; y <= x1y1_.y; y++) {
      for(int xx = other.x0y0_.x; xx <= other.x1y1_.x; xx++) {
        for(int yy = other.x0y0_.y; yy <= other.x1y1_.y; yy++) {
          if(engine->mapTests->isCellsNeighbours(x, y, xx, yy, false)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

Rect Region::getRandomCoordsForRoom(Engine* eng) const {
  const bool TINY_ALLOWED_HOR = eng->dice.coinToss();

  const coord minDim(TINY_ALLOWED_HOR ? 2 : 4, TINY_ALLOWED_HOR ? 4 : 2);
  const coord maxDim = x1y1_ - x0y0_ - coord(1, 1);

  const coord dim(eng->dice.getInRange(minDim.x, maxDim.x - 1), eng->dice.getInRange(minDim.y, maxDim.y - 1));

  const int X0 = x0y0_.x + 1 + eng->dice.getInRange(0, maxDim.x - dim.x - 1);
  const int Y0 = x0y0_.y + 1 + eng->dice.getInRange(0, maxDim.y - dim.y - 1);
  const int X1 = X0 + dim.x - 1;
  const int Y1 = Y0 + dim.y - 1;

  return Rect(coord(X0, Y0), coord(X1, Y1));
}



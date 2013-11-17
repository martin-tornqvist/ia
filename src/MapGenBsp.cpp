#include "MapGen.h"

#include <algorithm>
#include <stdlib.h>
#include <cassert>

#include "Engine.h"
#include "MapGen.h"
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

//============================================================= MAPBUILD-BSP
void MapGenBsp::specificRun() {
  trace << "MapGenBsp::specificRun()..." << endl;

  eng->map->clearMap();

  trace << "MapGenBsp: Resetting helper arrays" << endl;
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      roomCells[x][y] = false;
//      eng->featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
      globalDoorPositionCandidates[x][y] = false;
      forbiddenStairCellsGlobal[x][y] = false;
    }
  }

  const int SPLIT_X1 = MAP_X_CELLS / 3 + eng->dice.range(-1, 1);
  const int SPLIT_X2 = 2 * (MAP_X_CELLS / 3) + eng->dice.range(-1, 1);
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

  const int FIRST_DUNGEON_LEVEL_CAVES_ALLOWED = 10;
  const int DLVL = eng->map->getDLVL();
  const int CHANCE_FOR_CAVE_AREA = (DLVL - FIRST_DUNGEON_LEVEL_CAVES_ALLOWED + 1) * 20;
  if(eng->dice.percentile() < CHANCE_FOR_CAVE_AREA) {
    const bool IS_TWO_CAVES = eng->dice.percentile() < CHANCE_FOR_CAVE_AREA / 3;
    for(int nrCaves = IS_TWO_CAVES ? 2 : 1; nrCaves > 0; nrCaves--) {
      int nrTriesToMark = 1000;
      while(nrTriesToMark > 0) {
        Pos c(eng->dice.range(0, 2), eng->dice.range(0, 2));
        if(regions[c.x][c.y] == NULL && regionsToBuildCave[c.x][c.y] == false) {
          regionsToBuildCave[c.x][c.y] = true;
          nrTriesToMark = 0;
        }
        nrTriesToMark--;
      }
    }
  }

  trace << "MapGenBsp: Making rooms" << endl;
  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      if(regions[x][y] == NULL) {
        const int X0 = x == 0 ? 0 : x == 1 ? SPLIT_X1 : SPLIT_X2;
        const int Y0 = y == 0 ? 0 : y == 1 ? SPLIT_Y1 : SPLIT_Y2;
        const int X1 = x == 0 ? SPLIT_X1 - 1 : x == 1 ? SPLIT_X2 - 1 : MAP_X_CELLS - 1;
        const int Y1 = y == 0 ? SPLIT_Y1 - 1 : y == 1 ? SPLIT_Y2 - 1 : MAP_Y_CELLS - 1;
        Region* region = new Region(Pos(X0, Y0), Pos(X1, Y1));
        regions[x][y] = region;
        const Rect roomPoss = region->getRandomPossForRoom(eng);

        eng->map->rooms.push_back(buildRoom(roomPoss));
        regions[x][y]->mainRoom = eng->map->rooms.back();

        if(eng->dice.percentile() < 30) {
          reshapeRoom(*(regions[x][y]->mainRoom));
        }
      }
    }
  }

  connectRegions(regions);

  buildAuxRooms(regions);

  buildCaves(regions);

  buildRoomsInRooms();

  postProcessFillDeadEnds();

  trace << "MapGenBsp: Placing doors" << endl;
  const int CHANCE_TO_PLACE_DOOR = 70;
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(globalDoorPositionCandidates[x][y] == true) {
        if(eng->dice.percentile() < CHANCE_TO_PLACE_DOOR) {
          placeDoorAtPosIfSuitable(Pos(x, y));
        }
      }
    }
  }

  trace << "MapGenBsp: Moving player to nearest floor cell" << endl;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeWalkBlockingArrayFeaturesOnly(blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<Pos> freeCells;
  eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
  sort(freeCells.begin(), freeCells.end(),
       IsCloserToOrigin(eng->player->pos, eng));
  eng->player->pos = freeCells.front();

  trace << "MapGenBsp: Calling RoomThemeMaker::run()" << endl;
  eng->roomThemeMaker->run();

  trace << "MapGenBsp: Moving player to nearest floor cell again ";
  trace << "after room theme maker" << endl;
  eng->mapTests->makeWalkBlockingArrayFeaturesOnly(blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
  sort(freeCells.begin(), freeCells.end(),
       IsCloserToOrigin(eng->player->pos, eng));
  eng->player->pos = freeCells.front();

  const Pos stairsPos = placeStairs();

  const int LAST_LEVEL_TO_REVEAL_STAIRS_PATH = 9;
  if(eng->map->getDLVL() <= LAST_LEVEL_TO_REVEAL_STAIRS_PATH) {
    revealAllDoorsBetweenPlayerAndStairs(stairsPos);
  }

//  makeLevers();

  //This must be run last, everything depends on all walls being stone walls
  decorate();

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      delete regions[x][y];
      regions[x][y] = NULL;
    }
  }

  trace << "MapGenBsp::specificRun() [DONE]" << endl;
}

//TODO Should be in Map
void MapGenBsp::deleteAndRemoveRoomFromList(Room* const room) {
  vector<Room*>& rooms = eng->map->rooms;
  for(unsigned int i = 0; i < rooms.size(); i++) {
    if(rooms.at(i) == room) {
      delete room;
      rooms.erase(rooms.begin() + i);
      return;
    }
  }
  trace << "[WARNING] Tried to remove room that is not in list, ";
  trace << "in MapGenBsp::deleteAndRemoveRoomFromList()" << endl;
}

//void MapGenBsp::makeLevers() {
//  trace << "MapGenBsp::makeLeverPuzzle()..." << endl;
//
//  trace << "MapGenBsp: Picking a random door" << endl;
//  vector<Door*> doorCandidates;
//  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
//    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
//      Feature* const feature = eng->map->featuresStatic[x][y];
//      if(feature->getId() == feature_door) {
//        Door* const door = dynamic_cast<Door*>(feature);
//        doorCandidates.push_back(door);
//      }
//    }
//  }
//  Door* const doorToLink = doorCandidates.at(eng->dice.range(0, doorCandidates.size() - 1));
//
//  trace << "MapGenBsp: Making floodfill and keeping only positions with lower value than the door" << endl;
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(actorBodyType_normal, blockers);
//  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
//    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
//      Feature* const feature = eng->map->featuresStatic[x][y];
//      if(feature->getId() == feature_door) {
//        blockers[x][y] = false;
//      }
//    }
//  }
//  int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->mapTests->floodFill(eng->player->pos, blockers, floodFill, 99999, Pos(-1, -1));
//  const int FLOOD_VALUE_AT_DOOR = floodFill[doorToLink->pos_.x][doorToLink->pos_.y];
//  vector<Pos> leverPosCandidates;
//  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
//    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
//      if(floodFill[x][y] < FLOOD_VALUE_AT_DOOR) {
//        if(eng->map->featuresStatic[x][y]->canHaveStaticFeature()) {
//          leverPosCandidates.push_back(Pos(x, y));
//        }
//      }
//    }
//  }
//
//  if(leverPosCandidates.size() > 0) {
//    const int ELEMENT = eng->dice.range(0, leverPosCandidates.size() - 1);
//    const Pos leverPos(leverPosCandidates.at(ELEMENT));
//    spawnLeverAdaptAndLinkDoor(leverPos, *doorToLink);
//  } else {
//    trace << "[WARNING] Could not find position to place lever, in MapGenBsp::makeLeverPuzzle()" << endl;
//  }
//  trace << "MapGenBsp::makeLeverPuzzle() [DONE]" << endl;
//}

//void MapGenBsp::spawnLeverAdaptAndLinkDoor(const Pos& leverPos, Door& door) {
//  trace << "MapGenBsp: Spawning lever and linking it to the door" << endl;
//  eng->featureFactory->spawnFeatureAt(feature_lever, leverPos, new LeverSpawnData(&door));
//
//  trace << "MapGenBsp: Changing door properties" << endl;
//  door.material_ = doorMaterial_metal;
//  door.isOpen_ = false;
//  door.isStuck_ = false;
//  door.isOpenedAndClosedExternally_ = true;
//}

void MapGenBsp::buildCaves(Region* regions[3][3]) {
  trace << "MapGenBsp::buildCaves()..." << endl;
  for(int regY = 0; regY <= 2; regY++) {
    for(int regX = 0; regX <= 2; regX++) {

      if(regionsToBuildCave[regX][regY]) {

        Region* const region = regions[regX][regY];

        //This region no longer has a room, delete it from list
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
                  if(IS_FLOOR && eng->mapTests->isCellInside(Pos(x + dx, y + dy), region->getRegionPoss()) == false) {
                    blockers[x][y] = true;
                  }
                }
              }
            }
          }
        }

        const Pos origin(region->getX0Y0() + Pos(1, 1));
        int floodFillResult[MAP_X_CELLS][MAP_Y_CELLS];

        const int FLOOD_FILL_TRAVEL_LIMIT = 20;

        eng->mapTests->floodFill(origin, blockers, floodFillResult,
                                 FLOOD_FILL_TRAVEL_LIMIT, Pos(-1, -1));

        for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
          for(int x = 1; x < MAP_X_CELLS - 1; x++) {
            const Pos c(x, y);
            if(c == origin || floodFillResult[x][y] > 0) {

              eng->featureFactory->spawnFeatureAt(feature_caveFloor, c);

//              eng->featureFactory->spawnFeatureAt(feature_shallowMud, c);

              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  if(eng->map->featuresStatic[x + dx][y + dy]->getId() == feature_stoneWall) {
                    eng->featureFactory->spawnFeatureAt(feature_stoneWall, c + Pos(dx , dy));
                    Wall* const wall = dynamic_cast<Wall*>(eng->map->featuresStatic[x + dx][y + dy]);
                    wall->wallType = wall_cave;
                    wall->setRandomIsMossGrown();
                  }
                }
              }
            }
          }
        }

        const int CHANCE_TO_MAKE_CHASM = 25;

        if(eng->dice.percentile() < CHANCE_TO_MAKE_CHASM) {
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

          eng->mapTests->floodFill(
            origin, blockers, floodFillResult, FLOOD_FILL_TRAVEL_LIMIT / 2,
            Pos(-1, -1));

          for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
            for(int x = 1; x < MAP_X_CELLS - 1; x++) {
              const Pos c(x, y);
              if(
                blockers[x][y] == false &&
                (c == origin || floodFillResult[x][y] > 0)) {
                eng->featureFactory->spawnFeatureAt(feature_chasm, c);
              }
            }
          }
        }
      }
    }
  }
  trace << "MapGenBsp::buildCaves()[DONE]" << endl;
}

void MapGenBsp::buildMergedRegionsAndRooms(
  Region* regions[3][3], const int SPLIT_X1, const int SPLIT_X2,
  const int SPLIT_Y1, const int SPLIT_Y2) {

  const int NR_OF_MERGED_REGIONS_TO_ATTEMPT = eng->dice.range(0, 2);

  for(
    int attemptCount = 0;
    attemptCount < NR_OF_MERGED_REGIONS_TO_ATTEMPT;
    attemptCount++) {

    Pos regionIndex1, regionIndex2;

    //Find two non-occupied regions
    int nrTriesToFindRegions = 100;
    bool isGoodRegionsFound = false;
    while(isGoodRegionsFound == false) {
      nrTriesToFindRegions--;
      if(nrTriesToFindRegions <= 0) {
        return;
      }

      regionIndex1 = Pos(eng->dice.range(0, 2), eng->dice.range(0, 1));
      regionIndex2 = Pos(regionIndex1 + Pos(0, 1));
      isGoodRegionsFound = regions[regionIndex1.x][regionIndex1.y] == NULL && regions[regionIndex2.x][regionIndex2.y] == NULL;
    }

    const int MERGED_X0 = regionIndex1.x == 0 ? 0 : regionIndex1.x == 1 ? SPLIT_X1 : SPLIT_X2;
    const int MERGED_Y0 = regionIndex1.y == 0 ? 0 : regionIndex1.y == 1 ? SPLIT_Y1 : SPLIT_Y2;
    const int MERGED_X1 = regionIndex2.x == 0 ? SPLIT_X1 - 1 : regionIndex2.x == 1 ? SPLIT_X2 - 1 : MAP_X_CELLS - 1;
    const int MERGED_Y1 = regionIndex2.y == 0 ? SPLIT_Y1 - 1 : regionIndex2.y == 1 ? SPLIT_Y2 - 1 : MAP_Y_CELLS - 1;

    const int AREA_2_X0 = regionIndex2.x == 0 ? 0 : regionIndex2.x == 1 ? SPLIT_X1 : SPLIT_X2;
    const int AREA_2_Y0 = regionIndex2.y == 0 ? 0 : regionIndex2.y == 1 ? SPLIT_Y1 : SPLIT_Y2;
    const int AREA_2_X1 = MERGED_X1;
    const int AREA_2_Y1 = MERGED_Y1;

    const bool AREA_2_IS_BELOW = regionIndex2.y > regionIndex1.y;

    const int AREA_1_X0 = MERGED_X0;
    const int AREA_1_X1 = AREA_2_IS_BELOW ? MERGED_X1 - 1 : AREA_2_X0 - 1;
    const int AREA_1_Y0 = MERGED_Y0;
    const int AREA_1_Y1 = AREA_2_IS_BELOW ? AREA_2_Y0 - 1 : MERGED_Y1;

    const Rect area1(Pos(AREA_1_X0, AREA_1_Y0), Pos(AREA_1_X1, AREA_1_Y1));
    const Rect area2(Pos(AREA_2_X0, AREA_2_Y0), Pos(AREA_2_X1, AREA_2_Y1));

    Region* region1 = new Region(area1.x0y0, area1.x1y1);
    Region* region2 = new Region(area2.x0y0, area2.x1y1);
    regions[regionIndex1.x][regionIndex1.y] = region1;
    regions[regionIndex2.x][regionIndex2.y] = region2;

    const int OFFSET_X0 = eng->dice.range(1, 4);
    const int OFFSET_Y0 = eng->dice.range(1, 4);
    const int OFFSET_X1 = eng->dice.range(1, 4);
    const int OFFSET_Y1 = eng->dice.range(1, 4);
    Rect roomPoss(area1.x0y0 + Pos(OFFSET_X0, OFFSET_Y0), area2.x1y1 - Pos(OFFSET_X1, OFFSET_Y1));
    Room* const room = buildRoom(roomPoss);
    eng->map->rooms.push_back(room);

    region1->mainRoom = region2->mainRoom = room;

    region1->regionsConnectedTo[regionIndex2.x][regionIndex2.y] = true;
    region2->regionsConnectedTo[regionIndex1.x][regionIndex1.y] = true;

    if(eng->dice.percentile() < 33) {
      reshapeRoom(*room);
    }
  }
}

void MapGenBsp::buildRoomsInRooms() {

  const int NR_OF_TRIES = 40;
  const int MAX_NR_INNER_ROOMS = 7;
  const int MIN_DIM_W = 4;

  const int MIN_DIM_H = 4;

  vector<Room*>& rooms = eng->map->rooms;
  for(unsigned int i = 0; i < rooms.size(); i++) {

    const Pos roomX0Y0 = rooms.at(i)->getX0Y0();
    const Pos roomX1Y1 = rooms.at(i)->getX1Y1();

    const int ROOM_WI = roomX1Y1.x - roomX0Y0.x + 1;
    const int ROOM_HE = roomX1Y1.y - roomX0Y0.y + 1;

    const bool IS_ROOM_BIG = ROOM_WI > 16 || ROOM_HE > 8;

    if(IS_ROOM_BIG || eng->dice.percentile() < 30) {
      const int MAX_DIM_W = min(16, ROOM_WI);
      const int MAX_DIM_H = min(16, ROOM_HE);

      if(MAX_DIM_W >= MIN_DIM_W && MAX_DIM_H >= MIN_DIM_H) {

        for(
          int nrRoomsCount = 0;
          nrRoomsCount < MAX_NR_INNER_ROOMS;
          nrRoomsCount++) {
          for(int tryCount = 0; tryCount < NR_OF_TRIES; tryCount++) {

            const int W = eng->dice.range(MIN_DIM_W, MAX_DIM_W);
            const int H = eng->dice.range(MIN_DIM_H, MAX_DIM_H);

            const int X0 = eng->dice.range(roomX0Y0.x - 1, roomX1Y1.x - W + 2);
            const int Y0 = eng->dice.range(roomX0Y0.y - 1, roomX1Y1.y - H + 2);
            const int X1 = X0 + W - 1;
            const int Y1 = Y0 + H - 1;

            bool isSpaceFree = true;

            for(int y = Y0 - 1; y <= Y1 + 1; y++) {
              for(int x = X0 - 1; x <= X1 + 1; x++) {
                if(
                  eng->mapTests->isCellInside(
                    Pos(x, y), Rect(roomX0Y0 - Pos(1, 1),
                                    roomX1Y1 + Pos(1, 1)))) {
                  if(
                    x == roomX0Y0.x - 1 ||
                    x == roomX1Y1.x + 1 ||
                    y == roomX0Y0.y - 1 ||
                    y == roomX1Y1.y + 1) {
                    if(
                      eng->map->featuresStatic[x][y]->getId() !=
                      feature_stoneWall) {
                      isSpaceFree = false;
                    }
                  } else {
                    if(
                      eng->map->featuresStatic[x][y]->getId() !=
                      feature_stoneFloor) {
                      isSpaceFree = false;
                    }
                  }
                }
              }
            }

            if(
              (X0 == roomX0Y0.x && X1 == roomX1Y1.x) ||
              (Y0 == roomX0Y0.y && Y1 == roomX1Y1.y)) {
              isSpaceFree = false;
            }

            if(isSpaceFree) {
              vector<Pos> doorCandidates;
              rooms.push_back(
                new Room(Rect(Pos(X0 + 1, Y0 + 1), Pos(X1 - 1, Y1 - 1))));
              for(int y = Y0; y <= Y1; y++) {
                for(int x = X0; x <= X1; x++) {
                  if(x == X0 || x == X1 || y == Y0 || y == Y1) {

                    eng->featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));

                    if(
                      x != roomX0Y0.x - 1 && x != roomX0Y0.x &&
                      x != roomX1Y1.x && x != roomX1Y1.x + 1 &&
                      y != roomX0Y0.y - 1 && y != roomX0Y0.y &&
                      y != roomX1Y1.y && y != roomX1Y1.y + 1) {
                      if((x != X0 && x != X1) || (y != Y0 && y != Y1)) {
                        doorCandidates.push_back(Pos(x, y));
                      }
                    }
                  }
                }
              }
              if(eng->dice.coinToss() || doorCandidates.size() <= 2) {
                const int DOOR_POS_ELEMENT =
                  eng->dice.range(0, doorCandidates.size() - 1);
                const Pos doorPos = doorCandidates.at(DOOR_POS_ELEMENT);
//                if(eng->dice.coinToss()) {
//                  eng->featureFactory->spawnFeatureAt(feature_door, doorPos, new DoorSpawnData(eng->featureData->getFeatureDef(feature_stoneWall)));
                eng->featureFactory->spawnFeatureAt(feature_stoneFloor, doorPos);
                globalDoorPositionCandidates[doorPos.x][doorPos.y] = true;
//                } else {
//                  eng->featureFactory->spawnFeatureAt(feature_stoneFloor, doorPos);
//                }
              } else {
                vector<Pos> positionsWithDoor;
                const int NR_TRIES = eng->dice.range(1, 10);
                for(int j = 0; j < NR_TRIES; j++) {
                  const int DOOR_POS_ELEMENT =
                    eng->dice.range(0, doorCandidates.size() - 1);
                  const Pos doorPos = doorCandidates.at(DOOR_POS_ELEMENT);

                  bool positionOk = true;
                  for(unsigned int n = 0; n < positionsWithDoor.size(); n++) {
                    if(eng->mapTests->isCellsAdj(
                          doorPos, positionsWithDoor.at(n), false)) {
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

void MapGenBsp::postProcessFillDeadEnds() {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(
    actorBodyType_normal, blockers);

  //Find an origin with no adjacent walls, to ensure we don't start in a dead end
  Pos origin;
  for(int y = 2; y < MAP_Y_CELLS - 2; y++) {
    for(int x = 2; x < MAP_X_CELLS - 2; x++) {
      if(isAreaFree(x - 1, y - 1, x + 1, y + 1, blockers)) {
        origin = Pos(x, y);
        y = 999;
        x = 999;
      }
    }
  }

  //Floodfill from origin, then sort the positions for flood value
  int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->floodFill(origin, blockers, floodFill, 99999, Pos(-1, -1));
  vector<PosAndVal> floodFillVector;
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(blockers[x][y] == false) {
        floodFillVector.push_back(PosAndVal(Pos(x, y), floodFill[x][y]));
      }
    }
  }
  std::sort(floodFillVector.begin(), floodFillVector.end(),
            PosAndVal_compareForVal());

  //Fill all positions with only one cardinal floor neighbour
  for(int i = int(floodFillVector.size()) - 1; i >= 0; i--) {
    const Pos& pos = floodFillVector.at(i).pos;
    const int x = pos.x;
    const int y = pos.y;
    const int NR_ADJ_CARDINAL_WALLS = blockers[x + 1][y] + blockers[x - 1][y] +
                                      blockers[x][y + 1] + blockers[x][y - 1];
    if(NR_ADJ_CARDINAL_WALLS == 3) {
      eng->featureFactory->spawnFeatureAt(feature_stoneWall, pos);
      blockers[x][y] = true;
    }
  }
}

//void MapGenBsp::buildNaturalArea(Region* regions[3][3]) {
//  const Pos origin(1,1);
//  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
//    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
//      if(eng->basicUtils->pointDist(origin.x, origin.y, x, y) < 20) {
//        eng->featureFactory->spawnFeatureAt(feature_deepWater, Pos(x,y));
//        for(int yRegion = 0; yRegion < 3; yRegion++) {
//          for(int xRegion = 0; xRegion < 3; xRegion++) {
//            Region* region = regions[xRegion][yRegion];
//            if(eng->mapTests->isCellInside(Pos(x,y), region->getX0Y0(), region->getX1Y1())) {
//              region->mapArea.isSpecialRoomAllowed = false;
//            }
//          }
//        }
//      }
//    }
//  }
//  makeRiver(regions);
//}

//void MapGenBsp::makeRiver(Region* regions[3][3]) {
//  (void)regions;
//
//  const int WIDTH = eng->dice.range(4, 12);
//  const int START_X_OFFSET_MAX = 5;
//  const int X_POS_START = MAP_X_CELLS/2 + eng->dice.range(-START_X_OFFSET_MAX, START_X_OFFSET_MAX);
//
//  Pos leftPos(X_POS_START, 0);
//  while(eng->mapTests->isCellInsideMap(leftPos) && eng->mapTests->isCellInsideMap(leftPos + Pos(WIDTH,0))) {
//    coverAreaWithFeature(Rect(leftPos, leftPos + Pos(WIDTH, 0)), feature_deepWater);
//    leftPos += Pos(eng->dice.range(-1,1), 1);
//  }
//}

Pos MapGenBsp::placeStairs() {
  trace << "MapGenBsp::placeStairs()..." << endl;
  bool forbiddenStairCells[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(forbiddenStairCells, true);

  trace << "MapGenBsp: Setting local forbiddenStairCells from global" << endl;
  vector<Room*>& rooms = eng->map->rooms;
  for(unsigned int i = 0; i < rooms.size(); i++) {
    const Room* const room = rooms.at(i);
    for(int y = room->getY0(); y <= room->getY1(); y++) {
      for(int x = room->getX0(); x <= room->getX1(); x++) {
        if(forbiddenStairCellsGlobal[x][y] == false) {
          forbiddenStairCells[x][y] = false;
        }
      }
    }
  }

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  trace << "MapGenBsp: Calling ";
  trace << "MapTests::makeMoveBlockerArrayFeaturesOnly()" << endl;
  eng->mapTests->makeMoveBlockerArrayFeaturesOnly(eng->player, blockers);

  trace << "MapGenBsp: Setting all door cells to non-blocking" << endl;
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
        blockers[x][y] = false;
      }
    }
  }

  int floodFill[MAP_X_CELLS][MAP_Y_CELLS];
  trace << "MapGenBsp: Calling MapTests::floodFill()" << endl;
  eng->mapTests->floodFill(
    eng->player->pos, blockers, floodFill, 99999, Pos(-1, -1));

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(forbiddenStairCells[x][y] == true || floodFill[x][y] == 0) {
        blockers[x][y] = true;
      }
    }
  }

  eng->basicUtils->reverseBoolArray(blockers);
  vector<Pos> freeCells;
  eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCells);

  trace << "MapGenBsp: Sorting the free cells vector (size:";
  trace << freeCells.size() << "), and removing the furthest cells" << endl;
  const unsigned int FREE_STAIR_CELLS_DIV = 4;
  if(freeCells.size() > FREE_STAIR_CELLS_DIV) {
    IsCloserToOrigin isCloserToOrigin(eng->player->pos, eng);
    sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);
    reverse(freeCells.begin(), freeCells.end());
    freeCells.resize(freeCells.size() / FREE_STAIR_CELLS_DIV);
  }

  trace << "MapGenBsp: Picking the furthest cell from the remaining(size:";
  trace << freeCells.size() <<  ")" << endl;
  const int STAIR_Pos_INDEX = eng->dice(1, freeCells.size()) - 1;
  const Pos stairsPos(freeCells.at(STAIR_Pos_INDEX));

  trace << "MapGenBsp: Spawning down stairs at chosen Pos" << endl;
  Feature* f = eng->featureFactory->spawnFeatureAt(
                 feature_stairsDown, stairsPos);
  f->setHasBlood(false);

  trace << "MapGenBsp::placeStairs()[DONE]" << endl;
  return stairsPos;
}

void MapGenBsp::revealAllDoorsBetweenPlayerAndStairs(const Pos& stairsPos) {
  trace << "MapGenBsp::revealAllDoorsBetweenPlayerAndStairs()..." << endl;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(
    actorBodyType_normal, blockers);

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_door) {
        blockers[x][y] = false;
      }
    }
  }

  vector<Pos> path = eng->pathfinder->findPath(eng->player->pos, blockers, stairsPos);

  trace << "MapGenBsp: Travelling along Pos vector of size " << path.size();
  trace << " and revealing all doors" << endl;
  const unsigned int PATH_SIZE = path.size();
  for(unsigned int i = 0; i < PATH_SIZE; i++) {
    const Pos& pos = path.at(i);
    Feature* const feature = eng->map->featuresStatic[pos.x][pos.y];
    if(feature->getId() == feature_door) {
      dynamic_cast<Door*>(feature)->reveal(false);
    }
  }

  trace << "MapGenBsp::revealAllDoorsBetweenPlayerAndStairs()[DONE]" << endl;
}

void MapGenBsp::decorate() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_stoneWall) {

        int nrAdjFloor = 0;

        for(int yy = max(0, y - 1); yy <= min(MAP_Y_CELLS - 1, y + 1); yy++) {
          for(int xx = max(0, x - 1); xx <= min(MAP_X_CELLS - 1, x + 1); xx++) {
            if(xx != x || yy != y) {
              if(eng->map->featuresStatic[xx][yy]->getId() == feature_stoneFloor) {
                nrAdjFloor++;
              }
            }
          }
        }

        //Randomly convert walls to rubble
        if(eng->dice.percentile() < 10) {
          eng->featureFactory->spawnFeatureAt(feature_rubbleHigh, Pos(x, y));
          continue;
        }

        //Moss grown walls
        Wall* const wall = dynamic_cast<Wall*>(eng->map->featuresStatic[x][y]);
        wall->setRandomIsMossGrown();

        //Convert walls with no adjacent stone floor to cave walls
        if(nrAdjFloor == 0) {
          dynamic_cast<Wall*>(eng->map->featuresStatic[x][y])->wallType = wall_cave;
        } else {
          wall->setRandomNormalWall();
        }
      }
    }
  }

//  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
//    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
//      if(eng->map->featuresStatic[x][y]->getId() == feature_stoneFloor) {
//        //Randomly convert stone floor to low rubble
//        if(eng->dice.percentile() == 1) {
//          eng->featureFactory->spawnFeatureAt(feature_rubbleLow, Pos(x, y));
//          continue;
//        }
//      }
//    }
//  }
}

void MapGenBsp::connectRegions(Region* regions[3][3]) {
  trace << "MapGenBsp::connectRegions()..." << endl;

  bool isAllConnected = false;
  while(isAllConnected == false) {
    isAllConnected = isAllRoomsConnected();

    Pos c1(eng->dice.range(0, 2), eng->dice.range(0, 2));
    Region* r1 = regions[c1.x][c1.y];

    Pos delta(0, 0);
    bool isDeltaOk = false;
    while(isDeltaOk == false) {
      delta.set(eng->dice.range(-1, 1), eng->dice.range(-1, 1));
      Pos c2(c1 + delta);
      const bool IS_INSIDE_BOUNDS = c2.x >= 0 && c2.y >= 0 && c2.x <= 2 && c2.y <= 2;
      const bool IS_ZERO = delta.x == 0 && delta.y == 0;
      const bool IS_DIAGONAL = delta.x != 0 && delta.y != 0;
      isDeltaOk = IS_ZERO == false && IS_DIAGONAL == false && IS_INSIDE_BOUNDS == true;
    }
    Pos c2(c1 + delta);

    Region* const r2 = regions[c2.x][c2.y];

    if(r1->regionsConnectedTo[c2.x][c2.y] == false) {
      buildCorridorBetweenRooms(r1, r2);
      r1->regionsConnectedTo[c2.x][c2.y] = true;
      r2->regionsConnectedTo[c1.x][c1.y] = true;
    }
//    if(totalNrConnections >= MAX_NR_CONNECTIONS_LIMIT) {
//      break;
//    }
  }
  trace << "MapGenBsp::connectRegions()[DONE]" << endl;
}

bool MapGenBsp::isAllRoomsConnected() {
  Pos c;
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
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
  eng->mapTests->floodFill(c, blockers, floodFill, 99999, Pos(-1, -1));
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_stoneFloor) {
        if(Pos(x, y) != c && floodFill[x][y] == 0) {
          return false;
        }
      }
    }
  }
  return true;
}


bool MapGenBsp::isRegionFoundInCardinalDirection(
  const Pos pos, bool region[MAP_X_CELLS][MAP_Y_CELLS]) const {
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

void MapGenBsp::buildCorridorBetweenRooms(
  const Region* const r1, const Region* r2) {
  //Find all floor in both regions
  vector<Pos> floorInR1Vector;
  bool floorInR1Grid[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(floorInR1Grid, false);
  for(int y = r1->mainRoom->getY0(); y <= r1->mainRoom->getY1(); y++) {
    for(int x = r1->mainRoom->getX0(); x <= r1->mainRoom->getX1(); x++) {
      const Pos c = Pos(x, y);
      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
        floorInR1Vector.push_back(c);
        floorInR1Grid[x][y] = true;
      }
    }
  }

  const Pos regionDelta = r2->getX0Y0() - r1->getX0Y0();
  const Pos regionDeltaAbs(abs(regionDelta.x), abs(regionDelta.y));
  const Pos regionDeltaSign = regionDelta.getSigns();

  vector<Pos> PossInR1closeToR2;

  vector<Pos> floorInR2Vector;
  bool floorInR2Grid[MAP_X_CELLS][MAP_Y_CELLS];
  eng->basicUtils->resetBoolArray(floorInR2Grid, false);
  for(int y = r2->mainRoom->getY0(); y <= r2->mainRoom->getY1(); y++) {
    for(int x = r2->mainRoom->getX0(); x <= r2->mainRoom->getX1(); x++) {
      Pos c = Pos(x, y);
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
              PossInR1closeToR2.push_back(c);
            }
          }
          if(eng->mapTests->isCellInsideMap(c) == false) {
            doneTravelling = true;
          }
        }
      }
    }
  }

  if(PossInR1closeToR2.size() > 0) {
    Pos c = PossInR1closeToR2.at(eng->dice(1, PossInR1closeToR2.size()) - 1);
    while(floorInR2Grid[c.x][c.y] == false) {
      c += regionDeltaSign;
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
      globalDoorPositionCandidates[c.x][c.y] = true;
    }
  } else {

    /*
    * Description of the following algorithm:
    * ======================================================================
    * What has been done so far is to pick one position in each room with floor in it (c1 and c2).
    * There is also a vector of all floor in in c1 and one of all floor in c2 (floorInR1Vector,
    * floorInR2Vector), as well as bool arrays (floorInR1Grid, floorInR2Grid).
    *
    * First, two points that has only one or two wall cells between them was attempted to be found,
    * so a short, straight connection could be made. Given that this point was reached, no two such
    * position were found.
    *
    * A constant value is stored that is used heavily for corridor navigation: "regionDeltaSign".
    * It tells the direction from region 1 to region 2 (e.g. 1,0 means R2 lies to the east of R1).
    *
    * (1) One position in each room is picked randomly (c1 and c2).
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
    * called "deltaCurSign". It tells the direction from the position we stand in at the start
    * of this phase to the position we are aiming for in room 2 (c2). If regionDeltaSign is
    * subtracted from deltaCurSign, a position is had which tells the direction 90 degrees from
    * the tunnel dug in (2). Example: Region 1 lies east of region 2, regionDeltaSign = (1,0).
    * c2 lies to the east and below the current position, deltaCurSign = (1,1). So
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
    const Pos c1 = floorInR1Vector.at(
                     eng->dice(1, floorInR1Vector.size()) - 1);
    const Pos c2 = floorInR2Vector.at(
                     eng->dice(1, floorInR2Vector.size()) - 1);

    Pos c = c1;

    // (2)
    while(floorInR1Grid[c.x][c.y] == true) {
      c += regionDeltaSign;
    }
    eng->featureFactory->spawnFeatureAt(feature_stoneFloor, c, NULL);
    globalDoorPositionCandidates[c.x][c.y] = true;
    c += regionDeltaSign;

    // (3)
    Pos cTemp(c - regionDeltaSign);
    while(floorInR1Grid[cTemp.x][cTemp.y] == false) {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, cTemp, NULL);
      cTemp -= regionDeltaSign;
    }

    const Pos deltaCur(c2 - c);
    const Pos deltaCurAbs(abs(deltaCur.x), abs(deltaCur.y));
    const Pos deltaCurSign(
      deltaCurAbs.x == 0 ? 0 : deltaCur.x / (deltaCurAbs.x),
      deltaCurAbs.y == 0 ? 0 : deltaCur.y / (deltaCurAbs.y));

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
      globalDoorPositionCandidates[c.x][c.y] = true;
    }
  }
}

void MapGenBsp::placeDoorAtPosIfSuitable(const Pos pos) {
  for(int dx = -2; dx <= 2; dx++) {
    for(int dy = -2; dy <= 2; dy++) {
      if(dx != 0 || dy != 0) {
        if(eng->mapTests->isCellInsideMap(pos + Pos(dx, dy))) {
          if(eng->map->featuresStatic[pos.x + dx][pos.y + dy]->getId() ==
              feature_door) {
            return;
          }
        }
      }
    }
  }

  bool goodVertical = true;
  bool goodHorizontal = true;

  for(int d = -1; d <= 1; d++) {
    if(eng->map->featuresStatic[pos.x + d][pos.y]->getId() ==
        feature_stoneWall) {
      goodHorizontal = false;
    }

    if(eng->map->featuresStatic[pos.x][pos.y + d]->getId() ==
        feature_stoneWall) {
      goodVertical = false;
    }

    if(d != 0) {
      if(eng->map->featuresStatic[pos.x][pos.y + d]->getId() !=
          feature_stoneWall) {
        goodHorizontal = false;
      }

      if(eng->map->featuresStatic[pos.x + d][pos.y]->getId() !=
          feature_stoneWall) {
        goodVertical = false;
      }
    }
  }

  if(goodHorizontal || goodVertical) {
    const FeatureData* const mimicFeatData =
      eng->featureDataHandler->getData(feature_stoneWall);
    eng->featureFactory->spawnFeatureAt(
      feature_door, pos, new DoorSpawnData(mimicFeatData));
  }
}

Room* MapGenBsp::buildRoom(const Rect& roomPoss) {
  coverAreaWithFeature(roomPoss, feature_stoneFloor);
  for(int y = roomPoss.x0y0.y; y <= roomPoss.x1y1.y; y++) {
    for(int x = roomPoss.x0y0.x; x <= roomPoss.x1y1.x; x++) {
      roomCells[x][y] = true;
    }
  }
  return new Room(roomPoss);
}

//void MapGenBsp::findEdgesOfRoom(const Rect roomPoss, vector<Pos>& vectorToFill) {
//  bool PossToAdd[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->basicUtils->resetBoolArray(PossToAdd, false);
//
//  Pos c;
//
//  //Top to bottom
//  for(c.x = roomPoss.x0y0.x; c.x <= roomPoss.x1y1.x; c.x++) {
//    for(c.y = roomPoss.x0y0.y; c.y <= roomPoss.x1y1.y; c.y++) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        PossToAdd[c.x][c.y] = true;
//        c.y = 9999;
//      }
//    }
//  }
//  //Left to right
//  for(c.y = roomPoss.x0y0.y; c.y <= roomPoss.x1y1.y; c.y++) {
//    for(c.x = roomPoss.x0y0.x; c.x <= roomPoss.x1y1.x; c.x++) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        PossToAdd[c.x][c.y] = true;
//        c.x = 99999;
//      }
//    }
//  }
//  //Bottom to top
//  for(c.x = roomPoss.x0y0.x; c.x <= roomPoss.x1y1.x; c.x++) {
//    for(c.y = roomPoss.x1y1.y; c.y >= roomPoss.x0y0.y; c.y--) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        PossToAdd[c.x][c.y] = true;
//        c.y = -9999;
//      }
//    }
//  }
//  //Right to left
//  for(c.y = roomPoss.x0y0.y; c.y <= roomPoss.x1y1.y; c.y++) {
//    for(c.x = roomPoss.x1y1.x; c.x >= roomPoss.x0y0.x; c.x--) {
//      if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneFloor) {
//        PossToAdd[c.x][c.y] = true;
//        c.x = -9999;
//      }
//    }
//  }
//
//  for(c.x = roomPoss.x0y0.x; c.x <= roomPoss.x1y1.x; c.x++) {
//    for(c.y = roomPoss.x0y0.y; c.y <= roomPoss.x1y1.y; c.y++) {
//      if(PossToAdd[c.x][c.y] == true) {
//        vectorToFill.push_back(c);
//      }
//    }
//  }
//}

// The parameter rectangle does not have to go up-left to bottom-right,
// the method adjusts the order
void MapGenBsp::coverAreaWithFeature(const Rect& area, const Feature_t feature) {
  const Pos x0y0 =
    Pos(min(area.x0y0.x, area.x1y1.x), min(area.x0y0.y, area.x1y1.y));
  const Pos x1y1 =
    Pos(max(area.x0y0.x, area.x1y1.x), max(area.x0y0.y, area.x1y1.y));

  for(int x = x0y0.x; x <= x1y1.x; x++) {
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      eng->featureFactory->spawnFeatureAt(feature, Pos(x, y), NULL);
    }
  }
}

void MapGenBsp::reshapeRoom(const Room& room) {
  const int ROOM_W = room.getX1() - room.getX0() + 1;
  const int ROOM_H = room.getY1() - room.getY0() + 1;

  if(ROOM_W >= 4 && ROOM_H >= 4) {

    vector<RoomReshape_t> reshapesToPerform;
    if(eng->dice.percentile() < 75) {
      reshapesToPerform.push_back(roomReshape_trimCorners);
    }
    if(eng->dice.percentile() < 75) {
      reshapesToPerform.push_back(roomReshape_pillarsRandom);
    }

    for(unsigned int i = 0; i < reshapesToPerform.size(); i++) {
      switch(reshapesToPerform.at(i)) {
        case roomReshape_trimCorners: {
          const int W_DIV =
            3 + (eng->dice.coinToss() ? eng->dice(1, 2) - 1 : 0);
          const int H_DIV =
            3 + (eng->dice.coinToss() ? eng->dice(1, 2) - 1 : 0);

          const int W = max(1, ROOM_W / W_DIV);
          const int H = max(1, ROOM_H / H_DIV);

          const bool TRIM_ALL = false;

          if(TRIM_ALL || eng->dice.coinToss()) {
            const Pos upLeft(room.getX0() + W - 1, room.getY0() + H - 1);
            Rect rect(room.getX0Y0(), upLeft);
            MapGenBsp::coverAreaWithFeature(rect, feature_stoneWall);
          }

          if(TRIM_ALL || eng->dice.coinToss()) {
            const Pos upRight(room.getX1() - W + 1, room.getY0() + H - 1);
            Rect rect(Pos(room.getX0() + ROOM_W - 1, room.getY0()), upRight);
            MapGenBsp::coverAreaWithFeature(rect, feature_stoneWall);
          }

          if(TRIM_ALL || eng->dice.coinToss()) {
            const Pos downLeft(room.getX0() + W - 1, room.getY1() - H + 1);
            Rect rect(Pos(room.getX0(), room.getY0() + ROOM_H - 1), downLeft);
            MapGenBsp::coverAreaWithFeature(rect, feature_stoneWall);
          }

          if(TRIM_ALL || eng->dice.coinToss()) {
            const Pos downRight(room.getX1() - W + 1, room.getY1() - H + 1);
            Rect rect(room.getX1Y1(), downRight);
            MapGenBsp::coverAreaWithFeature(rect, feature_stoneWall);
          }
        }
        break;

        case roomReshape_pillarsRandom: {
          for(int x = room.getX0() + 1; x <= room.getX1() - 1; x++) {
            for(int y = room.getY0() + 1; y <= room.getY1() - 1; y++) {
              Pos c(x + eng->dice(1, 3) - 2, y + eng->dice(1, 3) - 2);
              bool isNextToWall = false;
              for(int dxCheck = -1; dxCheck <= 1; dxCheck++) {
                for(int dyCheck = -1; dyCheck <= 1; dyCheck++) {
                  const FeatureStatic* const f =
                    eng->map->featuresStatic[c.x + dxCheck][c.y + dyCheck];
                  if(f->getId() == feature_stoneWall) {
                    isNextToWall = true;
                  }
                }
              }
              if(isNextToWall == false) {
                if(eng->dice.percentile() < 20) {
                  eng->featureFactory->spawnFeatureAt(feature_stoneWall, c);
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

int MapGenBsp::getNrStepsInDirectionUntilWallFound(
  Pos c, const Direction_t dir) const {

  int stepsTaken = 0;
  bool done = false;
  while(done == false) {
    if(eng->mapTests->isCellInsideMap(c) == false) {
      return -1;
    }
    if(eng->map->featuresStatic[c.x][c.y]->getId() == feature_stoneWall) {
      return stepsTaken;
    }
    c +=
      dir == directionRight ? Pos(1, 0) :
      dir == directionUp    ? Pos(0, -1) :
      dir == directionLeft  ? Pos(-1, 0) :
      dir == directionDown  ? Pos(0, 1) :
      Pos(0, 0);
    stepsTaken++;
  }
  return -1;
}

bool MapGenBsp::isAreaFree(const Rect& area,
                           bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]) {
  return isAreaFree(
           area.x0y0.x, area.x0y0.y, area.x1y1.x, area.x1y1.y, blockingCells);
}

bool MapGenBsp::isAreaFree(const int X0, const int Y0,
                           const int X1, const int Y1,
                           bool blockingCells[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(blockingCells[x][y]) {
        return false;
      }
    }
  }
  return true;
}

void MapGenBsp::buildAuxRooms(Region* regions[3][3]) {
  trace << "MapGenBsp::buildAuxRooms()..." << endl;
  const int NR_TRIES_PER_SIDE = 10;

  for(int regionY = 0; regionY < 3; regionY++) {
    for(int regionX = 0; regionX < 3; regionX++) {

      const int CHANCE_TO_BUILD_AUX_ROOMS = 40;

      if(eng->dice.range(1, 100) < CHANCE_TO_BUILD_AUX_ROOMS) {
        const Region* const region = regions[regionX][regionY];
        const Room* const mainRoom = region->mainRoom;

        if(mainRoom != NULL) {

          bool cellsWithFloor[MAP_X_CELLS][MAP_Y_CELLS];
          eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(
            actorBodyType_normal, cellsWithFloor);
          eng->basicUtils->reverseBoolArray(cellsWithFloor);

          int connectX, connectY, auxRoomW, auxRoomH, auxRoomX, auxRoomY;

          //Right
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = mainRoom->getX1() + 1;
            connectY = eng->dice.range(
                         mainRoom->getY0() + 1, mainRoom->getY1() - 1);
            auxRoomW = eng->dice.range(3, 7);
            auxRoomH = eng->dice.range(3, 7);
            auxRoomX = connectX + 1;
            auxRoomY = eng->dice.range(connectY - auxRoomH + 1, connectY);
            if(cellsWithFloor[connectX - 1][connectY]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGenBsp: Aux room placed right" << endl;
                i = 99999;
              }
            }
          }

          //Up
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = eng->dice.range(
                         mainRoom->getX0() + 1, mainRoom->getX1() - 1);
            connectY = mainRoom->getY0() - 1;
            auxRoomW = eng->dice.range(3, 7);
            auxRoomH = eng->dice.range(3, 7);
            auxRoomX = eng->dice.range(connectX - auxRoomW + 1, connectX);
            auxRoomY = connectY - auxRoomH;
            if(cellsWithFloor[connectX][connectY + 1]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGenBsp: Aux room placed up" << endl;
                i = 99999;
              }
            }
          }

          //Left
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = mainRoom->getX0() - 1;
            connectY = eng->dice.range(
                         mainRoom->getY0() + 1, mainRoom->getY1() - 1);
            auxRoomW = eng->dice.range(3, 7);
            auxRoomH = eng->dice.range(3, 7);
            auxRoomX = connectX - auxRoomW;
            auxRoomY = eng->dice.range(connectY - auxRoomH + 1, connectY);
            if(cellsWithFloor[connectX + 1][connectY]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGenBsp: Aux room placed left" << endl;
                i = 99999;
              }
            }
          }

          //Down
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = eng->dice.range(
                         mainRoom->getX0() + 1, mainRoom->getX1() - 1);
            connectY = mainRoom->getY1() + 1;
            auxRoomW = eng->dice.range(3, 7);
            auxRoomH = eng->dice.range(3, 7);
            auxRoomX = eng->dice.range(connectX - auxRoomW + 1, connectX);
            auxRoomY = connectY + 1;
            if(cellsWithFloor[connectX][connectY - 1]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGenBsp: Aux room placed down" << endl;
                i = 99999;
              }
            }
          }
        }
      }
    }
  }
  trace << "MapGenBsp::buildAuxRooms() [DONE]" << endl;
}

bool MapGenBsp::tryPlaceAuxRoom(const int X0, const int Y0,
                                const int W, const int H,
                                bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
                                const Pos doorPos) {
  Rect auxArea, auxAreaWithWalls;
  auxArea.x0y0.set(X0, Y0);
  auxArea.x1y1.set(X0 + W - 1, Y0 + H - 1);
  auxAreaWithWalls.x0y0.set(auxArea.x0y0 - Pos(1, 1));
  auxAreaWithWalls.x1y1.set(auxArea.x1y1 + Pos(1, 1));
  if(
    isAreaFree(auxAreaWithWalls, blockers) &&
    eng->mapTests->isAreaInsideMap(auxAreaWithWalls)) {
    Room* room = buildRoom(auxArea);
    eng->map->rooms.push_back(room);
    for(int y = auxArea.x0y0.y; y <= auxArea.x1y1.y; y++) {
      for(int x = auxArea.x0y0.x; x <= auxArea.x1y1.x; x++) {
        blockers[x][y] = true;
      }
    }

    const int CHANCE_FOR_CRUMLE_ROOM = 20;

    if(eng->dice.range(1, 100) < CHANCE_FOR_CRUMLE_ROOM) {
      makeCrumbleRoom(auxAreaWithWalls, doorPos);
      //If we're making a "crumble room" we don't want to keep it
      //for applying a theme and such
      deleteAndRemoveRoomFromList(room);
      room = NULL;
    } else {
      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, doorPos);
      globalDoorPositionCandidates[doorPos.x][doorPos.y] = true;
    }

    return true;
  }
  return false;
}

void MapGenBsp::makeCrumbleRoom(const Rect roomAreaIncludingWalls,
                                const Pos proxEventPos) {
  vector<Pos> wallCells;
  vector<Pos> innerCells;

  const Rect a(roomAreaIncludingWalls);

  for(int y = a.x0y0.y; y <= a.x1y1.y; y++) {
    for(int x = a.x0y0.x; x <= a.x1y1.x; x++) {
      if(x == a.x0y0.x || x == a.x1y1.x || y == a.x0y0.y || y == a.x1y1.y) {
        wallCells.push_back(Pos(x, y));
      } else {
        innerCells.push_back(Pos(x, y));
      }
      eng->featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
    }
  }

  ProxEventWallCrumbleSpawnData* const spawnData =
    new ProxEventWallCrumbleSpawnData(wallCells, innerCells);
  eng->featureFactory->spawnFeatureAt(
    feature_proxEventWallCrumble, proxEventPos, spawnData);
}


//=============================================================== REGION
Region::Region(Pos x0y0, Pos x1y1) :
  mainRoom(NULL), isConnected(false), x0y0_(x0y0), x1y1_(x1y1) {
  for(int x = 0; x <= 2; x++) {
    for(int y = 0; y <= 2; y++) {
      regionsConnectedTo[x][y] = false;
    }
  }
}

Region::Region() :
  mainRoom(NULL), isConnected(false), x0y0_(Pos(-1, -1)), x1y1_(Pos(-1, -1)) {
  for(int x = 0; x <= 2; x++) {
    for(int y = 0; y <= 2; y++) {
      regionsConnectedTo[x][y] = false;
    }
  }
}

Region::~Region() {
}

int Region::getNrOfConnections() {
  int nRconnections = 0;
  for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
      if(regionsConnectedTo[x][y]) {
        nRconnections++;
      }
    }
  }
  return nRconnections;
}

bool Region::isRegionNeighbour(const Region& other, Engine* const engine) {
  for(int x = x0y0_.x; x <= x1y1_.x; x++) {
    for(int y = x0y0_.y; y <= x1y1_.y; y++) {
      for(int xx = other.x0y0_.x; xx <= other.x1y1_.x; xx++) {
        for(int yy = other.x0y0_.y; yy <= other.x1y1_.y; yy++) {
          if(engine->mapTests->isCellsAdj(
                Pos(x, y), Pos(xx, yy), false)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

Rect Region::getRandomPossForRoom(Engine* eng) const {
  const bool TINY_ALLOWED_HOR = eng->dice.coinToss();

  const Pos minDim(TINY_ALLOWED_HOR ? 2 : 4, TINY_ALLOWED_HOR ? 4 : 2);
  const Pos maxDim = x1y1_ - x0y0_ - Pos(2, 2);

  const int H = eng->dice.range(minDim.y, maxDim.y);
  const bool ALLOW_BIG_W = H > (maxDim.y * 5) / 6;
  const int W = eng->dice.range(
                  minDim.x, ALLOW_BIG_W ? maxDim.x :
                  minDim.x + (maxDim.x - minDim.x) / 5);

  const Pos dim(W, H);

  const int X0 = x0y0_.x + 1 + eng->dice.range(0, maxDim.x - dim.x);
  const int Y0 = x0y0_.y + 1 + eng->dice.range(0, maxDim.y - dim.y);
  const int X1 = X0 + dim.x - 1;
  const int Y1 = Y0 + dim.y - 1;

  return Rect(Pos(X0, Y0), Pos(X1, Y1));
}



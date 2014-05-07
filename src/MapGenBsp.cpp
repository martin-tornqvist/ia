#include "MapGen.h"

#include <algorithm>
#include <climits>

#include "Init.h"
#include "MapGen.h"
#include "FeatureProxEvent.h"
#include "ActorPlayer.h"
#include "FeatureDoor.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "ItemDrop.h"
#include "ItemFactory.h"
#include "Map.h"
#include "FeatureWall.h"
#include "MapParsing.h"
#include "Renderer.h"
#include "Utils.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"

#ifdef DEMO_MODE
#include "SdlWrapper.h"
#endif // DEMO_MODE

using namespace std;

//============================================================= MAPBUILD-BSP

namespace MapGen {

namespace Bsp {

namespace {

//Used for helping build the map
bool roomCells[MAP_W][MAP_H];
bool regionsToBuildCave[3][3];

bool globalDoorPosBucket[MAP_W][MAP_H];

//Note: The parameter rectangle does not have to go up-left to bottom-right,
//the method adjusts the order
void coverAreaWithFeature(const Rect& area, const FeatureId feature) {
  const Pos x0y0 =
    Pos(min(area.x0y0.x, area.x1y1.x), min(area.x0y0.y, area.x1y1.y));
  const Pos x1y1 =
    Pos(max(area.x0y0.x, area.x1y1.x), max(area.x0y0.y, area.x1y1.y));

  for(int x = x0y0.x; x <= x1y1.x; x++) {
    for(int y = x0y0.y; y <= x1y1.y; y++) {
      FeatureFactory::spawn(feature, Pos(x, y), NULL);

#ifdef DEMO_MODE
      Renderer::drawMapAndInterface();
      SdlWrapper::sleep(1);
#endif // DEMO_MODE
    }
  }

#ifdef DEMO_MODE
  SdlWrapper::sleep(2000);
#endif // DEMO_MODE
}

bool isAreaFree(const int X0, const int Y0, const int X1, const int Y1,
                bool blockingCells[MAP_W][MAP_H]) {
  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(blockingCells[x][y]) {return false;}
    }
  }
  return true;
}

bool isAreaFree(const Rect& a, bool blockingCells[MAP_W][MAP_H]) {
  return isAreaFree(a.x0y0.x, a.x0y0.y, a.x1y1.x, a.x1y1.y, blockingCells);
}

int getNrStepsInDirUntilWallFound(Pos c, const Dir dir) {
  int nrSteps = 0;
  bool isDone = false;
  while(isDone == false) {
    if(Utils::isPosInsideMap(c) == false) {return -1;}

    if(Map::cells[c.x][c.y].featureStatic->getId() == FeatureId::wall) {
      return nrSteps;
    }

    c += DirUtils::getOffset(dir);

    nrSteps++;
  }
  return -1;
}

bool isAllRoomsConnected() {
  Pos c;
  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      c.set(x, y);
      const FeatureStatic* const f = Map::cells[c.x][c.y].featureStatic;
      if(f->getId() == FeatureId::floor) {goto stop;}
    }
  }
  stop:

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);
  int floodFill[MAP_W][MAP_H];
  FloodFill::run(c, blockers, floodFill, INT_MAX, Pos(-1, -1));

  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(Map::cells[x][y].featureStatic->getId() == FeatureId::floor) {
        if(Pos(x, y) != c && floodFill[x][y] == 0) {return false;}
      }
    }
  }
  return true;
}

Room* buildRoom(const Rect& roomRect) {
  coverAreaWithFeature(roomRect, FeatureId::floor);
  for(int y = roomRect.x0y0.y; y <= roomRect.x1y1.y; y++) {
    for(int x = roomRect.x0y0.x; x <= roomRect.x1y1.x; x++) {
      roomCells[x][y] = true;
    }
  }

  return new Room(roomRect);
}

void makeCrumbleRoom(const Rect& roomAreaInclWalls, const Pos& proxEventPos) {
  vector<Pos> wallCells;
  vector<Pos> innerCells;

  const Rect& a = roomAreaInclWalls; //abbreviation

  for(int y = a.x0y0.y; y <= a.x1y1.y; y++) {
    for(int x = a.x0y0.x; x <= a.x1y1.x; x++) {
      if(x == a.x0y0.x || x == a.x1y1.x || y == a.x0y0.y || y == a.x1y1.y) {
        wallCells.push_back(Pos(x, y));
      } else {
        innerCells.push_back(Pos(x, y));
      }
      FeatureFactory::spawn(FeatureId::wall, Pos(x, y));
    }
  }

  ProxEventWallCrumbleSpawnData* const spawnData =
    new ProxEventWallCrumbleSpawnData(wallCells, innerCells);
  FeatureFactory::spawn(
    FeatureId::proxEventWallCrumble, proxEventPos, spawnData);
}

void connectRegions(Region* regions[3][3]) {
  trace << "MapGen::Bsp::connectRegions()..." << endl;

  bool isAllConnected = false;
  while(isAllConnected == false) {
    isAllConnected = isAllRoomsConnected();

    Pos c1(Rnd::range(0, 2), Rnd::range(0, 2));
    Region* r1 = regions[c1.x][c1.y];

    Pos delta(0, 0);
    bool isDeltaOk = false;
    while(isDeltaOk == false) {
      delta.set(Rnd::range(-1, 1), Rnd::range(-1, 1));
      Pos c2(c1 + delta);
      const bool IS_INSIDE_BOUNDS = Utils::isPosInside(c2, Rect(0, 0, 2, 2));
      const bool IS_ZERO_DELTA  = delta.x == 0 && delta.y == 0;
      const bool IS_DIAGONAL    = delta.x != 0 && delta.y != 0;
      isDeltaOk = IS_ZERO_DELTA     == false &&
                  IS_DIAGONAL       == false &&
                  IS_INSIDE_BOUNDS  == true;
    }
    Pos c2(c1 + delta);

    Region* const r2 = regions[c2.x][c2.y];

    if(r1->regionsConnectedTo[c2.x][c2.y] == false) {
      const Dir regionDir = DirUtils::getDir(c2 - c1);

      MapGenUtils::buildZCorridorBetweenRooms(
        *(r1->mainRoom), *(r2->mainRoom), regionDir, globalDoorPosBucket);

#ifdef DEMO_MODE
      Renderer::drawMapAndInterface();
      SdlWrapper::sleep(1000);
#endif // DEMO_MODE

      r1->regionsConnectedTo[c2.x][c2.y] = true;
      r2->regionsConnectedTo[c1.x][c1.y] = true;
    }
  }
  trace << "MapGen::Bsp::connectRegions()[DONE]" << endl;
}

//TODO Should be in Map
void deleteAndRemoveRoomFromList(Room* const room) {
  for(size_t i = 0; i < Map::rooms.size(); i++) {
    if(Map::rooms.at(i) == room) {
      delete room;
      Map::rooms.erase(Map::rooms.begin() + i);
      return;
    }
  }
  assert(false && "Tried to remove non-existing room");
}

bool tryPlaceAuxRoom(const int X0, const int Y0, const int W, const int H,
                     bool blockers[MAP_W][MAP_H], const Pos& doorPos) {
  Rect auxArea, auxAreaWithWalls;
  auxArea.x0y0.set(X0, Y0);
  auxArea.x1y1.set(X0 + W - 1, Y0 + H - 1);
  auxAreaWithWalls.x0y0.set(auxArea.x0y0 - Pos(1, 1));
  auxAreaWithWalls.x1y1.set(auxArea.x1y1 + Pos(1, 1));
  if(
    isAreaFree(auxAreaWithWalls, blockers) &&
    Utils::isAreaInsideMap(auxAreaWithWalls)) {
    Room* room = buildRoom(auxArea);
    Map::rooms.push_back(room);
    for(int y = auxArea.x0y0.y; y <= auxArea.x1y1.y; y++) {
      for(int x = auxArea.x0y0.x; x <= auxArea.x1y1.x; x++) {
        blockers[x][y] = true;
      }
    }

    const int CHANCE_FOR_CRUMLE_ROOM = 20;

    if(Rnd::range(1, 100) < CHANCE_FOR_CRUMLE_ROOM) {
      makeCrumbleRoom(auxAreaWithWalls, doorPos);
      //If we're making a "crumble room" we don't want to keep it
      //for applying a theme and such
      deleteAndRemoveRoomFromList(room);
      room = NULL;
    } else {
      FeatureFactory::spawn(FeatureId::floor, doorPos);
#ifdef DEMO_MODE
      Renderer::drawMapAndInterface();
      SdlWrapper::sleep(1000);
#endif // DEMO_MODE
      globalDoorPosBucket[doorPos.x][doorPos.y] = true;
    }

    return true;
  }
  return false;
}

void buildAuxRooms(Region* regions[3][3]) {
  trace << "MapGen::Bsp::buildAuxRooms()..." << endl;
  const int NR_TRIES_PER_SIDE = 10;

  for(int regionY = 0; regionY < 3; regionY++) {
    for(int regionX = 0; regionX < 3; regionX++) {

      if(Rnd::oneIn(3)) {
        const Region* const region    = regions[regionX][regionY];
        const Room*   const mainRoom  = region->mainRoom;

        if(mainRoom != NULL) {

          bool cellsWithFloor[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksMoveCmn(false), cellsWithFloor);

          Utils::reverseBoolArray(cellsWithFloor);

          int connectX, connectY, auxRoomW, auxRoomH, auxRoomX, auxRoomY;

          //Right
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = mainRoom->getX1() + 1;
            connectY = Rnd::range(mainRoom->getY0() + 1, mainRoom->getY1() - 1);
            auxRoomW = Rnd::range(3, 7);
            auxRoomH = Rnd::range(3, 7);
            auxRoomX = connectX + 1;
            auxRoomY = Rnd::range(connectY - auxRoomH + 1, connectY);
            if(cellsWithFloor[connectX - 1][connectY]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGen::Bsp: Aux room placed right" << endl;
                break;
              }
            }
          }

          //Up
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = Rnd::range(mainRoom->getX0() + 1, mainRoom->getX1() - 1);
            connectY = mainRoom->getY0() - 1;
            auxRoomW = Rnd::range(3, 7);
            auxRoomH = Rnd::range(3, 7);
            auxRoomX = Rnd::range(connectX - auxRoomW + 1, connectX);
            auxRoomY = connectY - auxRoomH;
            if(cellsWithFloor[connectX][connectY + 1]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGen::Bsp: Aux room placed up" << endl;
                break;
              }
            }
          }

          //Left
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = mainRoom->getX0() - 1;
            connectY = Rnd::range(mainRoom->getY0() + 1, mainRoom->getY1() - 1);
            auxRoomW = Rnd::range(3, 7);
            auxRoomH = Rnd::range(3, 7);
            auxRoomX = connectX - auxRoomW;
            auxRoomY = Rnd::range(connectY - auxRoomH + 1, connectY);
            if(cellsWithFloor[connectX + 1][connectY]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGen::Bsp: Aux room placed left" << endl;
                break;
              }
            }
          }

          //Down
          for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
            connectX = Rnd::range(mainRoom->getX0() + 1, mainRoom->getX1() - 1);
            connectY = mainRoom->getY1() + 1;
            auxRoomW = Rnd::range(3, 7);
            auxRoomH = Rnd::range(3, 7);
            auxRoomX = Rnd::range(connectX - auxRoomW + 1, connectX);
            auxRoomY = connectY + 1;
            if(cellsWithFloor[connectX][connectY - 1]) {
              Pos c(connectX, connectY);
              if(
                tryPlaceAuxRoom(
                  auxRoomX, auxRoomY, auxRoomW, auxRoomH, cellsWithFloor, c)) {
                trace << "MapGen::Bsp: Aux room placed down" << endl;
                break;
              }
            }
          }
        }
      }
    }
  }
#ifdef DEMO_MODE
  Renderer::drawMapAndInterface();
  SdlWrapper::sleep(2000);
#endif // DEMO_MODE

  trace << "MapGen::Bsp::buildAuxRooms() [DONE]" << endl;
}

void reshapeRoom(const Room& room) {
  const int ROOM_W = room.getX1() - room.getX0() + 1;
  const int ROOM_H = room.getY1() - room.getY0() + 1;

  if(ROOM_W >= 4 && ROOM_H >= 4) {

    vector<RoomReshapeType> reshapesToPerform;
    if(Rnd::fraction(3, 4)) {
      reshapesToPerform.push_back(RoomReshapeType::trimCorners);
    }
    if(Rnd::fraction(3, 4)) {
      reshapesToPerform.push_back(RoomReshapeType::pillarsRandom);
    }

    for(RoomReshapeType reshapeType : reshapesToPerform) {
      switch(reshapeType) {
        case RoomReshapeType::trimCorners: {
          const int W_DIV = 3 + (Rnd::coinToss() ? Rnd::range(0, 1) : 0);
          const int H_DIV = 3 + (Rnd::coinToss() ? Rnd::range(0, 1) : 0);

          const int W = max(1, ROOM_W / W_DIV);
          const int H = max(1, ROOM_H / H_DIV);

          const bool TRIM_ALL = false;

          if(TRIM_ALL || Rnd::coinToss()) {
            const Pos upLeft(room.getX0() + W - 1, room.getY0() + H - 1);
            Rect rect(room.getX0Y0(), upLeft);
            coverAreaWithFeature(rect, FeatureId::wall);
          }

          if(TRIM_ALL || Rnd::coinToss()) {
            const Pos upRight(room.getX1() - W + 1, room.getY0() + H - 1);
            Rect rect(Pos(room.getX0() + ROOM_W - 1, room.getY0()), upRight);
            coverAreaWithFeature(rect, FeatureId::wall);
          }

          if(TRIM_ALL || Rnd::coinToss()) {
            const Pos downLeft(room.getX0() + W - 1, room.getY1() - H + 1);
            Rect rect(Pos(room.getX0(), room.getY0() + ROOM_H - 1), downLeft);
            coverAreaWithFeature(rect, FeatureId::wall);
          }

          if(TRIM_ALL || Rnd::coinToss()) {
            const Pos downRight(room.getX1() - W + 1, room.getY1() - H + 1);
            Rect rect(room.getX1Y1(), downRight);
            coverAreaWithFeature(rect, FeatureId::wall);
          }
        }
        break;

        case RoomReshapeType::pillarsRandom: {
          for(int x = room.getX0() + 1; x <= room.getX1() - 1; x++) {
            for(int y = room.getY0() + 1; y <= room.getY1() - 1; y++) {
              Pos c(x + Rnd::dice(1, 3) - 2, y + Rnd::dice(1, 3) - 2);
              bool isNextToWall = false;
              for(int dx = -1; dx <= 1; dx++) {
                for(int dy = -1; dy <= 1; dy++) {
                  const FeatureStatic* const f =
                    Map::cells[c.x + dx][c.y + dy].featureStatic;
                  if(f->getId() == FeatureId::wall) {
                    isNextToWall = true;
                  }
                }
              }
              if(isNextToWall == false) {
                if(Rnd::oneIn(5)) {
                  FeatureFactory::spawn(FeatureId::wall, c);
                }
              }
            }
          }
        }
        break;
      }
    }
  }

#ifdef DEMO_MODE
  Renderer::drawMapAndInterface();
  SdlWrapper::sleep(2000);
#endif // DEMO_MODE
}

void buildMergedRegionsAndRooms(Region* regions[3][3],
                                const int SPL_X1, const int SPL_X2,
                                const int SPL_Y1, const int SPL_Y2) {

  const int NR_OF_MERGED_REGIONS_TO_ATTEMPT = Rnd::range(0, 2);

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

      regionIndex1 = Pos(Rnd::range(0, 2), Rnd::range(0, 1));
      regionIndex2 = Pos(regionIndex1 + Pos(0, 1));
      isGoodRegionsFound =
        regions[regionIndex1.x][regionIndex1.y] == NULL &&
        regions[regionIndex2.x][regionIndex2.y] == NULL;
    }

    const int MERGED_X0 =
      regionIndex1.x == 0 ? 0 : regionIndex1.x == 1 ? SPL_X1 : SPL_X2;
    const int MERGED_Y0 =
      regionIndex1.y == 0 ? 0 : regionIndex1.y == 1 ? SPL_Y1 : SPL_Y2;
    const int MERGED_X1 =
      regionIndex2.x == 0 ? SPL_X1 - 1 :
      regionIndex2.x == 1 ? SPL_X2 - 1 : MAP_W - 1;
    const int MERGED_Y1 =
      regionIndex2.y == 0 ? SPL_Y1 - 1 :
      regionIndex2.y == 1 ? SPL_Y2 - 1 : MAP_H - 1;

#ifdef DEMO_MODE
    const int P_W = Config::getCellW();
    const int P_H = Config::getCellH();
    const int P_O = Config::mainscreenOffsetY;
    Pos dims((MERGED_X1 - MERGED_X0) * P_W, (MERGED_Y1 - MERGED_Y0) * P_H);
    Renderer::drawRectangleSolid(
      Pos(MERGED_X0 * P_W, P_O + MERGED_Y0 * P_H), dims,
      clrBlueLgt);
    Renderer::updateScreen();
    SdlWrapper::sleep(500);
#endif // DEMO_MODE

    const int AREA_2_X0 =
      regionIndex2.x == 0 ? 0 : regionIndex2.x == 1 ? SPL_X1 : SPL_X2;
    const int AREA_2_Y0 =
      regionIndex2.y == 0 ? 0 : regionIndex2.y == 1 ? SPL_Y1 : SPL_Y2;
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

    const int OFFSET_X0 = Rnd::range(1, 4);
    const int OFFSET_Y0 = Rnd::range(1, 4);
    const int OFFSET_X1 = Rnd::range(1, 4);
    const int OFFSET_Y1 = Rnd::range(1, 4);
    Rect roomRect(area1.x0y0 + Pos(OFFSET_X0, OFFSET_Y0),
                  area2.x1y1 - Pos(OFFSET_X1, OFFSET_Y1));
    Room* const room = buildRoom(roomRect);
    Map::rooms.push_back(room);

    region1->mainRoom = region2->mainRoom = room;

    region1->regionsConnectedTo[regionIndex2.x][regionIndex2.y] = true;
    region2->regionsConnectedTo[regionIndex1.x][regionIndex1.y] = true;

    if(Rnd::oneIn(3)) {reshapeRoom(*room);}
  }
}

void buildCaves(Region* regions[3][3]) {
  trace << "MapGen::Bsp::buildCaves()..." << endl;
  for(int regY = 0; regY <= 2; regY++) {
    for(int regX = 0; regX <= 2; regX++) {

      if(regionsToBuildCave[regX][regY]) {

        Region* const region = regions[regX][regY];

        //This region no longer has a room, delete it from list
        deleteAndRemoveRoomFromList(region->mainRoom);
        region->mainRoom = NULL;

        bool blockers[MAP_W][MAP_H];

        for(int y = 0; y < MAP_H; y++) {
          for(int x = 0; x < MAP_W; x++) {

            blockers[x][y] = false;

            if(
              x == 0 || y == 0 ||
              x == MAP_W - 1 || y == MAP_H - 1) {
              blockers[x][y] = true;
            } else {
              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  const FeatureId featureId =
                    Map::cells[x + dx][y + dy].featureStatic->getId();
                  const bool IS_FLOOR = featureId == FeatureId::floor ||
                                        featureId == FeatureId::caveFloor;
                  if(
                    IS_FLOOR &&
                    Utils::isPosInside(
                      Pos(x + dx, y + dy), region->getRegionRect()) == false) {
                    blockers[x][y] = true;
                  }
                }
              }
            }
          }
        }

        const Pos origin(region->getX0Y0() + Pos(1, 1));
        int floodFillResult[MAP_W][MAP_H];

        const int FLOOD_FILL_TRAVEL_LIMIT = 20;

        FloodFill::run(origin, blockers, floodFillResult,
                       FLOOD_FILL_TRAVEL_LIMIT, Pos(-1, -1));

        for(int y = 1; y < MAP_H - 1; y++) {
          for(int x = 1; x < MAP_W - 1; x++) {
            const Pos c(x, y);
            if(c == origin || floodFillResult[x][y] > 0) {

              FeatureFactory::spawn(FeatureId::caveFloor, c);
#ifdef DEMO_MODE
              Renderer::drawMapAndInterface();
              SdlWrapper::sleep(5);
#endif // DEMO_MODE

              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  Cell& adjCell = Map::cells[x + dx][y + dy];
                  if(adjCell.featureStatic->getId() == FeatureId::wall) {
                    FeatureFactory::spawn(
                      FeatureId::wall, c + Pos(dx , dy));
                    Wall* const wall =
                      dynamic_cast<Wall*>(adjCell.featureStatic);
                    wall->wallType = WallType::cave;
                    wall->setRandomIsMossGrown();
#ifdef DEMO_MODE
                    Renderer::drawMapAndInterface();
                    SdlWrapper::sleep(5);
#endif // DEMO_MODE
                  }
                }
              }
            }
          }
        }

        const int CHANCE_TO_MAKE_CHASM = 25;

        if(Rnd::percentile() < CHANCE_TO_MAKE_CHASM) {
          Utils::resetArray(blockers, false);

          for(int y = 1; y < MAP_H - 1; y++) {
            for(int x = 1; x < MAP_W - 1; x++) {
              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  Cell& adjCell = Map::cells[x + dx][y + dy];
                  if(adjCell.featureStatic->getId() == FeatureId::wall) {
                    blockers[x][y] = blockers[x + dx][y + dy] = true;
                  }
                }
              }
            }
          }

          FloodFill::run(origin, blockers, floodFillResult,
                         FLOOD_FILL_TRAVEL_LIMIT / 2, Pos(-1, -1));

          for(int y = 1; y < MAP_H - 1; y++) {
            for(int x = 1; x < MAP_W - 1; x++) {
              const Pos c(x, y);
              if(
                blockers[x][y] == false &&
                (c == origin || floodFillResult[x][y] > 0)) {
                FeatureFactory::spawn(FeatureId::chasm, c);
#ifdef DEMO_MODE
                Renderer::drawMapAndInterface();
                SdlWrapper::sleep(5);
#endif // DEMO_MODE
              }
            }
          }
        }
      }
    }
  }
#ifdef DEMO_MODE
  SdlWrapper::sleep(2000);
#endif // DEMO_MODE
  trace << "MapGen::Bsp::buildCaves()[DONE]" << endl;
}

void placeDoorAtPosIfSuitable(const Pos& p) {
  //Check that no other doors are within a certain distance
  const int R = 2;
  for(int dx = -R; dx <= R; dx++) {
    for(int dy = -R; dy <= R; dy++) {
      const Pos checkPos = p + Pos(dx, dy);
      if((dx != 0 || dy != 0) && Utils::isPosInsideMap(checkPos)) {
        const Cell& cell = Map::cells[checkPos.x][checkPos.y];
        if(cell.featureStatic->getId() == FeatureId::door) {return;}
      }
    }
  }

  bool isGoodVer = true;
  bool isGoodHor = true;

  for(int d = -1; d <= 1; d++) {
    if(Map::cells[p.x + d][p.y].featureStatic->getId() == FeatureId::wall) {
      isGoodHor = false;
    }

    if(Map::cells[p.x][p.y + d].featureStatic->getId() == FeatureId::wall) {
      isGoodVer = false;
    }

    if(d != 0) {
      if(Map::cells[p.x][p.y + d].featureStatic->getId() != FeatureId::wall) {
        isGoodHor = false;
      }

      if(Map::cells[p.x + d][p.y].featureStatic->getId() != FeatureId::wall) {
        isGoodVer = false;
      }
    }
  }

  if(isGoodHor || isGoodVer) {
    const FeatureDataT* const mimicData = FeatureData::getData(FeatureId::wall);
    FeatureFactory::spawn(FeatureId::door, p,
                          new DoorSpawnData(mimicData));
#ifdef DEMO_MODE
    Renderer::drawMapAndInterface();
    SdlWrapper::sleep(1000);
#endif // DEMO_MODE
  }
}

void buildRoomsInRooms() {
  const int NR_OF_TRIES         = 40;
  const int MAX_NR_INNER_ROOMS  = 7;
  const int MIN_DIM_W           = 4;
  const int MIN_DIM_H           = 4;

  for(size_t i = 0; i < Map::rooms.size(); i++) {

    const Pos roomX0Y0 = Map::rooms.at(i)->getX0Y0();
    const Pos roomX1Y1 = Map::rooms.at(i)->getX1Y1();

    const int ROOM_WI = roomX1Y1.x - roomX0Y0.x + 1;
    const int ROOM_HE = roomX1Y1.y - roomX0Y0.y + 1;

    const bool IS_ROOM_BIG = ROOM_WI > 16 || ROOM_HE > 8;

    if(IS_ROOM_BIG || Rnd::percentile() < 30) {
      const int MAX_DIM_W = min(16, ROOM_WI);
      const int MAX_DIM_H = min(16, ROOM_HE);

      if(MAX_DIM_W >= MIN_DIM_W && MAX_DIM_H >= MIN_DIM_H) {

        for(
          int nrRoomsCount = 0;
          nrRoomsCount < MAX_NR_INNER_ROOMS;
          nrRoomsCount++) {
          for(int tryCount = 0; tryCount < NR_OF_TRIES; tryCount++) {

            const int W = Rnd::range(MIN_DIM_W, MAX_DIM_W);
            const int H = Rnd::range(MIN_DIM_H, MAX_DIM_H);

            const int X0 = Rnd::range(roomX0Y0.x - 1, roomX1Y1.x - W + 2);
            const int Y0 = Rnd::range(roomX0Y0.y - 1, roomX1Y1.y - H + 2);
            const int X1 = X0 + W - 1;
            const int Y1 = Y0 + H - 1;

            bool isSpaceFree = true;

            for(int y = Y0 - 1; y <= Y1 + 1; y++) {
              for(int x = X0 - 1; x <= X1 + 1; x++) {
                if(
                  Utils::isPosInside(
                    Pos(x, y), Rect(roomX0Y0 - Pos(1, 1),
                                    roomX1Y1 + Pos(1, 1)))) {
                  if(
                    x == roomX0Y0.x - 1 || x == roomX1Y1.x + 1 ||
                    y == roomX0Y0.y - 1 || y == roomX1Y1.y + 1) {
                    if(
                      Map::cells[x][y].featureStatic->getId() !=
                      FeatureId::wall) {
                      isSpaceFree = false;
                    }
                  } else {
                    if(
                      Map::cells[x][y].featureStatic->getId() !=
                      FeatureId::floor) {
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
              vector<Pos> doorBucket;
              Map::rooms.push_back(
                new Room(Rect(Pos(X0 + 1, Y0 + 1), Pos(X1 - 1, Y1 - 1))));
              for(int y = Y0; y <= Y1; y++) {
                for(int x = X0; x <= X1; x++) {
                  if(x == X0 || x == X1 || y == Y0 || y == Y1) {

                    FeatureFactory::spawn(
                      FeatureId::wall, Pos(x, y));

                    if(
                      x != roomX0Y0.x - 1 && x != roomX0Y0.x      &&
                      x != roomX1Y1.x     && x != roomX1Y1.x + 1  &&
                      y != roomX0Y0.y - 1 && y != roomX0Y0.y      &&
                      y != roomX1Y1.y     && y != roomX1Y1.y + 1) {
                      if((x != X0 && x != X1) || (y != Y0 && y != Y1)) {
                        doorBucket.push_back(Pos(x, y));
                      }
                    }
                  }
                }
              }
              if(Rnd::coinToss() || doorBucket.size() <= 2) {
                const int DOOR_POS_ELEMENT =
                  Rnd::range(0, doorBucket.size() - 1);
                const Pos doorPos = doorBucket.at(DOOR_POS_ELEMENT);
                FeatureFactory::spawn(FeatureId::floor, doorPos);
                globalDoorPosBucket[doorPos.x][doorPos.y] = true;
              } else {
                vector<Pos> positionsWithDoor;
                const int NR_TRIES = Rnd::range(1, 10);
                for(int j = 0; j < NR_TRIES; j++) {
                  const int DOOR_POS_ELEMENT =
                    Rnd::range(0, doorBucket.size() - 1);
                  const Pos posCand = doorBucket.at(DOOR_POS_ELEMENT);

                  bool isPosOk = true;
                  for(Pos & posWithDoor : positionsWithDoor) {
                    if(Utils::isPosAdj(posCand, posWithDoor, false)) {
                      isPosOk = false;
                      break;
                    }
                  }
                  if(isPosOk) {
                    FeatureFactory::spawn(FeatureId::floor, posCand);
                    positionsWithDoor.push_back(posCand);
                  }
                }
              }
#ifdef DEMO_MODE
              Renderer::drawMapAndInterface();
              SdlWrapper::sleep(2000);
#endif // DEMO_MODE
              break;
            }
          }
        }
      }
    }
  }
}

void postProcessFillDeadEnds() {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);

  //Find an origin with no adjacent walls, to ensure not starting in a dead end
  Pos origin;
  for(int y = 2; y < MAP_H - 2; y++) {
    for(int x = 2; x < MAP_W - 2; x++) {
      if(isAreaFree(x - 1, y - 1, x + 1, y + 1, blockers)) {
        origin = Pos(x, y);
        goto stop;
      }
    }
  }
  stop:

  //Floodfill from origin, then sort the positions for flood value
  int floodFill[MAP_W][MAP_H];
  FloodFill::run(origin, blockers, floodFill, INT_MAX, Pos(-1, -1));
  vector<PosAndVal> floodFillVector;
  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(blockers[x][y] == false) {
        floodFillVector.push_back(PosAndVal(Pos(x, y), floodFill[x][y]));
      }
    }
  }

  std::sort(floodFillVector.begin(), floodFillVector.end(),
  [](const PosAndVal & a, const PosAndVal & b) {return a.val < b.val;});

  //Fill all positions with only one cardinal floor neighbour
  for(int i = int(floodFillVector.size()) - 1; i >= 0; i--) {
    const Pos& pos = floodFillVector.at(i).pos;
    const int x = pos.x;
    const int y = pos.y;
    const int NR_ADJ_CARDINAL_WALLS = blockers[x + 1][y] + blockers[x - 1][y] +
                                      blockers[x][y + 1] + blockers[x][y - 1];
    if(NR_ADJ_CARDINAL_WALLS == 3) {
      FeatureFactory::spawn(FeatureId::wall, pos);
      blockers[x][y] = true;
    }
  }
}

//void findEdgesOfRoom(const Rect roomRect, vector<Pos>& vectorRef) {
//  bool PossToAdd[MAP_W][MAP_H];
//  Utils::resetArray(PossToAdd, false);
//
//  Pos c;
//
//  //Top to bottom
//  for(c.x = roomRect.x0y0.x; c.x <= roomRect.x1y1.x; c.x++) {
//    for(c.y = roomRect.x0y0.y; c.y <= roomRect.x1y1.y; c.y++) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.y = INT_MAX;
//      }
//    }
//  }
//  //Left to right
//  for(c.y = roomRect.x0y0.y; c.y <= roomRect.x1y1.y; c.y++) {
//    for(c.x = roomRect.x0y0.x; c.x <= roomRect.x1y1.x; c.x++) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.x = INT_MAX;
//      }
//    }
//  }
//  //Bottom to top
//  for(c.x = roomRect.x0y0.x; c.x <= roomRect.x1y1.x; c.x++) {
//    for(c.y = roomRect.x1y1.y; c.y >= roomRect.x0y0.y; c.y--) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.y = INT_MIN;
//      }
//    }
//  }
//  //Right to left
//  for(c.y = roomRect.x0y0.y; c.y <= roomRect.x1y1.y; c.y++) {
//    for(c.x = roomRect.x1y1.x; c.x >= roomRect.x0y0.x; c.x--) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.x = INT_MIN;
//      }
//    }
//  }
//
//  for(c.x = roomRect.x0y0.x; c.x <= roomRect.x1y1.x; c.x++) {
//    for(c.y = roomRect.x0y0.y; c.y <= roomRect.x1y1.y; c.y++) {
//      if(PossToAdd[c.x][c.y] == true) {
//        vectorRef.push_back(c);
//      }
//    }
//  }
//}

bool isRegionFoundInCardinalDir(const Pos& pos, bool region[MAP_W][MAP_H]) {
  for(Pos offset : DirUtils::cardinalOffsets) {
    if(region[pos.x + offset.x][pos.y + offset.y]) {return true;}
  }
  return false;
}

void decorate() {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(Map::cells[x][y].featureStatic->getId() == FeatureId::wall) {

        int nrAdjFloor = 0;

        for(int yy = max(0, y - 1); yy <= min(MAP_H - 1, y + 1); yy++) {
          for(int xx = max(0, x - 1); xx <= min(MAP_W - 1, x + 1); xx++) {
            if(xx != x || yy != y) {
              const FeatureStatic* const f = Map::cells[xx][yy].featureStatic;
              if(f->getId() == FeatureId::floor) {nrAdjFloor++;}
            }
          }
        }

        //Randomly convert walls to rubble
        if(Rnd::oneIn(10)) {
          FeatureFactory::spawn(FeatureId::rubbleHigh, Pos(x, y));
#ifdef DEMO_MODE
          Renderer::drawMapAndInterface();
          SdlWrapper::sleep(1);
#endif // DEMO_MODE
          continue;
        }

        //Moss grown walls
        FeatureStatic* const f = Map::cells[x][y].featureStatic;
        Wall* const wall = dynamic_cast<Wall*>(f);
        wall->setRandomIsMossGrown();
#ifdef DEMO_MODE
        Renderer::drawMapAndInterface();
        SdlWrapper::sleep(1);
#endif // DEMO_MODE

        //Convert walls with no adjacent stone floor to cave walls
        if(nrAdjFloor == 0) {
          wall->wallType = WallType::cave;
        } else {
          wall->setRandomNormalWall();
#ifdef DEMO_MODE
          Renderer::drawMapAndInterface();
          SdlWrapper::sleep(1);
#endif // DEMO_MODE
        }
      }
    }
  }

  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(Map::cells[x][y].featureStatic->getId() == FeatureId::floor) {
        //Randomly convert stone floor to low rubble
        if(Rnd::oneIn(100)) {
          FeatureFactory::spawn(FeatureId::rubbleLow, Pos(x, y));
#ifdef DEMO_MODE
          Renderer::drawMapAndInterface();
          SdlWrapper::sleep(1);
#endif // DEMO_MODE
          continue;
        }
      }
    }
  }
#ifdef DEMO_MODE
  SdlWrapper::sleep(2000);
#endif // DEMO_MODE
}

void getAllowedStairCells(bool cellsToSet[MAP_W][MAP_H]) {
  trace << "MapGen::Bsp::getAllowedStairCells()..." << endl;

  //Stairs are only allowed in cells with, and completely surrounded by,
  //stone floor or cave floor

  vector<FeatureId> featIdsOk {FeatureId::floor, FeatureId::caveFloor};

  MapParse::parse(CellPred::AllAdjIsAnyOfFeatures(featIdsOk), cellsToSet);

  trace << "MapGen::Bsp::getAllowedStairCells() [DONE]" << endl;
}

Pos placeStairs() {
  trace << "MapGen::Bsp::placeStairs()..." << endl;

  bool allowedCells[MAP_W][MAP_H];
  getAllowedStairCells(allowedCells);

  vector<Pos> allowedCellsList;
  Utils::makeVectorFromBoolMap(true, allowedCells, allowedCellsList);

  const int NR_OK_CELLS = allowedCellsList.size();

  const int MIN_NR_OK_CELLS_REQ = 80;

  if(NR_OK_CELLS < MIN_NR_OK_CELLS_REQ) {
    trace << "MapGen::Bsp: Number of allowed cells too low ";
    trace << "(" << NR_OK_CELLS << "), discarding map" << endl;
    return Pos(-1, -1);
  }

  trace << "MapGen::Bsp: Sorting the allowed cells vector ";
  trace << "(" << allowedCellsList.size() << " cells)" << endl;
  IsCloserToOrigin isCloserToOrigin(Map::player->pos);
  sort(allowedCellsList.begin(), allowedCellsList.end(), isCloserToOrigin);

  trace << "MapGen::Bsp: Picking random cell from furthest half" << endl;
  const int ELEMENT = Rnd::range(NR_OK_CELLS / 2, NR_OK_CELLS - 1);
  const Pos stairsPos(allowedCellsList.at(ELEMENT));

  trace << "MapGen::Bsp: Spawning stairs at chosen cell" << endl;
  Feature* f = FeatureFactory::spawn(FeatureId::stairs, stairsPos);
  f->setHasBlood(false);

  trace << "MapGen::Bsp::placeStairs()[DONE]" << endl;
  return stairsPos;
}

void movePlayerToNearestAllowedPos() {
  trace << "MapGen::Bsp::movePlayerToNearestAllowedPos()..." << endl;

  bool allowedCells[MAP_W][MAP_H];
  getAllowedStairCells(allowedCells);

  vector<Pos> allowedCellsList;
  Utils::makeVectorFromBoolMap(true, allowedCells, allowedCellsList);

  assert(allowedCellsList.empty() == false);

  trace << "MapGen::Bsp: Sorting the allowed cells vector ";
  trace << "(" << allowedCellsList.size() << " cells)" << endl;
  IsCloserToOrigin isCloserToOrigin(Map::player->pos);
  sort(allowedCellsList.begin(), allowedCellsList.end(), isCloserToOrigin);

  Map::player->pos = allowedCellsList.front();

  trace << "MapGen::Bsp::movePlayerToNearestAllowedPos() [DONE]" << endl;
}

//void makeLevers() {
//  trace << "MapGen::Bsp::makeLeverPuzzle()..." << endl;
//
//  trace << "MapGen::Bsp: Picking a random door" << endl;
//  vector<Door*> doorBucket;
//  for(int y = 1; y < MAP_H - 1; y++) {
//    for(int x = 1; x < MAP_W - 1; x++) {
//      Feature* const feature = Map::featuresStatic[x][y];
//      if(feature->getId() == FeatureId::door) {
//        Door* const door = dynamic_cast<Door*>(feature);
//        doorBucket.push_back(door);
//      }
//    }
//  }
//  Door* const doorToLink = doorBucket.at(Rnd::range(0, doorBucket.size() - 1));
//
//  trace << "MapGen::Bsp: Making floodfill and keeping only positions with lower value than the door" << endl;
//  bool blockers[MAP_W][MAP_H];
//  eng.mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(bodyType_normal, blockers);
//  for(int y = 1; y < MAP_H - 1; y++) {
//    for(int x = 1; x < MAP_W - 1; x++) {
//      Feature* const feature = Map::featuresStatic[x][y];
//      if(feature->getId() == FeatureId::door) {
//        blockers[x][y] = false;
//      }
//    }
//  }
//  int floodFill[MAP_W][MAP_H];
//  FloodFill::run(Map::player->pos, blockers, floodFill, INT_MAX, Pos(-1, -1));
//  const int FLOOD_VALUE_AT_DOOR = floodFill[doorToLink->pos_.x][doorToLink->pos_.y];
//  vector<Pos> leverPosBucket;
//  for(int y = 1; y < MAP_H - 1; y++) {
//    for(int x = 1; x < MAP_W - 1; x++) {
//      if(floodFill[x][y] < FLOOD_VALUE_AT_DOOR) {
//        if(Map::featuresStatic[x][y]->canHaveStaticFeature()) {
//          leverPosBucket.push_back(Pos(x, y));
//        }
//      }
//    }
//  }
//
//  if(leverPosBucket.size() > 0) {
//    const int ELEMENT = Rnd::range(0, leverPosBucket.size() - 1);
//    const Pos leverPos(leverPosBucket.at(ELEMENT));
//    spawnLeverAdaptAndLinkDoor(leverPos, *doorToLink);
//  } else {
//    trace << "[WARNING] Could not find position to place lever, in makeLeverPuzzle()" << endl;
//  }
//  trace << "MapGen::Bsp::makeLeverPuzzle() [DONE]" << endl;
//}

//void spawnLeverAdaptAndLinkDoor(const Pos& leverPos, Door& door) {
//  trace << "MapGen::Bsp: Spawning lever and linking it to the door" << endl;
//  FeatureFactory::spawn(FeatureId::lever, leverPos, new LeverSpawnData(&door));
//
//  trace << "MapGen::Bsp: Changing door properties" << endl;
//  door.material_ = doorMaterial_metal;
//  door.isOpen_ = false;
//  door.isStuck_ = false;
//  door.isOpenedAndClosedExternally_ = true;
//}

void revealDoorsOnPathToStairs(const Pos& stairsPos) {
  trace << "MapGen::Bsp::revealDoorsOnPathToStairs()..." << endl;

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);

  blockers[stairsPos.x][stairsPos.y] = false;

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(Map::cells[x][y].featureStatic->getId() == FeatureId::door) {
        blockers[x][y] = false;
      }
    }
  }

  vector<Pos> path;
  PathFind::run(Map::player->pos, stairsPos, blockers, path);

  assert(path.empty() == false);

  trace << "MapGen::Bsp: Travelling along path and revealing all doors" << endl;
  for(Pos & pos : path) {
    Feature* const feature = Map::cells[pos.x][pos.y].featureStatic;
    if(feature->getId() == FeatureId::door) {
      dynamic_cast<Door*>(feature)->reveal(false);
    }
  }

  trace << "MapGen::Bsp::revealDoorsOnPathToStairs() [DONE]" << endl;
}

//void buildNaturalArea(Region* regions[3][3]) {
//  const Pos origin(1,1);
//  for(int y = 1; y < MAP_H - 1; y++) {
//    for(int x = 1; x < MAP_W - 1; x++) {
//      if(Utils::pointDist(origin.x, origin.y, x, y) < 20) {
//        FeatureFactory::spawn(FeatureId::deepWater, Pos(x,y));
//        for(int yRegion = 0; yRegion < 3; yRegion++) {
//          for(int xRegion = 0; xRegion < 3; xRegion++) {
//            Region* region = regions[xRegion][yRegion];
//            if(Utils::isPosInside(Pos(x,y), region->getX0Y0(), region->getX1Y1())) {
//              region->mapArea.isSpecialRoomAllowed = false;
//            }
//          }
//        }
//      }
//    }
//  }
//  makeRiver(regions);
//}

//void makeRiver(Region* regions[3][3]) {
//  (void)regions;
//
//  const int W = Rnd::range(4, 12);
//  const int START_X_OFFSET_MAX = 5;
//  const int X_POS_START = MAP_W/2 + Rnd::range(-START_X_OFFSET_MAX, START_X_OFFSET_MAX);
//
//  Pos leftPos(X_POS_START, 0);
//  while(Utils::isPosInsideMap(leftPos) && Utils::isPosInsideMap(leftPos + Pos(W,0))) {
//    coverAreaWithFeature(Rect(leftPos, leftPos + Pos(W, 0)), FeatureId::deepWater);
//    leftPos += Pos(Rnd::range(-1,1), 1);
//  }
//}

} //namespace

bool run() {
  trace << "MapGen::Bsp::run()..." << endl;

  Renderer::clearScreen();
  Renderer::updateScreen();

  Map::resetMap();

  trace << "MapGen::Bsp: Resetting helper arrays" << endl;
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      roomCells[x][y] = false;
      globalDoorPosBucket[x][y] = false;

#ifdef DEMO_MODE
      Cell& cell = Map::cells[x][y];
      cell.isSeenByPlayer = cell.isExplored = true;
#endif // DEMO_MODE
    }
  }

  const int SPL_X1 = MAP_W / 3 + Rnd::range(-1, 1);
  const int SPL_X2 = 2 * (MAP_W / 3) + Rnd::range(-1, 1);
  const int SPL_Y1 = MAP_H / 3;
  const int SPL_Y2 = 2 * (MAP_H / 3);

#ifdef DEMO_MODE
  Renderer::drawMapAndInterface();
  SdlWrapper::sleep(1000);
  const int P_W = Config::getCellW();
  const int P_H = Config::getCellH();
  const int P_M_O = Config::mainscreenOffsetY;
  const int P_S_W = Config::getScreenPixelW();
  const int P_C_O = Config::characterLinesOffsetY;
  const int P_M_H = P_C_O - P_M_O;
  Renderer::drawLineHor(Pos(0, SPL_Y1 * P_H + P_M_O), P_S_W, clrRedLgt);
  Renderer::updateScreen();
  SdlWrapper::sleep(2000);
  Renderer::drawLineHor(Pos(0, SPL_Y2 * P_H + P_M_O), P_S_W, clrRedLgt);
  Renderer::updateScreen();
  SdlWrapper::sleep(2000);
  Renderer::drawLineVer(Pos(SPL_X1 * P_W, P_M_O), P_M_H, clrRedLgt);
  Renderer::updateScreen();
  SdlWrapper::sleep(2000);
  Renderer::drawLineVer(Pos(SPL_X2 * P_W, P_M_O), P_M_H, clrRedLgt);
  Renderer::updateScreen();
  SdlWrapper::sleep(2000);
  Renderer::drawMapAndInterface(true);
  SdlWrapper::sleep(2000);
#endif // DEMO_MODE

  Region* regions[3][3];

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      regions[x][y] = NULL;
      regionsToBuildCave[x][y] = false;
    }
  }

  buildMergedRegionsAndRooms(regions, SPL_X1, SPL_X2, SPL_Y1, SPL_Y2);

  const int FIRST_DUNGEON_LEVEL_CAVES_ALLOWED = 10;
  const int CHANCE_CAVE_AREA =
    (Map::dlvl - FIRST_DUNGEON_LEVEL_CAVES_ALLOWED + 1) * 20;
  if(Rnd::percentile() < CHANCE_CAVE_AREA) {
    const bool IS_TWO_CAVES = Rnd::percentile() < (CHANCE_CAVE_AREA / 3);
    for(int nrCaves = IS_TWO_CAVES ? 2 : 1; nrCaves > 0; nrCaves--) {
      int nrTriesToMark = 1000;
      while(nrTriesToMark > 0) {
        Pos c(Rnd::range(0, 2), Rnd::range(0, 2));
        if(regions[c.x][c.y] == NULL && regionsToBuildCave[c.x][c.y] == false) {
          regionsToBuildCave[c.x][c.y] = true;
          nrTriesToMark = 0;
        }
        nrTriesToMark--;
      }
    }
  }

  trace << "MapGen::Bsp: Making rooms" << endl;
  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      if(regions[x][y] == NULL) {
        const int X0 = x == 0 ? 0 : x == 1 ? SPL_X1 : SPL_X2;
        const int Y0 = y == 0 ? 0 : y == 1 ? SPL_Y1 : SPL_Y2;
        const int X1 = x == 0 ? SPL_X1 - 1 :
                       x == 1 ? SPL_X2 - 1 : MAP_W - 1;
        const int Y1 = y == 0 ? SPL_Y1 - 1 :
                       y == 1 ? SPL_Y2 - 1 : MAP_H - 1;
        Region* region = new Region(Pos(X0, Y0), Pos(X1, Y1));
        regions[x][y] = region;

#ifdef DEMO_MODE
        const Pos dims((X1 - X0) * P_W, (Y1 - Y0) * P_H);
        Renderer::drawRectangleSolid(
          Pos(X0 * P_W, P_M_O + Y0 * P_H), dims, clrGreenLgt);
        Renderer::updateScreen();
        SdlWrapper::sleep(2000);
#endif // DEMO_MODE

        const Rect roomRect = region->getRandomRectForRoom();

        Map::rooms.push_back(buildRoom(roomRect));
        regions[x][y]->mainRoom = Map::rooms.back();

        if(Rnd::oneIn(3)) {reshapeRoom(*(regions[x][y]->mainRoom));}
      }
    }
  }

  connectRegions(regions);

  buildAuxRooms(regions);

  buildCaves(regions);

  buildRoomsInRooms();

  postProcessFillDeadEnds();

  trace << "MapGen::Bsp: Placing doors" << endl;
  const int CHANCE_TP_OLACE_DOOR = 70;
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(globalDoorPosBucket[x][y] == true) {
        if(Rnd::percentile() < CHANCE_TP_OLACE_DOOR) {
          placeDoorAtPosIfSuitable(Pos(x, y));
        }
      }
    }
  }
#ifdef DEMO_MODE
  SdlWrapper::sleep(2000);
#endif // DEMO_MODE

  trace << "MapGen::Bsp: Calling RoomThemeMaking::run()" << endl;
  RoomThemeMaking::run();
#ifdef DEMO_MODE
  Renderer::drawMapAndInterface();
  SdlWrapper::sleep(3000);
#endif // DEMO_MODE

  movePlayerToNearestAllowedPos();

  PopulateMonsters::populateRoomAndCorridorLevel();
  PopulateTraps::populateRoomAndCorridorLevel();

  const Pos stairsPos = placeStairs();
  if(stairsPos.x == -1) {return false;}

  const int LAST_LEVEL_TO_REVEAL_STAIRS_PATH = 9;
  if(Map::dlvl <= LAST_LEVEL_TO_REVEAL_STAIRS_PATH) {
    revealDoorsOnPathToStairs(stairsPos);
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

#ifdef DEMO_MODE
  Renderer::drawMapAndInterface();
  SdlWrapper::sleep(5000);

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Cell& cell = Map::cells[x][y];
      cell.isSeenByPlayer = cell.isExplored = false;
    }
  }
#endif // DEMO_MODE
  trace << "MapGen::Bsp::run() [DONE]" << endl;
  return true;
}

} //Bsp

} //MapGen

//=============================================================== REGION
Region::Region(const Pos& x0y0, const Pos& x1y1) :
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

Region::~Region() {}

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

bool Region::isRegionNeighbour(const Region& other) {
  for(int x = x0y0_.x; x <= x1y1_.x; x++) {
    for(int y = x0y0_.y; y <= x1y1_.y; y++) {
      for(int xx = other.x0y0_.x; xx <= other.x1y1_.x; xx++) {
        for(int yy = other.x0y0_.y; yy <= other.x1y1_.y; yy++) {
          if(Utils::isPosAdj(
                Pos(x, y), Pos(xx, yy), false)) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

Rect Region::getRandomRectForRoom() const {
  const bool TINY_ALLOWED_HOR = Rnd::coinToss();

  const Pos minDim(TINY_ALLOWED_HOR ? 2 : 4, TINY_ALLOWED_HOR ? 4 : 2);
  const Pos maxDim = x1y1_ - x0y0_ - Pos(2, 2);

  const int H = Rnd::range(minDim.y, maxDim.y);
  const bool ALLOW_BIG_W = H > (maxDim.y * 5) / 6;
  const int W = Rnd::range(minDim.x, ALLOW_BIG_W ? maxDim.x :
                           (minDim.x + ((maxDim.x - minDim.x) / 5)));

  const Pos dim(W, H);

  const int X0 = x0y0_.x + 1 + Rnd::range(0, maxDim.x - dim.x);
  const int Y0 = x0y0_.y + 1 + Rnd::range(0, maxDim.y - dim.y);
  const int X1 = X0 + dim.x - 1;
  const int Y1 = Y0 + dim.y - 1;

  return Rect(Pos(X0, Y0), Pos(X1, Y1));
}

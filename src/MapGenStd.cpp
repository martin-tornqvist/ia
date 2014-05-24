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

//Some options (comment out to disable)
//#define MK_MERGED_REGIONS   1
//#define RESHAPE_ROOMS       1
//#define MK_AUX_ROOMS        1
//#define MK_CAVES            1
//#define MK_SUB_ROOMS        1
//#define FILL_DEAD_ENDS      1
//#define ROOM_THEMING        1
//#define DECORATE            1

using namespace std;

namespace MapGen {

namespace Std {

namespace {

//Used for helping build the map
bool roomCells[MAP_W][MAP_H];
bool regionsToMkCave[3][3];

bool doorPosProposals[MAP_W][MAP_H];

bool isAllRoomsConnected() {
  Pos c;
  for(int y = 1; y < MAP_H - 1; y++) {
    bool isFound = false;
    for(int x = 1; x < MAP_W - 1; x++) {
      c.set(x, y);
      const FeatureStatic* const f = Map::cells[c.x][c.y].featureStatic;
      if(f->getId() == FeatureId::floor) {
        isFound = true;
        break;
      }
    }
    if(isFound) {break;}
  }

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  int floodFill[MAP_W][MAP_H];
  FloodFill::run(c, blocked, floodFill, INT_MAX, Pos(-1, -1));

  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(Map::cells[x][y].featureStatic->getId() == FeatureId::floor) {
        if(Pos(x, y) != c && floodFill[x][y] == 0) {return false;}
      }
    }
  }
  return true;
}

Room* mkRoom(const Rect& roomRect) {
  MapGenUtils::mk(roomRect, FeatureId::floor);
  for(int y = roomRect.p0.y; y <= roomRect.p1.y; y++) {
    for(int x = roomRect.p0.x; x <= roomRect.p1.x; x++) {
      roomCells[x][y] = true;
    }
  }

  return new Room(roomRect);
}

void mkCrumbleRoom(const Rect& roomAreaInclWalls, const Pos& proxEventPos) {
  vector<Pos> wallCells;
  vector<Pos> innerCells;

  const Rect& a = roomAreaInclWalls; //abbreviation

  for(int y = a.p0.y; y <= a.p1.y; y++) {
    for(int x = a.p0.x; x <= a.p1.x; x++) {
      if(x == a.p0.x || x == a.p1.x || y == a.p0.y || y == a.p1.y) {
        wallCells.push_back(Pos(x, y));
      } else {
        innerCells.push_back(Pos(x, y));
      }
      FeatureFactory::mk(FeatureId::wall, Pos(x, y));
    }
  }

  ProxEventWallCrumbleSpawnData* const spawnData =
    new ProxEventWallCrumbleSpawnData(wallCells, innerCells);
  FeatureFactory::mk(
    FeatureId::proxEventWallCrumble, proxEventPos, spawnData);
}

void connectRegions(Region* regions[3][3]) {
  trace << "MapGen::Std::connectRegions()..." << endl;

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
                  IS_INSIDE_BOUNDS;
    }
    Pos c2(c1 + delta);

    Region* const r2 = regions[c2.x][c2.y];

    if(r1->regionsConnectedTo_[c2.x][c2.y] == false) {
      const Dir regionDir = DirUtils::getDir(c2 - c1);

      MapGenUtils::mkZCorridorBetweenRooms(
        *(r1->mainRoom_), *(r2->mainRoom_), regionDir, doorPosProposals);

      r1->regionsConnectedTo_[c2.x][c2.y] = true;
      r2->regionsConnectedTo_[c1.x][c1.y] = true;
    }
  }
  trace << "MapGen::Std::connectRegions()[DONE]" << endl;
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

//Note: The positions and size can be outside map (e.g. negative positions).
//This function should handle that, and simply return false in that case.
bool tryMkAuxRoom(const Pos& p, const Pos& d, bool blocked[MAP_W][MAP_H],
                  const Pos& doorP) {
  Rect auxRect(p, p + d - 1);
  Rect auxRectWithBorder(auxRect.p0 - 1, auxRect.p1 + 1);

  if(Utils::isAreaInsideMap(auxRectWithBorder)) {
    if(MapParse::isValInArea(auxRectWithBorder, blocked)) {
      Room* room = mkRoom(auxRect);
      Map::rooms.push_back(room);
      for(int y = auxRect.p0.y; y <= auxRect.p1.y; y++) {
        for(int x = auxRect.p0.x; x <= auxRect.p1.x; x++) {
          blocked[x][y] = true;
        }
      }

      if(Rnd::oneIn(5)) {
        mkCrumbleRoom(auxRectWithBorder, doorP);
        deleteAndRemoveRoomFromList(room); //Don't apply themes etc to the room
        room = nullptr;
      } else {
        FeatureFactory::mk(FeatureId::floor, doorP);
        doorPosProposals[doorP.x][doorP.y] = true;
      }
      return true;
    }
  }
  return false;
}

Pos rndAuxRoomDim() {
  return Pos(Rnd::range(3, 7), Rnd::range(3, 7));
}

void mkAuxRooms(Region* regions[3][3]) {
  trace << "MapGen::Std::mkAuxRooms()..." << endl;
  const int NR_TRIES_PER_SIDE = 10;

  for(int regionY = 0; regionY < 3; regionY++) {
    for(int regionX = 0; regionX < 3; regionX++) {

      const Region* const region    = regions[regionX][regionY];

      if(region->mainRoom_ != nullptr && Rnd::oneIn(3)) {

        Room& mainR = *region->mainRoom_;

        bool floorCells[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksMoveCmn(false), floorCells);

        Utils::reverseBoolArray(floorCells);

        //Right
        for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
          const Pos conP(mainR.getX1() + 1,
                         Rnd::range(mainR.getY0() + 1, mainR.getY1() - 1));
          const Pos auxD(rndAuxRoomDim());
          const Pos auxP(conP +
                         Pos(1, Rnd::range(conP.y - auxD.y + 1, auxD.y)));
          if(floorCells[conP.x - 1][conP.y]) {
            if(tryMkAuxRoom(auxP, auxD, floorCells, conP)) {
              traceVerbose << "MapGen::Std: Aux room placed right" << endl;
              break;
            }
          }
        }

        //Up
        for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
          const Pos conP(Rnd::range(mainR.getX0() + 1, mainR.getX1() - 1),
                         mainR.getY0() - 1);
          const Pos auxD(rndAuxRoomDim());
          const Pos auxP(conP +
                         Pos(Rnd::range(conP.x - auxD.x + 1, conP.x), -auxD.y));
          if(floorCells[conP.x][conP.y + 1]) {
            if(tryMkAuxRoom(auxP, auxD, floorCells, conP)) {
              traceVerbose << "MapGen::Std: Aux room placed up" << endl;
              break;
            }
          }
        }

        //Left
        for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
          const Pos conP(mainR.getX0() - 1,
                         Rnd::range(mainR.getY0() + 1, mainR.getY1() - 1));
          const Pos auxD(rndAuxRoomDim());
          const Pos auxP(conP +
                         Pos(-auxD.x, Rnd::range(conP.y - auxD.y + 1, conP.y)));
          if(floorCells[conP.x + 1][conP.y]) {
            if(tryMkAuxRoom(auxP, auxD, floorCells, conP)) {
              traceVerbose << "MapGen::Std: Aux room placed left" << endl;
              break;
            }
          }
        }

        //Down
        for(int i = 0; i < NR_TRIES_PER_SIDE; i++) {
          const Pos conP(Rnd::range(mainR.getX0() + 1, mainR.getX1() - 1),
                         mainR.getY1() + 1);
          const Pos auxD(rndAuxRoomDim());
          const Pos auxP(conP +
                         Pos(Rnd::range(conP.x - auxD.x + 1, conP.x), 1));
          if(floorCells[conP.x][conP.y - 1]) {
            if(tryMkAuxRoom(auxP, auxD, floorCells, conP)) {
              traceVerbose << "MapGen::Std: Aux room placed down" << endl;
              break;
            }
          }
        }
      }
    }
  }

  trace << "MapGen::Std::mkAuxRooms() [DONE]" << endl;
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
            Rect rect(Rect(room.getP0(), upLeft));
            MapGenUtils::mk(rect, FeatureId::wall);
          }

          if(TRIM_ALL || Rnd::coinToss()) {
            const Pos upRight(room.getX1() - W + 1, room.getY0() + H - 1);
            Rect rect(Pos(room.getX0() + ROOM_W - 1, room.getY0()), upRight);
            MapGenUtils::mk(rect, FeatureId::wall);
          }

          if(TRIM_ALL || Rnd::coinToss()) {
            const Pos downLeft(room.getX0() + W - 1, room.getY1() - H + 1);
            Rect rect(Pos(room.getX0(), room.getY0() + ROOM_H - 1), downLeft);
            MapGenUtils::mk(rect, FeatureId::wall);
          }

          if(TRIM_ALL || Rnd::coinToss()) {
            const Pos downRight(room.getX1() - W + 1, room.getY1() - H + 1);
            Rect rect(room.getP1(), downRight);
            MapGenUtils::mk(rect, FeatureId::wall);
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
                  FeatureFactory::mk(FeatureId::wall, c);
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

void mkMergedRegionsAndRooms(Region* regions[3][3],
                             const int SPL_X0, const int SPL_X1,
                             const int SPL_Y0, const int SPL_Y1) {

  const int NR_OF_MERGED_REGIONS_TO_ATTEMPT = Rnd::range(0, 2);

  for(
    int attemptCount = 0;
    attemptCount < NR_OF_MERGED_REGIONS_TO_ATTEMPT;
    attemptCount++) {

    Pos regI1, regI2;

    //Find two non-occupied regions
    int nrTriesToFindRegions = 100;
    bool isGoodRegionsFound = false;
    while(isGoodRegionsFound == false) {
      nrTriesToFindRegions--;
      if(nrTriesToFindRegions <= 0) {
        return;
      }

      regI1 = Pos(Rnd::range(0, 2), Rnd::range(0, 1));
      regI2 = Pos(regI1 + Pos(0, 1));
      isGoodRegionsFound =
        regions[regI1.x][regI1.y]->hasBuiltInside_ == false &&
        regions[regI2.x][regI2.y]->hasBuiltInside_ == false;
    }

    const int MERGED_X0 = regI1.x == 0 ? 0 : regI1.x == 1 ? SPL_X0 : SPL_X1;
    const int MERGED_Y0 = regI1.y == 0 ? 0 : regI1.y == 1 ? SPL_Y0 : SPL_Y1;
    const int MERGED_X1 = regI2.x == 0 ? SPL_X0 - 1 :
                          regI2.x == 1 ? SPL_X1 - 1 : MAP_W - 1;
    const int MERGED_Y1 = regI2.y == 0 ? SPL_Y0 - 1 :
                          regI2.y == 1 ? SPL_Y1 - 1 : MAP_H - 1;

    const int AREA_2_X0 = regI2.x == 0 ? 0 : regI2.x == 1 ? SPL_X0 : SPL_X1;
    const int AREA_2_Y0 = regI2.y == 0 ? 0 : regI2.y == 1 ? SPL_Y0 : SPL_Y1;
    const int AREA_2_X1 = MERGED_X1;
    const int AREA_2_Y1 = MERGED_Y1;

    const bool AREA_2_IS_BELOW = regI2.y > regI1.y;

    const int AREA_1_X0 = MERGED_X0;
    const int AREA_1_X1 = AREA_2_IS_BELOW ? MERGED_X1 - 1 : AREA_2_X0 - 1;
    const int AREA_1_Y0 = MERGED_Y0;
    const int AREA_1_Y1 = AREA_2_IS_BELOW ? AREA_2_Y0 - 1 : MERGED_Y1;

    const Rect rect1(Pos(AREA_1_X0, AREA_1_Y0), Pos(AREA_1_X1, AREA_1_Y1));
    const Rect rect2(Pos(AREA_2_X0, AREA_2_Y0), Pos(AREA_2_X1, AREA_2_Y1));

    Region* reg1 = new Region(rect1);
    Region* reg2 = new Region(rect2);
    regions[regI1.x][regI1.y] = reg1;
    regions[regI2.x][regI2.y] = reg2;

    const int OFFSET_X0 = Rnd::range(1, 4);
    const int OFFSET_Y0 = Rnd::range(1, 4);
    const int OFFSET_X1 = Rnd::range(1, 4);
    const int OFFSET_Y1 = Rnd::range(1, 4);
    Rect roomRect(rect1.p0 + Pos(OFFSET_X0, OFFSET_Y0),
                  rect2.p1 - Pos(OFFSET_X1, OFFSET_Y1));
    Room* const room = mkRoom(roomRect);
    Map::rooms.push_back(room);

    reg1->mainRoom_ = reg2->mainRoom_ = room;

    reg1->regionsConnectedTo_[regI2.x][regI2.y] = true;
    reg2->regionsConnectedTo_[regI1.x][regI1.y] = true;
    reg1->hasBuiltInside_                       = true;
    reg2->hasBuiltInside_                       = true;

#ifdef RESHAPE_ROOMS
    if(Rnd::oneIn(3)) {reshapeRoom(*room);}
#endif
  }
}

void mkCaves(Region* regions[3][3]) {
  trace << "MapGen::Std::mkCaves()..." << endl;
  for(int regY = 0; regY <= 2; regY++) {
    for(int regX = 0; regX <= 2; regX++) {

      if(regionsToMkCave[regX][regY]) {

        Region* const region = regions[regX][regY];

        //This region no longer has a room, delete it from list
        deleteAndRemoveRoomFromList(region->mainRoom_);
        region->mainRoom_ = nullptr;

        bool blocked[MAP_W][MAP_H];

        for(int y = 0; y < MAP_H; y++) {
          for(int x = 0; x < MAP_W; x++) {

            blocked[x][y] = false;

            if(x == 0 || y == 0 || x == MAP_W - 1 || y == MAP_H - 1) {
              blocked[x][y] = true;
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
                      Pos(x + dx, y + dy), region->getRect()) == false) {
                    blocked[x][y] = true;
                  }
                }
              }
            }
          }
        }

        const Pos origin(region->getP0() + Pos(1, 1));
        int floodFillResult[MAP_W][MAP_H];

        const int FLOOD_FILL_TRAVEL_LIMIT = 20;

        FloodFill::run(origin, blocked, floodFillResult,
                       FLOOD_FILL_TRAVEL_LIMIT, Pos(-1, -1));

        for(int y = 1; y < MAP_H - 1; y++) {
          for(int x = 1; x < MAP_W - 1; x++) {
            const Pos c(x, y);
            if(c == origin || floodFillResult[x][y] > 0) {

              FeatureFactory::mk(FeatureId::caveFloor, c);

              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  Cell& adjCell = Map::cells[x + dx][y + dy];
                  if(adjCell.featureStatic->getId() == FeatureId::wall) {
                    FeatureFactory::mk(
                      FeatureId::wall, c + Pos(dx , dy));
                    Wall* const wall =
                      dynamic_cast<Wall*>(adjCell.featureStatic);
                    wall->wallType = WallType::cave;
                    wall->setRandomIsMossGrown();
                  }
                }
              }
            }
          }
        }

        const int CHANCE_TO_MAKE_CHASM = 25;

        if(Rnd::percentile() < CHANCE_TO_MAKE_CHASM) {
          Utils::resetArray(blocked, false);

          for(int y = 1; y < MAP_H - 1; y++) {
            for(int x = 1; x < MAP_W - 1; x++) {
              for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                  Cell& adjCell = Map::cells[x + dx][y + dy];
                  if(adjCell.featureStatic->getId() == FeatureId::wall) {
                    blocked[x][y] = blocked[x + dx][y + dy] = true;
                  }
                }
              }
            }
          }

          FloodFill::run(origin, blocked, floodFillResult,
                         FLOOD_FILL_TRAVEL_LIMIT / 2, Pos(-1, -1));

          for(int y = 1; y < MAP_H - 1; y++) {
            for(int x = 1; x < MAP_W - 1; x++) {
              const Pos c(x, y);
              if(
                blocked[x][y] == false &&
                (c == origin || floodFillResult[x][y] > 0)) {
                FeatureFactory::mk(FeatureId::chasm, c);
              }
            }
          }
        }
      }
    }
  }
  trace << "MapGen::Std::mkCaves()[DONE]" << endl;
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
    FeatureFactory::mk(FeatureId::door, p,
                       new DoorSpawnData(mimicData));
  }
}

void mkSubRooms() {
  const int NR_OF_TRIES         = 40;
  const int MAX_NR_INNER_ROOMS  = 7;
  const int MIN_DIM_W           = 4;
  const int MIN_DIM_H           = 4;

  for(size_t i = 0; i < Map::rooms.size(); i++) {

    const Pos roomX0Y0 = Map::rooms.at(i)->getP0();
    const Pos roomX1Y1 = Map::rooms.at(i)->getP1();

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

                    FeatureFactory::mk(
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
                FeatureFactory::mk(FeatureId::floor, doorPos);
                doorPosProposals[doorPos.x][doorPos.y] = true;
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
                    FeatureFactory::mk(FeatureId::floor, posCand);
                    positionsWithDoor.push_back(posCand);
                  }
                }
              }
              break;
            }
          }
        }
      }
    }
  }
}

void fillDeadEnds() {
  //Find an origin with no adjacent walls, to ensure not starting in a dead end
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);

  bool expandedBlockers[MAP_W][MAP_H];
  MapParse::expand(blocked, expandedBlockers, 1);

  Pos origin;
  bool isDone = false;
  for(int y = 2; y < MAP_H - 2; y++) {
    for(int x = 2; x < MAP_W - 2; x++) {
      if(expandedBlockers[x][y] == false) {
        origin = Pos(x, y);
        isDone = true;
        break;
      }
    }
    if(isDone) {break;}
  }

  //Floodfill from origin, then sort the positions for flood value
  int floodFill[MAP_W][MAP_H];
  FloodFill::run(origin, blocked, floodFill, INT_MAX, Pos(-1, -1));
  vector<PosAndVal> floodFillVector;
  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(blocked[x][y] == false) {
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
    const int NR_ADJ_CARDINAL_WALLS = blocked[x + 1][y] + blocked[x - 1][y] +
                                      blocked[x][y + 1] + blocked[x][y - 1];
    if(NR_ADJ_CARDINAL_WALLS == 3) {
      FeatureFactory::mk(FeatureId::wall, pos);
      blocked[x][y] = true;
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
//  for(c.x = roomRect.p0.x; c.x <= roomRect.p1.x; c.x++) {
//    for(c.y = roomRect.p0.y; c.y <= roomRect.p1.y; c.y++) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.y = INT_MAX;
//      }
//    }
//  }
//  //Left to right
//  for(c.y = roomRect.p0.y; c.y <= roomRect.p1.y; c.y++) {
//    for(c.x = roomRect.p0.x; c.x <= roomRect.p1.x; c.x++) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.x = INT_MAX;
//      }
//    }
//  }
//  //Bottom to top
//  for(c.x = roomRect.p0.x; c.x <= roomRect.p1.x; c.x++) {
//    for(c.y = roomRect.p1.y; c.y >= roomRect.p0.y; c.y--) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.y = INT_MIN;
//      }
//    }
//  }
//  //Right to left
//  for(c.y = roomRect.p0.y; c.y <= roomRect.p1.y; c.y++) {
//    for(c.x = roomRect.p1.x; c.x >= roomRect.p0.x; c.x--) {
//      if(Map::featuresStatic[c.x][c.y]->getId() == FeatureId::floor) {
//        PossToAdd[c.x][c.y] = true;
//        c.x = INT_MIN;
//      }
//    }
//  }
//
//  for(c.x = roomRect.p0.x; c.x <= roomRect.p1.x; c.x++) {
//    for(c.y = roomRect.p0.y; c.y <= roomRect.p1.y; c.y++) {
//      if(PossToAdd[c.x][c.y]) {
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

        //Randomly convert walls to rubble
        if(Rnd::oneIn(10)) {
          FeatureFactory::mk(FeatureId::rubbleHigh, Pos(x, y));
          continue;
        }

        //Moss grown walls
        FeatureStatic* const f = Map::cells[x][y].featureStatic;
        Wall* const wall = dynamic_cast<Wall*>(f);
        wall->setRandomIsMossGrown();

        //Convert walls with no adjacent stone floor to cave walls
        CellPred::IsAnyOfFeatures pred(vector<FeatureId> {FeatureId::floor});
        if(MapParse::getNrAdjCellsWithFeature(Pos(x, y), pred) == 0) {
          wall->wallType = WallType::cave;
        } else {
          wall->setRandomNormalWall();
        }
      }
    }
  }

  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(Map::cells[x][y].featureStatic->getId() == FeatureId::floor) {
        //Randomly convert stone floor to low rubble
        if(Rnd::oneIn(100)) {
          FeatureFactory::mk(FeatureId::rubbleLow, Pos(x, y));
          continue;
        }
      }
    }
  }
}

void getAllowedStairCells(bool cellsToSet[MAP_W][MAP_H]) {
  trace << "MapGen::Std::getAllowedStairCells()..." << endl;

  //Stairs are only allowed in cells with, and completely surrounded by,
  //stone floor or cave floor

  vector<FeatureId> featIdsOk {FeatureId::floor, FeatureId::caveFloor};

  MapParse::parse(CellPred::AllAdjIsAnyOfFeatures(featIdsOk), cellsToSet);

  trace << "MapGen::Std::getAllowedStairCells() [DONE]" << endl;
}

Pos placeStairs() {
  trace << "MapGen::Std::placeStairs()..." << endl;

  bool allowedCells[MAP_W][MAP_H];
  getAllowedStairCells(allowedCells);

  vector<Pos> allowedCellsList;
  Utils::mkVectorFromBoolMap(true, allowedCells, allowedCellsList);

  const int NR_OK_CELLS = allowedCellsList.size();

  const int MIN_NR_OK_CELLS_REQ = 80;

  if(NR_OK_CELLS < MIN_NR_OK_CELLS_REQ) {
    trace << "MapGen::Std: Number of allowed cells too low ";
    trace << "(" << NR_OK_CELLS << "), discarding map" << endl;
    return Pos(-1, -1);
  }

  trace << "MapGen::Std: Sorting the allowed cells vector ";
  trace << "(" << allowedCellsList.size() << " cells)" << endl;
  IsCloserToOrigin isCloserToOrigin(Map::player->pos);
  sort(allowedCellsList.begin(), allowedCellsList.end(), isCloserToOrigin);

  trace << "MapGen::Std: Picking random cell from furthest half" << endl;
  const int ELEMENT = Rnd::range(NR_OK_CELLS / 2, NR_OK_CELLS - 1);
  const Pos stairsPos(allowedCellsList.at(ELEMENT));

  trace << "MapGen::Std: Spawning stairs at chosen cell" << endl;
  Feature* f = FeatureFactory::mk(FeatureId::stairs, stairsPos);
  f->setHasBlood(false);

  trace << "MapGen::Std::placeStairs()[DONE]" << endl;
  return stairsPos;
}

void movePlayerToNearestAllowedPos() {
  trace << "MapGen::Std::movePlayerToNearestAllowedPos()..." << endl;

  bool allowedCells[MAP_W][MAP_H];
  getAllowedStairCells(allowedCells);

  vector<Pos> allowedCellsList;
  Utils::mkVectorFromBoolMap(true, allowedCells, allowedCellsList);

  assert(allowedCellsList.empty() == false);

  trace << "MapGen::Std: Sorting the allowed cells vector ";
  trace << "(" << allowedCellsList.size() << " cells)" << endl;
  IsCloserToOrigin isCloserToOrigin(Map::player->pos);
  sort(allowedCellsList.begin(), allowedCellsList.end(), isCloserToOrigin);

  Map::player->pos = allowedCellsList.front();

  trace << "MapGen::Std::movePlayerToNearestAllowedPos() [DONE]" << endl;
}

//void mkLevers() {
//  trace << "MapGen::Std::mkLeverPuzzle()..." << endl;
//
//  trace << "MapGen::Std: Picking a random door" << endl;
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
//  trace << "MapGen::Std: Making floodfill and keeping only positions with lower value than the door" << endl;
//  bool blocked[MAP_W][MAP_H];
//  eng.mapTests->mkMoveBlockerArrayForBodyTypeFeaturesOnly(bodyType_normal, blocked);
//  for(int y = 1; y < MAP_H - 1; y++) {
//    for(int x = 1; x < MAP_W - 1; x++) {
//      Feature* const feature = Map::featuresStatic[x][y];
//      if(feature->getId() == FeatureId::door) {
//        blocked[x][y] = false;
//      }
//    }
//  }
//  int floodFill[MAP_W][MAP_H];
//  FloodFill::run(Map::player->pos, blocked, floodFill, INT_MAX, Pos(-1, -1));
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
//    trace << "[WARNING] Could not find position to place lever, in mkLeverPuzzle()" << endl;
//  }
//  trace << "MapGen::Std::mkLeverPuzzle() [DONE]" << endl;
//}

//void spawnLeverAdaptAndLinkDoor(const Pos& leverPos, Door& door) {
//  trace << "MapGen::Std: Spawning lever and linking it to the door" << endl;
//  FeatureFactory::mk(FeatureId::lever, leverPos, new LeverSpawnData(&door));
//
//  trace << "MapGen::Std: Changing door properties" << endl;
//  door.material_ = doorMaterial_metal;
//  door.isOpen_ = false;
//  door.isStuck_ = false;
//  door.isOpenedAndClosedExternally_ = true;
//}

void revealDoorsOnPathToStairs(const Pos& stairsPos) {
  trace << "MapGen::Std::revealDoorsOnPathToStairs()..." << endl;

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);

  blocked[stairsPos.x][stairsPos.y] = false;

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(Map::cells[x][y].featureStatic->getId() == FeatureId::door) {
        blocked[x][y] = false;
      }
    }
  }

  vector<Pos> path;
  PathFind::run(Map::player->pos, stairsPos, blocked, path);

  assert(path.empty() == false);

  trace << "MapGen::Std: Travelling along path and revealing all doors" << endl;
  for(Pos & pos : path) {
    Feature* const feature = Map::cells[pos.x][pos.y].featureStatic;
    if(feature->getId() == FeatureId::door) {
      dynamic_cast<Door*>(feature)->reveal(false);
    }
  }

  trace << "MapGen::Std::revealDoorsOnPathToStairs() [DONE]" << endl;
}

//void mkNaturalArea(Region* regions[3][3]) {
//  const Pos origin(1,1);
//  for(int y = 1; y < MAP_H - 1; y++) {
//    for(int x = 1; x < MAP_W - 1; x++) {
//      if(Utils::pointDist(origin.x, origin.y, x, y) < 20) {
//        FeatureFactory::mk(FeatureId::deepWater, Pos(x,y));
//        for(int yRegion = 0; yRegion < 3; yRegion++) {
//          for(int xRegion = 0; xRegion < 3; xRegion++) {
//            Region* region = regions[xRegion][yRegion];
//            if(Utils::isPosInside(Pos(x,y), region->getP0(), region->getP1())) {
//              region->mapArea.isSpecialRoomAllowed = false;
//            }
//          }
//        }
//      }
//    }
//  }
//  mkRiver(regions);
//}

//void mkRiver(Region* regions[3][3]) {
//  (void)regions;
//
//  const int W = Rnd::range(4, 12);
//  const int START_X_OFFSET_MAX = 5;
//  const int X_POS_START = MAP_W/2 + Rnd::range(-START_X_OFFSET_MAX, START_X_OFFSET_MAX);
//
//  Pos leftPos(X_POS_START, 0);
//  while(Utils::isPosInsideMap(leftPos) && Utils::isPosInsideMap(leftPos + Pos(W,0))) {
//    MapGenUtils::mk(Rect(leftPos, leftPos + Pos(W, 0)), FeatureId::deepWater);
//    leftPos += Pos(Rnd::range(-1,1), 1);
//  }
//}

} //namespace

bool run() {
  trace << "MapGen::Std::run()..." << endl;

  Renderer::clearScreen();
  Renderer::updateScreen();

  Map::resetMap();

  trace << "MapGen::Std: Resetting helper arrays" << endl;
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      roomCells[x][y]         = false;
      doorPosProposals[x][y]  = false;
    }
  }

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      regionsToMkCave[x][y] = false;
    }
  }

  trace << "MapGen::Std: Init regions" << endl;
  const int MAP_W_THIRD = MAP_W / 3;
  const int MAP_H_THIRD = MAP_H / 3;
  const int SPL_X0 = MAP_W_THIRD; //+ Rnd::range(-1, 1);
  const int SPL_X1 = (MAP_W_THIRD * 2) + 1; // + Rnd::range(-1, 1);
  const int SPL_Y0 = MAP_H_THIRD;
  const int SPL_Y1 = MAP_H_THIRD * 2;

  Region* regions[3][3];

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      const Rect r(
        x == 0 ? 1 : x == 1 ? SPL_X0 + 1 : SPL_X1 + 1,
        y == 0 ? 1 : y == 1 ? SPL_Y0 + 1 : SPL_Y1 + 1,
        x == 0 ? SPL_X0 - 1 : x == 1 ? SPL_X1 - 1 : MAP_W - 2,
        y == 0 ? SPL_Y0 - 1 : y == 1 ? SPL_Y1 - 1 : MAP_H - 2);

      regions[x][y] = new Region(r);
      trace << "MapGen::Std: Region(" << x << "," << y << "): "
            << "W: " << r.p1.x - r.p0.x + 1 << " "
            << "H: " << r.p1.y - r.p0.y + 1 << " "
            << r.p0.x << "," << r.p0.y << "," << r.p1.x << "," << r.p1.y
            << endl;
    }
  }

#ifdef MK_MERGED_REGIONS
  mkMergedRegionsAndRooms(regions, SPL_X0, SPL_X1, SPL_Y0, SPL_Y1);
#endif

  const int FIRST_DUNGEON_LVL_CAVES_ALLOWED = 10;
  const int CHANCE_CAVE_AREA =
    (Map::dlvl - FIRST_DUNGEON_LVL_CAVES_ALLOWED + 1) * 20;
  if(Rnd::percentile() < CHANCE_CAVE_AREA) {
    const bool IS_TWO_CAVES = Rnd::percentile() < (CHANCE_CAVE_AREA / 3);
    for(int nrCaves = IS_TWO_CAVES ? 2 : 1; nrCaves > 0; nrCaves--) {
      int nrTriesToMark = 1000;
      while(nrTriesToMark > 0) {
        Pos c(Rnd::range(0, 2), Rnd::range(0, 2));
        if(regions[c.x][c.y] == nullptr && regionsToMkCave[c.x][c.y] == false) {
          regionsToMkCave[c.x][c.y] = true;
          nrTriesToMark = 0;
        }
        nrTriesToMark--;
      }
    }
  }

  trace << "MapGen::Std: Making rooms" << endl;
  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      Region& region = *regions[x][y];
      if(region.hasBuiltInside_ == false) {
        const Rect roomRect = region.getRndRoomRect();
        Room* room = mkRoom(roomRect);
        Map::rooms.push_back(room);
        region.mainRoom_ = room;
        region.hasBuiltInside_ = true;
#ifdef RESHAPE_ROOMS
        if(Rnd::oneIn(3)) {reshapeRoom(room);}
#endif
      }
    }
  }

  connectRegions(regions);

#ifdef MK_AUX_ROOMS
  mkAuxRooms(regions);
#endif

#ifdef MK_CAVES
  mkCaves(regions);
#endif

#ifdef MK_SUB_ROOMS
  mkSubRooms();
#endif

#ifdef FILL_DEAD_ENDS
  fillDeadEnds();
#endif

  trace << "MapGen::Std: Placing doors" << endl;
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(doorPosProposals[x][y] && Rnd::fraction(7, 10)) {
        placeDoorAtPosIfSuitable(Pos(x, y));
      }
    }
  }

#ifdef ROOM_THEMING
  trace << "MapGen::Std: Calling RoomThemeMaking::run()" << endl;
  RoomThemeMaking::run();
#endif

  movePlayerToNearestAllowedPos();

  PopulateMonsters::populateStdLvl();
  PopulateTraps::populateStdLvl();

  bool isMapOk = true;

  const Pos stairsPos = placeStairs();
  if(stairsPos.x == -1) {
    isMapOk = false;
  } else {
    const int LAST_LVL_TO_REVEAL_STAIRS_PATH = 9;
    if(Map::dlvl <= LAST_LVL_TO_REVEAL_STAIRS_PATH) {
      revealDoorsOnPathToStairs(stairsPos);
    }

//  mkLevers();

    //This must be run last, everything depends on all walls being stone walls
#ifdef DECORATE
    decorate();
#endif
  }

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      delete regions[x][y];
      regions[x][y] = nullptr;
    }
  }

  trace << "MapGen::Std::run() [DONE]" << endl;
  return isMapOk;
}

} //Std

} //MapGen

//=============================================================== REGION
Region::Region(const Rect& r) :
  mainRoom_(nullptr), isConnected_(false), hasBuiltInside_(false), r_(r) {

  for(int x = 0; x <= 2; x++) {
    for(int y = 0; y <= 2; y++) {
      regionsConnectedTo_[x][y] = false;
    }
  }
}

Region::Region() :
  mainRoom_(nullptr), isConnected_(false), hasBuiltInside_(false), r_() {

  for(int x = 0; x <= 2; x++) {
    for(int y = 0; y <= 2; y++) {
      regionsConnectedTo_[x][y] = false;
    }
  }
}

int Region::getNrConnections() {
  int nr = 0;
  for(int x = 0; x < 3; x++) {
    for(int y = 0; y < 3; y++) {
      if(regionsConnectedTo_[x][y]) {
        nr++;
      }
    }
  }
  return nr;
}

bool Region::isRegionNeighbour(const Region& other) {
  for(int x = r_.p0.x; x <= r_.p1.x; x++) {
    for(int y = r_.p0.y; y <= r_.p1.y; y++) {
      for(int xx = other.r_.p0.x; xx <= other.r_.p1.x; xx++) {
        for(int yy = other.r_.p0.y; yy <= other.r_.p1.y; yy++) {
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

Rect Region::getRndRoomRect() const {
  const bool TINY_ALLOWED_HOR = Rnd::coinToss();

  const Pos minDims(TINY_ALLOWED_HOR ? 2 : 4, TINY_ALLOWED_HOR ? 4 : 2);
  const Pos maxDims(r_.p1 - r_.p0 + 1);

  const int H = Rnd::range(minDims.y, maxDims.y);
  const bool ALLOW_BIG_W = H > (maxDims.y * 5) / 6;
  const int W = Rnd::range(minDims.x, ALLOW_BIG_W ? maxDims.x :
                           (minDims.x + ((maxDims.x - minDims.x) / 5)));

  const Pos p0(r_.p0.x + Rnd::range(0, maxDims.x - W),
               r_.p0.y + Rnd::range(0, maxDims.y - H));

  const Pos p1(p0.x + W - 1, p0.y + H - 1);

  return Rect(p0, p1);
}

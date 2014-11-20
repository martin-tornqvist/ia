#include "MapGen.h"

#include "Init.h"

#include <algorithm>
#include <climits>

#include "Room.h"
#include "MapGen.h"
#include "FeatureProxEvent.h"
#include "ActorPlayer.h"
#include "FeatureDoor.h"
#include "ActorFactory.h"
#include "ActorMon.h"
#include "ItemDrop.h"
#include "ItemFactory.h"
#include "Map.h"
#include "MapParsing.h"
#include "Render.h"
#include "Utils.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"
#include "PopulateItems.h"
#include "Gods.h"

#ifdef DEMO_MODE
#include "Render.h"
#include "SdlWrapper.h"
#include "Query.h"
#endif // DEMO_MODE

//-------------------------------------
//Some options (comment out to disable)
//-------------------------------------
//#define MK_RIVER                1
#define MK_MERGED_REGIONS       1
#define RANDOMLY_BLOCK_REGIONS  1
#define MK_AUX_ROOMS            1
#define MK_CRUMBLE_ROOMS        1
#define MK_SUB_ROOMS            1
//#define FILL_DEAD_ENDS          1
#define DECORATE                1

using namespace std;

namespace MapGen
{

namespace
{

//All cells marked as true in this array will be considered for door placement
bool doorProposals[MAP_W][MAP_H];

bool isAllRoomsConnected()
{
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  return MapParse::isMapConnected(blocked);
}

//Adds the room to the room list and the room map
void registerRoom(Room& room)
{
#ifndef NDEBUG
  for(Room* const roomInList : Map::roomList)
  {
    assert(roomInList != &room); //Check that the room is not already added
  }
#endif // NDEBUG

  Map::roomList.push_back(&room);

  for(int y = room.r_.p0.y; y <= room.r_.p1.y; ++y)
  {
    for(int x = room.r_.p0.x; x <= room.r_.p1.x; ++x)
    {
      Map::roomMap[x][y] = &room;
    }
  }
}

void mkFloorInRoom(const Room& room)
{
  for(int y = room.r_.p0.y; y <= room.r_.p1.y; ++y)
  {
    for(int x = room.r_.p0.x; x <= room.r_.p1.x; ++x)
    {
      Map::put(new Floor(Pos(x, y)));
    }
  }
}

void connectRooms()
{
  TRACE_FUNC_BEGIN;

  int nrTriesLeft = 5000;

  while(true)
  {
    //Note: Keep this counter at the top of the loop, since otherwise a "continue"
    //statement could bypass it so we get stuck in the loop.
    --nrTriesLeft;
    if(nrTriesLeft == 0)
    {
      MapGen::isMapValid = false;
#ifdef DEMO_MODE
      Render::coverPanel(Panel::log);
      Render::drawText("Failed to connect map", Panel::screen, {0, 0}, clrRedLgt);
      Render::updateScreen();
      SdlWrapper::sleep(8000);
#endif // DEMO_MODE
      break;
    }

    auto getRndRoom = []()
    {
      return Map::roomList[Rnd::range(0, Map::roomList.size() - 1)];
    };

    auto isStdRoom = [](const Room & r)
    {
      return int(r.type_) < int(RoomType::END_OF_STD_ROOMS);
    };

    Room* room0 = getRndRoom();

    //Room 0 must be a standard room or corridor link
    if(!isStdRoom(*room0) && room0->type_ != RoomType::corrLink)
    {
      continue;
    }

    TRACE_VERBOSE << "Finding second room to connect to" << endl;
    Room* room1 = getRndRoom();
    //Room 1 must not be the same as room 0, and it must be a standard room (connections
    //are only allowed between two standard rooms, or from a corridor link to a standard
    //room - never between two corridor links)
    while(room1 == room0 || !isStdRoom(*room1))
    {
      room1 = getRndRoom();
    }

    //Do not allow two rooms to be connected twice
    const auto& consRoom0 = room0->roomsConTo_;
    if(find(consRoom0.begin(), consRoom0.end(), room1) != consRoom0.end())
    {
      TRACE_VERBOSE << "Rooms are already connected, trying other combination" << endl;
      continue;
    }

    //Do not connect room 0 and 1 if another room (except for sub rooms) lies anywhere
    //in a rectangle defined by the two center points of those rooms.
    bool isOtherRoomInWay = false;
    const Pos c0(room0->r_.getCenterPos());
    const Pos c1(room1->r_.getCenterPos());
    const int X0 = min(c0.x, c1.x);
    const int Y0 = min(c0.y, c1.y);
    const int X1 = max(c0.x, c1.x);
    const int Y1 = max(c0.y, c1.y);
    for(int y = Y0; y <= Y1; ++y)
    {
      for(int x = X0; x <= X1; ++x)
      {
        const Room* const roomHere = Map::roomMap[x][y];
        if(roomHere && roomHere != room0 && roomHere != room1 && !roomHere->isSubRoom_)
        {
          isOtherRoomInWay = true;
        }
      }
      if(isOtherRoomInWay)
      {
        break;
      }
    }
    if(isOtherRoomInWay)
    {
      TRACE_VERBOSE << "Blocked by room between, trying other combination" << endl;
      continue;
    }

    MapGenUtils::mkPathFindCor(*room0, *room1, doorProposals);

    if((nrTriesLeft <= 2 || Rnd::oneIn(4)) && isAllRoomsConnected())
    {
      break;
    }
  }
  TRACE_FUNC_END;
}

void mkCrumbleRoom(const Rect& roomAreaInclWalls, const Pos& proxEventPos)
{
  vector<Pos> wallCells;
  vector<Pos> innerCells;

  const Rect& a = roomAreaInclWalls; //abbreviation

  for(int y = a.p0.y; y <= a.p1.y; ++y)
  {
    for(int x = a.p0.x; x <= a.p1.x; ++x)
    {
      const Pos p(x, y);
      if(x == a.p0.x || x == a.p1.x || y == a.p0.y || y == a.p1.y)
      {
        wallCells.push_back(p);
      }
      else
      {
        innerCells.push_back(p);
      }
      Map::put(new Wall(p));
    }
  }

  GameTime::addMob(new ProxEventWallCrumble(proxEventPos, wallCells, innerCells));
}

//Note: The positions and size can be outside map (e.g. negative positions).
//This function just returns false in that case.
bool tryMkAuxRoom(const Pos& p, const Pos& d, bool blocked[MAP_W][MAP_H],
                  const Pos& doorP)
{
  Rect auxRect(p, p + d - 1);
  Rect auxRectWithBorder(auxRect.p0 - 1, auxRect.p1 + 1);

  assert(Utils::isPosInside(doorP, auxRectWithBorder));

  if(Utils::isAreaInsideMap(auxRectWithBorder))
  {
    if(!MapParse::isValInArea(auxRectWithBorder, blocked))
    {
      for(int y = auxRect.p0.y; y <= auxRect.p1.y; ++y)
      {
        for(int x = auxRect.p0.x; x <= auxRect.p1.x; ++x)
        {
          blocked[x][y] = true;
          assert(!Map::roomMap[x][y]);
        }
      }

#ifdef MK_CRUMBLE_ROOMS
      if(Rnd::oneIn(12))
      {
        Room* const room = RoomFactory::mk(RoomType::crumbleRoom, auxRect);
        registerRoom(*room);
        mkCrumbleRoom(auxRectWithBorder, doorP);
      }
      else
#endif // MK_CRUMBLE_ROOMS
      {
        Room* const room = RoomFactory::mkRandomAllowedStdRoom(auxRect, false);
        registerRoom(*room);
        mkFloorInRoom(*room);
      }
      return true;
    }
  }
  return false;
}

void mkAuxRooms(Region regions[3][3])
{
  TRACE_FUNC_BEGIN;
  const int NR_TRIES_PER_SIDE = 10;

  auto getRndAuxRoomDim = []()
  {
    return Pos(Rnd::range(3, 7), Rnd::range(3, 7));
  };

  bool floorCells[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), floorCells);
  Utils::reverseBoolArray(floorCells);

  for(int regionY = 0; regionY < 3; regionY++)
  {
    for(int regionX = 0; regionX < 3; regionX++)
    {
      const Region& region = regions[regionX][regionY];

      if(region.mainRoom_)
      {
        Room& mainR = *region.mainRoom_;

        //Right
        if(Rnd::oneIn(4))
        {
          for(int i = 0; i < NR_TRIES_PER_SIDE; ++i)
          {
            const Pos conP(mainR.r_.p1.x + 1,
                           Rnd::range(mainR.r_.p0.y + 1, mainR.r_.p1.y - 1));
            const Pos auxD(getRndAuxRoomDim());
            const Pos auxP(conP.x + 1, Rnd::range(conP.y - auxD.y + 1, conP.y));
            if(floorCells[conP.x - 1][conP.y])
            {
              if(tryMkAuxRoom(auxP, auxD, floorCells, conP))
              {
                TRACE_VERBOSE << "Aux room placed right" << endl;
                break;
              }
            }
          }
        }

        //Up
        if(Rnd::oneIn(4))
        {
          for(int i = 0; i < NR_TRIES_PER_SIDE; ++i)
          {
            const Pos conP(Rnd::range(mainR.r_.p0.x + 1, mainR.r_.p1.x - 1),
                           mainR.r_.p0.y - 1);
            const Pos auxD(getRndAuxRoomDim());
            const Pos auxP(Rnd::range(conP.x - auxD.x + 1, conP.x), conP.y - 1);
            if(floorCells[conP.x][conP.y + 1])
            {
              if(tryMkAuxRoom(auxP, auxD, floorCells, conP))
              {
                TRACE_VERBOSE << "Aux room placed up" << endl;
                break;
              }
            }
          }
        }

        //Left
        if(Rnd::oneIn(4))
        {
          for(int i = 0; i < NR_TRIES_PER_SIDE; ++i)
          {
            const Pos conP(mainR.r_.p0.x - 1,
                           Rnd::range(mainR.r_.p0.y + 1, mainR.r_.p1.y - 1));
            const Pos auxD(getRndAuxRoomDim());
            const Pos auxP(conP.x - 1, Rnd::range(conP.y - auxD.y + 1, conP.y));
            if(floorCells[conP.x + 1][conP.y])
            {
              if(tryMkAuxRoom(auxP, auxD, floorCells, conP))
              {
                TRACE_VERBOSE << "Aux room placed left" << endl;
                break;
              }
            }
          }
        }

        //Down
        if(Rnd::oneIn(4))
        {
          for(int i = 0; i < NR_TRIES_PER_SIDE; ++i)
          {
            const Pos conP(Rnd::range(mainR.r_.p0.x + 1, mainR.r_.p1.x - 1),
                           mainR.r_.p1.y + 1);
            const Pos auxD(getRndAuxRoomDim());
            const Pos auxP(Rnd::range(conP.x - auxD.x + 1, conP.x), conP.y + 1);
            if(floorCells[conP.x][conP.y - 1])
            {
              if(tryMkAuxRoom(auxP, auxD, floorCells, conP))
              {
                TRACE_VERBOSE << "Aux room placed down" << endl;
                break;
              }
            }
          }
        }
      }
    }
  }
  TRACE_FUNC_END;
}

void mkMergedRegionsAndRooms(Region regions[3][3])
{
  TRACE_FUNC_BEGIN;

  const int NR_ATTEMPTS = Rnd::range(0, 2);

  for(int attemptCnt = 0; attemptCnt < NR_ATTEMPTS; ++attemptCnt)
  {

    Pos regI1, regI2;

    //Find two non-occupied regions
    int nrTriesToFindRegions = 100;
    bool isGoodRegionsFound = false;
    while(!isGoodRegionsFound)
    {
      --nrTriesToFindRegions;
      if(nrTriesToFindRegions <= 0) {return;}

      regI1 = Pos(Rnd::range(0, 2), Rnd::range(0, 1));
      regI2 = Pos(regI1 + Pos(0, 1));
      isGoodRegionsFound = regions[regI1.x][regI1.y].isFree_ &&
                           regions[regI2.x][regI2.y].isFree_;
    }

    //Expand region 1 over both areas
    auto& reg1    = regions[regI1.x][regI1.y];
    auto& reg2    = regions[regI2.x][regI2.y];

    reg1.r_       = Rect(reg1.r_.p0, reg2.r_.p1);

    reg2.r_       = Rect(-1, -1, -1, -1);

    reg1.isFree_  = reg2.isFree_ = false;

    //Make a room for region 1
    auto rndPadding = []() {return Rnd::range(0, 4);};
    const Rect padding(rndPadding(), rndPadding(), rndPadding(), rndPadding());

    const Rect roomRect(reg1.r_.p0 + padding.p0, reg1.r_.p1 - padding.p1);
    Room* const room  = RoomFactory::mkRandomAllowedStdRoom(roomRect, false);
    reg1.mainRoom_    = room;
    registerRoom(*room);
    mkFloorInRoom(*room);
  }
  TRACE_FUNC_END;
}

void randomlyBlockRegions(Region regions[3][3])
{
  TRACE_FUNC_BEGIN << "Marking some (possibly zero) regions as built, to "
                   << "prevent rooms there (so it's not always 3x3 rooms)"
                   << endl;
  //Note: The max number to try can go above the hard limit of regions that
  //could ever be blocked (i.e five regions - blocking is only allowed if
  //no cardinally adjacent region is already blocked). However, this will push
  //the number of attempts towards the upper limit, and increase the chance
  //of a higher number of attempts.
  const int MAX_NR_TO_TRY_BLOCK = max(1, Map::dlvl / 3);
  const int NR_TO_TRY_BLOCK     = min(Rnd::range(0, MAX_NR_TO_TRY_BLOCK), 5);
  for(int i = 0; i < NR_TO_TRY_BLOCK; ++i)
  {
    TRACE_VERBOSE << "Attempting to block region " << i + 1 << "/"
                  << NR_TO_TRY_BLOCK << endl;
    vector<Pos> blockBucket;
    for(int y = 0; y < 3; ++y)
    {
      for(int x = 0; x < 3; ++x)
      {
        if(regions[x][y].isFree_)
        {
          bool isAllAdjFree = true;
          const Pos p(x, y);
          for(const Pos& d : DirUtils::cardinalList)
          {
            const Pos pAdj(p + d);
            if(pAdj >= 0 && pAdj <= 2 && pAdj != p)
            {
              if(!regions[pAdj.x][pAdj.y].isFree_)
              {
                isAllAdjFree = false;
                break;
              }
            }
          }
          if(isAllAdjFree) {blockBucket.push_back(p);}
        }
      }
    }
    if(blockBucket.empty())
    {
      TRACE_VERBOSE << "Failed to find eligible regions to block, after "
                    << "blocking " << i << " regions" << endl;
      break;
    }
    else
    {
      const Pos& p(blockBucket[Rnd::range(0, blockBucket.size() - 1)]);
      TRACE_VERBOSE << "Blocking region at " << p.x << "," << p.y << endl;
      regions[p.x][p.y].isFree_ = false;
    }
  }
  TRACE_FUNC_END;
}

void reserveRiver(Region regions[3][3])
{
  TRACE_FUNC_BEGIN;

  Rect      roomRect;
  Region*   riverRegion       = nullptr;
  const int RESERVED_PADDING  = 2;

  auto initRoomRect = [&](int& len0, int& len1, int& breadth0, int& breadth1,
                          const Pos & reg0, const Pos & reg2)
  {
    const Rect regionsTotRect(regions[reg0.x][reg0.y].r_.p0,
                              regions[reg2.x][reg2.y].r_.p1);
    roomRect    = regionsTotRect;
    riverRegion = &regions[reg0.x][reg0.y];
    const int C = (breadth1 + breadth0) / 2;
    breadth0    = C - RESERVED_PADDING;
    breadth1    = C + RESERVED_PADDING;

    assert(Utils::isAreaInsideOther(roomRect, regionsTotRect, true));

    len0--; //Extend room rectangle to map edge
    len1++;
  };

  const HorizontalVertical dir = Rnd::coinToss() ? hor : ver;

  if(dir == hor)
  {
    initRoomRect(roomRect.p0.x, roomRect.p1.x, roomRect.p0.y, roomRect.p1.y,
                 Pos(0, 1), Pos(2, 1));
  }
  else
  {
    initRoomRect(roomRect.p0.y, roomRect.p1.y, roomRect.p0.x, roomRect.p1.x,
                 Pos(1, 0), Pos(1, 2));
  }

  Room* const       room      = RoomFactory::mk(RoomType::river, roomRect);
  RiverRoom* const  riverRoom = static_cast<RiverRoom*>(room);
  riverRoom->dir_             = dir;
  riverRegion->mainRoom_      = room;
  riverRegion->isFree_        = false;

  if(dir == hor)
  {
    regions[1][1] = regions[2][1] = *riverRegion;
  }
  else
  {
    regions[1][1] = regions[1][2] = *riverRegion;
  }

  Map::roomList.push_back(room);

  auto mk = [&](const int X0, const int X1, const int Y0, const int Y1)
  {
    TRACE_VERBOSE << "Reserving river space with floor cells "
                  << "X0: " << X0 << " X1: " << X1 << " Y0: " << Y0 << " Y1: " << Y1
                  << endl;
    for(int y = Y0; y <= Y1; ++y)
    {
      for(int x = X0; x <= X1; ++x)
      {
        //Just put floor for now, water will be placed later
        Map::put(new Floor(Pos(x, y)));
        Map::roomMap[x][y] = room;
      }
    }
  };

  if(dir == hor)
  {
    mk(roomRect.p0.x + 1, roomRect.p1.x - 1,  roomRect.p0.y,      roomRect.p1.y);
  }
  else
  {
    mk(roomRect.p0.x,     roomRect.p1.x,      roomRect.p0.y + 1,  roomRect.p1.y - 1);
  }

  TRACE_FUNC_END;
}

//void mkCaves(Region regions[3][3])
//{
//  TRACE_FUNC_BEGIN;
//  for(int regY = 0; regY <= 2; regY++)
//  {
//    for(int regX = 0; regX <= 2; regX++)
//    {
//
//      if(regionsToMkCave[regX][regY])
//      {
//
//        Region& region = regions[regX][regY];
//
//        //This region no longer has a room, delete it from list
//        Map::deleteAndRemoveRoomFromList(region.mainRoom_);
//        region.mainRoom_ = nullptr;
//
//        bool blocked[MAP_W][MAP_H];
//
//        for(int x = 0; x < MAP_W; ++x)
//        {
//          for(int y = 0; y < MAP_H; ++y)
//          {
//
//            blocked[x][y] = false;
//
//            if(x == 0 || y == 0 || x == MAP_W - 1 || y == MAP_H - 1)
//            {
//              blocked[x][y] = true;
//            }
//            else
//            {
//              for(int dx = -1; dx <= 1; ++dx)
//              {
//                for(int dy = -1; dy <= 1; ++dy)
//                {
//                  const auto featureId =
//                    Map::cells[x + dx][y + dy].rigid->getId();
//                  if(
//                    featureId == FeatureId::floor &&
//                    !Utils::isPosInside(Pos(x + dx, y + dy), region.r_))
//                  {
//                    blocked[x][y] = true;
//                  }
//                }
//              }
//            }
//          }
//        }
//
//        const Pos origin(region.r_.p0 + Pos(1, 1));
//        int floodFillResult[MAP_W][MAP_H];
//
//        const int FLOOD_FILL_TRAVEL_LIMIT = 20;
//
//        FloodFill::run(origin, blocked, floodFillResult,
//                       FLOOD_FILL_TRAVEL_LIMIT, Pos(-1, -1), true);
//
//        for(int y = 1; y < MAP_H - 1; ++y)
//        {
//          for(int x = 1; x < MAP_W - 1; ++x)
//          {
//            const Pos p(x, y);
//            if(p == origin || floodFillResult[x][y] > 0)
//            {
//
//              Floor* const floor  = new Floor(p);
//              floor->type_        = FloorType::cave;
//              Map::put(floor);
//
//              for(int dx = -1; dx <= 1; ++dx)
//              {
//                for(int dy = -1; dy <= 1; ++dy)
//                {
//                  const Pos adjP(p + Pos(dx, dy));
//                  Cell& adjCell = Map::cells[adjP.x][adjP.y];
//                  if(adjCell.rigid->getId() == FeatureId::wall)
//                  {
//                    Wall* const wall  = new Wall(adjP);
//                    wall->type_       = WallType::cave;
//                    wall->setRandomIsMossGrown();
//                    Map::put(wall);
//                  }
//                }
//              }
//            }
//          }
//        }
//
//        if(Rnd::oneIn(4))
//        {
//          Utils::resetArray(blocked, false);
//
//          for(int y = 1; y < MAP_H - 1; ++y)
//          {
//            for(int x = 1; x < MAP_W - 1; ++x)
//            {
//              for(int dx = -1; dx <= 1; ++dx)
//              {
//                for(int dy = -1; dy <= 1; ++dy)
//                {
//                  Cell& adjCell = Map::cells[x + dx][y + dy];
//                  if(adjCell.rigid->getId() == FeatureId::wall)
//                  {
//                    blocked[x][y] = blocked[x + dx][y + dy] = true;
//                  }
//                }
//              }
//            }
//          }
//
//          FloodFill::run(origin, blocked, floodFillResult,
//                         FLOOD_FILL_TRAVEL_LIMIT / 2, Pos(-1, -1), true);
//
//          for(int y = 1; y < MAP_H - 1; ++y)
//          {
//            for(int x = 1; x < MAP_W - 1; ++x)
//            {
//              const Pos p(x, y);
//              if(!blocked[x][y] && (p == origin || floodFillResult[x][y] > 0))
//              {
//                Map::put(new Chasm(p));
//              }
//            }
//          }
//        }
//      }
//    }
//  }
//  TRACE_FUNC_END;
//}

void placeDoorAtPosIfAllowed(const Pos& p)
{
  //Check that no other doors are within a certain distance
  const int R = 2;
  for(int dx = -R; dx <= R; ++dx)
  {
    for(int dy = -R; dy <= R; ++dy)
    {
      const Pos checkPos = p + Pos(dx, dy);
      if((dx != 0 || dy != 0) && Utils::isPosInsideMap(checkPos))
      {
        const Cell& cell = Map::cells[checkPos.x][checkPos.y];
        if(cell.rigid->getId() == FeatureId::door) {return;}
      }
    }
  }

  bool isGoodVer = true;
  bool isGoodHor = true;

  for(int d = -1; d <= 1; d++)
  {
    if(Map::cells[p.x + d][p.y].rigid->getId() == FeatureId::wall)
    {
      isGoodHor = false;
    }

    if(Map::cells[p.x][p.y + d].rigid->getId() == FeatureId::wall)
    {
      isGoodVer = false;
    }

    if(d != 0)
    {
      if(Map::cells[p.x][p.y + d].rigid->getId() != FeatureId::wall)
      {
        isGoodHor = false;
      }

      if(Map::cells[p.x + d][p.y].rigid->getId() != FeatureId::wall)
      {
        isGoodVer = false;
      }
    }
  }

  if(isGoodHor || isGoodVer)
  {
    const auto& d = FeatureData::getData(FeatureId::wall);
    const auto* const mimic = static_cast<const Rigid*>(d.mkObj(p));
    Map::put(new Door(p, mimic));
  }
}

//Assumes that all rooms are rectangular
void mkSubRooms()
{
  TRACE_FUNC_BEGIN;

  const int NR_TRIES_TO_MK_ROOM = 40;
  const int MAX_NR_INNER_ROOMS  = 7;

  const Pos minD(4, 4);

  for(size_t i = 0; i < Map::roomList.size(); ++i)
  {
    auto* const outerRoom     = Map::roomList[i];

    const Rect  outerRoomRect = outerRoom->r_;
    const Pos   outerRoomD    = (outerRoomRect.p1 - outerRoomRect.p0) + 1;

    const bool IS_ROOM_BIG = outerRoomD.x > 16 || outerRoomD.y > 8;

    const bool IS_STD_ROOM = int(outerRoom->type_) < int(RoomType::END_OF_STD_ROOMS);

    if(IS_STD_ROOM && (IS_ROOM_BIG || Rnd::oneIn(4)))
    {
      const Pos maxD(min(16, outerRoomD.x),  min(16, outerRoomD.y));

      if(maxD >= minD)
      {
        for(int nrRooms = 0; nrRooms < MAX_NR_INNER_ROOMS; nrRooms++)
        {
          for(int tryCount = 0; tryCount < NR_TRIES_TO_MK_ROOM; tryCount++)
          {
            const Pos d(Rnd::range(minD.x, maxD.x),
                        Rnd::range(minD.y, maxD.y));

            const Pos p0(Rnd::range(outerRoomRect.p0.x, outerRoomRect.p1.x - d.x + 1),
                         Rnd::range(outerRoomRect.p0.y, outerRoomRect.p1.y - d.y + 1));

            const Pos p1(p0 + d - 1);

            const Rect r(p0, p1);

            if(r.p0 == outerRoomRect.p0 && r.p1 == outerRoomRect.p1)
            {
              continue;
            }

            bool isAreaFree = true;

            for(int y = p0.y - 1; y <= p1.y + 1; ++y)
            {
              for(int x = p0.x - 1; x <= p1.x + 1; ++x)
              {
                const Pos pCheck(x, y);

                const auto& fId = Map::cells[x][y].rigid->getId();

                if(Utils::isPosInside(pCheck, outerRoomRect))
                {
                  if(fId != FeatureId::floor) {isAreaFree = false;}
                }
                else
                {
                  if(fId != FeatureId::wall)  {isAreaFree = false;}
                }

                if(!isAreaFree) {break;}
              }
              if(!isAreaFree) {break;}
            }

            if(!isAreaFree)
            {
              continue;
            }

            Room* const room = RoomFactory::mkRandomAllowedStdRoom(r, true);
            registerRoom(*room);

            outerRoom->subRooms_.push_back(room);

            vector<Pos> doorBucket;
            for(int y = p0.y; y <= p1.y; ++y)
            {
              for(int x = p0.x; x <= p1.x; ++x)
              {
                if(x == p0.x || x == p1.x || y == p0.y || y == p1.y)
                {
                  Map::put(new Wall(Pos(x, y)));

                  if(
                    x != outerRoomRect.p0.x - 1 && x != outerRoomRect.p0.x      &&
                    x != outerRoomRect.p1.x     && x != outerRoomRect.p1.x + 1  &&
                    y != outerRoomRect.p0.y - 1 && y != outerRoomRect.p0.y      &&
                    y != outerRoomRect.p1.y     && y != outerRoomRect.p1.y + 1)
                  {
                    if((x != p0.x && x != p1.x) || (y != p0.y && y != p1.y))
                    {
                      doorBucket.push_back(Pos(x, y));
                    }
                  }
                }
              }
            }
            if(Rnd::coinToss() || doorBucket.size() <= 2)
            {
              const int DOOR_POS_ELEMENT  = Rnd::range(0, doorBucket.size() - 1);
              const Pos& doorPos          = doorBucket[DOOR_POS_ELEMENT];
              Map::put(new Floor(doorPos));
              doorProposals[doorPos.x][doorPos.y] = true;
            }
            else
            {
              vector<Pos> positionsWithDoor;
              const int NR_TRIES = Rnd::range(1, 10);
              for(int j = 0; j < NR_TRIES; j++)
              {
                const int DOOR_POS_ELEMENT = Rnd::range(0, doorBucket.size() - 1);
                const Pos posCand = doorBucket[DOOR_POS_ELEMENT];

                bool isPosOk = true;
                for(Pos& posWithDoor : positionsWithDoor)
                {
                  if(Utils::isPosAdj(posCand, posWithDoor, false))
                  {
                    isPosOk = false;
                    break;
                  }
                }
                if(isPosOk)
                {
                  Map::put(new Floor(posCand));
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
  TRACE_FUNC_END;
}

void fillDeadEnds()
{
  //Find an origin with no adjacent walls, to ensure not starting in a dead end
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  bool expandedBlockers[MAP_W][MAP_H];
  MapParse::expand(blocked, expandedBlockers);

  Pos origin;
  bool isDone = false;
  for(int y = 2; y < MAP_H - 2; ++y)
  {
    for(int x = 2; x < MAP_W - 2; ++x)
    {
      if(!expandedBlockers[x][y])
      {
        origin = Pos(x, y);
        isDone = true;
        break;
      }
    }
    if(isDone) {break;}
  }

  //Floodfill from origin, then sort the positions for flood value
  int floodFill[MAP_W][MAP_H];
  FloodFill::run(origin, blocked, floodFill, INT_MAX, Pos(-1, -1), true);
  vector<PosAndVal> floodFillVector;
  for(int y = 1; y < MAP_H - 1; ++y)
  {
    for(int x = 1; x < MAP_W - 1; ++x)
    {
      if(!blocked[x][y])
      {
        floodFillVector.push_back(PosAndVal(Pos(x, y), floodFill[x][y]));
      }
    }
  }

  std::sort(floodFillVector.begin(), floodFillVector.end(),
  [](const PosAndVal & a, const PosAndVal & b) {return a.val < b.val;});

  //Fill all positions with only one cardinal floor neighbour
  for(int i = int(floodFillVector.size()) - 1; i >= 0; --i)
  {
    const Pos& pos = floodFillVector[i].pos;
    const int x = pos.x;
    const int y = pos.y;
    const int NR_ADJ_CARDINAL_WALLS = blocked[x + 1][y] + blocked[x - 1][y] +
                                      blocked[x][y + 1] + blocked[x][y - 1];
    if(NR_ADJ_CARDINAL_WALLS == 3)
    {
      Map::put(new Wall(pos));
      blocked[x][y] = true;
    }
  }
}

void decorate()
{
  for(int x = 0; x < MAP_W; ++x)
  {
    for(int y = 0; y < MAP_H; ++y)
    {
      Cell& cell = Map::cells[x][y];

      if(cell.rigid->getId() == FeatureId::wall)
      {
        //Randomly convert walls to rubble
        if(Rnd::oneIn(10))
        {
          Map::put(new RubbleHigh(Pos(x, y)));
          continue;
        }

        //Moss grown walls
        Wall* const wall = static_cast<Wall*>(cell.rigid);
        wall->setRandomIsMossGrown();

        //Convert walls with no adjacent floor or with adjacent cave floor to cave walls
        bool hasAdjFloor      = false;
        bool hasAdjCaveFloor  = false;

        for(const Pos& d : DirUtils::dirList)
        {
          const Pos pAdj(Pos(x, y) + d);

          if(Utils::isPosInsideMap(pAdj))
          {
            auto& adjCell = Map::cells[pAdj.x][pAdj.y];

            if(adjCell.rigid->getId() == FeatureId::floor)
            {
              hasAdjFloor = true;
              if(static_cast<Floor*>(adjCell.rigid)->type_ == FloorType::cave)
              {
                hasAdjCaveFloor = true;
                break;
              }
            }
          }
        }

        if(!hasAdjFloor || hasAdjCaveFloor)
        {
          wall->type_ = WallType::cave;
        }
        else
        {
          wall->setRndCmnWall();
        }

//        if(CellCheck::AllAdjIsNotFeature(FeatureId::floor).check(cell))
//        {
//          wall->type_ = WallType::cave;
//        }
//        else
//        {
//          wall->setRndCmnWall();
//        }
      }
    }
  }

  for(int y = 1; y < MAP_H - 1; ++y)
  {
    for(int x = 1; x < MAP_W - 1; ++x)
    {
      if(Map::cells[x][y].rigid->getId() == FeatureId::floor)
      {
        //Randomly convert stone floor to low rubble
        if(Rnd::oneIn(100))
        {
          Map::put(new RubbleLow(Pos(x, y)));
          continue;
        }
      }
    }
  }
}

void getAllowedStairCells(bool cellsToSet[MAP_W][MAP_H])
{
  TRACE_FUNC_BEGIN;

  vector<FeatureId> featIdsOk {FeatureId::floor, FeatureId::carpet, FeatureId::grass};

  MapParse::parse(CellCheck::AllAdjIsAnyOfFeatures(featIdsOk), cellsToSet);

  for(int x = 0; x < MAP_W; ++x)
  {
    for(int y = 0; y < MAP_H; ++y)
    {
      if(Map::cells[x][y].item)
      {
        cellsToSet[x][y] = false;
      }
    }
  }

  TRACE_FUNC_END;
}

Pos placeStairs()
{
  TRACE_FUNC_BEGIN;

  bool allowedCells[MAP_W][MAP_H];
  getAllowedStairCells(allowedCells);

  vector<Pos> allowedCellsList;
  Utils::mkVectorFromBoolMap(true, allowedCells, allowedCellsList);

  const int NR_OK_CELLS = allowedCellsList.size();

  const int MIN_NR_OK_CELLS_REQ = 10;

  if(NR_OK_CELLS < MIN_NR_OK_CELLS_REQ)
  {
    TRACE << "Nr available cells to place stairs too low "
          << "(" << NR_OK_CELLS << "), discarding map" << endl;
    isMapValid = false;
#ifdef DEMO_MODE
    Render::coverPanel(Panel::log);
    Render::drawMap();
    Render::drawText("To few cells to place stairs", Panel::screen, {0, 0}, clrRedLgt);
    Render::updateScreen();
    SdlWrapper::sleep(8000);
#endif // DEMO_MODE
    return Pos(-1, -1);
  }

  TRACE << "Sorting the allowed cells vector "
        << "(" << allowedCellsList.size() << " cells)" << endl;
  IsCloserToPos isCloserToOrigin(Map::player->pos);
  sort(allowedCellsList.begin(), allowedCellsList.end(), isCloserToOrigin);

  TRACE << "Picking random cell from furthest half" << endl;
  //const int ELEMENT = Rnd::range(NR_OK_CELLS / 2, NR_OK_CELLS - 1);
  const Pos stairsPos(allowedCellsList[NR_OK_CELLS - 1]);

  TRACE << "Spawning stairs at chosen cell" << endl;
  Map::put(new Stairs(stairsPos));

  TRACE_FUNC_END;
  return stairsPos;
}

void movePlayerToNearestAllowedPos()
{
  TRACE_FUNC_BEGIN;

  bool allowedCells[MAP_W][MAP_H];
  getAllowedStairCells(allowedCells);

  vector<Pos> allowedCellsList;
  Utils::mkVectorFromBoolMap(true, allowedCells, allowedCellsList);

  if(allowedCellsList.empty())
  {
    isMapValid = false;
  }
  else
  {
    TRACE << "Sorting the allowed cells vector "
          << "(" << allowedCellsList.size() << " cells)" << endl;
    IsCloserToPos isCloserToOrigin(Map::player->pos);
    sort(allowedCellsList.begin(), allowedCellsList.end(), isCloserToOrigin);

    Map::player->pos = allowedCellsList.front();

  }
  TRACE_FUNC_END;
}

//void mkLevers() {
//  TRACE_FUNC_BEGIN;
//
//  TRACE << "Picking a random door" << endl;
//  vector<Door*> doorBucket;
//  for(int y = 1; y < MAP_H - 1; ++y) {
//    for(int x = 1; x < MAP_W - 1; ++x) {
//      Feature* const feature = Map::featuresStatic[x][y];
//      if(feature->getId() == FeatureId::door) {
//        Door* const door = static_cast<Door*>(feature);
//        doorBucket.push_back(door);
//      }
//    }
//  }
//  Door* const doorToLink = doorBucket[Rnd::range(0, doorBucket.size() - 1)];
//
//  TRACE << "Making floodfill and keeping only positions with lower value than the door" << endl;
//  bool blocked[MAP_W][MAP_H];
//  eng.mapTests->mkMoveBlockerArrayForBodyTypeFeaturesOnly(bodyType_normal, blocked);
//  for(int y = 1; y < MAP_H - 1; ++y) {
//    for(int x = 1; x < MAP_W - 1; ++x) {
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
//  for(int y = 1; y < MAP_H - 1; ++y) {
//    for(int x = 1; x < MAP_W - 1; ++x) {
//      if(floodFill[x][y] < FLOOD_VALUE_AT_DOOR) {
//        if(Map::featuresStatic[x][y]->canHaveRigid()) {
//          leverPosBucket.push_back(Pos(x, y));
//        }
//      }
//    }
//  }
//
//  if(leverPosBucket.size() > 0) {
//    const int ELEMENT = Rnd::range(0, leverPosBucket.size() - 1);
//    const Pos leverPos(leverPosBucket[ELEMENT]);
//    spawnLeverAdaptAndLinkDoor(leverPos, *doorToLink);
//  } else {
//    TRACE << "Failed to find position to place lever" << endl;
//  }
//  TRACE_FUNC_END;
//}

//void spawnLeverAdaptAndLinkDoor(const Pos& leverPos, Door& door) {
//  TRACE << "Spawning lever and linking it to the door" << endl;
//  FeatureFactory::mk(FeatureId::lever, leverPos, new LeverSpawnData(&door));
//
//  TRACE << "Changing door properties" << endl;
//  door.matl_ = DoorType::metal;
//  door.isOpen_ = false;
//  door.isStuck_ = false;
//  door.isHandledExternally_ = true;
//}

void revealDoorsOnPathToStairs(const Pos& stairsPos)
{
  TRACE_FUNC_BEGIN;

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksMoveCmn(false), blocked);

  blocked[stairsPos.x][stairsPos.y] = false;

  for(int x = 0; x < MAP_W; ++x)
  {
    for(int y = 0; y < MAP_H; ++y)
    {
      if(Map::cells[x][y].rigid->getId() == FeatureId::door)
      {
        blocked[x][y] = false;
      }
    }
  }

  vector<Pos> path;
  PathFind::run(Map::player->pos, stairsPos, blocked, path);

  assert(!path.empty());

  TRACE << "Travelling along path and revealing all doors" << endl;
  for(Pos& pos : path)
  {
    auto* const feature = Map::cells[pos.x][pos.y].rigid;
    if(feature->getId() == FeatureId::door)
    {
      static_cast<Door*>(feature)->reveal(false);
    }
  }

  TRACE_FUNC_END;
}

} //namespace

bool mkStdLvl()
{
  TRACE_FUNC_BEGIN;

  isMapValid = true;

  Render::clearScreen();
  Render::updateScreen();

  Map::resetMap();

  TRACE << "Resetting helper arrays" << endl;
  for(int x = 0; x < MAP_W; ++x)
  {
    for(int y = 0; y < MAP_H; ++y)
    {
      doorProposals[x][y] = false;
    }
  }

  //Note: This must be called before any rooms are created
  RoomFactory::initRoomBucket();

  TRACE << "Init regions" << endl;
  const int MAP_W_THIRD = MAP_W / 3;
  const int MAP_H_THIRD = MAP_H / 3;
  const int SPL_X0      = MAP_W_THIRD;
  const int SPL_X1      = (MAP_W_THIRD * 2) + 1;
  const int SPL_Y0      = MAP_H_THIRD;
  const int SPL_Y1      = MAP_H_THIRD * 2;

  Region regions[3][3];

  for(int y = 0; y < 3; ++y)
  {
    for(int x = 0; x < 3; ++x)
    {
      const Rect r(x == 0 ? 1 : x == 1 ? SPL_X0 + 1 : SPL_X1 + 1,
                   y == 0 ? 1 : y == 1 ? SPL_Y0 + 1 : SPL_Y1 + 1,
                   x == 0 ? SPL_X0 - 1 : x == 1 ? SPL_X1 - 1 : MAP_W - 2,
                   y == 0 ? SPL_Y0 - 1 : y == 1 ? SPL_Y1 - 1 : MAP_H - 2);

      regions[x][y] = Region(r);
    }
  }

#ifdef MK_RIVER
  if(isMapValid) {reserveRiver(regions);}
#endif // MK_RIVER

#ifdef MK_MERGED_REGIONS
  if(isMapValid) {mkMergedRegionsAndRooms(regions);}
#endif // MK_MERGED_REGIONS

#ifdef RANDOMLY_BLOCK_REGIONS
  if(isMapValid) {randomlyBlockRegions(regions);}
#endif // RANDOMLY_BLOCK_REGIONS

  if(isMapValid)
  {
    TRACE << "Making main rooms" << endl;
    for(int x = 0; x < 3; ++x)
    {
      for(int y = 0; y < 3; ++y)
      {
        auto& region = regions[x][y];
        if(!region.mainRoom_ && region.isFree_)
        {
          const Rect roomRect = region.getRndRoomRect();
          auto* room          = RoomFactory::mkRandomAllowedStdRoom(roomRect, false);
          registerRoom(*room);
          mkFloorInRoom(*room);
          region.mainRoom_    = room;
          region.isFree_      = false;
        }
      }
    }
  }

#ifdef MK_AUX_ROOMS
#ifdef DEMO_MODE
  Render::coverPanel(Panel::log);
  Render::drawMap();
  Render::drawText("Press any key to make aux rooms...", Panel::screen, {0, 0},
                   clrWhite);
  Render::updateScreen();
  Query::waitForKeyPress();
#endif // DEMO_MODE
  if(isMapValid) {mkAuxRooms(regions);}
#endif // MK_AUX_ROOMS

#ifdef MK_SUB_ROOMS
  if(isMapValid)
  {
#ifdef DEMO_MODE
    Render::coverPanel(Panel::log);
    Render::drawMap();
    Render::drawText("Press any key to make sub rooms...", Panel::screen, {0, 0},
                     clrWhite);
    Render::updateScreen();
    Query::waitForKeyPress();
#endif // DEMO_MODE
    mkSubRooms();
  }
#endif // MK_SUB_ROOMS

  if(isMapValid)
  {
    TRACE << "Sorting the room list according to room type" << endl;
    //Note: This allows common rooms to assume that they are rectangular and
    //have their walls untouched when their reshaping functions run.
    auto cmp = [](const Room * r0, const Room * r1)
    {
      return int(r0->type_) < int(r1->type_);
    };
    sort(Map::roomList.begin(), Map::roomList.end(), cmp);
  }

  TRACE << "Running pre-connect functions for all rooms" << endl;
  if(isMapValid)
  {
#ifdef DEMO_MODE
    Render::coverPanel(Panel::log);
    Render::drawMap();
    Render::drawText("Press any key to run pre-connect functions on rooms...",
                     Panel::screen, {0, 0}, clrWhite);
    Render::updateScreen();
    Query::waitForKeyPress();
#endif // DEMO_MODE
    Gods::setNoGod();
    for(Room* room : Map::roomList) {room->onPreConnect(doorProposals);}
  }

  //Connect
  if(isMapValid)
  {
#ifdef DEMO_MODE
    Render::coverPanel(Panel::log);
    Render::drawMap();
    Render::drawText("Press any key to connect rooms...", Panel::screen, {0, 0},
                     clrWhite);
    Render::updateScreen();
    Query::waitForKeyPress();
#endif // DEMO_MODE
    connectRooms();
  }

  TRACE << "Running post-connect functions for all rooms" << endl;
  if(isMapValid)
  {
#ifdef DEMO_MODE
    Render::coverPanel(Panel::log);
    Render::drawMap();
    Render::drawText("Press any key to run post-connect functions on rooms...",
                     Panel::screen, {0, 0}, clrWhite);
    Render::updateScreen();
    Query::waitForKeyPress();
#endif // DEMO_MODE
    for(Room* room : Map::roomList) {room->onPostConnect(doorProposals);}
  }

#ifdef FILL_DEAD_ENDS
  if(isMapValid)
  {
#ifdef DEMO_MODE
    Render::coverPanel(Panel::log);
    Render::drawMap();
    Render::drawText("Press any key to fill dead ends...", Panel::screen, {0, 0},
                     clrWhite);
    Render::updateScreen();
    Query::waitForKeyPress();
#endif // DEMO_MODE
    fillDeadEnds();
  }
#endif // FILL_DEAD_ENDS

  if(isMapValid)
  {
    TRACE << "Placing doors" << endl;
    for(int x = 0; x < MAP_W; ++x)
    {
      for(int y = 0; y < MAP_H; ++y)
      {
        if(doorProposals[x][y] && Rnd::fraction(7, 10))
        {
          placeDoorAtPosIfAllowed(Pos(x, y));
        }
      }
    }
  }

  if(isMapValid) {movePlayerToNearestAllowedPos();}

  if(isMapValid) {PopulateMon::populateStdLvl();}
  if(isMapValid) {PopulateTraps::populateStdLvl();}
  if(isMapValid) {PopulateItems::mkItems();}

  Pos stairsPos;
  if(isMapValid) {stairsPos = placeStairs();}

  if(isMapValid)
  {
    const int LAST_LVL_TO_REVEAL_STAIRS_PATH = 9;
    if(Map::dlvl <= LAST_LVL_TO_REVEAL_STAIRS_PATH)
    {
      revealDoorsOnPathToStairs(stairsPos);
    }
  }

#ifdef DECORATE
  if(isMapValid) {decorate();}
#endif // DECORATE

  for(auto* r : Map::roomList) {delete r;}
  Map::roomList.clear();
  Utils::resetArray(Map::roomMap);

  TRACE_FUNC_END;
  return isMapValid;
}

} //MapGen

//=============================================================== REGION
Rect Region::getRndRoomRect() const
{
  const bool ALLOW_TINY_W = Rnd::coinToss();

  const Pos minSizeLmt(ALLOW_TINY_W ? 2 : 4, ALLOW_TINY_W ? 4 : 2);
  const Pos maxSizeLmt(r_.p1 - r_.p0 + 1);

  const int   H           = Rnd::range(minSizeLmt.y, maxSizeLmt.y);
  const int   W_MAX_SMALL = minSizeLmt.x + ((maxSizeLmt.x - minSizeLmt.x) / 5);
  const int   W_MAX_BIG   = maxSizeLmt.x;
  const bool  ALLOW_BIG_W = H > (maxSizeLmt.y * 5) / 6;
  const int   W_MAX       = ALLOW_BIG_W ? W_MAX_BIG : W_MAX_SMALL;
  const int   W           = Rnd::range(minSizeLmt.x, W_MAX);

  const Pos p0(r_.p0.x + Rnd::range(0, maxSizeLmt.x - W),
               r_.p0.y + Rnd::range(0, maxSizeLmt.y - H));

  const Pos p1(p0.x + W - 1, p0.y + H - 1);

  return Rect(p0, p1);
}

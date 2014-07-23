#include "Room.h"

#include "Init.h"
#include "Utils.h"
#include "Map.h"
#include "MapParsing.h"
#include "FeatureStatic.h"
#ifdef DEMO_MODE
#include "Renderer.h"
#include "SdlWrapper.h"
#endif // DEMO_MODE

using namespace std;

//-------------------------------------
//Some options (comment out to disable)
//-------------------------------------
#define RESHAPE_STD_ROOMS 1

void Room::onPreConnect(bool doorProposals[MAP_W][MAP_H]) {
  (void)doorProposals;
#ifdef RESHAPE_STD_ROOMS
  if(Rnd::fraction(3, 4)) {MapGenUtils::cutRoomCorners(*this);}
  if(Rnd::fraction(1, 3)) {MapGenUtils::mkPillarsInRoom(*this);}
#endif // RESHAPE_STD_ROOMS
}

void RiverRoom::onPreConnect(bool doorProposals[MAP_W][MAP_H]) {
  TRACE_FUNC_BEGIN;

  //The strategy here is to expand the the river on both sides until parallel
  //to the closest center cell of another room

  const bool IS_HOR = dir_ == hor;

  TRACE << "Finding room centers" << endl;
  bool centers[MAP_W][MAP_H];
  Utils::resetArray(centers, false);

  for(Room* const room : Map::roomList) {
    if(room != this) {
      const Pos cPos(room->getCenterPos());
      centers[cPos.x][cPos.y] = true;
    }
  }

  TRACE << "Finding closest room center coordinates on both sides "
        << "(y coordinate if horizontal river, x if vertical)" << endl;
  int closestCenter0 = -1;
  int closestCenter1 = -1;

  //Using nestled scope to avoid declaring x and y at function scope
  {
    int x, y;

    //iOuter and iInner should be references to x or y.
    auto findClosestCenter0 =
    [&](const Range & rOuter, const Range & rInner, int& iOuter, int& iInner) {
      for(iOuter = rOuter.lower; iOuter >= rOuter.upper; --iOuter) {
        for(iInner = rInner.lower; iInner <= rInner.upper; ++iInner) {
          if(centers[x][y]) {
            closestCenter0 = iOuter;
            break;
          }
        }
        if(closestCenter0 != -1) {break;}
      }
    };

    auto findClosestCenter1 =
    [&](const Range & rOuter, const Range & rInner, int& iOuter, int& iInner) {
      for(iOuter = rOuter.lower; iOuter <= rOuter.upper; ++iOuter) {
        for(iInner = rInner.lower; iInner <= rInner.upper; ++iInner) {
          if(centers[x][y]) {
            closestCenter1 = iOuter;
            break;
          }
        }
        if(closestCenter1 != -1) {break;}
      }
    };

    if(IS_HOR) {
      const int RIVER_Y = r_.p0.y;
      findClosestCenter0(Range(RIVER_Y - 1, 1),         Range(1, MAP_W - 2),  y, x);
      findClosestCenter1(Range(RIVER_Y + 1, MAP_H - 2), Range(1, MAP_W - 2),  y, x);
    } else {
      const int RIVER_X = r_.p0.x;
      findClosestCenter0(Range(RIVER_X - 1, 1),         Range(1, MAP_H - 2),  x, y);
      findClosestCenter1(Range(RIVER_X + 1, MAP_W - 2), Range(1, MAP_H - 2),  x, y);
    }
  }

  TRACE << "Expanding and filling river" << endl;

  bool blocked[MAP_W][MAP_H];

  //Within the expansion limits, mark all cells not belonging to another room as free.
  //All other cells are considered as blocking.
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      blocked[x][y] = true;
      if(
        (dir_ == hor && (y >= closestCenter0 && y <= closestCenter1)) ||
        (dir_ == ver && (x >= closestCenter0 && x <= closestCenter1))
      ) {
        Room* r       = Map::roomMap[x][y];
        blocked[x][y] = r && r != this;
      }
    }
  }
  bool blockedExpanded[MAP_W][MAP_H];
  MapParse::expand(blocked, blockedExpanded, 1, true);

  int flood[MAP_W][MAP_H];
  const Pos origin(getCenterPos());
  FloodFill::run(origin, blockedExpanded, flood, INT_MAX, Pos(-1, -1), true);

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      const Pos p(x, y);
      if(flood[x][y] > 0 || p == origin) {
        LiquidDeep* const liquid  = new LiquidDeep(p);
        liquid->type_             = LiquidType::water;
        Map::put(liquid);
        Map::roomMap[x][y] = this;
        r_.p0.x = min(r_.p0.x, x);
        r_.p0.y = min(r_.p0.y, y);
        r_.p1.x = max(r_.p1.x, x);
        r_.p1.y = max(r_.p1.y, y);
      }
    }
  }

  TRACE << "Making bridge(s)" << endl;

  //Mark which side each cell belongs to
  enum Side {inRiver, side0, side1};
  Side sides[MAP_W][MAP_H];

  //Using nestled scope to avoid declaring x and y at function scope
  {
    int x, y;

    //iOuter and iInner should be references to x or y.
    auto markSides =
    [&](const Range & rOuter, const Range & rInner, int& iOuter, int& iInner) {
      for(iOuter = rOuter.lower; iOuter <= rOuter.upper; ++iOuter) {
        bool isOnSide0 = true;
        for(iInner = rInner.lower; iInner <= rInner.upper; ++iInner) {
          if(Map::roomMap[x][y] == this) {
            isOnSide0 = false;
            sides[x][y] = inRiver;
          } else {
            sides[x][y] = isOnSide0 ? side0 : side1;
          }
        }
      }
    };

    if(dir_ == hor) {
      markSides(Range(1, MAP_W - 2), Range(1, MAP_H - 2), x, y);
    } else {
      markSides(Range(1, MAP_H - 2), Range(1, MAP_W - 2), y, x);
    }
  }

  bool validRoomEntries0[MAP_W][MAP_H];
  bool validRoomEntries1[MAP_W][MAP_H];
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      validRoomEntries0[x][y] = validRoomEntries1[x][y] = false;
    }
  }

  const int EDGE_D = 4;
  for(int x = EDGE_D; x < MAP_W - EDGE_D; ++x) {
    for(int y = EDGE_D; y < MAP_H - EDGE_D; ++y) {
      const FeatureId featureId = Map::cells[x][y].featureStatic->getId();
      if(featureId == FeatureId::wall && !Map::roomMap[x][y]) {
        const Pos p(x, y);
        int nrCardinalFloor  = 0;
        int nrCardinalRiver  = 0;
        for(const auto& d : DirUtils::cardinalList) {
          const auto pAdj(p + d);
          const auto* const f = Map::cells[pAdj.x][pAdj.y].featureStatic;
          if(f->getId() == FeatureId::floor)        {nrCardinalFloor++;}
          if(Map::roomMap[pAdj.x][pAdj.y] == this)  {nrCardinalRiver++;}
        }
        if(nrCardinalFloor == 1 && nrCardinalRiver == 1) {
          switch(sides[x][y]) {
            case side0:   {validRoomEntries0[x][y] = true;} break;
            case side1:   {validRoomEntries1[x][y] = true;} break;
            case inRiver: {} break;
          }
        }
      }
    }
  }

#ifdef DEMO_MODE
  Renderer::drawMap();
  for(int y = 1; y < MAP_H - 1; ++y) {
    for(int x = 1; x < MAP_W - 1; ++x) {
      Pos p(x, y);
      if(validRoomEntries0[x][y]) {
        Renderer::drawGlyph('0', Panel::map, p, clrRedLgt);
      }
      if(validRoomEntries1[x][y]) {
        Renderer::drawGlyph('1', Panel::map, p, clrRedLgt);
      }
      if(validRoomEntries0[x][y] || validRoomEntries1[x][y]) {
        Renderer::updateScreen();
        SdlWrapper::sleep(100);
      }
    }
  }
#endif // DEMO_MODE

  vector<int> coordinates(IS_HOR ? MAP_W : MAP_H);
  iota(begin(coordinates), end(coordinates), 0);
  random_shuffle(coordinates.begin(), coordinates.end());

  vector<int> cBuilt;

  const int MIN_EDGE_DIST   = 6;
  const int MAX_NR_BRIDGES  = Rnd::range(1, 3);

  for(const int BRIDGE_C : coordinates) {
    if(
      BRIDGE_C < MIN_EDGE_DIST ||
      (IS_HOR  && BRIDGE_C > MAP_W - 1 - MIN_EDGE_DIST) ||
      (!IS_HOR && BRIDGE_C > MAP_H - 1 - MIN_EDGE_DIST)) {
      continue;
    }
    bool isTooCloseToOtherBridge = false;
    const int MIN_D = 2;
    for(int cOther : cBuilt) {
      if(Utils::isValInRange(BRIDGE_C, Range(cOther - MIN_D, cOther + MIN_D))) {
        isTooCloseToOtherBridge = true;
        break;
      }
    }
    if(isTooCloseToOtherBridge) {continue;}

    //Check if current bridge coord would connect matching room connections.
    //If so both roomCon0 and roomCon1 will be set.
    Pos roomCon0(-1, -1);
    Pos roomCon1(-1, -1);
    const int C0_0 = IS_HOR ? r_.p1.y : r_.p1.x;
    const int C1_0 = IS_HOR ? r_.p0.y : r_.p0.x;
    for(int c = C0_0; c != C1_0; --c) {
      if(
        (IS_HOR  && sides[BRIDGE_C][c] == side0) ||
        (!IS_HOR && sides[c][BRIDGE_C] == side0)) {
        break;
      }
      const Pos pNxt = IS_HOR ? Pos(BRIDGE_C, c - 1) : Pos(c - 1, BRIDGE_C);
      if(validRoomEntries0[pNxt.x][pNxt.y]) {
        roomCon0 = pNxt;
        break;
      }
    }
    const int C0_1 = IS_HOR ? r_.p0.y : r_.p0.x;
    const int C1_1 = IS_HOR ? r_.p1.y : r_.p1.x;
    for(int c = C0_1; c != C1_1; ++c) {
      if(
        (IS_HOR  && sides[BRIDGE_C][c] == side1) ||
        (!IS_HOR && sides[c][BRIDGE_C] == side1)) {
        break;
      }
      const Pos pNxt = IS_HOR ? Pos(BRIDGE_C, c + 1) : Pos(c + 1, BRIDGE_C);
      if(validRoomEntries1[pNxt.x][pNxt.y]) {
        roomCon1 = pNxt;
        break;
      }
    }

    //Make the bridge if valid connection pairs found
    if(roomCon0.x != -1 && roomCon1.x != -1) {
#ifdef DEMO_MODE
      Renderer::drawMap();
      Renderer::drawGlyph('0', Panel::map, roomCon0, clrGreenLgt);
      Renderer::drawGlyph('1', Panel::map, roomCon1, clrYellow);
      Renderer::updateScreen();
      SdlWrapper::sleep(2000);
#endif // DEMO_MODE

      TRACE << "Found valid connection pair at: "
            << roomCon0.x << "," << roomCon0.y << " / "
            << roomCon1.x << "," << roomCon1.y << endl
            << "Making bridge at coord: " << BRIDGE_C << endl;
      if(IS_HOR) {
        for(int y = roomCon0.y; y <= roomCon1.y; ++y) {
          if(Map::roomMap[BRIDGE_C][y] == this) {
            auto* const bridge = new Bridge(Pos(BRIDGE_C, y));
            bridge->setDir(ver);
            Map::put(bridge);
          }
        }
      } else {
        for(int x = roomCon0.x; x <= roomCon1.x; ++x) {
          if(Map::roomMap[x][BRIDGE_C] == this) {
            auto* const bridge = new Bridge(Pos(x, BRIDGE_C));
            bridge->setDir(hor);
            Map::put(bridge);
          }
        }
      }
      Map::put(new Floor(roomCon0));
      Map::put(new Floor(roomCon1));
      doorProposals[roomCon0.x][roomCon0.y] = true;
      doorProposals[roomCon1.x][roomCon1.y] = true;
      cBuilt.push_back(BRIDGE_C);
    }
    if(int(cBuilt.size()) >= MAX_NR_BRIDGES) {
      TRACE << "Enough bridges built" << endl;
      break;
    }
  }
  TRACE << "Bridges built/attempted: " << cBuilt.size() << "/"
        << MAX_NR_BRIDGES << endl;

  if(cBuilt.empty()) {
    MapGen::isMapValid = false;
  } else {
    TRACE << "Converting some remaining valid room entries to floor" << endl;
    for(int y = 0; y < MAP_H; ++y) {
      for(int x = 0; x < MAP_W; ++x) {
        if(validRoomEntries0[x][y] || validRoomEntries1[x][y]) {
          if(
            find(cBuilt.begin(), cBuilt.end(), x) == cBuilt.end()) {
            if(Rnd::oneIn(4)) {
              Map::put(new Floor(Pos(x, y)));
              Map::roomMap[x][y] = this;
            }
          }
        }
      }
    }
  }

  TRACE_FUNC_END;
}

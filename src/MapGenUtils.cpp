#include "Init.h"

#include "MapGen.h"

#include <vector>
#include <assert.h>

#include "Map.h"
#include "FeatureFactory.h"
#include "MapParsing.h"
#include "Utils.h"
#include "MapTemplates.h"

#ifdef DEMO_MODE
#include "SdlWrapper.h"
#include "Renderer.h"
#endif // DEMO_MODE

using namespace std;

namespace MapGen {

bool isMapValid = true;

}

namespace MapGenUtils {

namespace {

FeatureId backup[MAP_W][MAP_H];

void getFloorCellsInRoom(const Room& room, const bool floor[MAP_W][MAP_H],
                         vector<Pos>& out) {
  assert(Utils::isAreaInsideMap(room.r_));

  for(int y = room.r_.p0.y; y <= room.r_.p1.y; ++y) {
    for(int x = room.r_.p0.x; x <= room.r_.p1.x; ++x) {
      if(floor[x][y]) {out.push_back(Pos(x, y));}
    }
  }
}

} //namespace

void cutRoomCorners(const Room& room) {
  const Pos& roomP0  = room.r_.p0;
  const Pos& roomP1  = room.r_.p1;

  const Pos roomDims = roomP1 - roomP0 + 1;

  if(roomDims.x < 6 || roomDims.y < 6) {return;}

  const Pos maxDims(roomDims - 4);

  const Pos crossDims(Rnd::range(2, maxDims.x), Rnd::range(2, maxDims.y));

  const Pos crossX0Y0(Rnd::range(roomP0.x + 2, roomP1.x - crossDims.x - 1),
                      Rnd::range(roomP0.y + 2, roomP1.y - crossDims.y - 1));

  const Pos crossX1Y1(crossX0Y0 + crossDims - 1);

  //Which corners to place - up-left, up-right, down-left, down-right
  bool c[4] = {true, true, true, true};
  if(Rnd::fraction(2, 3)) {
    while(true) {
      int nrCorners = 0;
      for(int i = 0; i < 4; ++i) {
        if(Rnd::coinToss()) {c[i] = true; nrCorners++;} else {c[i] = false;}
      }
      if(nrCorners > 0) {break;}
    }
  }

  for(int y = roomP0.y; y <= roomP1.y; ++y) {
    for(int x = roomP0.x; x <= roomP1.x; ++x) {
      const bool X_OK = x < crossX0Y0.x ? (c[0] || c[2]) :
                        x > crossX1Y1.x ? (c[1] || c[3]) : false;
      const bool Y_OK = y < crossX0Y0.y ? (c[0] || c[1]) :
                        y > crossX1Y1.y ? (c[2] || c[3]) : false;

      if(X_OK && Y_OK) {
        FeatureFactory::mk(FeatureId::wall, Pos(x, y), nullptr);
        Map::roomMap[x][y] = nullptr;
      }
    }
  }
}

void mkPillarsInRoom(const Room& room) {
  const Pos& roomP0(room.r_.p0);
  const Pos& roomP1(room.r_.p1);

  auto isFree = [](const Pos & p) {
    for(int dx = -1; dx <= 1; dx++) {
      for(int dy = -1; dy <= 1; dy++) {
        const auto* const f = Map::cells[p.x + dx][p.y + dy].featureStatic;
        if(f->getId() == FeatureId::wall) {return false;}
      }
    }
    return true;
  };

  const FeatureId pillarId = FeatureId::wall;

  if(Rnd::fraction(2, 3)) {
    //Place pillars in rows and columns (but occasionally skip a pillar)
    auto getStepSize = []() {return Rnd::range(1, 2);};
    const int DX = getStepSize();
    const int DY = getStepSize();

    for(int y = roomP0.y + 1; y <= roomP1.y - 1; y += DY) {
      for(int x = roomP0.x + 1; x <= roomP1.x - 1; x += DX) {
        const Pos p(x, y);
        if(isFree(p) && Rnd::fraction(2, 3)) {FeatureFactory::mk(pillarId, p);}
      }
    }
  } else {
    //Scatter pillars randomly
    for(int y = roomP0.y + 1; y <= roomP1.y - 1; ++y) {
      for(int x = roomP0.x + 1; x <= roomP1.x - 1; ++x) {
        const Pos p(x + Rnd::range(-1, 1), y + Rnd::range(-1, 1));
        if(isFree(p) && Rnd::oneIn(5)) {FeatureFactory::mk(pillarId, p);}
      }
    }
  }
}

void getValidRoomCorrEntries(const Room& room, vector<Pos>& out) {
  TRACE_FUNC_BEGIN_VERBOSE;
  //Find all cells that meets all of the following criteria:
  //(1) Is a wall cell
  //(2) Is a cell not belonging to any room
  //(3) Is not on the edge of the map
  //(4) Is cardinally adjacent to a floor cell belonging to the room
  //(5) Is cardinally adjacent to a cell not in the room or room outline

  out.resize(0);

  bool roomCells[MAP_W][MAP_H];
  bool roomFloorCells[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      const bool IS_ROOM_CELL = Map::roomMap[x][y] == &room;
      roomCells[x][y]         = IS_ROOM_CELL;
      const auto* const f     = Map::cells[x][y].featureStatic;
      roomFloorCells[x][y]    = IS_ROOM_CELL && f->getId() == FeatureId::floor;
    }
  }

  bool roomCellsExpanded[MAP_W][MAP_H];
  MapParse::expand(roomCells, roomCellsExpanded, 1, true);

  for(int y = room.r_.p0.y - 1; y <= room.r_.p1.y + 1; ++y) {
    for(int x = room.r_.p0.x - 1; x <= room.r_.p1.x + 1; ++x) {
      //Condition (1)
      if(Map::cells[x][y].featureStatic->getId() != FeatureId::wall) {continue;}

      //Condition (2)
      if(Map::roomMap[x][y]) {continue;}

      //Condition (3)
      if(x <= 1 || y <= 1 || x >= MAP_W - 2 || y >= MAP_H - 2) {continue;}

      bool isAdjToFloorInRoom = false;
      bool isAdjToCellOutside = false;

      const Pos p(x, y);

      bool isAdjToFloorNotInRoom = false;

      for(const Pos& d : DirUtils::dirList) {
        const Pos& pAdj(p + d);

        if(DirUtils::isCardinal(d)) {
          //Condition (4)
          if(roomFloorCells[pAdj.x][pAdj.y])      {isAdjToFloorInRoom = true;}

          //Condition (5)
          if(!roomCellsExpanded[pAdj.x][pAdj.y])  {isAdjToCellOutside = true;}
        }
      }

      if(!isAdjToFloorNotInRoom && isAdjToFloorInRoom && isAdjToCellOutside) {
        out.push_back(p);
      }
    }
  }
  TRACE_FUNC_END_VERBOSE;
}

//Note: The parameter rectangle does not have to go up-left to bottom-right,
//the method adjusts the order
//void mk(const Rect& area, const FeatureId id) {
//  const Pos p0 = Pos(min(area.p0.x, area.p1.x), min(area.p0.y, area.p1.y));
//  const Pos p1 = Pos(max(area.p0.x, area.p1.x), max(area.p0.y, area.p1.y));
//
//  for(int x = p0.x; x <= p1.x; ++x) {
//    for(int y = p0.y; y <= p1.y; ++y) {
//      FeatureFactory::mk(id, Pos(x, y), nullptr);
//    }
//  }
//}

void mkPathFindCor(Room& r0, Room& r1, bool doorProposals[MAP_W][MAP_H]) {
  TRACE_FUNC_BEGIN_VERBOSE << "Making corridor between rooms "
                           << &r0 << " and " << &r1 << endl;

  assert(Utils::isAreaInsideMap(r0.r_));
  assert(Utils::isAreaInsideMap(r1.r_));

  vector<Pos> p0Bucket;
  vector<Pos> p1Bucket;
  getValidRoomCorrEntries(r0, p0Bucket);
  getValidRoomCorrEntries(r1, p1Bucket);

  if(p0Bucket.empty()) {
    TRACE_FUNC_END_VERBOSE << "No entry points found in room 0" << endl;
    return;
  }
  if(p1Bucket.empty()) {
    TRACE_FUNC_END_VERBOSE << "No entry points found in room 1" << endl;
    return;
  }

  int shortestDist = INT_MAX;

  TRACE_VERBOSE << "Finding shortest possible dist between entries" << endl;
  for(const Pos& p0 : p0Bucket) {
    for(const Pos& p1 : p1Bucket) {
      const int CUR_DIST = Utils::kingDist(p0, p1);
      if(CUR_DIST < shortestDist) {shortestDist = CUR_DIST;}
    }
  }

  TRACE_VERBOSE << "Storing entry pairs with shortest dist ("
                << shortestDist << ")" << endl;
  vector< pair<Pos, Pos> > entriesBucket;

  for(const Pos& p0 : p0Bucket) {
    for(const Pos& p1 : p1Bucket) {
      const int CUR_DIST = Utils::kingDist(p0, p1);
      if(CUR_DIST == shortestDist) {
        entriesBucket.push_back(pair<Pos, Pos>(p0, p1));
      }
    }
  }

  TRACE_VERBOSE << "Picking a random stored entry pair" << endl;
  const pair<Pos, Pos>& entries =
    entriesBucket.at(Rnd::range(0, entriesBucket.size() - 1));
  const Pos& p0 = entries.first;
  const Pos& p1 = entries.second;

  vector<Pos> path;

  //Is entry points same cell (rooms are adjacent)? Then simply use that
  if(p0 == p1) {
    path.push_back(p0);
  } else {
    //Else, try to find a path to the other entry point
    bool blocked[MAP_W][MAP_H];
    Utils::resetArray(blocked, false);

    for(int y = 0; y < MAP_H; ++y) {
      for(int x = 0; x < MAP_W; ++x) {
        blocked[x][y] =
          Map::roomMap[x][y] ||
          Map::cells[x][y].featureStatic->getId() != FeatureId::wall;
      }
    }

    bool blockedExpanded[MAP_W][MAP_H];
    MapParse::expand(blocked, blockedExpanded, 1, true);

    blockedExpanded[p0.x][p0.y] = blockedExpanded[p1.x][p1.y] = false;

    PathFind::run(p0, p1, blockedExpanded, path, false);
  }

  if(!path.empty()) {
    path.push_back(p0);

    TRACE_VERBOSE << "Check that the path doesn't circle around the origin "
                  << "or target room" << endl;
    vector<Room*> rooms {&r0, &r1};
    for(Room* room : rooms) {
      bool isLeftOfRoom   = false;
      bool isRightOfRoom  = false;
      bool isAboveRoom    = false;
      bool isBelowRoom    = false;
      for(Pos& p : path) {
        if(p.x < room->r_.p0.x) {isLeftOfRoom   = true;}
        if(p.x > room->r_.p1.x) {isRightOfRoom  = true;}
        if(p.y < room->r_.p0.y) {isAboveRoom    = true;}
        if(p.y > room->r_.p1.y) {isBelowRoom    = true;}
      }
      if((isLeftOfRoom && isRightOfRoom) || (isAboveRoom && isBelowRoom)) {
        TRACE_FUNC_END_VERBOSE
            << "Path circled around room (looks bad), not making corridor"
            << endl;
        return;
      }
    }

    vector<Room*> prevJunctions;

    for(size_t i = 0; i < path.size(); ++i) {
      const Pos& p(path.at(i));
      FeatureFactory::mk(FeatureId::floor, p, nullptr);

      if(i > 1 && int(i) < int(path.size() - 3) && i % 6 == 0) {
        Room* junction  = new Room(Rect(p, p));
        junction->type_ = RoomType::corridorJunction;
        Map::roomList.push_back(junction);
        Map::roomMap[p.x][p.y] = junction;
        junction->roomsConTo_.push_back(&r0);
        junction->roomsConTo_.push_back(&r1);
        r0.roomsConTo_.push_back(junction);
        r1.roomsConTo_.push_back(junction);
        for(Room* prevJunction : prevJunctions) {
          junction->roomsConTo_.push_back(prevJunction);
          prevJunction->roomsConTo_.push_back(junction);
        }
        prevJunctions.push_back(junction);
      }
    }

    if(doorProposals) {
      doorProposals[p0.x][p0.y] = doorProposals[p1.x][p1.y] = true;
    }
    r0.roomsConTo_.push_back(&r1);
    r1.roomsConTo_.push_back(&r0);
    TRACE_FUNC_END_VERBOSE << "Successfully connected roooms" << endl;
    return;
  }

  TRACE_FUNC_END_VERBOSE << "Failed to connect roooms" << endl;
}

void backupMap() {
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      backup[x][y] = Map::cells[x][y].featureStatic->getId();
    }
  }
}

void restoreMap() {
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      FeatureFactory::mk(backup[x][y], Pos(x, y));
    }
  }
}

void mkWithPathfinder(const Pos& p0, const Pos& p1, FeatureId feature,
                      const bool IS_SMOOTH, const bool DIG_ANY_FEATURE) {
  bool blocked[MAP_W][MAP_H];
  Utils::resetArray(blocked, false);
  vector<Pos> path;
  PathFind::run(p0, p1, blocked, path);
  const int PATH_SIZE = path.size();
  for(int i = 0; i < PATH_SIZE; ++i) {
    const Pos c = path.at(i);
    const auto* const f = Map::cells[c.x][c.y].featureStatic;
    if(f->canHaveStaticFeature() || DIG_ANY_FEATURE) {
      FeatureFactory::mk(feature, c);
      if(!IS_SMOOTH && Rnd::percentile() < 33) {
        mkByRandomWalk(c, Rnd::dice(1, 6), feature, true);
      }
    }
  }
}

void mkByRandomWalk(const Pos& p0, int len, FeatureId featureToMk,
                    const bool DIG_ANY_FEATURE,
                    const bool ONLY_STRAIGHT, const Pos& p0Lim,
                    const Pos& p1Lim) {
  int dx = 0;
  int dy = 0;
  int xPos = p0.x;
  int yPos = p0.y;

  vector<Pos> positionsToSet;

  bool dirOk = false;
  while(len > 0) {
    while(!dirOk) {
      dx = Rnd::dice(1, 3) - 2;
      dy = Rnd::dice(1, 3) - 2;
      //TODO This is really ugly!
      dirOk =
        !(
          (dx == 0 && dy == 0) || xPos + dx < p0Lim.x ||
          yPos + dy < p0Lim.y || xPos + dx > p1Lim.x ||
          yPos + dy > p1Lim.y ||
          (ONLY_STRAIGHT && dx != 0 && dy != 0)
        );
    }
    const auto* const f = Map::cells[xPos + dx][yPos + dy].featureStatic;
    if(f->canHaveStaticFeature() || DIG_ANY_FEATURE) {
      positionsToSet.push_back(Pos(xPos + dx, yPos + dy));
      xPos += dx;
      yPos += dy;
      len--;
    }
    dirOk = false;
  }
  for(unsigned int i = 0; i < positionsToSet.size(); ++i) {
    FeatureFactory::mk(featureToMk, positionsToSet.at(i));
  }
}

void mkFromTempl(const Pos& pos, const MapTempl& t) {
  for(int dy = 0; dy < t.h; dy++) {
    for(int dx = 0; dx < t.w; dx++) {
      const auto featureId = t.featureVector[dy][dx];
      if(featureId != FeatureId::empty) {
        FeatureFactory::mk(featureId, pos + Pos(dx, dy));
      }
    }
  }
}

void mkFromTempl(const Pos& pos, MapTemplId templateId) {
  mkFromTempl(pos, MapTemplHandling::getTempl(templateId));
}

} //MapGenUtils

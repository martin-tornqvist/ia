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

namespace MapGenUtils {

namespace {

FeatureId backup[MAP_W][MAP_H];

void getFloorCellsInRoom(const Room& room, const bool floor[MAP_W][MAP_H],
                         vector<Pos>& out) {
  assert(Utils::isAreaInsideMap(room.r_));

  for(int y = room.r_.p0.y; y <= room.r_.p1.y; y++) {
    for(int x = room.r_.p0.x; x <= room.r_.p1.x; x++) {
      if(floor[x][y]) {out.push_back(Pos(x, y));}
    }
  }
}

} //namespace

void getValidRoomCorrEntries(const Room& room, std::vector<Pos>& out) {
  //Find all cells that meets all of the following criteria:
  //(1) Is a cell not belonging to any room
  //(2) Is not on the edge of the map
  //(3) Is cardinally adjacent to a floor cell belonging to the room
  //(4) Is cardinally adjacent to a cell not in the room or room outline

  out.resize(0);

  bool roomCells[MAP_W][MAP_H];
  bool roomFloorCells[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      const bool IS_ROOM_CELL = Map::roomMap[x][y] == &room;
      roomCells[x][y]         = IS_ROOM_CELL;
      const auto* const f     = Map::cells[x][y].featureStatic;
      roomFloorCells[x][y]    = IS_ROOM_CELL && f->getId() == FeatureId::floor;
    }
  }

  bool roomAndOutlineCells[MAP_W][MAP_H];
  MapParse::expand(roomCells, roomAndOutlineCells, 1, true);

  const vector<Pos> cardinals {Pos(1, 0), Pos(-1, 0), Pos(0, 1), Pos(0, -1)};

  for(int y = room.r_.p0.y - 1; y <= room.r_.p1.y + 1; y++) {
    for(int x = room.r_.p0.x - 1; x <= room.r_.p1.x + 1; x++) {
      //Condition (1)
      if(Map::roomMap[x][y]) {continue;}

      //Condition (2)
      if(x <= 1 || y <= 1 || x >= MAP_W - 2 || y >= MAP_H - 2) {continue;}

      bool isAdjToFloorInRoom = false;
      bool isAdjToCellOutside = false;

      const Pos p(x, y);

      for(const Pos& d : cardinals) {
        const Pos& pAdj(p + d);

        //Condition 3
        if(roomFloorCells[pAdj.x][pAdj.y])      {isAdjToFloorInRoom = true;}

        //Condition 4
        if(roomAndOutlineCells[pAdj.x][pAdj.y]) {isAdjToCellOutside = true;}
      }

      if(isAdjToFloorInRoom && isAdjToCellOutside) {out.push_back(p);}
    }
  }
}

//Note: The parameter rectangle does not have to go up-left to bottom-right,
//the method adjusts the order
void mk(const Rect& area, const FeatureId id) {
  const Pos p0 = Pos(min(area.p0.x, area.p1.x), min(area.p0.y, area.p1.y));
  const Pos p1 = Pos(max(area.p0.x, area.p1.x), max(area.p0.y, area.p1.y));

  for(int x = p0.x; x <= p1.x; x++) {
    for(int y = p0.y; y <= p1.y; y++) {
      FeatureFactory::mk(id, Pos(x, y), nullptr);
    }
  }
}

void mkPathFindCor(Room& r0, Room& r1, bool doorPosProposals[MAP_W][MAP_H]) {
  TRACE_FUNC_BEGIN_VERBOSE;
  TRACE_VERBOSE << "Making corridor between rooms "
                << &r0 << " and " << &r1 << endl;

  assert(Utils::isAreaInsideMap(r0.r_));
  assert(Utils::isAreaInsideMap(r1.r_));

  bool floor[MAP_W][MAP_H];
  MapParse::parse(CellPred::IsFeature(FeatureId::floor), floor);

  vector<Pos> p0Bucket;
  vector<Pos> p1Bucket;
  getValidRoomCorrEntries(r0, p0Bucket);
  getValidRoomCorrEntries(r1, p1Bucket);
  assert(!p0Bucket.empty());
  assert(!p1Bucket.empty());

  int shortestDist = INT_MAX;

  for(const Pos& p0 : p0Bucket) {
    for(const Pos& p1 : p1Bucket) {
      const int CUR_DIST = Utils::kingDist(p0, p1);
      if(CUR_DIST < shortestDist) {shortestDist = CUR_DIST;}
    }
  }

  vector< pair<Pos, Pos> > entriesBucket;

//  for(int i = 0; i < 10; i++)  {

  for(const Pos& p0 : p0Bucket) {
    for(const Pos& p1 : p1Bucket) {
      const int CUR_DIST = Utils::kingDist(p0, p1);
      if(CUR_DIST == shortestDist) {
        entriesBucket.push_back(pair<Pos, Pos>(p0, p1));
      }
    }
  }

  assert(!entriesBucket.empty());

  const pair<Pos, Pos>& entries =
    entriesBucket.at(Rnd::range(0, entriesBucket.size() - 1));
  const Pos& p0 = entries.first;
  const Pos& p1 = entries.second;

#ifdef DEMO_MODE
  Renderer::drawBlastAnimAtPositions(vector<Pos> {p0}, clrGreenLgt);
  Renderer::drawBlastAnimAtPositions(vector<Pos> {p1}, clrRedLgt);
#endif // DEMO_MODE

  vector<Pos> path;

  //IS entry points same cell (rooms are adjacent)? Then simply use that
  if(p0 == p1) {
    path.push_back(p0);
  } else {
    //Else, try to find a path to the other entry point
    bool blocked[MAP_W][MAP_H];
    Utils::resetArray(blocked, false);

    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        blocked[x][y] =
          Map::cells[x][y].featureStatic->getId() != FeatureId::wall ||
          Map::roomMap[x][y];
      }
    }
    bool blockedExpanded[MAP_W][MAP_H];
    MapParse::expand(blocked, blockedExpanded, 1, true);
    blockedExpanded[p0.x][p0.y] = blockedExpanded[p1.x][p1.y] = false;

    PathFind::run(p0, p1, blockedExpanded, path, false);
  }

  if(!path.empty()) {
    path.push_back(p0);
    for(Pos& p : path) {FeatureFactory::mk(FeatureId::floor, p, nullptr);}
    doorPosProposals[p0.x][p0.y] = doorPosProposals[p1.x][p1.y] = true;
    r0.roomsConTo_.push_back(&r1);
    r1.roomsConTo_.push_back(&r0);
    TRACE_FUNC_END_VERBOSE << "Successfully connected roooms" << endl;
    return;
  }
//  }
  TRACE_FUNC_END_VERBOSE << "Failed to connect roooms" << endl;
}

void backupMap() {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      backup[x][y] = Map::cells[x][y].featureStatic->getId();
    }
  }
}

void restoreMap() {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
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
  for(int i = 0; i < PATH_SIZE; i++) {
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
          (ONLY_STRAIGHT == true && dx != 0 && dy != 0)
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
  for(unsigned int i = 0; i < positionsToSet.size(); i++) {
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

#include "MapGen.h"

#include "ActorPlayer.h"
#include "ItemFactory.h"
#include "Map.h"
#include "MapParsing.h"
#include "Utils.h"
#include "FeatureStatic.h"

using namespace std;

namespace MapGen {

namespace TrapezohedronLvl {

bool run() {
  Map::resetMap();

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      auto* const wall  = new Wall(Pos(x, y));
      Map::put(wall);
      wall->type_       = WallType::cave;
      wall->isMossy_    = false;
    }
  }

  const Pos& origin     = Map::player->pos;
  const Pos  mapCenter  = Pos(MAP_W_HALF, MAP_H_HALF);

  auto putCaveFloor = [](const vector<Pos>& positions) {
    for(const Pos& p : positions) {
      auto* const floor = new Floor(p);
      Map::put(floor);
      floor->type_      = FloorType::cave;
    }
  };

  vector<Pos> floorPositions;

  MapGenUtils::rndWalk(origin, 150, floorPositions, true);
  putCaveFloor(floorPositions);

  MapGenUtils::rndWalk(mapCenter, 800, floorPositions, true);
  putCaveFloor(floorPositions);

  MapGenUtils::pathfinderWalk(origin, mapCenter, floorPositions, false);
  putCaveFloor(floorPositions);

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  vector<Pos> itemPosBucket;
  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      if(!blocked[x][y] && Pos(x, y) != origin) {
        itemPosBucket.push_back(Pos(x, y));
      }
    }
  }

  const int ELEMENT = Rnd::range(0, itemPosBucket.size() - 1);
  ItemFactory::mkItemOnMap(ItemId::trapezohedron, itemPosBucket.at(ELEMENT));
  return true;
}

} //TrapezohedronLvl

} //MapGen

#include "MapGen.h"

#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "ItemFactory.h"
#include "Map.h"
#include "FeatureWall.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace MapGen {

namespace TrapezohedronLvl {

bool run() {
  Map::resetMap();

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Wall* const wall =
        dynamic_cast<Wall*>(FeatureFactory::mk(FeatureId::wall, Pos(x, y)));
      wall->wallType    = WallType::cave;
      wall->isMossGrown = false;
    }
  }

  const Pos& origin       = Map::player->pos;
  const Pos  mapCenter    = Pos(MAP_W_HALF, MAP_H_HALF);
  const FeatureId floorId = FeatureId::caveFloor;

  MapGenUtils::digByRandomWalk(origin, 150, floorId, true);
  MapGenUtils::digByRandomWalk(mapCenter, 800, floorId, true);
  MapGenUtils::digWithPathfinder(origin, mapCenter, floorId, false, true);

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  vector<Pos> itemPosBucket;
  itemPosBucket.resize(0);
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(blocked[x][y] == false && Pos(x, y) != origin) {
        itemPosBucket.push_back(Pos(x, y));
      }
    }
  }

  const int ELEMENT = Rnd::range(0, itemPosBucket.size() - 1);
  ItemFactory::mkItemOnMap(ItemId::trapezohedron,
                           itemPosBucket.at(ELEMENT));
  return true;
}

} //TrapezohedronLvl

} //MapGen

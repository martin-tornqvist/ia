#include "MapGen.h"

#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "ItemFactory.h"
#include "Map.h"
#include "FeatureWall.h"
#include "MapParsing.h"
#include "Utils.h"

bool MapGenTrapezohedronLvl::run_() {
  Map::resetMap();

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      eng.featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
      dynamic_cast<Wall*>(
        Map::cells[x][y].featureStatic)->wallType = wall_cave;
    }
  }

  makePathByRandomWalk(
    Map::player->pos.x, Map::player->pos.y, 150, feature_caveFloor, true);
  makePathByRandomWalk(
    MAP_W_HALF, MAP_H_HALF, 800, feature_caveFloor, true);
  makeStraightPathByPathfinder(
    Map::player->pos, Pos(MAP_W_HALF, MAP_H_HALF),
    feature_caveFloor, false, true);

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false, eng), blockers);
  vector<Pos> spawnCandidates;
  spawnCandidates.resize(0);
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(blockers[x][y] == false && Pos(x, y) != Map::player->pos) {
        spawnCandidates.push_back(Pos(x, y));
      }
    }
  }
  const int ELEMENT = Rnd::range(0, spawnCandidates.size() - 1);
  eng.itemFactory->spawnItemOnMap(
    ItemId::trapezohedron, spawnCandidates.at(ELEMENT));

  return true;
}

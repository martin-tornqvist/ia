#include "MapGen.h"

#include "Engine.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "ItemFactory.h"
#include "Map.h"
#include "FeatureWall.h"
#include "MapParsing.h"

bool MapGenTrapezohedronLvl::specificRun() {
  eng.map->resetMap();

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      eng.featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
      dynamic_cast<Wall*>(
        eng.map->cells[x][y].featureStatic)->wallType = wall_cave;
    }
  }

  makePathByRandomWalk(
    eng.player->pos.x, eng.player->pos.y, 150, feature_caveFloor, true);
  makePathByRandomWalk(
    MAP_W_HALF, MAP_H_HALF, 800, feature_caveFloor, true);
  makeStraightPathByPathfinder(
    eng.player->pos, Pos(MAP_W_HALF, MAP_H_HALF),
    feature_caveFloor, false, true);

  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, false, eng),
                   blockers);
  vector<Pos> spawnCandidates;
  spawnCandidates.resize(0);
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(blockers[x][y] == false && Pos(x, y) != eng.player->pos) {
        spawnCandidates.push_back(Pos(x, y));
      }
    }
  }
  const int ELEMENT = eng.dice.range(0, spawnCandidates.size() - 1);
  eng.itemFactory->spawnItemOnMap(
    item_trapezohedron, spawnCandidates.at(ELEMENT));

  return true;
}

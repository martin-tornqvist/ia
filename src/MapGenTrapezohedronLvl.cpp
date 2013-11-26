#include "MapGen.h"

#include "Engine.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "ItemFactory.h"
#include "Map.h"
#include "FeatureWall.h"

bool MapGenTrapezohedronLvl::specificRun() {
  eng->map->clearMap();

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
      dynamic_cast<Wall*>(eng->map->featuresStatic[x][y])->wallType = wall_cave;
    }
  }

  makePathByRandomWalk(
    eng->player->pos.x, eng->player->pos.y, 150, feature_caveFloor, true);
  makePathByRandomWalk(
    MAP_X_CELLS_HALF, MAP_Y_CELLS_HALF, 800, feature_caveFloor, true);
  makeStraightPathByPathfinder(
    eng->player->pos, Pos(MAP_X_CELLS_HALF, MAP_Y_CELLS_HALF),
    feature_caveFloor, false, true);

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(
    actorBodyType_normal, blockers);
  vector<Pos> spawnCandidates;
  spawnCandidates.resize(0);
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      if(blockers[x][y] == false && Pos(x, y) != eng->player->pos) {
        spawnCandidates.push_back(Pos(x, y));
      }
    }
  }
  const int ELEMENT = eng->dice.range(0, spawnCandidates.size() - 1);
  eng->itemFactory->spawnItemOnMap(
    item_trapezohedron, spawnCandidates.at(ELEMENT));

  return true;
}

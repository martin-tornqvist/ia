#include "MapBuild.h"

#include <iostream>
#include <vector>

#include "Engine.h"
#include "Converters.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "FeatureWall.h"

void MapBuild::buildCavern() {
  eng->map->clearDungeon();

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(x, y));
      Wall* const wall = dynamic_cast<Wall*>(eng->map->featuresStatic[x][y]);
      wall->wallType = wall_common;
      wall->isSlimy = false;
    }
  }

  const coord& playerCoord = eng->player->pos;
  vector<coord> previousCenters(1, playerCoord);

  //Make a random walk path from stairs
  int length = 40 + eng->dice(1, 40);
  makePathByRandomWalk(playerCoord.x, playerCoord.y, length, feature_caveFloor, true);
  const bool IS_TUNNEL_CAVE = eng->dice.coinToss();

  //Make some more at random places, connect them to each other.
  const int NR_OPEN_PLACES = IS_TUNNEL_CAVE ? eng->dice.getInRange(6, 8) : 4;
  for(int i = 0; i < NR_OPEN_PLACES; i++) {
    const coord curCenter(10 + eng->dice(1, MAP_X_CELLS - 1 - 10) - 1, 2 + eng->dice(1, MAP_Y_CELLS - 1 - 2) - 1);
    length = IS_TUNNEL_CAVE ? 30 + eng->dice(1, 30) : 650;
    makePathByRandomWalk(curCenter.x, curCenter.y, length, feature_caveFloor, true);
    const coord prevCenter = previousCenters.at(eng->dice.getInRange(0, previousCenters.size() - 1));
    makeStraightPathByPathfinder(prevCenter, curCenter, feature_caveFloor, false, true);
    previousCenters.push_back(curCenter);
  }

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
  const coord stairCell(freeCells.at(eng->dice(1, freeCells.size() - 1)));
  eng->featureFactory->spawnFeatureAt(feature_caveFloor, playerCoord);
  eng->featureFactory->spawnFeatureAt(feature_stairsDown, stairCell);
}

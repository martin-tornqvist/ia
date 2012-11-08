#include "MapBuild.h"

#include <iostream>
#include <vector>

#include "Engine.h"
#include "Converters.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "Map.h"

void MapBuild::buildCavern() {
  eng->map->clearDungeon();

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->featureFactory->spawnFeatureAt(feature_caveWall, coord(x, y));
    }
  }

  const coord& playerCoord = eng->player->pos;
  coord preCenter(playerCoord);
  coord curCenter;

  //Make a random walk path from stairs
  int length = 40 + eng->dice(1, 40);
  makePathByRandomWalk(playerCoord.x, playerCoord.y, length, feature_caveFloor, true);
  const bool IS_TUNNEL_CAVE = eng->dice.coinToss();

  //Make some more at random places, connect them to each other.
  for(int i = 0; i < 4; i++) {
    curCenter.x = 20 + eng->dice(1, MAP_X_CELLS - 1 - 20) - 1;
    curCenter.y = 4 + eng->dice(1, MAP_Y_CELLS - 1 - 4) - 1;
    length = IS_TUNNEL_CAVE ? 20 + eng->dice(1, 20) : 750;
    makePathByRandomWalk(curCenter.x, curCenter.y, length, feature_caveFloor, true);
    makeStraightPathByPathfinder(preCenter, curCenter, feature_caveFloor, false, true);
    preCenter.set(curCenter);
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

#include "MapBuild.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureWall.h"

void MapBuild::buildKingsTomb() {
  eng->map->clearDungeon();

  eng->player->pos = coord(1, MAP_Y_CELLS_HALF - 1);

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      eng->featureFactory->spawnFeatureAt(feature_caveFloor, coord(x, y));
      if(x == 0 || y == 0 || x == MAP_X_CELLS - 1 || y == MAP_Y_CELLS - 1) {
        eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(x, y));
        dynamic_cast<Wall*>(eng->map->featuresStatic[x][y])->wallType = wall_cave;
      } else {
        const int CENTER_WIDTH = 4;
        if(x > MAP_X_CELLS_HALF + 2 && (y >= MAP_Y_CELLS_HALF - CENTER_WIDTH && y <= MAP_Y_CELLS_HALF + CENTER_WIDTH - 1)) {
          eng->featureFactory->spawnFeatureAt(feature_caveFloor, coord(x, y));
        } else {
          if((x / 2) * 2 == x && (y / 3) * 3 == y) {
            if(eng->dice(1, 100) < max(30, x * 3) || (x <= 4 && (y >= MAP_Y_CELLS_HALF - 3 && y <= MAP_Y_CELLS_HALF + 3))) {
              eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(x, y));
              dynamic_cast<Wall*>(eng->map->featuresStatic[x][y])->wallType = wall_cave;
            } else {
              eng->featureFactory->spawnFeatureAt(feature_caveFloor, coord(x, y));
            }
          }
        }
      }
    }
  }

  Feature* f = NULL;
  eng->featureFactory->spawnFeatureAt(feature_stairsDown, coord(MAP_X_CELLS - 2, MAP_Y_CELLS_HALF - 1));
  f = eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(MAP_X_CELLS - 2, MAP_Y_CELLS_HALF - 2));
  dynamic_cast<Wall*>(f)->wallType = wall_cave;
  f = eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(MAP_X_CELLS - 3, MAP_Y_CELLS_HALF - 2));
  dynamic_cast<Wall*>(f)->wallType = wall_cave;
  f = eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(MAP_X_CELLS - 3, MAP_Y_CELLS_HALF - 1));
  dynamic_cast<Wall*>(f)->wallType = wall_cave;
  f = eng->featureFactory->spawnFeatureAt(feature_stoneWall, coord(MAP_X_CELLS - 3, MAP_Y_CELLS_HALF));
  dynamic_cast<Wall*>(f)->wallType = wall_cave;

  const FeatureDef* const mimicFeature = eng->featureData->getFeatureDef(feature_caveFloor);
  eng->featureFactory->spawnFeatureAt(feature_trap, coord(MAP_X_CELLS - 2, MAP_Y_CELLS_HALF), new TrapSpawnData(mimicFeature, trap_teleport));

  coord c(MAP_X_CELLS - 5, MAP_Y_CELLS_HALF - 1);
  Monster* const mummyBoss = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_khephren, c));
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      Monster* const monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_fireVortex, c + coord(dx, dy)));
      monster->leader = mummyBoss;
    }
  }


  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);

  for(int i = eng->dice(1, 2) + 3; i > 0 && freeCells.size() != 0; i--) {
    const unsigned int ELEMENT = eng->dice(1, freeCells.size()) - 1;
    const coord pos = freeCells.at(ELEMENT);
    freeCells.erase(freeCells.begin() + ELEMENT);
    eng->actorFactory->spawnActor(actor_mummy, pos);
  }
}

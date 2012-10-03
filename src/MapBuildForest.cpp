#include "MapBuild.h"

#include <iostream>
#include <vector>

#include "Engine.h"
#include "Converters.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "Pathfinding.h"
#include "ActorFactory.h"
#include "ActorMonster.h"

void MapBuild::buildForestLimit() {
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    eng->featureFactory->spawnFeatureAt(feature_tree, coord(0, y));
  }

  for(int x = 0; x < MAP_X_CELLS; x++) {
    eng->featureFactory->spawnFeatureAt(feature_tree, coord(x, 0));
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    eng->featureFactory->spawnFeatureAt(feature_tree, coord(MAP_X_CELLS - 1, y));
  }

  for(int x = 0; x < MAP_X_CELLS; x++) {
    eng->featureFactory->spawnFeatureAt(feature_tree, coord(x, MAP_Y_CELLS - 1));
  }
}

void MapBuild::buildForestOuterTreeline() {
  const int MAX_LEN = 2;

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x <= MAX_LEN; x++) {
      if(eng->dice(1, 4) > 1 || x == 0) {
        eng->featureFactory->spawnFeatureAt(feature_tree, coord(x, y));
      } else {
        x = 9999;
      }
    }
  }

  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAX_LEN; y++) {
      if(eng->dice(1, 4) > 1 || y == 0) {
        eng->featureFactory->spawnFeatureAt(feature_tree, coord(x, y));
      } else {
        y = 9999;
      }
    }
  }

  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = MAP_X_CELLS - 1; x >= MAP_X_CELLS - MAX_LEN; x--) {
      if(eng->dice(1, 4) > 1 || x == MAP_X_CELLS - 1) {
        eng->featureFactory->spawnFeatureAt(feature_tree, coord(x, y));
      } else {
        x = -1;
      }
    }
  }

  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = MAP_Y_CELLS - 1; y >= MAP_Y_CELLS - MAX_LEN; y--) {
      if(eng->dice(1, 4) > 1 || y == MAP_Y_CELLS - 1) {
        eng->featureFactory->spawnFeatureAt(feature_tree, coord(x, y));
      } else {
        y = -1;
      }
    }
  }
}

void MapBuild::buildForestTreePatch() {
  int terrain_size_min = 5;
  int terrain_size_max = 17;

  int terrain_size = terrain_size_min + eng->dice(1, terrain_size_max - terrain_size_min);

  int terrain_size_created = 0;

  //Set a start position where trees start to spawn
  int terrain_start_x = eng->dice(1, MAP_X_CELLS - 1);
  int terrain_start_y = eng->dice(1, MAP_Y_CELLS - 1);

  int step_x = 0;
  int step_y = 0;

  int x_cur = terrain_start_x + step_x;
  int y_cur = terrain_start_y + step_y;

  while(terrain_size_created < terrain_size) {
    const int playerX = eng->player->pos.x;
    const int playerY = eng->player->pos.y;

    if(eng->mapTests->isCellInsideMainScreen(x_cur, y_cur) == true && eng->basicUtils->chebyshevDistance(x_cur, y_cur, playerX, playerY) > 2) {
      eng->featureFactory->spawnFeatureAt(feature_tree, coord(x_cur, y_cur));
      terrain_size_created++;

      while(eng->map->featuresStatic[x_cur][y_cur]->getId() == feature_tree || eng->basicUtils->chebyshevDistance(x_cur, y_cur, playerX,
            playerY) <= 2) {

        if(eng->dice(1, 2) == 1) {
          while(step_x == 0) {
            step_x = eng->dice(1, 3) - 2;
          }
          step_y = 0;
        } else {
          while(step_y == 0) {
            step_y = eng->dice(1, 3) - 2;
          }
          step_x = 0;
        }

        x_cur += step_x;
        y_cur += step_y;

        if(eng->mapTests->isCellInsideMainScreen(x_cur, y_cur) == false) {
          terrain_size_created = 99999;
          break;
        }
      }
    } else terrain_size_created = 9999;
  }
}

void MapBuild::buildForestTrees(const coord& stairsCoord) {
  unsigned minPathLength = 1;
  unsigned maxPathLength = 999;

  int forestPatches = 40 + eng->dice(1, 15);

  vector<coord> path;

  backupMap();

  bool proceed = false;
  while(proceed == false) {
    for(int i = 0; i < forestPatches; i++) {
      buildForestTreePatch();
    }

    buildFromTemplate(stairsCoord - coord(26, 7), templateDevName_church, false);

    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, blockers);

    path = eng->pathfinder->findPath(eng->player->pos, blockers, stairsCoord);

    eng->featureFactory->spawnFeatureAt(feature_stairsDown, stairsCoord);

    if(path.size() >= minPathLength && path.size() <= maxPathLength) {
      proceed = true;
    } else {
      restoreMap();
    }

    maxPathLength++;
  }

  const int RND = eng->dice.getInRange(1, 4);
  const coord cultistCoordRelStairs =
    RND == 1 ? coord(-5, -2) : RND == 2 ? coord(-5, 4) : RND == 3 ? coord(-12, -2) : coord(-12, 4);
  const coord cultistCoord(stairsCoord + cultistCoordRelStairs);

  Monster* const monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_cultistPistol, cultistCoord));
  monster->isRoamingAllowed = false;

  //Build path
  for(unsigned int i = 0; i < path.size(); i++) {
    for(int dx = -1; dx < 1; dx++) {
      for(int dy = -1; dy < 1; dy++) {
        const coord c(path.at(i) + coord(dx, dy));
        if(eng->map->featuresStatic[c.x][c.y]->canHaveStaticFeature() && eng->mapTests->isCellInsideMainScreen(c)) {
          eng->featureFactory->spawnFeatureAt(feature_forestPath, c);
        }
      }
    }
  }

  //Place a graveyard somewhere along the path
  const int graveyardPathPos = path.size() * 3 / 4;// - dice(1,4);
  //Build graveyard under/over path depending on which vertical
  //side of the screen the path position is on.
  if(path.at(graveyardPathPos).y < MAP_Y_CELLS_HALF) {
    buildFromTemplate(path.at(graveyardPathPos) + coord(2, 1), templateDevName_graveYard, false);
  } else {
    buildFromTemplate(path.at(graveyardPathPos) + coord(2, -6), templateDevName_graveYard, false);
  }
}

void MapBuild::buildForest() {
  int grass = 0;
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      const coord c(x, y);
      grass = eng->dice(1, 12);
      if(grass == 1) {
        eng->featureFactory->spawnFeatureAt(feature_bush, c);
      }
      if(grass == 2) {
        eng->featureFactory->spawnFeatureAt(feature_bushWithered, c);
      }
      if(grass == 3 || grass == 4) {
        eng->featureFactory->spawnFeatureAt(feature_grassWithered, c);
      }
      if(grass >= 5) {
        eng->featureFactory->spawnFeatureAt(feature_grass, c);
      }
    }
  }

  coord stairCell(MAP_X_CELLS - 6, 9);

  buildForestOuterTreeline();

  buildForestTrees(stairCell);

  buildForestLimit();
}


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
#include "Map.h"
#include "Highscore.h"
#include "Fov.h"
#include "FeatureGrave.h"
#include "TextFormatting.h"
#include "PopulateMonsters.h"

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

  int nrTerrainCreated = 0;

  //Set a start position where trees start to spawn
  int terrainStartX = eng->dice(1, MAP_X_CELLS - 1);
  int terrainStartY = eng->dice(1, MAP_Y_CELLS - 1);

  int stepX = 0;
  int stepY = 0;

  coord curPos(terrainStartX + stepX, terrainStartY + stepY);

  while(nrTerrainCreated < terrain_size) {
    if(
      eng->mapTests->isCellInsideMap(curPos) &&
      eng->basicUtils->chebyshevDistance(curPos, eng->player->pos) > 2) {
      eng->featureFactory->spawnFeatureAt(feature_tree, curPos);
      nrTerrainCreated++;

      while(
        eng->map->featuresStatic[curPos.x][curPos.y]->getId() == feature_tree ||
        eng->basicUtils->chebyshevDistance(curPos, eng->player->pos) <= 2) {

        if(eng->dice(1, 2) == 1) {
          while(stepX == 0) {
            stepX = eng->dice(1, 3) - 2;
          }
          stepY = 0;
        } else {
          while(stepY == 0) {
            stepY = eng->dice(1, 3) - 2;
          }
          stepX = 0;
        }

        curPos += coord(stepX, stepY);

        if(eng->mapTests->isCellInsideMap(curPos) == false) {
          nrTerrainCreated = 99999;
          break;
        }
      }
    } else nrTerrainCreated = 9999;
  }
}

void MapBuild::buildForestTrees(const coord& stairsCoord) {
  unsigned minPathLength = 1;
  unsigned maxPathLength = 999;

  int forestPatches = 40 + eng->dice(1, 15);

  vector<coord> path;

  backupMap();

  const coord churchPos = stairsCoord - coord(26, 7);

  bool proceed = false;
  while(proceed == false) {
    for(int i = 0; i < forestPatches; i++) {
      buildForestTreePatch();
    }

    buildFromTemplate(churchPos, mapTemplate_church);

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

//  const coord cultistCoordRelStairs = coord(-1, 1);
//  const coord cultistCoord(stairsCoord + cultistCoordRelStairs);

//  Monster* const monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actor_cultist, cultistCoord));
//  monster->isRoamingAllowed = false;

  //Build path
  for(unsigned int i = 0; i < path.size(); i++) {
    for(int dx = -1; dx < 1; dx++) {
      for(int dy = -1; dy < 1; dy++) {
        const coord c(path.at(i) + coord(dx, dy));
        if(eng->map->featuresStatic[c.x][c.y]->canHaveStaticFeature() && eng->mapTests->isCellInsideMap(c)) {
          eng->featureFactory->spawnFeatureAt(feature_forestPath, c);
        }
      }
    }
  }

  //Place graves
  vector<HighScoreEntry> highscoreEntries = eng->highScore->getEntriesSorted();
  const unsigned PLACE_TOP_N_HIGHSCORES = 7;
  const int NR_HIGHSCORES = min(PLACE_TOP_N_HIGHSCORES, highscoreEntries.size());
  if(NR_HIGHSCORES > 0) {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeMoveBlockerArrayForMoveTypeFeaturesOnly(moveType_walk, blockers);

    bool vision[MAP_X_CELLS][MAP_Y_CELLS];

    const int SEARCH_RADI = FOV_STANDARD_RADI_INT - 2;
    const int TRY_PLACE_EVERY_N_STEP = 2;

    vector<coord> gravePositions;

    int pathWalkCount = 0;
    for(unsigned int i = 0; i < path.size(); i++) {
      if(pathWalkCount == TRY_PLACE_EVERY_N_STEP) {

        eng->fov->runFovOnArray(blockers, path.at(i), vision, false);

        for(int dy = -SEARCH_RADI; dy <= SEARCH_RADI; dy++) {
          for(int dx = -SEARCH_RADI; dx <= SEARCH_RADI; dx++) {

            const int X = path.at(i).x + dx;
            const int Y = path.at(i).y + dy;

            const bool IS_LEFT_OF_CHURCH = X < churchPos.x - (SEARCH_RADI) + 2;
            const bool IS_ON_STONE_PATH = eng->map->featuresStatic[X][Y]->getId() == feature_forestPath;

            bool isLeftOfPrev = true;
            if(gravePositions.empty() == false) {
              isLeftOfPrev = X < gravePositions.back().x;
            }

            bool isPosOk = vision[X][Y] && IS_LEFT_OF_CHURCH && IS_ON_STONE_PATH == false && isLeftOfPrev;

            if(isPosOk) {
              for(int dy_small = -1; dy_small <= 1; dy_small++) {
                for(int dx_small = -1; dx_small <= 1; dx_small++) {
                  if(blockers[X + dx_small][Y + dy_small]) {
                    isPosOk = false;
                  }
                }
              }
              if(isPosOk) {
                gravePositions.push_back(coord(X, Y));
                blockers[X][Y] = true;
                if(gravePositions.size() == static_cast<unsigned int>(NR_HIGHSCORES)) {
                  i = 9999;
                }
                dy = 99999;
                dx = 99999;
              }
            }
          }
        }
        pathWalkCount = 0;
      }
      pathWalkCount++;
    }
    for(unsigned int i = 0; i < gravePositions.size(); i++) {
      Feature* f = eng->featureFactory->spawnFeatureAt(feature_gravestone, gravePositions.at(i));
      Grave* const grave = dynamic_cast<Grave*>(f);
      HighScoreEntry curHighscore = highscoreEntries.at(i);
      const string name = curHighscore.getName();
      const string date = eng->textFormatting->getSpaceSeparatedList(curHighscore.getDateAndTime()).at(0);
      const string score = intToString(curHighscore.getScore());
      grave->setInscription("RIP " + name + " " + date + " Score: " + score);
    }
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

  eng->populateMonsters->populateIntroLevel();
}


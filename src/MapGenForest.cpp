#include "MapGen.h"

#include <iostream>
#include <vector>

#include "Converters.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Highscore.h"
#include "Fov.h"
#include "TextFormatting.h"
#include "PopulateMonsters.h"
#include "MapParsing.h"
#include "Utils.h"

void MapGenIntroForest::buildForestLimit() {
  for(int y = 0; y < MAP_H; y++) {
    FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(0, y));
  }

  for(int x = 0; x < MAP_W; x++) {
    FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(x, 0));
  }

  for(int y = 0; y < MAP_H; y++) {
    FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(MAP_W - 1, y));
  }

  for(int x = 0; x < MAP_W; x++) {
    FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(x, MAP_H - 1));
  }
}

void MapGenIntroForest::buildForestOuterTreeline() {
  const int MAX_LEN = 2;

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x <= MAX_LEN; x++) {
      if(Rnd::range(1, 4) > 1 || x == 0) {
        FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(x, y));
      } else {
        x = 9999;
      }
    }
  }

  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAX_LEN; y++) {
      if(Rnd::range(1, 4) > 1 || y == 0) {
        FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(x, y));
      } else {
        y = 9999;
      }
    }
  }

  for(int y = 0; y < MAP_H; y++) {
    for(int x = MAP_W - 1; x >= MAP_W - MAX_LEN; x--) {
      if(Rnd::range(1, 4) > 1 || x == MAP_W - 1) {
        FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(x, y));
      } else {
        x = -1;
      }
    }
  }

  for(int x = 0; x < MAP_W; x++) {
    for(int y = MAP_H - 1; y >= MAP_H - MAX_LEN; y--) {
      if(Rnd::range(1, 4) > 1 || y == MAP_H - 1) {
        FeatureFactory::spawnFeatureAt(FeatureId::tree, Pos(x, y));
      } else {
        y = -1;
      }
    }
  }
}

void MapGenIntroForest::buildForestTreePatch() {
  int terrain_size_min = 5;
  int terrain_size_max = 17;

  int terrain_size = terrain_size_min + Rnd::dice(1, terrain_size_max - terrain_size_min);

  int nrTerrainCreated = 0;

  //Set a start position where trees start to spawn
  int terrainStartX = Rnd::dice(1, MAP_W - 1);
  int terrainStartY = Rnd::dice(1, MAP_H - 1);

  int stepX = 0;
  int stepY = 0;

  Pos curPos(terrainStartX + stepX, terrainStartY + stepY);

  while(nrTerrainCreated < terrain_size) {
    if(
      Utils::isPosInsideMap(curPos) &&
      Utils::kingDist(curPos, Map::player->pos) > 2) {
      FeatureFactory::spawnFeatureAt(FeatureId::tree, curPos);
      nrTerrainCreated++;

      while(
        Map::cells[curPos.x][curPos.y].featureStatic->getId() == FeatureId::tree ||
        Utils::kingDist(curPos, Map::player->pos) <= 2) {

        if(Rnd::dice(1, 2) == 1) {
          while(stepX == 0) {
            stepX = Rnd::dice(1, 3) - 2;
          }
          stepY = 0;
        } else {
          while(stepY == 0) {
            stepY = Rnd::dice(1, 3) - 2;
          }
          stepX = 0;
        }

        curPos += Pos(stepX, stepY);

        if(Utils::isPosInsideMap(curPos) == false) {
          nrTerrainCreated = 99999;
          break;
        }
      }
    } else nrTerrainCreated = 9999;
  }
}

void MapGenIntroForest::buildForestTrees(const Pos& stairsPos) {
  unsigned minPathLength = 1;
  unsigned maxPathLength = 999;

  int forestPatches = 40 + Rnd::dice(1, 15);

  vector<Pos> path;

  backupMap();

  const Pos churchPos = stairsPos - Pos(26, 7);

  bool proceed = false;
  while(proceed == false) {
    for(int i = 0; i < forestPatches; i++) {
      buildForestTreePatch();
    }

    buildFromTemplate(churchPos, mapTemplate_church);

    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);

    PathFind::run(Map::player->pos, stairsPos, blockers, path);

    FeatureFactory::spawnFeatureAt(FeatureId::stairs, stairsPos);

    if(path.size() >= minPathLength && path.size() <= maxPathLength) {
      proceed = true;
    } else {
      restoreMap();
    }

    maxPathLength++;
  }

  //Build path
  for(unsigned int i = 0; i < path.size(); i++) {
    for(int dx = -1; dx < 1; dx++) {
      for(int dy = -1; dy < 1; dy++) {
        const Pos c(path.at(i) + Pos(dx, dy));
        if(
          Map::cells[c.x][c.y].featureStatic->canHaveStaticFeature() &&
          Utils::isPosInsideMap(c)) {
          FeatureFactory::spawnFeatureAt(FeatureId::forestPath, c);
        }
      }
    }
  }


  //Place graves
  vector<HighScoreEntry> highscoreEntries = HighScore::getEntriesSorted();
  const int PLACE_TOP_N_HIGHSCORES = 7;
  const int NR_HIGHSCORES =
    min(PLACE_TOP_N_HIGHSCORES, int(highscoreEntries.size()));
  if(NR_HIGHSCORES > 0) {
    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksMoveCmn(true), blockers);

    bool vision[MAP_W][MAP_H];

    const int SEARCH_RADI = FOV_STD_RADI_INT - 2;
    const int TRY_PLACE_EVERY_N_STEP = 2;

    vector<Pos> gravePositions;

    int pathWalkCount = 0;
    for(unsigned int i = 0; i < path.size(); i++) {
      if(pathWalkCount == TRY_PLACE_EVERY_N_STEP) {

        Fov::runFovOnArray(blockers, path.at(i), vision, false);

        for(int dy = -SEARCH_RADI; dy <= SEARCH_RADI; dy++) {
          for(int dx = -SEARCH_RADI; dx <= SEARCH_RADI; dx++) {

            const int X = path.at(i).x + dx;
            const int Y = path.at(i).y + dy;

            const bool IS_LEFT_OF_CHURCH = X < churchPos.x - (SEARCH_RADI) + 2;
            const bool IS_ON_STONE_PATH =
              Map::cells[X][Y].featureStatic->getId() == FeatureId::forestPath;

            bool isLeftOfPrev = true;
            if(gravePositions.empty() == false) {
              isLeftOfPrev = X < gravePositions.back().x;
            }

            bool isPosOk = vision[X][Y] && IS_LEFT_OF_CHURCH &&
                           IS_ON_STONE_PATH == false && isLeftOfPrev;

            if(isPosOk) {
              for(int dy_small = -1; dy_small <= 1; dy_small++) {
                for(int dx_small = -1; dx_small <= 1; dx_small++) {
                  if(blockers[X + dx_small][Y + dy_small]) {
                    isPosOk = false;
                  }
                }
              }
              if(isPosOk) {
                gravePositions.push_back(Pos(X, Y));
                blockers[X][Y] = true;
                if(gravePositions.size() == (unsigned int)NR_HIGHSCORES) {
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
      Feature* f = FeatureFactory::spawnFeatureAt(
                     FeatureId::gravestone, gravePositions.at(i));
      Grave* const grave = dynamic_cast<Grave*>(f);
      HighScoreEntry curHighscore = highscoreEntries.at(i);
      const string name = curHighscore.getName();
      vector<string> dateStrVector;
      dateStrVector.resize(0);
      TextFormatting::getSpaceSeparatedList(curHighscore.getDateAndTime(),
                                            dateStrVector);
      const string date = dateStrVector.at(0);
      const string score = toStr(curHighscore.getScore());
      grave->setInscription("RIP " + name + " " + date + " Score: " + score);
    }
  }
}

bool MapGenIntroForest::run_() {
  int grass = 0;
  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      const Pos c(x, y);
      grass = Rnd::dice(1, 12);
      if(grass == 1) {
        FeatureFactory::spawnFeatureAt(FeatureId::bush, c);
      }
      if(grass == 2) {
        FeatureFactory::spawnFeatureAt(FeatureId::bushWithered, c);
      }
      if(grass == 3 || grass == 4) {
        FeatureFactory::spawnFeatureAt(FeatureId::grassWithered, c);
      }
      if(grass >= 5) {
        FeatureFactory::spawnFeatureAt(FeatureId::grass, c);
      }
    }
  }

  Pos stairCell(MAP_W - 6, 9);
  buildForestOuterTreeline();
  buildForestTrees(stairCell);
  buildForestLimit();

  PopulateMonsters::populateIntroLevel();

  return true;
}


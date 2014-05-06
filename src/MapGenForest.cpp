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

using namespace std;

namespace MapGen {

namespace IntroForest {

namespace {

void buildForestLimit() {
  for(int y = 0; y < MAP_H; y++) {
    FeatureFactory::spawn(FeatureId::tree, Pos(0, y));
  }

  for(int x = 0; x < MAP_W; x++) {
    FeatureFactory::spawn(FeatureId::tree, Pos(x, 0));
  }

  for(int y = 0; y < MAP_H; y++) {
    FeatureFactory::spawn(FeatureId::tree, Pos(MAP_W - 1, y));
  }

  for(int x = 0; x < MAP_W; x++) {
    FeatureFactory::spawn(FeatureId::tree, Pos(x, MAP_H - 1));
  }
}

void buildForestOuterTreeline() {
  const int MAX_LEN = 2;

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x <= MAX_LEN; x++) {
      if(Rnd::range(1, 4) > 1 || x == 0) {
        FeatureFactory::spawn(FeatureId::tree, Pos(x, y));
      } else {
        break;
      }
    }
  }

  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAX_LEN; y++) {
      if(Rnd::range(1, 4) > 1 || y == 0) {
        FeatureFactory::spawn(FeatureId::tree, Pos(x, y));
      } else {
        break;
      }
    }
  }

  for(int y = 0; y < MAP_H; y++) {
    for(int x = MAP_W - 1; x >= MAP_W - MAX_LEN; x--) {
      if(Rnd::range(1, 4) > 1 || x == MAP_W - 1) {
        FeatureFactory::spawn(FeatureId::tree, Pos(x, y));
      } else {
        break;
      }
    }
  }

  for(int x = 0; x < MAP_W; x++) {
    for(int y = MAP_H - 1; y >= MAP_H - MAX_LEN; y--) {
      if(Rnd::range(1, 4) > 1 || y == MAP_H - 1) {
        FeatureFactory::spawn(FeatureId::tree, Pos(x, y));
      } else {
        break;
      }
    }
  }
}

void buildForestTreePatch() {
  const int NR_TREES_TO_PUT = Rnd::range(5, 17);

  Pos curPos(Rnd::range(1, MAP_W - 2), Rnd::range(1, MAP_H - 2));

  int nrTreesCreated = 0;

  while(nrTreesCreated < NR_TREES_TO_PUT) {
    if(
      Utils::isPosInsideMap(curPos) == false ||
      Utils::kingDist(curPos, Map::player->pos) <= 1) {
      return;
    }

    const FeatureId treeId = FeatureId::tree;

    FeatureFactory::spawn(treeId, curPos);
    nrTreesCreated++;

    //Find next pos
    while(
      Map::cells[curPos.x][curPos.y].featureStatic->getId() == treeId ||
      Utils::kingDist(curPos, Map::player->pos) <= 2) {

      if(Rnd::coinToss()) {
        curPos.x += Rnd::coinToss() ? -1 : 1;
      } else {
        curPos.y += Rnd::coinToss() ? -1 : 1;
      }

      if(Utils::isPosInsideMap(curPos) == false) {return;}
    }
  }
}

void buildForestTrees(const Pos& stairsPos) {
  MapGenUtils::backupMap();

  const Pos churchPos = stairsPos - Pos(26, 7);

  int nrForestPatches = Rnd::range(40, 55);

  vector<Pos> path;

  bool proceed = false;
  while(proceed == false) {
    for(int i = 0; i < nrForestPatches; i++) {
      buildForestTreePatch();
    }

    MapGenUtils::buildFromTempl(churchPos, MapTemplId::church);

    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksMoveCmn(false), blockers);

    PathFind::run(Map::player->pos, stairsPos, blockers, path);

    FeatureFactory::spawn(FeatureId::stairs, stairsPos);

    size_t minPathLength = 1;
    size_t maxPathLength = 999;

    if(path.size() >= minPathLength && path.size() <= maxPathLength) {
      proceed = true;
    } else {
      MapGenUtils::restoreMap();
    }

    maxPathLength++;
  }

  //Build path
  for(const Pos& pathPos : path) {
    for(int dx = -1; dx < 1; dx++) {
      for(int dy = -1; dy < 1; dy++) {
        const Pos c(pathPos + Pos(dx, dy));
        if(
          Map::cells[c.x][c.y].featureStatic->canHaveStaticFeature() &&
          Utils::isPosInsideMap(c)) {
          FeatureFactory::spawn(FeatureId::forestPath, c);
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
      Feature* f = FeatureFactory::spawn(
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

} //namespace

bool run() {
  int grass = 0;
  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      const Pos c(x, y);
      grass = Rnd::range(1, 12);
      FeatureId id = FeatureId::empty;
      if(grass == 1)                {id = FeatureId::bush;}
      if(grass == 2)                {id = FeatureId::bushWithered;}
      if(grass == 3 || grass == 4)  {id = FeatureId::grassWithered;}
      if(grass >= 5)                {id = FeatureId::grass;}
      FeatureFactory::spawn(id, c);
    }
  }

  Pos stairCell(MAP_W - 6, 9);
  buildForestOuterTreeline();
  buildForestTrees(stairCell);
  buildForestLimit();

  PopulateMonsters::populateIntroLevel();

  return true;
}

} //IntroForest

} //MapGen

#include "MapGen.h"

#include <vector>

#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Utils.h"
#include "ActorFactory.h"
#include "FeatureRigid.h"
#include "MapParsing.h"
#include "Highscore.h"
#include "Fov.h"
#include "TextFormatting.h"
#include "PopulateMonsters.h"
#include "PopulateItems.h"
#include "ItemFactory.h"
#include "FeatureDoor.h"

using namespace std;

namespace MapGen {

//------------------------------------------------------------------------- FOREST
namespace {

void mkForestLimit() {
  auto putTree = [](const int X, const int Y) {Map::put(new Tree(Pos(X, Y)));};

  for(int y = 0; y < MAP_H; ++y) {putTree(0,          y);}
  for(int x = 0; x < MAP_W; ++x) {putTree(x,          0);}
  for(int y = 0; y < MAP_H; ++y) {putTree(MAP_W - 1,  y);}
  for(int x = 0; x < MAP_W; ++x) {putTree(x,          MAP_H - 1);}
}

void mkForestOuterTreeline() {
  const int MAX_LEN = 2;

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x <= MAX_LEN; ++x) {
      if(Rnd::range(1, 4) > 1 || x == 0) {
        Map::put(new Tree(Pos(x, y)));
      } else {
        break;
      }
    }
  }

  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAX_LEN; ++y) {
      if(Rnd::range(1, 4) > 1 || y == 0) {
        Map::put(new Tree(Pos(x, y)));
      } else {
        break;
      }
    }
  }

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = MAP_W - 1; x >= MAP_W - MAX_LEN; x--) {
      if(Rnd::range(1, 4) > 1 || x == MAP_W - 1) {
        Map::put(new Tree(Pos(x, y)));
      } else {
        break;
      }
    }
  }

  for(int x = 0; x < MAP_W; ++x) {
    for(int y = MAP_H - 1; y >= MAP_H - MAX_LEN; y--) {
      if(Rnd::range(1, 4) > 1 || y == MAP_H - 1) {
        Map::put(new Tree(Pos(x, y)));
      } else {
        break;
      }
    }
  }
}

void mkForestTreePatch() {
  const int NR_TREES_TO_PUT = Rnd::range(5, 17);

  Pos curPos(Rnd::range(1, MAP_W - 2), Rnd::range(1, MAP_H - 2));

  int nrTreesCreated = 0;

  while(nrTreesCreated < NR_TREES_TO_PUT) {
    if(
      !Utils::isPosInsideMap(curPos) ||
      Utils::kingDist(curPos, Map::player->pos) <= 1) {
      return;
    }

    Map::put(new Tree(curPos));

    ++nrTreesCreated;

    //Find next pos
    while(
      Map::cells[curPos.x][curPos.y].rigid->getId() == FeatureId::tree ||
      Utils::kingDist(curPos, Map::player->pos) <= 2) {

      if(Rnd::coinToss()) {
        curPos.x += Rnd::coinToss() ? -1 : 1;
      } else {
        curPos.y += Rnd::coinToss() ? -1 : 1;
      }

      if(!Utils::isPosInsideMap(curPos)) {return;}
    }
  }
}

void mkForestTrees() {
  MapGenUtils::backupMap();

  const Pos churchPos(MAP_W - 33, 2);

  int nrForestPatches = Rnd::range(40, 55);

  vector<Pos> path;

  bool proceed = false;
  while(!proceed) {
    for(int i = 0; i < nrForestPatches; ++i) {mkForestTreePatch();}

    const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::church);
    const Pos       templDims = templ.getDims();

    for(int y = 0; y < templDims.y; ++y) {
      for(int x = 0; x < templDims.x; ++x) {
        const auto& templCell = templ.getCell(x, y);
        const auto  fId       = templCell.featureId;
        const Pos p(churchPos + Pos(x, y));
        if(fId != FeatureId::END) {
          Rigid* const f =
            Map::put(static_cast<Rigid*>(FeatureData::getData(fId).mkObj(p)));
          if(fId == FeatureId::grass) {
            static_cast<Grass*>(f)->type_ = GrassType::withered;
          }
        }
        if(templCell.val == 1) {
          Map::put(new Door(p, new Wall(p), DoorSpawnState::closed));
        }
      }
    }

    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);

    Pos stairsPos;
    for(int x = 0; x < MAP_W; ++x) {
      for(int y = 0; y < MAP_H; ++y) {
        const auto id = Map::cells[x][y].rigid->getId();
        if(id == FeatureId::stairs) {
          stairsPos.set(x, y);
          blocked[x][y] = false;
        } else if(id == FeatureId::door) {
          blocked[x][y] = false;
        }
      }
    }

    PathFind::run(Map::player->pos, stairsPos, blocked, path);

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
    for(int dx = -1; dx < 1; ++dx) {
      for(int dy = -1; dy < 1; ++dy) {
        const Pos p(pathPos + Pos(dx, dy));
        if(
          Map::cells[p.x][p.y].rigid->canHaveRigid() &&
          Utils::isPosInsideMap(p)) {
          Floor* const floor = new Floor(p);
          floor->type_ = FloorType::stonePath;
          Map::put(floor);
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
    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

    bool fov[MAP_W][MAP_H];

    const int SEARCH_RADI = FOV_STD_RADI_INT - 2;
    const int TRY_PLACE_EVERY_N_STEP = 2;

    vector<Pos> graveCells;

    int pathWalkCount = 0;
    for(size_t i = 0; i < path.size(); ++i) {
      if(pathWalkCount == TRY_PLACE_EVERY_N_STEP) {

        Fov::runFovOnArray(blocked, path.at(i), fov, false);

        for(int dy = -SEARCH_RADI; dy <= SEARCH_RADI; ++dy) {
          for(int dx = -SEARCH_RADI; dx <= SEARCH_RADI; ++dx) {

            const int X = path.at(i).x + dx;
            const int Y = path.at(i).y + dy;

            const bool IS_LEFT_OF_CHURCH = X < churchPos.x - (SEARCH_RADI) + 2;
            const bool IS_ON_STONE_PATH =
              Map::cells[X][Y].rigid->getId() == FeatureId::floor;

            bool isLeftOfPrev = true;
            if(!graveCells.empty()) {isLeftOfPrev = X < graveCells.back().x;}

            bool isPosOk = fov[X][Y]         &&
                           IS_LEFT_OF_CHURCH &&
                           !IS_ON_STONE_PATH &&
                           isLeftOfPrev;

            if(isPosOk) {
              for(int dy_small = -1; dy_small <= 1; dy_small++) {
                for(int dx_small = -1; dx_small <= 1; dx_small++) {
                  if(blocked[X + dx_small][Y + dy_small]) {
                    isPosOk = false;
                  }
                }
              }
              if(isPosOk) {
                graveCells.push_back(Pos(X, Y));
                blocked[X][Y] = true;
                if(int(graveCells.size()) == NR_HIGHSCORES) {i = 9999;}
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
    for(size_t i = 0; i < graveCells.size(); ++i) {
      GraveStone* grave = new GraveStone(graveCells[i]);
      HighScoreEntry curHighscore = highscoreEntries.at(i);
      const string name = curHighscore.getName();
      vector<string> dateStrVector;
      dateStrVector.clear();
      TextFormatting::getSpaceSeparatedList(curHighscore.getDateAndTime(),
                                            dateStrVector);
      const string date   = dateStrVector.at(0);
      const string score  = toStr(curHighscore.getScore());
      grave->setInscription("RIP " + name + " " + date + " Score: " + score);
      Map::put(grave);
    }
  }
}

} //namespace

bool mkIntroLvl() {
  for(int y = 1; y < MAP_H - 1; ++y) {
    for(int x = 1; x < MAP_W - 1; ++x) {
      const Pos p(x, y);
      if(Rnd::oneIn(6)) {
        Map::put(new Bush(p));
      } else {
        Map::put(new Grass(p));
      }
    }
  }

  mkForestOuterTreeline();
  mkForestTrees();
  mkForestLimit();

  PopulateMon::populateIntroLvl();

  return true;
}

//------------------------------------------------------------------------- EGYPT
bool mkEgyptLvl() {
  Map::resetMap();

  const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::egypt);
  const Pos       templDims = templ.getDims();
  const int       STAIR_VAL = Rnd::range(2, 3);

  for(int y = 0; y < templDims.y; ++y) {
    for(int x = 0; x < templDims.x; ++x) {
      const auto& templCell = templ.getCell(x, y);
      const Pos p(x, y);
      if(templCell.featureId != FeatureId::END) {
        if(templCell.val == STAIR_VAL) {
          Map::put(new Stairs(p));
        } else {
          const auto& d = FeatureData::getData(templCell.featureId);
          Map::put(static_cast<Rigid*>(d.mkObj(p)));
        }
      }
      if(templCell.actorId != ActorId::END) {ActorFactory::mk(templCell.actorId, p);}
      if(templCell.val == 1) {Map::player->pos = p;}
    }
  }

  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      Rigid* const f = Map::cells[x][y].rigid;
      if(f->getId() == FeatureId::wall) {
        static_cast<Wall*>(f)->type_ = WallType::egypt;
      }
    }
  }

  PopulateItems::mkItems();

  return true;
}

//------------------------------------------------------------------------- LENG
bool mkLengLvl() {
  Map::resetMap();

  const MapTempl& templ     = MapTemplHandling::getTempl(MapTemplId::leng);
  const Pos       templDims = templ.getDims();

  for(int y = 0; y < templDims.y; ++y) {
    for(int x = 0; x < templDims.x; ++x) {
      const auto& templCell = templ.getCell(x, y);
      const auto  fId       = templCell.featureId;
      const Pos p(x, y);
      if(fId != FeatureId::END) {
        const auto& d = FeatureData::getData(fId);
        auto* const f = Map::put(static_cast<Rigid*>(d.mkObj(p)));
        if(fId == FeatureId::grass) {
          if(Rnd::oneIn(50)) {
            auto* const bush = static_cast<Bush*>(Map::put(new Bush(p)));
            bush->type_ = Rnd::oneIn(5) ? GrassType::cmn : GrassType::withered;
          } else {
            auto* const grass = static_cast<Grass*>(f);
            grass->type_ = Rnd::oneIn(5) ? GrassType::cmn : GrassType::withered;
          }
        } else if(fId == FeatureId::wall) {
          auto* const wall = static_cast<Wall*>(f);
          if(templCell.val == 2) {
            wall->type_ = WallType::cliff;
          } else if(templCell.val == 3 || templCell.val == 5) {
            wall->type_ = WallType::lengMonestary;
          }
        }
      }
      if(templCell.actorId != ActorId::END) {ActorFactory::mk(templCell.actorId, p);}
      if(templCell.val == 1) {Map::player->pos = p;}
      if(templCell.val == 3) {Map::cells[x][y].isDark = true;}
      if(templCell.val == 6) {
        Wall* mimic   = new Wall(p);
        mimic->type_  = WallType::lengMonestary;
        Map::put(new Door(p, mimic, DoorSpawnState::closed));
      }
    }
  }

//  PopulateItems::mkItems();

  return true;
}


//------------------------------------------------------------------------- TRAPEZOHEDRON
bool mkTrapezohedronLvl() {
  Map::resetMap();

  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      auto* const wall  = new Wall(Pos(x, y));
      Map::put(wall);
      wall->type_       = WallType::cave;
      wall->isMossy_    = false;
    }
  }

  const Pos& origin     = Map::player->pos;
  const Pos  mapCenter  = Pos(MAP_W_HALF, MAP_H_HALF);

  auto putCaveFloor = [](const vector<Pos>& positions) {
    for(const Pos& p : positions) {
      auto* const floor = new Floor(p);
      Map::put(floor);
      floor->type_      = FloorType::cave;
    }
  };

  vector<Pos> floorCells;

  MapGenUtils::rndWalk(origin, 150, floorCells, true);
  putCaveFloor(floorCells);

  MapGenUtils::rndWalk(mapCenter, 800, floorCells, true);
  putCaveFloor(floorCells);

  MapGenUtils::pathfinderWalk(origin, mapCenter, floorCells, false);
  putCaveFloor(floorCells);

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  vector<Pos> itemPosBucket;
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      if(!blocked[x][y] && Pos(x, y) != origin) {
        itemPosBucket.push_back(Pos(x, y));
      }
    }
  }

  const int ELEMENT = Rnd::range(0, itemPosBucket.size() - 1);
  ItemFactory::mkItemOnMap(ItemId::trapezohedron, itemPosBucket.at(ELEMENT));
  return true;
}

} //MapGen

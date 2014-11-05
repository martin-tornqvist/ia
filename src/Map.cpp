#include "Init.h"

#include "Map.h"

#include "Feature.h"
#include "ActorFactory.h"
#include "ItemFactory.h"
#include "GameTime.h"
#include "Render.h"
#include "MapGen.h"
#include "Item.h"
#include "Utils.h"
#include "FeatureRigid.h"
#ifdef DEMO_MODE
#include "SdlWrapper.h"
#endif // DEMO_MODE

using namespace std;

Cell::Cell() :
  isExplored(false),
  isSeenByPlayer(false),
  isLit(false),
  isDark(false),
  item(nullptr),
  rigid(nullptr),
  playerVisualMemory(CellRenderData()),
  pos(Pos(-1, -1)) {}

Cell::~Cell() {
  if(rigid) {delete rigid;}
  if(item)  {delete item;}
}

namespace Map {

Player*       player  = nullptr;
int           dlvl    = 0;
Cell          cells[MAP_W][MAP_H];
vector<Room*> roomList;
Room*         roomMap[MAP_W][MAP_H];

namespace {

void resetCells(const bool MAKE_STONE_WALLS) {
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {

      cells[x][y]     = Cell();
      cells[x][y].pos = Pos(x, y);

      roomMap[x][y]   = nullptr;

      Render::renderArray[x][y]         = CellRenderData();
      Render::renderArrayNoActors[x][y] = CellRenderData();

      if(MAKE_STONE_WALLS) {Map::put(new Wall(Pos(x, y)));}
    }
  }
}

} //Namespace

void init() {
  dlvl = 0;

  roomList.clear();

  resetCells(false);

  if(player) {delete player; player = nullptr;}

  const Pos playerPos(PLAYER_START_X, PLAYER_START_Y);
  player = static_cast<Player*>(ActorFactory::mk(ActorId::player, playerPos));

  ActorFactory::deleteAllMon();

  GameTime::eraseAllMobs();
  GameTime::resetTurnTypeAndActorCounters();
}

void cleanup() {
  player = nullptr; //Note: GameTime has deleted player at this point

  resetMap();

  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      delete cells[x][y].rigid;
      cells[x][y].rigid = nullptr;
    }
  }
}

void storeToSaveLines(vector<string>& lines) {
  lines.push_back(toStr(dlvl));
}

void setupFromSaveLines(vector<string>& lines) {
  dlvl = toInt(lines.front());
  lines.erase(begin(lines));
}

void resetMap() {
  ActorFactory::deleteAllMon();

  for(auto* room : roomList) {delete room;}
  roomList.clear();

  resetCells(true);
  GameTime::eraseAllMobs();
  GameTime::resetTurnTypeAndActorCounters();
}

Rigid* put(Rigid* const f) {
  assert(f);

  const Pos p     = f->getPos();
  Cell&     cell  = cells[p.x][p.y];

  delete cell.rigid;

  cell.rigid = f;

#ifdef DEMO_MODE
  if(f->getId() == FeatureId::floor) {
    for(int x = 0; x < MAP_W; ++x) {
      for(int y = 0; y < MAP_H; ++y) {
        Map::cells[x][y].isSeenByPlayer = Map::cells[x][y].isExplored = true;
      }
    }
    Render::drawMap();
    Render::drawGlyph('X', Panel::map, p, clrYellow);
    Render::updateScreen();
    SdlWrapper::sleep(10); //Note: Delay must be >= 2 for user input to be read
  }
#endif // DEMO_MODE

  return f;
}

void updateVisualMemory() {
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      cells[x][y].playerVisualMemory = Render::renderArrayNoActors[x][y];
    }
  }
}

void mkBlood(const Pos& origin) {
  for(int dx = -1; dx <= 1; ++dx) {
    for(int dy = -1; dy <= 1; ++dy) {
      const Pos c = origin + Pos(dx, dy);
      Rigid* const f  = cells[c.x][c.y].rigid;
      if(f->canHaveBlood()) {
        if(Rnd::oneIn(3)) {f->mkBloody();}
      }
    }
  }
}

void mkGore(const Pos& origin) {
  for(int dx = -1; dx <= 1; ++dx) {
    for(int dy = -1; dy <= 1; ++dy) {
      const Pos c = origin + Pos(dx, dy);
      if(Rnd::oneIn(3)) {cells[c.x][c.y].rigid->tryPutGore();}
    }
  }
}

void deleteAndRemoveRoomFromList(Room* const room) {
  for(size_t i = 0; i < roomList.size(); ++i) {
    if(roomList.at(i) == room) {
      delete room;
      roomList.erase(roomList.begin() + i);
      return;
    }
  }
  assert(false && "Tried to remove non-existing room");
}

bool isPosSeenByPlayer(const Pos& p) {
  assert(Utils::isPosInsideMap(p));
  return cells[p.x][p.y].isSeenByPlayer;
}

} //Map

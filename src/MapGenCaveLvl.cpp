#include "MapGen.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include "Converters.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "PopulateMonsters.h"
#include "MapParsing.h"
#include "Utils.h"
#include "FeatureStatic.h"

using namespace std;

namespace MapGen {

namespace CaveLvl {

bool run() {
  Map::resetMap();

  for(int y = 0; y < MAP_H; ++y) {
    for(int x = 0; x < MAP_W; ++x) {
      auto* const wall  = new Wall(Pos(x, y));
      Map::put(wall);
      wall->type_     = WallType::cave;
      wall->isMossy_  = false;
    }
  }

  const Pos& origin = Map::player->pos;

  auto* const floor = new Floor(origin);
  Map::put(floor);
  floor->type_      = FloorType::cave;

  vector<Pos> prevCenters(1, origin);

  auto putCaveFloor = [](const vector<Pos>& positions){
    for(const Pos& p : positions) {
      auto* const f = new Floor(p);
      Map::put(f);
      f->type_ = FloorType::cave;
    }
  };

  //Make a random walk path from player
  int length = 40 + Rnd::range(0, 40);
  vector<Pos> floorPositions;
  MapGenUtils::rndWalk(origin, length, floorPositions, true);

  putCaveFloor(floorPositions);

  //Make some more at random places, connect them to each other.
  const bool  IS_TUNNEL_CAVE = Rnd::coinToss();
  const int   NR_OPEN_PLACES = IS_TUNNEL_CAVE ? Rnd::range(6, 8) : 4;

  const int EDGE_DIST_W = 10;
  const int EDGE_DIST_H = 2;

  for(int i = 0; i < NR_OPEN_PLACES; ++i) {
    const Pos curPos(Rnd::range(EDGE_DIST_W, MAP_W - EDGE_DIST_W - 1),
                     Rnd::range(EDGE_DIST_H, MAP_H - EDGE_DIST_H - 1));

    length = IS_TUNNEL_CAVE ? 30 + Rnd::range(0, 50) : 650;

    MapGenUtils::rndWalk(curPos, length, floorPositions, true);
    putCaveFloor(floorPositions);

    const Pos prevPos = prevCenters.at(Rnd::range(0, prevCenters.size() - 1));

    MapGenUtils::pathfinderWalk(prevPos, curPos, floorPositions, false);
    putCaveFloor(floorPositions);

    prevCenters.push_back(curPos);
  }

  //Make a floodfill and place the stairs in one of the furthest positions
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);
  int floodFill[MAP_W][MAP_H];
  FloodFill::run(origin, blocked, floodFill, 99999, Pos(-1, -1), true);
  vector<PosAndVal> floodVals;
  for(int y = 1; y < MAP_H - 1; ++y) {
    for(int x = 1; x < MAP_W - 1; ++x) {
      const int VAL = floodFill[x][y];
      if(VAL > 0) {floodVals.push_back(PosAndVal(Pos(x, y), VAL));}
    }
  }

  std::sort(floodVals.begin(), floodVals.end(),
  [](const PosAndVal & a, const PosAndVal & b) {return a.val < b.val;});

  const int NR_VALS       = floodVals.size();
  const int STAIR_ELEMENT = Rnd::range((NR_VALS * 4) / 5, NR_VALS - 1);

  Map::put(new Stairs(floodVals.at(STAIR_ELEMENT).pos));

  PopulateMonsters::populateCaveLvl();

  return true;
}

} //CaveLvl

} //MapGen

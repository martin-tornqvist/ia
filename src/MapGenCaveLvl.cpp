#include "MapGen.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include "Converters.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "FeatureWall.h"
#include "PopulateMonsters.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace MapGen {

namespace CaveLvl {

bool run() {
  Map::resetMap();

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Wall* const wall =
        dynamic_cast<Wall*>(FeatureFactory::mk(FeatureId::wall, Pos(x, y)));
      wall->wallType    = WallType::cave;
      wall->isMossGrown = false;
    }
  }

  const Pos& origin       = Map::player->pos;
  const FeatureId floorId = FeatureId::caveFloor;

  FeatureFactory::mk(floorId, origin);

  vector<Pos> prevCenters(1, origin);

  //Make a random walk path from player
  int length = 40 + Rnd::dice(1, 40);
  MapGenUtils::digByRandomWalk(origin, length, floorId, true);
  const bool IS_TUNNEL_CAVE = Rnd::coinToss();

  //Make some more at random places, connect them to each other.
  const int NR_OPEN_PLACES = IS_TUNNEL_CAVE ? Rnd::range(6, 8) : 4;

  const int EDGE_DIST_W = 10;
  const int EDGE_DIST_H = 2;

  for(int i = 0; i < NR_OPEN_PLACES; i++) {
    const Pos curPos(Rnd::range(EDGE_DIST_W, MAP_W - EDGE_DIST_W - 1),
                     Rnd::range(EDGE_DIST_H, MAP_H - EDGE_DIST_H - 1));

    length = IS_TUNNEL_CAVE ? 30 + Rnd::dice(1, 50) : 650;
    MapGenUtils::digByRandomWalk(curPos, length, floorId, true);
    const Pos prevPos = prevCenters.at(Rnd::range(0, prevCenters.size() - 1));

    MapGenUtils::digWithPathfinder(prevPos, curPos, floorId, false, true);

    prevCenters.push_back(curPos);
  }

  //Make a floodfill and place the stairs in one of the furthest positions
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);
  int floodFill[MAP_W][MAP_H];
  FloodFill::run(origin, blocked, floodFill, 99999, Pos(-1, -1));
  vector<PosAndVal> floodVals;
  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      const int VAL = floodFill[x][y];
      if(VAL > 0) {floodVals.push_back(PosAndVal(Pos(x, y), VAL));}
    }
  }

  std::sort(floodVals.begin(), floodVals.end(),
  [](const PosAndVal & a, const PosAndVal & b) {return a.val < b.val;});

  const int NR_VALS       = floodVals.size();
  const int STAIR_ELEMENT = Rnd::range((NR_VALS * 4) / 5, NR_VALS - 1);
  FeatureFactory::mk(FeatureId::stairs, floodVals.at(STAIR_ELEMENT).pos);
  PopulateMonsters::populateCaveLvl();

  return true;
}

} //CaveLvl

} //MapGen

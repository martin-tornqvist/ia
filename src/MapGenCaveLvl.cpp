#include "MapGen.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include "Engine.h"
#include "Converters.h"
#include "FeatureFactory.h"
#include "ActorPlayer.h"
#include "Map.h"
#include "FeatureWall.h"
#include "PopulateMonsters.h"
#include "MapParsing.h"

bool MapGenCaveLvl::specificRun() {
  eng.map->resetMap();

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      eng.featureFactory->spawnFeatureAt(feature_stoneWall, Pos(x, y));
      Wall* const wall = dynamic_cast<Wall*>(
                           eng.map->cells[x][y].featureStatic);
      wall->wallType = wall_cave;
      wall->isMossGrown = false;
    }
  }

  const Pos& playerPos = eng.player->pos;
  eng.featureFactory->spawnFeatureAt(feature_caveFloor, playerPos);

  vector<Pos> previousCenters(1, playerPos);

  //Make a random walk path from player
  int length = 40 + eng.dice(1, 40);
  makePathByRandomWalk(
    playerPos.x, playerPos.y, length, feature_caveFloor, true);
  const bool IS_TUNNEL_CAVE = eng.dice.coinToss();

  //Make some more at random places, connect them to each other.
  const int NR_OPEN_PLACES = IS_TUNNEL_CAVE ? eng.dice.range(6, 8) : 4;
  for(int i = 0; i < NR_OPEN_PLACES; i++) {
    const Pos curCenter(10 + eng.dice(1, MAP_W - 1 - 10) - 1,
                        2 + eng.dice(1, MAP_H - 1 - 2) - 1);
    length = IS_TUNNEL_CAVE ? 30 + eng.dice(1, 50) : 650;
    makePathByRandomWalk(
      curCenter.x, curCenter.y, length, feature_caveFloor, true);
    const Pos prevCenter = previousCenters.at(
                             eng.dice.range(0, previousCenters.size() - 1));
    makeStraightPathByPathfinder(
      prevCenter, curCenter, feature_caveFloor, false, true);
    previousCenters.push_back(curCenter);
  }

  //Make a floodfill and place the stairs in one of the furthest positions
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, true, eng),
                   blockers);
  int floodFill[MAP_W][MAP_H];
  eng.floodFill->run(playerPos, blockers, floodFill, 99999, Pos(-1, -1));
  vector<PosAndVal> floodFillVector;
  for(unsigned int y = 1; y < MAP_H - 1; y++) {
    for(unsigned int x = 1; x < MAP_W - 1; x++) {
      const int VAL = floodFill[x][y];
      if(VAL > 0) {
        floodFillVector.push_back(PosAndVal(Pos(x, y), VAL));
      }
    }
  }
  PosAndVal_compareForVal floodFillSorter;
  std::sort(floodFillVector.begin(), floodFillVector.end(), floodFillSorter);
  const unsigned int STAIR_ELEMENT =
    eng.dice.range((floodFillVector.size() * 4) / 5,
                    floodFillVector.size() - 1);
  eng.featureFactory->spawnFeatureAt(
    feature_stairsDown, floodFillVector.at(STAIR_ELEMENT).pos);
  eng.populateMonsters->populateCaveLevel();

  return true;
}

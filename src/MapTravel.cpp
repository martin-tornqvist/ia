#include "MapTravel.h"

#include <iostream>
#include <list>

#include "Init.h"
#include "Map.h"
#include "MapGen.h"
#include "PopulateItems.h"
#include "Render.h"
#include "Log.h"
#include "FeatureRigid.h"

using namespace std;

namespace MapTravel {

namespace {

struct MapData {
  MapType type;
  bool    isInMainDungeon;
};

vector<MapData> mapList_;

void mkLvl(const MapType& mapType) {
  TRACE_FUNC_BEGIN;

  bool isLvlBuilt = false;

#ifndef NDEBUG
  int nrAttempts = 0;
#endif

  while(!isLvlBuilt) {
#ifndef NDEBUG
    nrAttempts++;
#endif
    switch(mapType) {
      case MapType::intro:          isLvlBuilt = MapGen::mkIntroLvl();          break;
      case MapType::std:            isLvlBuilt = MapGen::mkStdLvl();            break;
      case MapType::caves:          isLvlBuilt = MapGen::mkCaveLvl();           break;
      case MapType::egypt:          isLvlBuilt = MapGen::mkEgyptLvl();          break;
      case MapType::leng:           isLvlBuilt = MapGen::mkLengLvl();           break;
      case MapType::trapezohedron:  isLvlBuilt = MapGen::mkTrapezohedronLvl();  break;
    }
  }

#ifndef NDEBUG
  TRACE << "Map built after " << nrAttempts << " attempt(s)" << endl;
#endif

//  //------------------------------------- TRAPEZOHEDRON LEVEL
//  if(!isLvlBuilt) {
//    if(Map::dlvl > LAST_CAVERN_LVL) {
//      while(!MapGen::mkTrapezohedronLvl()) {}
//      isLvlBuilt = true;
//    }
//  }
//
//  //------------------------------------- EGYPT
//  if(!isLvlBuilt) {
//    if(Map::dlvl == LAST_ROOM_AND_CORRIDOR_LVL + 1) {
//      while(!MapGen::mkEgyptLvl()) {}
//      isLvlBuilt = true;
//    }
//  }
//
//  //------------------------------------- STANDARD DUNGEON
//  if(!isLvlBuilt) {
//    if(Map::dlvl < FIRST_CAVERN_LVL) {
//      while(!MapGen::mkStdLvl()) {}
//      isLvlBuilt = true;
//    }
//  }
//  //------------------------------------- CAVERN LEVELS
//  if(!isLvlBuilt) {
//    if(Map::dlvl >= FIRST_CAVERN_LVL) {
//      while(!MapGen::mkCaveLvl()) {}
//    }
//  }

  TRACE_FUNC_END;
}

} //namespace

void init() {
  mapList_    = vector<MapData>(30, {MapType::std, true});
  mapList_[0] = {MapType::intro,  true};
//  mapList_[2] = {MapType::leng,   false};
}

void storeToSaveLines(std::vector<std::string>& lines) {
  lines.push_back(toStr(mapList_.size()));
  for(const MapData& entry : mapList_) {
    lines.push_back(toStr(int(entry.type)));
    lines.push_back(entry.isInMainDungeon ? "1" : "0");
  }
}

void setupFromSaveLines(std::vector<std::string>& lines) {
  const int SIZE = toInt(lines.front());
  lines.erase(begin(lines));
  for(int i = 0; i < SIZE; ++i) {
    const MapType type        = MapType(toInt(lines.front()));
    lines.erase(begin(lines));
    const bool    IS_IN_MAIN  = lines.front() == "1";
    lines.erase(begin(lines));
    mapList_.push_back({type, IS_IN_MAIN});
  }
}

void goToNxt() {
  TRACE_FUNC_BEGIN;

  Log::clearLog();

  if(Map::dlvl >= FIRST_CAVERN_LVL && Map::dlvl <= LAST_CAVERN_LVL) {
    Log::addMsg("I climb downwards.");
  } else {
    Log::addMsg("I descend the stairs.");
  }

  Map::player->restoreShock(999, true);

  mapList_.erase(mapList_.begin());
  const auto& mapData = mapList_.front();

  if(mapData.isInMainDungeon) {Map::dlvl++;}

  mkLvl(mapData.type);

  Map::player->target = nullptr;
  Map::player->updateFov();
  Map::player->updateClr();
  Render::drawMapAndInterface();
  Audio::tryPlayAmb(1);

  if(Map::player->phobias[int(Phobia::deepPlaces)]) {
    Log::addMsg("I am plagued by my phobia of deep places!");
    Map::player->getPropHandler().tryApplyProp(new PropTerrified(PropTurns::std));
    return;
  }

  TRACE_FUNC_END;
}

MapType getMapType() {
  return mapList_.front().type;
}

} //MapTravel

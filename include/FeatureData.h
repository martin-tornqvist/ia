#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include <vector>
#include <string>
#include <iostream>

#include "Art.h"
#include "CommonData.h"
#include "RoomTheme.h"
#include "MapPatterns.h"

using namespace std;

class Engine;

enum Feature_t {
  feature_empty,
  feature_stoneFloor,
  feature_stoneWall,
  feature_tree,
  feature_grass,
  feature_grassWithered,
  feature_bush,
  feature_bushWithered,
  feature_forestPath,
  feature_stairsDown,
  feature_lever,
  feature_brazierGolden,
  feature_caveFloor,
  feature_gravestone,
  feature_tomb,
  feature_churchBench,
  feature_altar,
  feature_churchCarpet,
  feature_rubbleHigh,
  feature_rubbleLow,
  feature_statue,
  feature_ghoulStatue,
  feature_cocoon,
  feature_chest,
  feature_cabinet,
  feature_fountain,
//  feature_barrel,
  feature_pillar,
  feature_pillarBroken,
//  feature_pillarCarved,
  feature_chasm,
  feature_shallowWater,
  feature_deepWater,
  feature_poolBlood,
  feature_shallowMud,

  feature_door,
  feature_litDynamite,
  feature_litFlare,
  feature_trap,
  feature_trashedSpiderWeb,
  feature_gore,
  feature_smoke,
  feature_proxEventWallCrumble,

  feature_pit,

  endOfFeatures
};

enum FeatureSpawnType_t {
  featureSpawnType_mob, featureSpawnType_static, featureSpawnType_other
};

enum MaterialType_t {
  materialType_empty, materialType_soft, materialType_hard, materialType_fluid
};

struct ThemedFeatureSpawnRules {
public:
  ThemedFeatureSpawnRules() : maxNrInRoom_(-1), placementRule_(placementRule_nextToWalls) {
    themesBelongingTo_.resize(0);
  }

  void reset() {
    maxNrInRoom_ = -1;
    placementRule_ = placementRule_awayFromWalls;
    themesBelongingTo_.resize(0);
  }

  void set(
    const int MAX_NR_IN_ROOM,
    const PlacementRule_t placementRule,
    RoomTheme_t theme1 = endOfRoomThemes,
    RoomTheme_t theme2 = endOfRoomThemes,
    RoomTheme_t theme3 = endOfRoomThemes,
    RoomTheme_t theme4 = endOfRoomThemes) {
    maxNrInRoom_ = MAX_NR_IN_ROOM;
    placementRule_ = placementRule;
    themesBelongingTo_.resize(0);
    if(theme1 != endOfRoomThemes) {
      themesBelongingTo_.push_back(theme1);
    }
    if(theme2 != endOfRoomThemes) {
      themesBelongingTo_.push_back(theme2);
    }
    if(theme3 != endOfRoomThemes) {
      themesBelongingTo_.push_back(theme3);
    }
    if(theme4 != endOfRoomThemes) {
      themesBelongingTo_.push_back(theme4);
    }
  }

  bool isBelongingToTheme(const RoomTheme_t theme) const {
    for(unsigned int i = 0; i < themesBelongingTo_.size(); i++) {
      if(themesBelongingTo_.at(i) == theme) {
        return true;
      }
    }
    return false;
  }

  PlacementRule_t getPlacementRule() const {
    return placementRule_;
  }

  int getMaxNrInRoom() const {
    return maxNrInRoom_;
  }

private:
  int maxNrInRoom_;
  PlacementRule_t placementRule_;
  vector<RoomTheme_t> themesBelongingTo_;
};

struct FeatureData {
  Feature_t id;
  FeatureSpawnType_t spawnType;
  char glyph;
  SDL_Color color;
  SDL_Color colorBg;
  Tile_t tile;
  bool isBodyTypePassable[endOfActorBodyTypes];
  bool isProjectilesPassable;
  bool isVisionPassable;
  bool isSmokePassable;
  bool canHaveBlood;
  bool canHaveGore;
  bool canHaveCorpse;
  bool canHaveStaticFeature;
  bool canHaveItem;
  bool isBottomless;
  MaterialType_t materialType;
  string name_a;
  string name_the;
  string messageOnPlayerBlocked;
  string messageOnPlayerBlockedBlind;
  int dodgeModifier;
  int shockWhenAdjacent;
  ThemedFeatureSpawnRules themedFeatureSpawnRules;
  vector<Feature_t> featuresOnDestroyed;
};

class FeatureDataHandler {
public:
  FeatureDataHandler(Engine& engine) : eng(engine) {initDataList();}

  ~FeatureDataHandler() {}

  inline const FeatureData* getData(const Feature_t id) const {
    return &(dataList[id]);
  }

  void initDataList();

private:
  void addToListAndReset(FeatureData& d);

  FeatureData dataList[endOfFeatures];

  void resetData(FeatureData& d);

  Engine& eng;
};

#endif

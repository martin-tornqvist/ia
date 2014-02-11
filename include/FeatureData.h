#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include <vector>
#include <string>
#include <iostream>

#include "Art.h"
#include "CommonData.h"
#include "RoomTheme.h"
#include "MapPatterns.h"
#include "Properties.h"

using namespace std;

class Engine;

enum FeatureId {
  feature_empty,
  feature_stoneFloor,
  feature_stoneWall,
  feature_tree,
  feature_grass,
  feature_grassWithered,
  feature_bush,
  feature_bushWithered,
  feature_forestPath,
  feature_stairs,
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
  feature_gore,
  feature_smoke,
  feature_proxEventWallCrumble,

  feature_pit,

  endOfFeatureId
};

enum FeatureSpawnType {
  featureSpawnType_mob, featureSpawnType_static, featureSpawnType_other
};

enum MaterialType {
  materialType_empty, materialType_soft, materialType_hard, materialType_fluid
};

struct FeatureThemeSpawnRules {
public:
  FeatureThemeSpawnRules() :
    maxNrInRoom_(-1), placementRule_(placementRule_nextToWalls) {
    themesBelongingTo_.resize(0);
  }

  void reset() {
    maxNrInRoom_ = -1;
    placementRule_ = placementRule_awayFromWalls;
    themesBelongingTo_.resize(0);
  }

  void set(
    const int MAX_NR_IN_ROOM,
    const PlacementRule placementRule,
    std::initializer_list<RoomThemeId> roomThemes) {
    maxNrInRoom_ = MAX_NR_IN_ROOM;
    placementRule_ = placementRule;
    themesBelongingTo_.resize(0);
    for(RoomThemeId id : roomThemes) {themesBelongingTo_.push_back(id);}
  }

  bool isBelongingToTheme(const RoomThemeId theme) const {
    for(RoomThemeId idToCheck : themesBelongingTo_) {
      if(idToCheck == theme) return true;
    }
    return false;
  }

  inline PlacementRule getPlacementRule() const {return placementRule_;}
  inline int getMaxNrInRoom() const {return maxNrInRoom_;}

private:
  int maxNrInRoom_;
  PlacementRule placementRule_;
  vector<RoomThemeId> themesBelongingTo_;
};

class Actor;

class MoveRules {
public:
  MoveRules() {reset();}

  ~MoveRules() {}

  void reset() {
    canMoveCmn_ = false;
    for(int i = 0; i < endOfPropIds; i++) {canMoveIfHaveProp_[i] = false;}
  }

  void setPropCanMove(const PropId id) {canMoveIfHaveProp_[id] = true;}
  void setCanMoveCmn() {canMoveCmn_ = true;}

  bool canMoveCmn() const {return canMoveCmn_;}

  bool canMove(const vector<PropId>& actorsProps) const;

private:
  bool canMoveCmn_;
  bool canMoveIfHaveProp_[endOfPropIds];
};

struct FeatureData {
  FeatureId id;
  FeatureSpawnType spawnType;
  char glyph;
  SDL_Color color;
  SDL_Color colorBg;
  TileId tile;
  MoveRules moveRules;
  bool isSoundPassable;
  bool isProjectilePassable;
  bool isVisionPassable;
  bool isSmokePassable;
  bool canHaveBlood;
  bool canHaveGore;
  bool canHaveCorpse;
  bool canHaveStaticFeature;
  bool canHaveItem;
  bool isBottomless;
  MaterialType materialType;
  string name_a;
  string name_the;
  string messageOnPlayerBlocked;
  string messageOnPlayerBlockedBlind;
  int dodgeModifier;
  int shockWhenAdjacent;
  FeatureThemeSpawnRules featureThemeSpawnRules;
  vector<FeatureId> featuresOnDestroyed;
};

class FeatureDataHandler {
public:
  FeatureDataHandler(Engine& engine) : eng(engine) {initDataList();}

  ~FeatureDataHandler() {}

  inline const FeatureData* getData(const FeatureId id) const {
    return &(dataList[id]);
  }

  void initDataList();

private:
  void addToListAndReset(FeatureData& d);

  FeatureData dataList[endOfFeatureId];

  void resetData(FeatureData& d);

  Engine& eng;
};

#endif

#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include <vector>
#include <string>
#include <iostream>

#include "Art.h"
#include "CmnData.h"
#include "RoomTheme.h"
#include "MapPatterns.h"
#include "Properties.h"

enum class FeatureId {
  empty,
  floor,
  wall,
  tree,
  grass,
  grassWithered,
  bush,
  bushWithered,
  forestPath,
  stairs,
  lever,
  brazierGolden,
  caveFloor,
  gravestone,
  tomb,
  churchBench,
  altar,
  churchCarpet,
  rubbleHigh,
  rubbleLow,
  statue,
  ghoulStatue,
  cocoon,
  chest,
  cabinet,
  fountain,
//  barrel,
  pillar,
  pillarBroken,
//  pillarCarved,
  chasm,
  shallowWater,
  deepWater,
  poolBlood,
  shallowMud,

  door,
  litDynamite,
  litFlare,
  trap,
  gore,
  smoke,
  proxEventWallCrumble,

  pit,

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
    maxNrInRoom_(-1), placementRule_(PlacementRule::nextToWalls) {
    themesBelongingTo_.resize(0);
  }

  void reset() {
    maxNrInRoom_ = -1;
    placementRule_ = PlacementRule::awayFromWalls;
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
  std::vector<RoomThemeId> themesBelongingTo_;
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

  bool canMove(const std::vector<PropId>& actorsProps) const;

private:
  bool canMoveCmn_;
  bool canMoveIfHaveProp_[endOfPropIds];
};

struct FeatureDataT {
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
  std::string name_a;
  std::string name_the;
  std::string messageOnPlayerBlocked;
  std::string messageOnPlayerBlockedBlind;
  int dodgeModifier;
  int shockWhenAdjacent;
  FeatureThemeSpawnRules featureThemeSpawnRules;
  std::vector<FeatureId> featuresOnDestroyed;
};

namespace FeatureData {

void init();

const FeatureDataT* getData(const FeatureId id);

} //FeatureData

#endif

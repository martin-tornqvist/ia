#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include <vector>
#include <string>
#include <iostream>
#include <functional>

#include "Art.h"
#include "CmnData.h"
#include "Room.h"
#include "MapPatterns.h"
#include "Properties.h"

enum class FeatureId {
  floor,
  bridge,
  wall,
  tree,
  grass,
  bush,
  stairs,
  lever,
  brazier,
  gravestone,
  tomb,
  churchBench,
  altar,
  carpet,
  rubbleHigh,
  rubbleLow,
  statue,
  cocoon,
  chest,
  cabinet,
  fountain,
//  barrel,
  pillar,
  chasm,
  liquidShallow,
  liquidDeep,
  door,
  litDynamite,
  litFlare,
  trap,
//  pit,
//  gore,
  smoke,
  proxEventWallCrumble,

  END
};

struct FeatureRoomSpawnRules {
public:
  FeatureRoomSpawnRules() :
    maxNrInRoom_(-1), placementRule_(PlacementRule::nextToWalls) {
    roomTypesNative_.clear();
  }

  void reset() {*this = FeatureRoomSpawnRules();}

  void set(const int MAX_NR_IN_ROOM,  const PlacementRule placementRule,
           std::initializer_list<RoomType> roomTypes) {
    maxNrInRoom_    = MAX_NR_IN_ROOM;
    placementRule_  = placementRule;
    roomTypesNative_.clear();
    for(RoomType id : roomTypes) {roomTypesNative_.push_back(id);}
  }

  bool isBelongingToRoomType(const RoomType type) const {
    for(RoomType id : roomTypesNative_) {if(id == type) {return true;}}
    return false;
  }

  PlacementRule getPlacementRule()  const {return placementRule_;}
  int           getMaxNrInRoom()    const {return maxNrInRoom_;}

private:
  int                   maxNrInRoom_;
  PlacementRule         placementRule_;
  std::vector<RoomType> roomTypesNative_;
};

class Actor;

class MoveRules {
public:
  MoveRules() {reset();}

  ~MoveRules() {}

  void reset() {
    canMoveCmn_ = false;
    for(int i = 0; i < endOfPropIds; ++i) {canMoveIfHaveProp_[i] = false;}
  }

  void setPropCanMove(const PropId id) {canMoveIfHaveProp_[id] = true;}
  void setCanMoveCmn() {canMoveCmn_ = true;}

  bool canMoveCmn() const {return canMoveCmn_;}

  bool canMove(const std::vector<PropId>& actorsProps) const;

private:
  bool canMoveCmn_;
  bool canMoveIfHaveProp_[endOfPropIds];
};

class Feature;

struct FeatureDataT {
  std::function<Feature*(const Pos& p)> mkObj;
  FeatureId id;
  char glyph;
  TileId tile;
  MoveRules moveRules;
  bool isSoundPassable;
  bool isProjectilePassable;
  bool isLosPassable;
  bool isSmokePassable;
  bool canHaveBlood;
  bool canHaveGore;
  bool canHaveCorpse;
  bool canHaveRigid;
  bool canHaveItem;
  bool isBottomless;
  Matl matlType;
  std::string msgOnPlayerBlocked;
  std::string msgOnPlayerBlockedBlind;
  int dodgeModifier;
  int shockWhenAdjacent;
  FeatureRoomSpawnRules themeSpawnRules;
};

namespace FeatureData {

void init();

const FeatureDataT& getData(const FeatureId id);

} //FeatureData

#endif

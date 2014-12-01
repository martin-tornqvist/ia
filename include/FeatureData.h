#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include "Art.h"
#include "CmnData.h"
#include "Room.h"
#include "MapPatterns.h"
#include "Properties.h"

enum class FeatureId
{
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
  smoke,
  proxEventWallCrumble,

  END
};

struct FeatureRoomSpawnRules
{
public:
  FeatureRoomSpawnRules();

  void reset();

  void set(const int MAX_NR_IN_ROOM, const Range dlvlsAllowed,
           const PlacementRule placementRule, std::initializer_list<RoomType> roomTypes);

  bool          isBelongingToRoomType(const RoomType type)  const;
  PlacementRule getPlacementRule()                          const;
  int           getMaxNrInRoom()                            const;
  Range         getDlvlsAllowed()                           const;

private:
  int                   maxNrInRoom_;
  Range                 dlvlsAllowed_;
  PlacementRule         placementRule_;
  std::vector<RoomType> roomTypesNative_;
};

class Actor;

class MoveRules
{
public:
  MoveRules() {reset();}

  ~MoveRules() {}

  void reset()
  {
    canMoveCmn_ = false;
    for (int i = 0; i < endOfPropIds; ++i) {canMoveIfHaveProp_[i] = false;}
  }

  void setPropCanMove(const PropId id) {canMoveIfHaveProp_[id] = true;}
  void setCanMoveCmn() {canMoveCmn_ = true;}

  bool canMoveCmn() const {return canMoveCmn_;}

  bool canMove(const bool actorPropIds[endOfPropIds]) const;

private:
  bool canMoveCmn_;
  bool canMoveIfHaveProp_[endOfPropIds];
};

class Feature;

struct FeatureDataT
{
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
  FeatureRoomSpawnRules roomSpawnRules;
};

namespace FeatureData
{

void init();

const FeatureDataT& getData(const FeatureId id);

} //FeatureData

#endif

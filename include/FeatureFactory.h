#ifndef FEATURE_FACTORY_H
#define FEATURE_FACTORY_H

#include <vector>
#include <string>
#include <iostream>

#include "GameTime.h"
#include "FeatureTrap.h"
#include "FeatureDoor.h"

struct FeatureDataT;

enum FeatureSpawnDataId {
  featureSpawnData_dynamite,
  featureSpawnData_trap,
  featureSpawnData_door,
  featureSpawnData_lever,
  featureSpawnData_smoke,
  featureSpawnData_proxEventWallCrumble
};

class FeatureSpawnData {
public:
  FeatureSpawnDataId getFeatureSpawnDataType() const {
    return featureSpawnDataType_;
  }
  virtual ~FeatureSpawnData() {}

  FeatureSpawnDataId getFeatureSpawnDataType() {return featureSpawnDataType_;}

protected:
  FeatureSpawnData(FeatureSpawnDataId featureSpawnDataType) :
    featureSpawnDataType_(featureSpawnDataType) {}
  const FeatureSpawnDataId featureSpawnDataType_;
};

class DynamiteSpawnData: public FeatureSpawnData {
public:
  DynamiteSpawnData(int turnsLeftToExplosion) :
    FeatureSpawnData(featureSpawnData_dynamite),
    turnsLeftToExplosion_(turnsLeftToExplosion) {}
  int turnsLeftToExplosion_;
};

class SmokeSpawnData: public FeatureSpawnData {
public:
  SmokeSpawnData(int life) :
    FeatureSpawnData(featureSpawnData_smoke), life_(life) {}
  int life_;
};

class ProxEventWallCrumbleSpawnData: public FeatureSpawnData {
public:
  ProxEventWallCrumbleSpawnData(std::vector<Pos> wallCells,
                                std::vector<Pos> innerCells) :
    FeatureSpawnData(featureSpawnData_proxEventWallCrumble),
    wallCells_(wallCells), innerCells_(innerCells) {}
  std::vector<Pos> wallCells_;
  std::vector<Pos> innerCells_;
};

class TrapSpawnData: public FeatureSpawnData {
public:
  TrapSpawnData(const FeatureDataT* const mimicFeature, TrapId trapType) :
    FeatureSpawnData(featureSpawnData_trap), mimicFeature_(mimicFeature),
    trapType_(trapType) {}
  TrapSpawnData(const FeatureDataT* const mimicFeature) :
    FeatureSpawnData(featureSpawnData_trap), mimicFeature_(mimicFeature),
    trapType_(trap_any) {}
  const FeatureDataT* const mimicFeature_;
  const TrapId trapType_;
};

class DoorSpawnData: public FeatureSpawnData {
public:
  DoorSpawnData(const FeatureDataT* const mimicFeature) :
    FeatureSpawnData(featureSpawnData_door), mimicFeature_(mimicFeature) {}
  const FeatureDataT* const mimicFeature_;
};

class LeverSpawnData: public FeatureSpawnData {
public:
  LeverSpawnData(Door* const doorLinkedTo) :
    FeatureSpawnData(featureSpawnData_lever), doorLinkedTo_(doorLinkedTo) {}
  Door* const doorLinkedTo_;
};

namespace FeatureFactory {

//Note: Position parameter should NOT be a reference here, because the
//calling object (e.g. a spider web) might get destroyed.
Feature* spawnFeatureAt(const FeatureId id, const Pos pos,
                        FeatureSpawnData* spawnData = NULL);

} //FeatureFactory

#endif

#ifndef FEATURE_FACTORY_H
#define FEATURE_FACTORY_H

#include <vector>
#include <string>
#include <iostream>

#include "GameTime.h"
#include "FeatureTrap.h"
#include "FeatureDoor.h"

using namespace std;

class Engine;
class FeatureDef;

enum FeatureSpawnData_t {
  featureSpawnData_dynamite,
  featureSpawnData_trap,
  featureSpawnData_door,
  featureSpawnData_lever,
  featureSpawnData_smoke,
  featureSpawnData_proxEventWallCrumble
};

class FeatureSpawnData {
public:
  FeatureSpawnData_t getFeatureSpawnDataType() const {
    return featureSpawnDataType_;
  }
  virtual ~FeatureSpawnData() {
  }

  FeatureSpawnData_t getFeatureSpawnDataType() {
    return featureSpawnDataType_;
  }

protected:
  FeatureSpawnData(FeatureSpawnData_t featureSpawnDataType) :
    featureSpawnDataType_(featureSpawnDataType) {
  }
  const FeatureSpawnData_t featureSpawnDataType_;
};

class DynamiteSpawnData: public FeatureSpawnData {
public:
  DynamiteSpawnData(int turnsLeftToExplosion) :
    FeatureSpawnData(featureSpawnData_dynamite), turnsLeftToExplosion_(turnsLeftToExplosion) {
  }
  int turnsLeftToExplosion_;
};

class SmokeSpawnData: public FeatureSpawnData {
public:
  SmokeSpawnData(int life) :
    FeatureSpawnData(featureSpawnData_smoke), life_(life) {
  }
  int life_;
};

class ProxEventWallCrumbleSpawnData: public FeatureSpawnData {
public:
  ProxEventWallCrumbleSpawnData(vector<coord> wallCells, vector<coord> innerCells) :
    FeatureSpawnData(featureSpawnData_proxEventWallCrumble), wallCells_(wallCells), innerCells_(innerCells) {
  }
  vector<coord> wallCells_;
  vector<coord> innerCells_;
};

class TrapSpawnData: public FeatureSpawnData {
public:
  TrapSpawnData(const FeatureDef* const mimicFeature, Trap_t trapType) :
    FeatureSpawnData(featureSpawnData_trap), mimicFeature_(mimicFeature), trapType_(trapType) {
  }
  TrapSpawnData(const FeatureDef* const mimicFeature) :
    FeatureSpawnData(featureSpawnData_trap), mimicFeature_(mimicFeature), trapType_(trap_any) {
  }
  const FeatureDef* const mimicFeature_;
  const Trap_t trapType_;
};

class DoorSpawnData: public FeatureSpawnData {
public:
  DoorSpawnData(const FeatureDef* const mimicFeature) :
    FeatureSpawnData(featureSpawnData_door), mimicFeature_(mimicFeature) {
  }
  const FeatureDef* const mimicFeature_;
};

class LeverSpawnData: public FeatureSpawnData {
public:
  LeverSpawnData(Door* const doorLinkedTo) :
    FeatureSpawnData(featureSpawnData_lever), doorLinkedTo_(doorLinkedTo) {
  }
  Door* const doorLinkedTo_;
};

class FeatureFactory {
public:
  FeatureFactory(Engine* engine) {
    eng = engine;
  }

  Feature* spawnFeatureAt(const Feature_t id, const coord pos, FeatureSpawnData* spawnData = NULL);

private:
  void replaceStaticFeatureAt(FeatureStatic* const newFeature, const coord pos);

  Engine* eng;
};

#endif

#ifndef FEATURE_SMOKE_H
#define FEATURE_SMOKE_H

#include "Feature.h"
#include "FeatureFactory.h"

class Smoke: public FeatureMob {
public:
  ~Smoke() {}

  void newTurn();

protected:
  friend class FeatureFactory;
  Smoke(FeatureId id, Pos pos, SmokeSpawnData* spawnData) :
    FeatureMob(id, pos), life_(spawnData->life_),
    isPermanent_(life_ == -1) {
  }

  int life_;
  bool isPermanent_;
};

#endif

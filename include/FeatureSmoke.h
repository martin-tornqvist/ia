#ifndef FEATURE_SMOKE_H
#define FEATURE_SMOKE_H

#include "Feature.h"
#include "FeatureFactory.h"

class Smoke: public FeatureMob {
public:
  Smoke(FeatureId id, Pos pos, SmokeSpawnData* spawnData) :
    FeatureMob(id, pos), life_(spawnData->life_),
    isPermanent_(life_ == -1) {}

  ~Smoke() {}

  void newTurn();

protected:
  int life_;
  bool isPermanent_;
};

#endif

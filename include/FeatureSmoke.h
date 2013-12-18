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
  Smoke(Feature_t id, Pos pos, Engine& engine, SmokeSpawnData* spawnData) :
    FeatureMob(id, pos, engine), life_(spawnData->life_),
    isPermanent_(life_ == -1) {
  }

  int life_;
  bool isPermanent_;
};

#endif

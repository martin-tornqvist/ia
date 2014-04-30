#ifndef FEATURE_LIT_DYNAMITE_H
#define FEATURE_LIT_DYNAMITE_H

#include "Feature.h"
#include "FeatureFactory.h"

class LitDynamite: public FeatureMob {
public:
  LitDynamite(FeatureId id, Pos pos, DynamiteSpawnData* spawnData) :
    FeatureMob(id, pos),
    turnsLeftToExplosion_(spawnData->turnsLeftToExplosion_) {}

  ~LitDynamite() {}

  void newTurn();

private:
  int turnsLeftToExplosion_;
};

class LitFlare: public FeatureMob {
public:
  LitFlare(FeatureId id, Pos pos, DynamiteSpawnData* spawnData);
  ~LitFlare() {}

  void newTurn();

  void addLight(bool light[MAP_W][MAP_H]) const;

private:
  int life_;
};

#endif

#ifndef FEATURE_LIT_DYNAMITE_H
#define FEATURE_LIT_DYNAMITE_H

#include "Feature.h"
#include "FeatureFactory.h"

using namespace std;

class LitDynamite: public FeatureMob {
public:
  ~LitDynamite() {
  }

  void newTurn();

private:
  friend class FeatureFactory;
  LitDynamite(FeatureId id, Pos pos, Engine& engine, DynamiteSpawnData* spawnData) :
    FeatureMob(id, pos, engine), turnsLeftToExplosion_(spawnData->turnsLeftToExplosion_) {
  }

  int turnsLeftToExplosion_;
};

class LitFlare: public FeatureMob {
public:
  ~LitFlare() {
  }

  void newTurn();

  void addLight(bool light[MAP_W][MAP_H]) const;

//  static int getLightRadius() {
//    return FOV_STD_RADI_INT;
//  }

private:
  friend class FeatureFactory;
  LitFlare(FeatureId id, Pos pos, Engine& engine, DynamiteSpawnData* spawnData);

//  vector<Pos> light_;
  int life_;
};

#endif

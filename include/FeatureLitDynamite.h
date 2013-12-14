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
  LitDynamite(Feature_t id, Pos pos, Engine& engine, DynamiteSpawnData* spawnData) :
    FeatureMob(id, pos, engine), turnsLeftToExplosion_(spawnData->turnsLeftToExplosion_) {
  }

  int turnsLeftToExplosion_;
};

class LitFlare: public FeatureMob {
public:
  ~LitFlare() {
  }

  void newTurn();

  void addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const;

//  static int getLightRadius() {
//    return FOV_STANDARD_RADI_INT;
//  }

private:
  friend class FeatureFactory;
  LitFlare(Feature_t id, Pos pos, Engine& engine, DynamiteSpawnData* spawnData);

//  vector<Pos> light_;
  int life_;
};

#endif

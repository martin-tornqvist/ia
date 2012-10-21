#ifndef FEATURE_LIQUID_H
#define FEATURE_LIQUID_H

#include "Feature.h"

class FeatureLiquidShallow: public FeatureStatic {
public:
  ~FeatureLiquidShallow() {
  }

  void bump(Actor* actorBumping);

protected:
  friend class FeatureFactory;
  FeatureLiquidShallow(Feature_t id, coord pos, Engine* engine);
};

class FeatureLiquidDeep: public FeatureStatic {
public:
  ~FeatureLiquidDeep() {
  }

  void bump(Actor* actorBumping);

protected:
  friend class FeatureFactory;
  FeatureLiquidDeep(Feature_t id, coord pos, Engine* engine);
};

#endif

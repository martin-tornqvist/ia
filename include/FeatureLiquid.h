#ifndef FEATURE_LIQUID_H
#define FEATURE_LIQUID_H

#include "Feature.h"

class FeatureLiquidShallow: public FeatureStatic {
public:
  FeatureLiquidShallow(FeatureId id, Pos pos);

  ~FeatureLiquidShallow() {}

  void bump(Actor& actorBumping) override;
};

class FeatureLiquidDeep: public FeatureStatic {
public:
  FeatureLiquidDeep(FeatureId id, Pos pos);

  ~FeatureLiquidDeep() {}

  void bump(Actor& actorBumping) override;
};

#endif

#ifndef FEATURE_LIQUID_H
#define FEATURE_LIQUID_H

#include "Feature.h"

class FeatureLiquidShallow: public FeatureStatic {
public:
  ~FeatureLiquidShallow() {}

  void bump(Actor& actorBumping) override;

protected:
  friend class FeatureFactory;
  FeatureLiquidShallow(FeatureId id, Pos pos);
};

class FeatureLiquidDeep: public FeatureStatic {
public:
  ~FeatureLiquidDeep() {}

  void bump(Actor& actorBumping) override;

protected:
  friend class FeatureFactory;
  FeatureLiquidDeep(FeatureId id, Pos pos);
};

#endif

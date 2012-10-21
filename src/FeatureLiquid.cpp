#include "FeatureLiquid.h"


FeatureLiquidShallow::FeatureLiquidShallow(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {

}

void FeatureLiquidShallow::bump(Actor* actorBumping) {

}


FeatureLiquidDeep::FeatureLiquidDeep(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {

}

void FeatureLiquidDeep::bump(Actor* actorBumping) {

}

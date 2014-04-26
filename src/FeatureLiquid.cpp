#include "FeatureLiquid.h"

#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "PlayerBon.h"
#include "Audio.h"

FeatureLiquidShallow::FeatureLiquidShallow(
  FeatureId id, Pos pos) :
  FeatureStatic(id, pos) {}

void FeatureLiquidShallow::bump(Actor& actorBumping) {
  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(
    find(props.begin(), props.end(), propEthereal)  == props.end() &&
    find(props.begin(), props.end(), propFlying)    == props.end()) {

    actorBumping.getPropHandler().tryApplyProp(
      new PropWaiting(eng, propTurnsStd));

    if(&actorBumping == Map::player) eng.log->addMsg("*glop*");
  }
}

FeatureLiquidDeep::FeatureLiquidDeep(FeatureId id, Pos pos) :
  FeatureStatic(id, pos) {}

void FeatureLiquidDeep::bump(Actor& actorBumping) {
  (void)actorBumping;
}

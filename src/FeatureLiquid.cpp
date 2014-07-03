#include "FeatureLiquid.h"

#include "Init.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "PlayerBon.h"
#include "Audio.h"
#include "Map.H"

using namespace std;

FeatureLiquidShallow::FeatureLiquidShallow(
  FeatureId id, Pos pos) :
  FeatureStatic(id, pos) {}

void FeatureLiquidShallow::bump(Actor& actorBumping) {
  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(
    find(begin(props), end(props), propEthereal)  == end(props) &&
    find(begin(props), end(props), propFlying)    == end(props)) {

    actorBumping.getPropHandler().tryApplyProp(
      new PropWaiting(propTurnsStd));

    if(&actorBumping == Map::player) Log::addMsg("*glop*");
  }
}

FeatureLiquidDeep::FeatureLiquidDeep(FeatureId id, Pos pos) :
  FeatureStatic(id, pos) {}

void FeatureLiquidDeep::bump(Actor& actorBumping) {
  (void)actorBumping;
}

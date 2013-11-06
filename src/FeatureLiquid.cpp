#include "FeatureLiquid.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "PlayerBonuses.h"

FeatureLiquidShallow::FeatureLiquidShallow(
  Feature_t id, Pos pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {}

void FeatureLiquidShallow::bump(Actor* actorBumping) {
  if(actorBumping->getData()->bodyType == actorBodyType_normal) {

    actorBumping->getPropHandler()->tryApplyProp(
      new PropWaiting(eng, propTurnsStandard));

//    const bool IS_PLAYER = actorBumping == eng->player;
//    if(IS_PLAYER) {
//      eng->log->addMsg("*glop*");
//      eng->renderer->updateScreen();
//    }
  }
}

FeatureLiquidDeep::FeatureLiquidDeep(Feature_t id, Pos pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {}

void FeatureLiquidDeep::bump(Actor* actorBumping) {
  (void)actorBumping;
}

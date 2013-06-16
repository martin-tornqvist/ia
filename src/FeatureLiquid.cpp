#include "FeatureLiquid.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Render.h"
#include "PlayerBonuses.h"

FeatureLiquidShallow::FeatureLiquidShallow(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {

}

void FeatureLiquidShallow::bump(Actor* actorBumping) {
  if(actorBumping->getDef()->moveType == moveType_walk) {

    actorBumping->getStatusEffectsHandler()->tryAddEffect(new StatusWaiting(eng));

//    const bool IS_PLAYER = actorBumping == eng->player;
//    if(IS_PLAYER) {
//      eng->log->addMessage("*glop*");
//      eng->renderer->updateScreen();
//    }
  }
}

FeatureLiquidDeep::FeatureLiquidDeep(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {

}

void FeatureLiquidDeep::bump(Actor* actorBumping) {
  (void)actorBumping;
}

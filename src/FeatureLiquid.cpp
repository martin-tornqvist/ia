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

    const bool IS_PLAYER = actorBumping == eng->player;
    const bool PLAYER_IS_QUICK = eng->playerBonusHandler->isBonusPicked(playerBonus_quick);

    if(IS_PLAYER == false || PLAYER_IS_QUICK == false) {
      actorBumping->getStatusEffectsHandler()->attemptAddEffect(new StatusWaiting(eng));
      if(IS_PLAYER) {
        if(actorBumping->getStatusEffectsHandler()->hasEffect(statusWaiting)) {
          eng->log->addMessage("*glop*");
          eng->renderer->flip();
        }
      }
    }
  }
}

FeatureLiquidDeep::FeatureLiquidDeep(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {

}

void FeatureLiquidDeep::bump(Actor* actorBumping) {
  (void)actorBumping;
}

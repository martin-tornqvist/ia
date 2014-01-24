#include "FeatureGrave.h"

#include "Engine.h"
#include "Log.h"
#include "ActorPlayer.h"

Grave::Grave(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine) {}

string Grave::getDescr(const bool DEFINITE_ARTICLE) const {
  return (DEFINITE_ARTICLE ?
          data_->name_the :
          data_->name_a) + "; " + inscription_;
}

void Grave::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {
    eng.log->addMsg(inscription_);
  }
}

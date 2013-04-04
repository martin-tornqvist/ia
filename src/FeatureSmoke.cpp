#include "FeatureSmoke.h"

#include "Engine.h"

void Smoke::newTurn() {
  if(isPermanent_ == false) {
    life_--;
    if(life_ <= 0) {
      eng->gameTime->eraseFeatureMob(this, true);
    }
  }
}

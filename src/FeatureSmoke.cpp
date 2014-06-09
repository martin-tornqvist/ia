#include "FeatureSmoke.h"

void Smoke::newTurn() {
  if(!isPermanent_) {
    life_--;
    if(life_ <= 0) {GameTime::eraseFeatureMob(this, true);}
  }
}

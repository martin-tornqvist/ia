#include "FeatureLever.h"

#include "FeatureDoor.h"
#include "FeatureFactory.h"
#include "Engine.h"
#include "Render.h"
#include "ActorPlayer.h"

FeatureLever::FeatureLever(Feature_t id, Pos pos, Engine* engine, LeverSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), isPositionLeft_(true), doorLinkedTo_(spawnData->doorLinkedTo_)  {
}

SDL_Color FeatureLever::getColor() const {
  return isPositionLeft_ ? clrGray : clrWhite;
}

Tile_t FeatureLever::getTile() const {
  return isPositionLeft_ ? tile_leverLeft : tile_leverRight;
}

void FeatureLever::examine() {
  pull();
}

void FeatureLever::pull() {
  trace << "FeatureLever::pull()..." << endl;
  isPositionLeft_ = !isPositionLeft_;

  if(doorLinkedTo_->isBroken_ == false) {
    trace << "FeatureLever: Door linked to is not broken" << endl;
    if(doorLinkedTo_->isOpen_ == false) {
      doorLinkedTo_->reveal(true);
    }
    doorLinkedTo_->isOpen_ = !doorLinkedTo_->isOpen_;
    doorLinkedTo_->isStuck_ = false;
  }
  eng->player->updateFov();
  eng->renderer->drawMapAndInterface();
  trace << "FeatureLever::pull() [DONE]" << endl;
}


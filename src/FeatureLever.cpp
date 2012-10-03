#include "FeatureLever.h"

#include "FeatureDoor.h"
#include "FeatureFactory.h"
#include "Engine.h"
#include "Render.h"
#include "ActorPlayer.h"

FeatureLever::FeatureLever(Feature_t id, coord pos, Engine* engine, LeverSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), isPositionLeft_(true), doorLinkedTo_(spawnData->doorLinkedTo_)  {

}

SDL_Color FeatureLever::getColor() const {
  return isPositionLeft_ ? clrGray : clrWhite;
}

Tile_t FeatureLever::getTile() const {
  return isPositionLeft_ ? tile_lever_left : tile_lever_right;
}

void FeatureLever::examine() {
  pull();
}

void FeatureLever::pull() {
  isPositionLeft_ = !isPositionLeft_;

  if(doorLinkedTo_->isBroken_ == false) {
    if(doorLinkedTo_->isOpen_ == false) {
      doorLinkedTo_->isOpen_ = true;
      doorLinkedTo_->isStuck_ = false;
      doorLinkedTo_->reveal(true);
    } else {
      doorLinkedTo_->isOpen_ = false;
    }
  }
  eng->player->FOVupdate();
  eng->renderer->drawMapAndInterface(true);
}

#include "FeatureLever.h"

#include "Init.h"
#include "FeatureDoor.h"
#include "FeatureFactory.h"
#include "Renderer.h"
#include "ActorPlayer.h"
#include "Map.h"

using namespace std;

FeatureLever::FeatureLever(FeatureId id, Pos pos, LeverSpawnData* spawnData) :
  FeatureStatic(id, pos),
  isPositionLeft_(true),
  doorLinkedTo_(spawnData->doorLinkedTo_)  {}

SDL_Color FeatureLever::getClr() const {
  return isPositionLeft_ ? clrGray : clrWhite;
}

TileId FeatureLever::getTile() const {
  return isPositionLeft_ ? TileId::leverLeft : TileId::leverRight;
}

void FeatureLever::examine() {
  pull();
}

void FeatureLever::pull() {
  trace << "FeatureLever::pull()..." << endl;
  isPositionLeft_ = !isPositionLeft_;

  //TODO Implement something like openByLever in the Door class
  //Others should not poke around in the doors internal variables

//  if(doorLinkedTo_->isBroken_ == false) {
//    trace << "FeatureLever: Door linked to is not broken" << endl;
//    if(doorLinkedTo_->isOpen_ == false) {doorLinkedTo_->reveal(true);}
//    doorLinkedTo_->isOpen_  = !doorLinkedTo_->isOpen_;
//    doorLinkedTo_->isStuck_ = false;
//  }
  Map::player->updateFov();
  Renderer::drawMapAndInterface();
  trace << "FeatureLever::pull() [DONE]" << endl;
}


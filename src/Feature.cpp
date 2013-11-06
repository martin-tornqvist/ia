#include "Feature.h"

#include "Engine.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"

Feature::Feature(Feature_t id, Pos pos, Engine* engine,
                 FeatureSpawnData* spawnData) :
  pos_(pos), eng(engine), data_(eng->featureDataHandler->getData(id)),
  hasBlood_(false) {
  (void)spawnData;
}

void Feature::bump(Actor* actorBumping) {
  if(isMovePassable(actorBumping) == false) {
    if(actorBumping == eng->player) {
      if(eng->player->getPropHandler()->allowSee()) {
        eng->log->addMsg(data_->messageOnPlayerBlocked);
      } else {
        eng->log->addMsg(data_->messageOnPlayerBlockedBlind);
      }
    }
  }
}

void Feature::addLight(bool light[MAP_X_CELLS][MAP_Y_CELLS]) const {
  (void)light;
}

void Feature::newTurn() {

}

bool Feature::isMovePassable(Actor* const actorMoving) const {
  return data_->isBodyTypePassable[actorMoving->getBodyType()];
}

bool Feature::isBodyTypePassable(const ActorBodyType_t bodyType) const {
  return data_->isBodyTypePassable[bodyType];
}

bool Feature::isVisionPassable() const {
  return data_->isVisionPassable;
}

bool Feature::isShootPassable() const {
  return data_->isShootPassable;
}

bool Feature::isSmokePassable() const {
  return data_->isSmokePassable;
}

bool Feature::isBottomless() const {
  return data_->isBottomless;
}

string Feature::getDescription(const bool DEFINITE_ARTICLE) const {
  return DEFINITE_ARTICLE ? data_->name_the : data_->name_a;
}

void Feature::hit(const int DMG, const DmgTypes_t dmgType) {
  (void)DMG;
  (void)dmgType;
}

SDL_Color Feature::getColor() const {
  return data_->color;
}

SDL_Color Feature::getColorBg() const {
  return data_->colorBg;
}

char Feature::getGlyph() const {
  return data_->glyph;
}

Tile_t Feature::getTile() const {
  return data_->tile;
}

bool Feature::canHaveCorpse() const {
  return data_->canHaveCorpse;
}

bool Feature::canHaveStaticFeature() const {
  return data_->canHaveStaticFeature;
}

bool Feature::canHaveBlood() const {
  return data_->canHaveBlood;
}

bool Feature::canHaveGore() const {
  return data_->canHaveGore;
}

bool Feature::canHaveItem() const {
  return data_->canHaveItem;
}

bool Feature::hasBlood() {
  return hasBlood_;
}

void Feature::setHasBlood(const bool HAS_BLOOD) {
  hasBlood_ = HAS_BLOOD;
}

Feature_t Feature::getId() const {
  return data_->id;
}

int Feature::getDodgeModifier() const {
  return data_->dodgeModifier;
}

int Feature::getShockWhenAdjacent() const {
  return data_->shockWhenAdjacent;
}

MaterialType_t Feature::getMaterialType() const {
  return data_->materialType;
}

void Feature::examine() {
  eng->log->addMsg("I find nothing specific there to examine or use.");
}

void FeatureStatic::setGoreIfPossible() {
  if(data_->canHaveGore) {
    const int ROLL_GLYPH = eng->dice(1, 4);
    switch(ROLL_GLYPH) {
      case 1: {
        goreGlyph_ = ',';
      }
      break;
      case 2: {
        goreGlyph_ = '`';
      }
      break;
      case 3: {
        goreGlyph_ = 39;
      }
      break;
      case 4: {
        goreGlyph_ = ';';
      }
      break;
    }

    const int ROLL_TILE = eng->dice(1, 8);
    switch(ROLL_TILE) {
      case 1: {
        goreTile_ = tile_gore1;
      }
      break;
      case 2: {
        goreTile_ = tile_gore2;
      }
      break;
      case 3: {
        goreTile_ = tile_gore3;
      }
      break;
      case 4: {
        goreTile_ = tile_gore4;
      }
      break;
      case 5: {
        goreTile_ = tile_gore5;
      }
      break;
      case 6: {
        goreTile_ = tile_gore6;
      }
      break;
      case 7: {
        goreTile_ = tile_gore7;
      }
      break;
      case 8: {
        goreTile_ = tile_gore8;
      }
      break;
    }
  }
}

string FeatureStatic::getDescription(const bool DEFINITE_ARTICLE) const {
  if(goreGlyph_ == ' ') {
    return DEFINITE_ARTICLE ? data_->name_the : data_->name_a;
  } else {
    return DEFINITE_ARTICLE ? "the blood and gore" : "blood and gore";
  }
}

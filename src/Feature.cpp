#include "Feature.h"

#include "Engine.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "GameTime.h"

//---------------------------------------------------------- FEATURE
Feature::Feature(Feature_t id, Pos pos, Engine& engine,
                 FeatureSpawnData* spawnData) :
  pos_(pos), eng(engine), data_(eng.featureDataHandler->getData(id)),
  hasBlood_(false) {
  (void)spawnData;
}

void Feature::bump(Actor& actorBumping) {
  if(isBodyTypePassable(actorBumping.getBodyType()) == false) {
    if(&actorBumping == eng.player) {
      if(eng.player->getPropHandler().allowSee()) {
        eng.log->addMsg(data_->messageOnPlayerBlocked);
      } else {
        eng.log->addMsg(data_->messageOnPlayerBlockedBlind);
      }
    }
  }
}

void Feature::addLight(bool light[MAP_W][MAP_H]) const {
  (void)light;
}

void Feature::newTurn() {

}

bool Feature::isBodyTypePassable(const BodyType_t bodyType) const {
  return data_->isBodyTypePassable[bodyType];
}

bool Feature::isVisionPassable() const {
  return data_->isVisionPassable;
}

bool Feature::isProjectilesPassable() const {
  return data_->isProjectilesPassable;
}

bool Feature::isSmokePassable() const {
  return data_->isSmokePassable;
}

bool Feature::isBottomless() const {
  return data_->isBottomless;
}

string Feature::getDescr(const bool DEFINITE_ARTICLE) const {
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

bool Feature::hasBlood() const {
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
  eng.log->addMsg("I find nothing specific there to examine or use.");
}

//---------------------------------------------------------- STATIC FEATURE
void FeatureStatic::tryBash(Actor& actorTrying) {

  const bool IS_PLAYER = &actorTrying == eng.player;

  string sndMsg = "";

  if(IS_PLAYER) {
    const bool IS_BLIND    = eng.player->getPropHandler().allowSee() == false;
    const bool IS_BLOCKING = isBodyTypePassable(bodyType_normal) == false;
    if(IS_BLOCKING) {
      eng.log->addMsg(
        "I smash into " + (IS_BLIND ? " something" : getDescr(false)) + "!");

      if(eng.dice.oneIn(4)) {
        eng.log->addMsg("I sprain myself.", clrMsgBad);
        const int SPRAIN_DMG = eng.dice.range(1, 5);
        actorTrying.hit(SPRAIN_DMG, dmgType_pure, false);
      }

      if(eng.dice.oneIn(4)) {
        eng.log->addMsg("I am off-balance.");

        actorTrying.getPropHandler().tryApplyProp(
          new PropParalyzed(eng, propTurnsSpecified, 2));
      }

    } else {
      eng.log->addMsg("I kick the air!");
    }
  } else {
    bool blockers[MAP_W][MAP_H];
    MapParser::parse(CellPredBlocksVision(eng), blockers);
    const bool PLAYER_SEE_TRYER =
      eng.player->checkIfSeeActor(actorTrying, blockers);

    if(PLAYER_SEE_TRYER) {
      eng.log->addMsg(actorTrying.getNameThe() + " bashes at a door!");
    }

    sndMsg = "I hear a loud banging on a door.";
  }

  //The sound emits from the actor instead of the bashed object, and the
  //parameter to ignore the message if source is seen is enabled.
  //This is because the player shoudl receive the ound message even if the
  //bashed object is seen - but never if the bashing actor is seen
  Sound snd(sndMsg, sfxDoorBang, true, actorTrying.pos, false, IS_PLAYER);
  eng.soundEmitter->emitSound(snd);

  tryBash_(actorTrying);

  eng.gameTime->actorDidAct();

  eng.player->updateFov();
  eng.renderer->drawMapAndInterface();
}

void FeatureStatic::playerTryDisarm() {
  eng.log->addMsg("I find nothing there to disarm.");
}

void FeatureStatic::setGoreIfPossible() {
  if(data_->canHaveGore) {
    const int ROLL_GLYPH = eng.dice(1, 4);
    switch(ROLL_GLYPH) {
      case 1: {
        goreGlyph_ = ',';
      } break;

      case 2: {
        goreGlyph_ = '`';
      } break;

      case 3: {
        goreGlyph_ = 39;
      } break;

      case 4: {
        goreGlyph_ = ';';
      } break;
    }

    const int ROLL_TILE = eng.dice(1, 8);
    switch(ROLL_TILE) {
      case 1: {
        goreTile_ = tile_gore1;
      } break;

      case 2: {
        goreTile_ = tile_gore2;
      } break;

      case 3: {
        goreTile_ = tile_gore3;
      } break;

      case 4: {
        goreTile_ = tile_gore4;
      } break;

      case 5: {
        goreTile_ = tile_gore5;
      } break;

      case 6: {
        goreTile_ = tile_gore6;
      } break;

      case 7: {
        goreTile_ = tile_gore7;
      } break;

      case 8: {
        goreTile_ = tile_gore8;
      } break;
    }
  }
}

string FeatureStatic::getDescr(const bool DEFINITE_ARTICLE) const {
  if(goreGlyph_ == ' ') {
    return DEFINITE_ARTICLE ? data_->name_the : data_->name_a;
  } else {
    return DEFINITE_ARTICLE ? "the blood and gore" : "blood and gore";
  }
}

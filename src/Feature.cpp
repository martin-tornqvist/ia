#include "Feature.h"

#include "Engine.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "GameTime.h"
#include "DungeonClimb.h"
#include "Query.h"
#include "SaveHandler.h"
#include "Popup.h"

//---------------------------------------------------------- FEATURE
Feature::Feature(FeatureId id, Pos pos, Engine& engine,
                 FeatureSpawnData* spawnData) :
  pos_(pos), eng(engine), data_(eng.featureDataHandler->getData(id)),
  hasBlood_(false) {
  (void)spawnData;
}

void Feature::bump(Actor& actorBumping) {
  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(canMove(props) == false) {
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

bool Feature::canMoveCmn() const {
  return data_->moveRules.canMoveCmn();
}

bool Feature::canMove(const vector<PropId>& actorsProps) const {
  return data_->moveRules.canMove(actorsProps);
}

bool Feature::isSoundPassable() const {
  return data_->isSoundPassable;
}

bool Feature::isVisionPassable() const {
  return data_->isVisionPassable;
}

bool Feature::isProjectilePassable() const {
  return data_->isProjectilePassable;
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

void Feature::hit(const int DMG, const DmgTypes dmgType) {
  (void)DMG;
  (void)dmgType;
}

SDL_Color Feature::getClr() const {
  return data_->color;
}

SDL_Color Feature::getClrBg() const {
  return data_->colorBg;
}

char Feature::getGlyph() const {
  return data_->glyph;
}

TileId Feature::getTile() const {
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

FeatureId Feature::getId() const {
  return data_->id;
}

int Feature::getDodgeModifier() const {
  return data_->dodgeModifier;
}

int Feature::getShockWhenAdjacent() const {
  return data_->shockWhenAdjacent;
}

MaterialType Feature::getMaterialType() const {
  return data_->materialType;
}

//---------------------------------------------------------- STATIC FEATURE
void FeatureStatic::examine() {
  eng.log->addMsg("I find nothing specific there to examine or use.");
}

void FeatureStatic::disarm() {
  eng.log->addMsg("I find nothing there to disarm.");
}

void FeatureStatic::bash(Actor& actorTrying) {
  if(&actorTrying == eng.player) {
    const bool IS_BLIND    = eng.player->getPropHandler().allowSee() == false;
    const bool IS_BLOCKING = canMoveCmn() == false && getId() != feature_stairs;
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
  }

  bash_(actorTrying);

  eng.gameTime->actorDidAct();

  eng.player->updateFov();
  eng.renderer->drawMapAndInterface();
}

void FeatureStatic::bash_(Actor& actorTrying) {
  //Emitting the sound from the actor instead of the bashed object, because the
  //sound massage should be received even if the object is seen
  const AlertsMonsters alertsMonsters = &actorTrying == eng.player ?
                                        AlertsMonsters::yes :
                                        AlertsMonsters::no;
  Snd snd("", endOfSfxId, IgnoreMsgIfOriginSeen::yes, actorTrying.pos,
          &actorTrying, SndVol::low, alertsMonsters);
  eng.sndEmitter->emitSnd(snd);
}

void FeatureStatic::setGoreIfPossible() {
  if(data_->canHaveGore) {
    const int ROLL_GLYPH = eng.dice(1, 4);
    switch(ROLL_GLYPH) {
      case 1: {goreGlyph_ = ',';} break;
      case 2: {goreGlyph_ = '`';} break;
      case 3: {goreGlyph_ = 39;}  break;
      case 4: {goreGlyph_ = ';';} break;
    }

    const int ROLL_TILE = eng.dice(1, 8);
    switch(ROLL_TILE) {
      case 1: {goreTile_ = tile_gore1;} break;
      case 2: {goreTile_ = tile_gore2;} break;
      case 3: {goreTile_ = tile_gore3;} break;
      case 4: {goreTile_ = tile_gore4;} break;
      case 5: {goreTile_ = tile_gore5;} break;
      case 6: {goreTile_ = tile_gore6;} break;
      case 7: {goreTile_ = tile_gore7;} break;
      case 8: {goreTile_ = tile_gore8;} break;
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

//---------------------------------------------------------- GRAVE
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

//---------------------------------------------------------- STAIRS
void Stairs::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {

    const vector<string> choices {"Descend", "Save and quit", "Cancel"};
    const int CHOICE = eng.popup->showMenuMsg("", true, choices,
                       "A staircase leading downwards");

    if(CHOICE == 0) {
      eng.player->pos = pos_;
      trace << "Stairs: Calling DungeonClimb::tryUseDownStairs()" << endl;
      eng.dungeonClimb->tryUseDownStairs();
    } else if(CHOICE == 1) {
      eng.player->pos = pos_;
      eng.saveHandler->save();
      eng.quitToMainMenu_ = true;
    } else {
      eng.log->clearLog();
      eng.renderer->drawMapAndInterface();
    }
  }
}

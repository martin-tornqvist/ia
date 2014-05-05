#include "Feature.h"

#include "Init.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "GameTime.h"
#include "DungeonClimb.h"
#include "Query.h"
#include "SaveHandling.h"
#include "Popup.h"
#include "Utils.h"
#include "Map.h"

using namespace std;

//---------------------------------------------------------- FEATURE
Feature::Feature(FeatureId id, Pos pos,
                 FeatureSpawnData* spawnData) :
  pos_(pos), data_(FeatureData::getData(id)),
  hasBlood_(false) {
  (void)spawnData;
}

void Feature::bump(Actor& actorBumping) {
  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(canMove(props) == false) {
    if(&actorBumping == Map::player) {
      if(Map::player->getPropHandler().allowSee()) {
        Log::addMsg(data_->messageOnPlayerBlocked);
      } else {
        Log::addMsg(data_->messageOnPlayerBlockedBlind);
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

void Feature::hit(const int DMG, const DmgType dmgType) {
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
  Log::addMsg("I find nothing specific there to examine or use.");
}

void FeatureStatic::disarm() {
  Log::addMsg(msgDisarmNoTrap);
  Renderer::drawMapAndInterface();
}

void FeatureStatic::bash(Actor& actorTrying) {
  if(&actorTrying == Map::player) {
    const bool IS_BLIND    = Map::player->getPropHandler().allowSee() == false;
    const bool IS_BLOCKING = canMoveCmn() == false && getId() != FeatureId::stairs;
    if(IS_BLOCKING) {
      Log::addMsg(
        "I smash into " + (IS_BLIND ? " something" : getDescr(false)) + "!");

      if(Rnd::oneIn(4)) {
        Log::addMsg("I sprain myself.", clrMsgBad);
        const int SPRAIN_DMG = Rnd::range(1, 5);
        actorTrying.hit(SPRAIN_DMG, DmgType::pure, false);
      }

      if(Rnd::oneIn(4)) {
        Log::addMsg("I am off-balance.");

        actorTrying.getPropHandler().tryApplyProp(
          new PropParalyzed(propTurnsSpecific, 2));
      }

    } else {
      Log::addMsg("I kick the air!");
      Audio::play(SfxId::missMedium);
    }
  }

  bash_(actorTrying);

  GameTime::actorDidAct();

  Map::player->updateFov();
  Renderer::drawMapAndInterface();
}

void FeatureStatic::bash_(Actor& actorTrying) {
  //Emitting the sound from the actor instead of the bashed object, because the
  //sound massage should be received even if the object is seen
  const AlertsMonsters alertsMonsters = &actorTrying == Map::player ?
                                        AlertsMonsters::yes :
                                        AlertsMonsters::no;
  Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, actorTrying.pos,
          &actorTrying, SndVol::low, alertsMonsters);
  SndEmit::emitSnd(snd);
}

void FeatureStatic::setGoreIfPossible() {
  if(data_->canHaveGore) {
    const int ROLL_GLYPH = Rnd::dice(1, 4);
    switch(ROLL_GLYPH) {
      case 1: {goreGlyph_ = ',';} break;
      case 2: {goreGlyph_ = '`';} break;
      case 3: {goreGlyph_ = 39;}  break;
      case 4: {goreGlyph_ = ';';} break;
    }

    const int ROLL_TILE = Rnd::dice(1, 8);
    switch(ROLL_TILE) {
      case 1: {goreTile_ = TileId::gore1;} break;
      case 2: {goreTile_ = TileId::gore2;} break;
      case 3: {goreTile_ = TileId::gore3;} break;
      case 4: {goreTile_ = TileId::gore4;} break;
      case 5: {goreTile_ = TileId::gore5;} break;
      case 6: {goreTile_ = TileId::gore6;} break;
      case 7: {goreTile_ = TileId::gore7;} break;
      case 8: {goreTile_ = TileId::gore8;} break;
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
  if(&actorBumping == Map::player) {
    Log::addMsg(inscription_);
  }
}

//---------------------------------------------------------- STAIRS
void Stairs::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {

    const vector<string> choices {"Descend", "Save and quit", "Cancel"};
    const int CHOICE = Popup::showMenuMsg("", true, choices,
                                          "A staircase leading downwards");

    if(CHOICE == 0) {
      Map::player->pos = pos_;
      trace << "Stairs: Calling DungeonClimb::tryUseDownStairs()" << endl;
      DungeonClimb::tryUseDownStairs();
    } else if(CHOICE == 1) {
      Map::player->pos = pos_;
      SaveHandling::save();
      Init::quitToMainMenu = true;
    } else {
      Log::clearLog();
      Renderer::drawMapAndInterface();
    }
  }
}

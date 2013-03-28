#include "Feature.h"

#include "Engine.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Render.h"

Feature::Feature(Feature_t id, coord pos, Engine* engine, FeatureSpawnData* spawnData) :
	pos_(pos), eng(engine), def_(eng->featureData->getFeatureDef(id)), hasBlood_(false) {
	(void)spawnData;
}

void Feature::bump(Actor* actorBumping) {
	if(isMovePassable(actorBumping) == false) {
		if(actorBumping == eng->player) {
			if(eng->player->getStatusEffectsHandler()->allowSee()) {
				eng->log->addMessage(def_->messageOnPlayerBlocked);
			} else {
				eng->log->addMessage(def_->messageOnPlayerBlockedBlind);
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
	return def_->isMovePassable[actorMoving->getMoveType()];
}

bool Feature::isMoveTypePassable(const MoveType_t moveType) const {
	return def_->isMovePassable[moveType];
}

bool Feature::isVisionPassable() const {
	return def_->isVisionPassable;
}

bool Feature::isShootPassable() const {
	return def_->isShootPassable;
}

bool Feature::isSmokePassable() const {
	return def_->isSmokePassable;
}

bool Feature::isBottomless() const {
	return def_->isBottomless;
}

string Feature::getDescription(const bool DEFINITE_ARTICLE) const {
	return DEFINITE_ARTICLE ? def_->name_the : def_->name_a;
}

void Feature::hit(const int DAMAGE, const DamageTypes_t damageType) {
	(void)DAMAGE;
	(void)damageType;
}

sf::Color Feature::getColor() const {
	return def_->color;
}

sf::Color Feature::getColorBg() const {
	return def_->colorBg;
}

char Feature::getGlyph() const {
	return def_->glyph;
}

Tile_t Feature::getTile() const {
	return def_->tile;
}

bool Feature::canHaveCorpse() const {
	return def_->canHaveCorpse;
}

bool Feature::canHaveStaticFeature() const {
	return def_->canHaveStaticFeature;
}

bool Feature::canHaveBlood() const {
	return def_->canHaveBlood;
}

bool Feature::canHaveGore() const {
	return def_->canHaveGore;
}

bool Feature::canHaveItem() const {
	return def_->canHaveItem;
}

bool Feature::hasBlood() {
	return hasBlood_;
}

void Feature::setHasBlood(const bool HAS_BLOOD) {
	hasBlood_ = HAS_BLOOD;
}

Feature_t Feature::getId() const {
	return def_->id;
}

int Feature::getDodgeModifier() const {
	return def_->dodgeModifier;
}

int Feature::getShockWhenAdjacent() const {
    return def_->shockWhenAdjacent;
}

MaterialType_t Feature::getMaterialType() const {
   return def_->materialType;
}

void Feature::examine() {
   eng->log->addMessage("I find nothing specific there to examine or use.");
}

void FeatureStatic::setGoreIfPossible() {
	if(def_->canHaveGore) {
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
		return DEFINITE_ARTICLE ? def_->name_the : def_->name_a;
	} else {
		return DEFINITE_ARTICLE ? "the blood and gore" : "blood and gore";
	}
}

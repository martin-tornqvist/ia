#include "FeatureTrap.h"

#include <cassert>

#include "Engine.h"

#include "FeatureFactory.h"
#include "FeatureData.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ItemDrop.h"
#include "Postmortem.h"
#include "Explosion.h"
#include "Popup.h"

Trap::Trap(Feature_t id, coord pos, Engine* engine, TrapSpawnData* spawnData) :
	FeatureStatic(id, pos, engine), mimicFeature_(spawnData->mimicFeature_), isHidden_(true) {

	assert(spawnData->trapType_ != endOfTraps);

	if(spawnData->trapType_ == trap_any) {
		const Trap_t ROLL = static_cast<Trap_t>(eng->dice(1, endOfTraps) - 1);
		setSpecificTrapFromId(ROLL);
	} else {
		setSpecificTrapFromId(spawnData->trapType_);
	}
	assert(specificTrap_ != NULL);
	assert(mimicFeature_ != NULL);
}

void Trap::setSpecificTrapFromId(const Trap_t id) {
	switch(id) {
	case trap_dart:
		specificTrap_ = new TrapDart(pos_, eng);
		break;
	case trap_spear:
		specificTrap_ = new TrapSpear(pos_, eng);
		break;
	case trap_gasConfusion:
		specificTrap_ = new TrapGasConfusion(pos_, eng);
		break;
	case trap_gasParalyze:
		specificTrap_ = new TrapGasParalyzation(pos_, eng);
		break;
	case trap_gasFear:
		specificTrap_ = new TrapGasFear(pos_, eng);
		break;
	case trap_blinding:
		specificTrap_ = new TrapBlindingFlash(pos_, eng);
		break;
	case trap_teleport:
		specificTrap_ = new TrapTeleport(pos_, eng);
		break;
	case trap_smoke:
		specificTrap_ = new TrapSmoke(pos_, eng);
		break;
	case trap_spiderWeb:
		specificTrap_ = new TrapSpiderWeb(pos_, eng);
		break;
	default: {
		specificTrap_ = NULL;
	}
	break;
	}
}

void Trap::triggerOnPurpose(Actor* actorTriggering) {
	const AbilityRollResult_t DODGE_RESULT = failSmall;
	specificTrap_->trapSpecificTrigger(actorTriggering, DODGE_RESULT);
}

void Trap::bump(Actor* actorBumping) {
	const bool IS_PLAYER = actorBumping == actorBumping->eng->player;
	const bool CAN_SEE = actorBumping->getStatusEffectsHandler()->allowSee();
	const int DODGE_SKILL_VALUE = actorBumping->getInstanceDefinition()->abilityValues.getAbilityValue(ability_dodge, true);

	if(IS_PLAYER == false) {
	} else {
		int CHANCE_TO_AVOID = isHidden_ == true ? 10 : (DODGE_SKILL_VALUE + 40);
		const AbilityRollResult_t result = actorBumping->eng->abilityRoll->roll(CHANCE_TO_AVOID);

		const string name = specificTrap_->getTrapSpecificTitle();

		if(result >= successSmall) {
			if(isHidden_ == false) {
				if(IS_PLAYER) {
					if(CAN_SEE) {
						actorBumping->eng->log->addMessage("I avoid a " + name + ".", clrMessageGood);
					}
				}
			}
		} else {
			const AbilityRollResult_t DODGE_RESULT = actorBumping->eng->abilityRoll->roll(DODGE_SKILL_VALUE);
			reveal(false);
			specificTrap_->trapSpecificTrigger(actorBumping, DODGE_RESULT);
		}
	}
}

void Trap::reveal(const bool PRINT_MESSSAGE) {
	isHidden_ = false;
	clearGore();

	Item* item = eng->map->items[pos_.x][pos_.y];
	if(item != NULL) {
		eng->map->items[pos_.x][pos_.y] = NULL;
		eng->itemDrop->dropItemOnMap(pos_, &item);
	}

	if(eng->map->playerVision[pos_.x][pos_.y] == true) {
		eng->renderer->drawMapAndInterface();

		if(PRINT_MESSSAGE) {
			const string name = specificTrap_->getTrapSpecificTitle();
			eng->log->addMessage("I spot a " + name + ".");
		}
	}
}

void Trap::playerTrySpotHidden() {
	if(isHidden_) {
		if(eng->mapTests->isCellsNeighbours(pos_, eng->player->pos, false)) {
			const Abilities_t abilityUsed = ability_searching;
			const int PLAYER_SKILL = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(abilityUsed, true);

			if(eng->abilityRoll->roll(PLAYER_SKILL) >= successSmall) {
				reveal(true);
			}
		}
	}
}

string Trap::getDescription(const bool DEFINITE_ARTICLE) const {
	if(isHidden_) {
		return DEFINITE_ARTICLE == true ? mimicFeature_->name_the : mimicFeature_->name_a;
	} else {
		return "a " + specificTrap_->getTrapSpecificTitle();
	}
}

SDL_Color Trap::getColor() const {
   return isHidden_ ? mimicFeature_->color : specificTrap_->getTrapSpecificColor();
}

char Trap::getGlyph() const {
   return isHidden_ ? mimicFeature_->glyph : specificTrap_->getTrapSpecificGlyph();
}

Tile_t Trap::getTile() const {
   return isHidden_ ? mimicFeature_->tile : specificTrap_->getTrapSpecificTile();
}

bool Trap::canHaveCorpse() const {
	return isHidden_;
}

bool Trap::canHaveBlood() const {
	return isHidden_;
}

bool Trap::canHaveGore() const {
	return isHidden_;
}

bool Trap::canHaveItem() const {
	return isHidden_;
}

coord Trap::actorAttemptLeave(Actor* const actor, const coord& pos, const coord& dest) {
	assert(specificTrap_ != NULL);
	return specificTrap_->specificTrapActorAttemptLeave(actor, pos, dest);
}

MaterialType_t Trap::getMaterialType() const {
   return isHidden_ ? mimicFeature_->materialType : def_->materialType;
}

//============================================================= TRAP LIST

void TrapDart::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
	//Dodge?
	if(dodgeResult >= successSmall) {
		if(IS_PLAYER) {
			if(CAN_SEE) {
				eng->log->addMessage("I dodge a dart!", clrMessageGood);
			} else {
				eng->log->addMessage("I feel a mechanism trigger and quickly leap aside!", clrMessageGood);
			}
		}
	} else {
		//Trap misses?
		const int CHANCE_TO_HIT = 75;
		if(eng->dice(1, 100) > CHANCE_TO_HIT) {
			if(IS_PLAYER) {
				if(CAN_SEE) {
					eng->log->addMessage("A dart barely misses me!", clrMessageGood);
				} else {
					eng->log->addMessage("A mechanism triggers, I sense something fly by!", clrMessageGood);
				}
			}
		} else {
			//Dodge failed and trap hits
			if(IS_PLAYER) {
				if(CAN_SEE) {
					eng->log->addMessage("I am hit by a dart!", clrMessageBad, false);
				} else {
					eng->log->addMessage("A mechanism triggers, I feel a needle piercing my skin!", clrMessageBad, false);
				}
			}

			const int DMG = eng->dice(1, 8);
			const bool DIED = actor->hit(DMG, damageType_physical);
			if(DIED == true) {
				if(actor == eng->player) {
					eng->postmortem->setCauseOfDeath("Killed by a dart trap");
				}
			}
		}
	}
}

void TrapSpear::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
	//Dodge?
	if(dodgeResult >= successSmall) {
		if(IS_PLAYER) {
			if(CAN_SEE) {
				eng->log->addMessage("I dodge a spear!", clrMessageGood);
			} else {
				eng->log->addMessage("I feel a mechanism trigger and quickly leap aside!", clrMessageGood);
			}
		}
	} else {
		//Trap misses?
		const int CHANCE_TO_HIT = 75;
		if(eng->dice(1, 100) > CHANCE_TO_HIT) {
			if(IS_PLAYER) {
				if(CAN_SEE) {
					eng->log->addMessage("A spear barely misses me!", clrMessageGood);
				} else {
					eng->log->addMessage("A mechanism triggers, I hear a *swoosh*!", clrMessageGood);
				}
			}
		} else {
			//Dodge failed and trap hits
			if(IS_PLAYER) {
				if(CAN_SEE) {
					eng->log->addMessage("I am hit by a spear!", clrMessageBad, false);
				} else {
					eng->log->addMessage("A mechanism triggers, something sharp pierces my skin!", clrMessageBad, false);
				}
			}

			const int DMG = eng->dice(2, 6);
			const bool DIED = actor->hit(DMG, damageType_physical);
			if(DIED == true) {
				if(actor == eng->player) {
					eng->postmortem->setCauseOfDeath("Killed by a spear trap");
				}
			}
		}
	}
}

void TrapGasConfusion::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	(void)dodgeResult;

	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();

	if(IS_PLAYER) {
		if(CAN_SEE) {
			eng->log->addMessage("I am hit by a burst of gas!");
		} else {
			eng->log->addMessage("A mechanism triggers, I am hit by a burst of gas!");
		}
	}

	actor->eng->explosionMaker->runExplosion(pos_, false, new StatusConfused(eng->dice(3, 6)), true, getTrapSpecificColor());
}

void TrapGasParalyzation::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	(void)dodgeResult;

	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();

	if(IS_PLAYER) {
		if(CAN_SEE) {
			eng->log->addMessage("I am hit by a burst of gas!");
		} else {
			eng->log->addMessage("A mechanism triggers, I am hit by a burst of gas!");
		}
	}

	actor->eng->explosionMaker-> runExplosion(pos_, false, new StatusParalyzed(eng->dice(3, 6)), true, getTrapSpecificColor());
}

void TrapGasFear::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	(void)dodgeResult;

	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();

	if(IS_PLAYER) {
		if(CAN_SEE) {
			eng->log->addMessage("I am hit by a burst of gas!");
		} else {
			eng->log->addMessage("A mechanism triggers, I am hit by a burst of gas!");
		}
	}

	actor->eng->explosionMaker-> runExplosion(pos_, false, new StatusTerrified(eng->dice(3, 6)), true, getTrapSpecificColor());
}

void TrapBlindingFlash::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
	//Dodge?
	if(dodgeResult >= successSmall) {
		if(IS_PLAYER) {
			if(CAN_SEE) {
				eng->log->addMessage("I cover my eyes just in time to avoid an intense flash!", clrMessageGood);
			} else {
				eng->log->addMessage("I feel a mechanism trigger!", clrMessageGood);
			}
		}
	} else {
		//Dodge failed
		if(IS_PLAYER) {
			if(CAN_SEE) {
				eng->log->addMessage("A sharp flash of light pierces my eyes!", clrWhite, false);
				actor->getStatusEffectsHandler()->attemptAddEffect(new StatusBlind(eng->dice(3, 6)));
			} else {
				eng->log->addMessage("I feel a mechanism trigger!", clrWhite, false);
			}
		}
	}
}

void TrapTeleport::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	(void)dodgeResult;

	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();

	if(IS_PLAYER) {
		eng->player->FOVupdate();
		if(CAN_SEE) {
			eng->popup->showMessage("A curious shape on the floor starts to glow!");
		} else {
			eng->popup->showMessage("I feel a peculiar energy around me!");
		}
	}

	actor->teleportToRandom();
}

void TrapSmoke::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	(void)dodgeResult;

	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();

	if(IS_PLAYER) {
		if(CAN_SEE) {
			eng->log->addMessage("Suddenly the air is thick with smoke!");
		} else {
			eng->log->addMessage("A mechanism triggers, the air is thick with smoke!");
		}
	}

	actor->eng->explosionMaker->runSmokeExplosion(pos_);
}

void TrapSpiderWeb::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
	(void)dodgeResult;

	isHoldingActor = true;

	const bool IS_PLAYER = actor == eng->player;
	const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();

	if(IS_PLAYER) {
		if(CAN_SEE) {
			eng->log->addMessage("I find myself entangled in a huge spider web!");
		} else {
			eng->log->addMessage("I am entangled by a sticky mass of threads!");
		}
	}
}

coord TrapSpiderWeb::specificTrapActorAttemptLeave(Actor* const actor, const coord& pos, const coord& dest) {
	if(isHoldingActor == true) {
		const int ABILITY_VALUE = max(30, actor->getInstanceDefinition()->abilityValues.getAbilityValue(ability_resistStatusBody, true));

		const AbilityRollResult_t rollResult = eng->abilityRoll->roll(ABILITY_VALUE);
		if(rollResult >= successSmall) {

			isHoldingActor = false;

			eng->log->addMessage("I break free.");

			if(eng->dice(1, 100) <= 50) {

				if(actor->getStatusEffectsHandler()->allowSee() == true) {
					eng->log->addMessage("The web is destroyed.");
				}

				eng->featureFactory->spawnFeatureAt(feature_trashedSpiderWeb, pos_);
			}
			return dest;
		} else {
			eng->log->addMessage("I struggle to break free.");
			return pos;
		}
	}
	return dest;
}


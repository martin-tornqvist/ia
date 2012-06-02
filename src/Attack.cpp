#include "Attack.h"

#include <cassert>

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Log.h"

using namespace std;

void Attack::getAttackData(AttackData& data, const coord target, const coord currentPos, Weapon* const weapon, const bool IS_MELEE) {
	data.isMelee = IS_MELEE;
	data.attacker = eng->gameTime->getCurrentActor();
	data.currentDefender = eng->mapTests->getActorAtPos(currentPos);
	assert(data.currentDefender != NULL || IS_MELEE == false);

	if(data.currentDefender != NULL) {
		data.currentDefenderSize = data.currentDefender->getInstanceDefinition()->actorSize;
	} else {
		data.currentDefenderSize = actorSize_none;
	}

	Actor* const aimedActor = eng->mapTests->getActorAtPos(target);

	//Get intended aim level
	if(aimedActor != NULL) {
		//aimedActorSize = aimedActor->getInstanceDefinition()->actorSize;
		data.aimLevel = aimedActor->getInstanceDefinition()->actorSize;
	} else {
		bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
		eng->mapTests->makeShootBlockerFeaturesArray(blockers);
		data.aimLevel = blockers[target.x][target.y] ? actorSize_humanoid : actorSize_floor;
	}

	assert(data.aimLevel != actorSize_none);

	data.attackerX = data.attacker->pos.x;
	data.attackerY = data.attacker->pos.y;

	if(data.isMelee == true) {
		data.abilityUsed = weapon->getInstanceDefinition().meleeAbilityUsed;
		data.wpnBaseSkill = weapon->getInstanceDefinition().meleeBaseAttackSkill;
	} else {
		data.abilityUsed = weapon->getInstanceDefinition().rangedAbilityUsed;
		data.wpnBaseSkill = weapon->getInstanceDefinition().rangedBaseAttackSkill;
	}

	data.isIntrinsic = weapon->getInstanceDefinition().isIntrinsicWeapon;
	data.attackSkill = data.attacker->getInstanceDefinition()->abilityValues.getAbilityValue(data.abilityUsed, true);
	data.totalSkill = data.wpnBaseSkill + data.attackSkill;
	data.attackResult = eng->abilityRoll->roll(data.totalSkill);
	data.isPlayerAttacking = data.attacker == eng->player;

	//Dodge melee attack?
	data.isDefenderDodging = false;
	if(data.currentDefender != NULL && IS_MELEE == true) {
		if(data.currentDefender->getInstanceDefinition()->canDodge == true) {
			const int DEFENDER_DODGE_SKILL = data.currentDefender->getInstanceDefinition()->abilityValues.getAbilityValue(ability_dodge, true);

			const Engine* const eng = data.attacker->eng;

			const coord defenderPos = data.currentDefender->pos;
			const int DODGE_MOD_AT_FEATURE = eng->map->featuresStatic[defenderPos.x][defenderPos.y]->getDodgeModifier();

			const int TOTAL_DODGE = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

			if(TOTAL_DODGE > 0) {
				bool isDefenderAware = true;

				if(data.attacker == eng->player) {
					isDefenderAware = dynamic_cast<Monster*>(data.currentDefender)->playerAwarenessCounter > 0;
				} else {
					isDefenderAware = eng->player->checkIfSeeActor(*data.attacker, NULL);
				}

				if(isDefenderAware == true) {
					data.isDefenderDodging = eng->abilityRoll->roll(TOTAL_DODGE) >= successSmall;
				}
			}
		}
	}

	//Get weapon damage properties
	if(IS_MELEE == true) {
		data.dmgRolls = weapon->getInstanceDefinition().meleeDmg.rolls;
		data.dmgSides = weapon->getInstanceDefinition().meleeDmg.sides;
		data.dmgPlus = weapon->getInstanceDefinition().meleeDmg.plus;
	} else {
		data.dmgRolls = weapon->getInstanceDefinition().rangedDmg.rolls;
		data.dmgSides = weapon->getInstanceDefinition().rangedDmg.sides;
		data.dmgPlus = weapon->getInstanceDefinition().rangedDmg.plus;
	}

	if(data.attackResult == successCritical) {
		data.dmgRoll = data.dmgRolls * data.dmgSides;
		data.dmg = max(0, data.dmgRoll + data.dmgPlus);
	} else {
		data.dmgRoll = eng->dice(data.dmgRolls, data.dmgSides);
		data.dmg = max(0, data.dmgRoll + data.dmgPlus);
	}

	data.isSwiftAttack = false;
	data.isMightyAttack = false;
	data.isBackStab = false;


	if(IS_MELEE) {
		//Back-stab and/or bonus to attack skill from unaware defender?
		bool isDefenderAware = true;
		if(data.attacker == eng->player) {
			isDefenderAware = dynamic_cast<Monster*>(data.currentDefender)->playerAwarenessCounter > 0;
		} else {
			isDefenderAware = eng->player->checkIfSeeActor(*data.attacker, NULL);
		}

		if(isDefenderAware == false) {
			data.totalSkill += 50;
			data.attackResult = eng->abilityRoll->roll(data.totalSkill);
			if(data.attackResult == successCritical) {
				data.dmgRoll = data.dmgRolls * data.dmgSides;
				data.dmg = max(0, data.dmgRoll + data.dmgPlus);
			} else {
				data.dmgRoll = eng->dice(data.dmgRolls, data.dmgSides);
				data.dmg = max(0, data.dmgRoll + data.dmgPlus);
			}
			const int BACKSTAB_SKILL = data.attacker->getInstanceDefinition()->abilityValues.getAbilityValue(ability_backstabbing, true);
			const AbilityRollResult_t BACKSTAB_ROLL = eng->abilityRoll->roll(BACKSTAB_SKILL);
			if(BACKSTAB_ROLL >= successSmall) {
				data.dmgRoll = data.dmgRolls * data.dmgSides;
				data.dmg = (data.dmgRoll + data.dmgPlus) * 3 / 2;
				data.isBackStab = true;
			}
		}

		// Attack bonus from light or heavy melee weapon?
		// (can not happen if backstab, to simplify the rules)
		if(data.attacker == eng->player && data.isBackStab == false) {
			const int PLAYER_SKILL = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(ability_weaponHandling, true);
			const bool SUCCESS = eng->abilityRoll->roll(PLAYER_SKILL) >= successSmall;

			if(SUCCESS) {
				const ItemWeight_t realWeight = weapon->getInstanceDefinition().itemWeight;
				const ItemWeight_t effectiveWeight = realWeight == itemWeight_medium ?
                                     eng->dice.coinToss() ? itemWeight_light : itemWeight_heavy : realWeight;

				if(effectiveWeight == itemWeight_light) {
					data.isSwiftAttack = true;
				}
				if(effectiveWeight == itemWeight_heavy) {
					data.isMightyAttack = true;
					data.dmgRoll = data.dmgRolls * data.dmgSides;
					data.dmg = max(0, data.dmgRoll + data.dmgPlus);
				}
			}
		}
	}



	data.weaponName_a = weapon->getInstanceDefinition().name.name_a;
	data.verbPlayerAttacksMissile = weapon->getInstanceDefinition().rangedAttackMessages.player;
	data.verbOtherAttacksMissile = weapon->getInstanceDefinition().rangedAttackMessages.other;

	if(data.currentDefender != NULL) {
		data.isTargetEthereal = data.currentDefender->getStatusEffectsHandler()->isEthereal();
	} else {
		data.isTargetEthereal = false;
	}

	if(data.isTargetEthereal == true) {
		data.attackResult = failSmall;
	}
}

void Attack::printRangedInitiateMessages(AttackData data) {
	if(data.isPlayerAttacking == true)
		eng->log->addMessage("You " + data.verbPlayerAttacksMissile + ".");
	else {
		if(eng->map->playerVision[data.attackerX][data.attackerY] == true) {
			const string attackerName = data.attacker->getNameThe();
			const string attackVerb = data.verbOtherAttacksMissile;
			eng->log->addMessage(attackerName + " " + attackVerb + ".", clrWhite, false);
		}
	}

	eng->renderer->flip();
}

void Attack::printProjectileAtActorMessages(AttackData data, ProjectileHitType_t hitType) {
	//Only print messages if player can see the cell
	const int defX = data.currentDefender->pos.x;
	const int defY = data.currentDefender->pos.y;
	if(eng->map->playerVision[defX][defY] == true) {
		if(data.isTargetEthereal == true) {
			if(data.currentDefender == eng->player) {
				//Perhaps no text is needed here?
			} else {
				eng->log->addMessage("Projectile hits nothing but void.");
			}
		} else {
			//Punctuation or exclamation marks depending on attack strength
			data.dmgDescript = ".";
			if(data.dmgRolls* data.dmgSides >= 4) {
				if(data.dmgRoll > data.dmgRolls * data.dmgSides / 2)
					data.dmgDescript = "!";
				if(data.dmgRoll > data.dmgRolls * data.dmgSides * 5 / 6)
					data.dmgDescript = "!!!";
			}

			if(hitType == projectileHitType_cleanHit || hitType == projectileHitType_strayHit) {
				if(data.currentDefender == eng->player) {
					eng->log->addMessage("You are hit" + data.dmgDescript, clrMessageBad, false);

					if(data.attackResult == successCritical) {
						eng->log->addMessage("It was a great hit!", clrMessageBad, false);
					}
				} else {
					string otherName = "It";

					if(eng->map->playerVision[defX][defY] == true)
						otherName = data.currentDefender->getNameThe();

					eng->log->addMessage(otherName + " is hit" + data.dmgDescript, clrMessageGood);

					if(data.attackResult == successCritical) {
						eng->log->addMessage("It was a great hit!", clrMessageGood);
					}
				}
			}
		}
		eng->renderer->flip();
	}
}

void Attack::printMeleeMessages(AttackData data, Weapon* weapon) {
	string otherName;

	//Punctuation or exclamation marks depending on attack strength
	data.dmgDescript = ".";
	if(data.dmgRolls* data.dmgSides >= 4) {
		if(data.dmgRoll > data.dmgRolls * data.dmgSides / 2)
			data.dmgDescript = "!";
		if(data.dmgRoll > data.dmgRolls * data.dmgSides * 5 / 6)
			data.dmgDescript = "!!!";
	}

	if(data.isTargetEthereal == true) {
		if(data.isPlayerAttacking == true) {
			eng->log->addMessage("You hit nothing but void" + data.dmgDescript);
		} else {
			if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
				otherName = data.attacker->getNameThe();
			} else {
				otherName = "its";
			}

			eng->log->addMessage("You are unaffected by " + otherName + " attack" + data.dmgDescript);
		}
	} else {
		//----- ATTACK FUMBLE -----
		if(data.attackResult == failCritical) {
			if(data.isPlayerAttacking) {
				eng->log->addMessage("You fumble.");
			} else {
				if(eng->player->checkIfSeeActor(*data.attacker, NULL))
					otherName = data.attacker->getNameThe();
				else otherName = "It";

				eng->log->addMessage(otherName + " fumbles.");
			}
		}

		//----- ATTACK MISS -------
		if(data.attackResult > failCritical && data.attackResult <= failSmall) {
			if(data.isPlayerAttacking) {
				if(data.attackResult == failSmall)
					eng->log->addMessage("You barely miss" + data.dmgDescript);
				if(data.attackResult == failNormal)
					eng->log->addMessage("You miss" + data.dmgDescript);
				if(data.attackResult == failBig)
					eng->log->addMessage("You miss completely" + data.dmgDescript);
			} else {
				if(eng->player->checkIfSeeActor(*data.attacker, NULL))
					otherName = data.attacker->getNameThe();
				else otherName = "It";

				if(data.attackResult == failSmall)
					eng->log->addMessage(otherName + " barely misses you" + data.dmgDescript);
				if(data.attackResult == failNormal)
					eng->log->addMessage(otherName + " misses you" + data.dmgDescript);
				if(data.attackResult == failBig)
					eng->log->addMessage(otherName + " misses you completely" + data.dmgDescript);
			}
		}

		//----- ATTACK HIT -------- //----- ATTACK CRITICAL ---
		if(data.attackResult >= successSmall) {
			if(data.isDefenderDodging) {
				if(data.isPlayerAttacking) {
					if(eng->player->checkIfSeeActor(*data.currentDefender, NULL)) {
						otherName = data.currentDefender->getNameThe();
					} else {
						otherName = "It ";
					}
					eng->log->addMessage(otherName + " dodges your attack.");
				} else {
					if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
						otherName = data.attacker->getNameThe();
					} else {
						otherName = "It";
					}
					eng->log->addMessage("You dodge an attack from " + otherName + ".", clrMessageGood);
				}
			} else {
				if(data.isPlayerAttacking) {
					const string wpnVerb = weapon->getInstanceDefinition().meleeAttackMessages.player;

					if(eng->player->checkIfSeeActor(*data.currentDefender, NULL)) {
						otherName = data.currentDefender->getNameThe();
					} else {
						otherName = "it";
					}

					if(data.isIntrinsic) {
						eng->log->addMessage("You " + wpnVerb + " " + otherName + data.dmgDescript, clrMessageGood);
					} else {
						const string BONUS_STR = data.isBackStab ? "covertly " : data.isSwiftAttack ? "swiftly " : data.isMightyAttack ? "mightily " : "";
						const SDL_Color clr = data.isBackStab || data.isSwiftAttack || data.isMightyAttack ? clrMagenta : clrMessageGood;
						eng->log->addMessage("You " + wpnVerb + " " + otherName + " " + BONUS_STR + "with " + data.weaponName_a + data.dmgDescript, clr);
					}
				} else {
					const string wpnVerb = weapon->getInstanceDefinition().meleeAttackMessages.other;

					if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
						otherName = data.attacker->getNameThe();
					} else {
						otherName = "It";
					}

					eng->log->addMessage(otherName + " " + wpnVerb + data.dmgDescript, clrMessageBad, false);
				}
			}
		}
	}
}


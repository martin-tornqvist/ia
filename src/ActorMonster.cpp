#include "ActorMonster.h"

#include <iostream>

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "GameTime.h"
#include "Attack.h"
#include "Reload.h"
#include "Inventory.h"

void Monster::act() {
	if(spellCoolDownCurrent != 0) {
		spellCoolDownCurrent--;
	}

	if(playerAwarenessCounter > 0) {
	   isRoamingAllowed = true;
	}

	const bool HAS_SNEAK_SKILL = m_instanceDefinition.abilityValues.getAbilityValue(ability_sneaking, true) > 0;
	isSneaking = eng->player->checkIfSeeActor(*this, NULL) == false && HAS_SNEAK_SKILL;

	actorSpecificAct();
	/*
	There should probably not be any code after "actorSpecificAct()", since it will
	probably end this monster's turn, and newTurn() stuff may be called, which deletes this
	monster if it didn't leave a corpse.
	*/
}

void Monster::monsterHit() {
	playerAwarenessCounter = m_instanceDefinition.nrTurnsAwarePlayer;
}

void Monster::moveToCell(const coord targetCell) {
	const coord targetCellReal = getStatusEffectsHandler()->changeMoveCoord(pos, targetCell);

	// Movement direction is stored for AI purposes
	lastDirectionTraveled = targetCellReal - pos;

	pos = targetCellReal;
	eng->gameTime->letNextAct();
}

void Monster::registerHeardSound(const Sound& sound) {
	if(deadState == actorDeadState_alive) {
		soundsHeard.push_back(sound);
	}
}

bool Monster::attemptAttack(const coord attackPos) {
	if(deadState == actorDeadState_alive) {
		if(playerAwarenessCounter > 0) {

			bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
			eng->mapTests->makeVisionBlockerArray(blockers);

			if(checkIfSeeActor(*eng->player, blockers)) {
				AttackOpport opport = getAttackOpport(attackPos);
				BestAttack attack = getBestAttack(opport);

				if(attack.weapon != NULL) {
					if(attack.melee == true) {
						if(attack.weapon->getInstanceDefinition().isMeleeWeapon == true) {
							eng->attack->melee(attackPos.x, attackPos.y, attack.weapon);
							return true;
						}
					} else {
						if(attack.weapon->getInstanceDefinition().isRangedWeapon == true) {
							if(opport.timeToReload == false) {
								eng->attack->ranged(attackPos.x, attackPos.y, attack.weapon);
								return true;
							} else {
								eng->reload->reloadWeapon(this);
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

AttackOpport Monster::getAttackOpport(const coord attackPos) {
	AttackOpport opport;
	if(m_statusEffectsHandler->allowAttack(false) == true) {
		opport.melee = eng->mapTests->isCellsNeighbours(pos, attackPos, false);

		Weapon* weapon = NULL;
		const unsigned nrOfIntrinsics = m_inventory->getIntrinsicsSize();
		if(opport.melee == true) {
			//If melee attacks aren't disabled because of status
			if(m_statusEffectsHandler->allowAttackMelee(false) == true) {
				//GET POSSIBLE MELEE ATTACKS

				//Melee weapon in wielded slot?
				weapon = dynamic_cast<Weapon*>(m_inventory->getItemInSlot(slot_wielded));
				if(weapon != NULL) {
					if(weapon->getInstanceDefinition().isMeleeWeapon == true) {
						opport.weapons.push_back(weapon);
					}
				}

				//Intrinsic melee attacks?
				for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
					weapon = dynamic_cast<Weapon*>(m_inventory->getIntrinsicInElement(i));
					if(weapon->getInstanceDefinition().isMeleeWeapon == true) {
						opport.weapons.push_back(weapon);
					}
				}
			}
		} else {
			//If ranged attacks aren't disabled because of status
			if(m_statusEffectsHandler->allowAttackRanged(false) == true && m_statusEffectsHandler->hasEffect(statusBurning) == false) {

				//GET POSSIBLE RANGED ATTACKS

				//Ranged weapon in wielded slot?
				weapon = dynamic_cast<Weapon*>(m_inventory->getItemInSlot(slot_wielded));

				if(weapon != NULL) {
					if(weapon->getInstanceDefinition().isRangedWeapon == true) {
						opport.weapons.push_back(weapon);

						//Check if reload time
						if(weapon->ammoLoaded == 0 && weapon->getInstanceDefinition().rangedHasInfiniteAmmo == false) {
							if(m_inventory->hasAmmoForFirearmInInventory() == true) {
								opport.timeToReload = true;
							}
						}
					}
				}

				//Intrinsic ranged attacks?
				for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
					weapon = dynamic_cast<Weapon*>(m_inventory->getIntrinsicInElement(i));
					if(weapon->getInstanceDefinition().isRangedWeapon == true) {
						opport.weapons.push_back(weapon);
					}
				}
			}
		}
	}

	return opport;
}

BestAttack Monster::getBestAttack(AttackOpport attackOpport) {
	BestAttack attack;
	attack.melee = attackOpport.melee;

	Weapon* newWeapon = NULL;

	const unsigned int nrOfWeapons = attackOpport.weapons.size();

	//If any possible attacks found
	if(nrOfWeapons > 0) {
		attack.weapon = attackOpport.weapons.at(0);

		const ItemDefinition* def = &(attack.weapon->getInstanceDefinition());

		//If there are more than one possible weapon, find strongest.
		if(nrOfWeapons > 1) {
			for(unsigned int i = 1; i < nrOfWeapons; i++) {

				//Found new weapon in element i.
				newWeapon = attackOpport.weapons.at(i);
				const ItemDefinition* newDef = &(newWeapon->getInstanceDefinition());

				//Compare definitions.
				//If weapon i is stronger -
				if(eng->itemData->isWeaponStronger(*def, *newDef, attack.melee) == true) {
					// - use new weapon instead.
					attack.weapon = newWeapon;
					def = newDef;
				}
			}
		}
	}

	return attack;
}

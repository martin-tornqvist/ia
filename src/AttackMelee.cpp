#include "Attack.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Postmortem.h"
#include "Blood.h"
#include "Knockback.h"

using namespace std;

void Attack::melee(int defenderX, int defenderY, Weapon* weapon) {
	eng->renderer->drawMapAndInterface(false);

	//Get attack data
	AttackData data;
	getAttackData(data, coord(defenderX, defenderY), coord(defenderX, defenderY), weapon, true);

	//Print messages
	printMeleeMessages(data, weapon);

	if(data.attackResult >= successSmall && data.isDefenderDodging == false) {
		//Do damage
		const bool DIED = data.currentDefender->hit(data.dmg, weapon->getInstanceDefinition().meleeDamageType);

		if(DIED == true) {
			if(data.currentDefender == eng->player) {
				eng->postmortem->setCauseOfDeath(weapon->getInstanceDefinition().causeOfDeathMessage);
			}
		} else {
			//Apply weapon status effects
			data.currentDefender->getStatusEffectsHandler()->attemptAddEffectsFromWeapon(weapon, true);
		}
		//Blood
		if(data.attackResult >= successNormal) {
			if(data.currentDefender->getInstanceDefinition()->canBleed == true) {
				eng->gore->makeBlood(coord(defenderX, defenderY));
			}
		}

		//Knock-back?
		if(DIED == false) {
			if(weapon->getInstanceDefinition().meleeCausesKnockBack) {
				if(data.attackResult > successSmall) {
					eng->knockBack->attemptKnockBack(data.currentDefender, data.attacker->pos, false);
				}
			}
		}
	}

	if(data.currentDefender != eng->player) {
		Monster* const monster = dynamic_cast<Monster*>(data.currentDefender);
		monster->playerAwarenessCounter = monster->getInstanceDefinition()->nrTurnsAwarePlayer;
	} else {
		if(data.attackResult >= failSmall) {
			Monster* const monster = dynamic_cast<Monster*>(data.attacker);
			monster->isStealth = false;
		}
	}

	//Let next act
	if(data.isSwiftAttack == false) {
		eng->gameTime->letNextAct();
	}
}


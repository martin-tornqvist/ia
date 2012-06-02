#include "AbilityValues.h"

#include <math.h>

#include "Engine.h"
#include "ActorPlayer.h"
#include "PlayerBonuses.h"
#include "BasicUtils.h"

//Return ability values for owning actor. Values are either returned
//affected by owned actors current status effects (like harder to hit
//when blinded), or as base value only, which is used for example when
//picking skills at level up. Players skills are affected by hunger.
int AbilityValues::getAbilityValue(const Abilities_t devName, const bool affectedByStatusEffects) const {
	int val = abilityList[devName];

	if(affectedByStatusEffects) {
		val += m_actor->getStatusEffectsHandler()->getAbilityModifier(devName);
	}

	const bool IS_PLAYER = m_actor == m_actor->eng->player;
	if(IS_PLAYER) {
		val += m_actor->eng->playerBonusHandler->getBonusAbilityModifier(devName);

		//Searching must be at least 1
		if(devName == ability_searching) {
			val = max(1, val);
		}
	}

	val = max(0, min(99, val));

	return val;
}

AbilityRollResult_t AbilityRoll::roll(const int totalAbilityValue) const {
    const int ROLL = eng->dice(1,100);

	const int successCriticalLimit  = static_cast<int>(ceil(static_cast<float>(totalAbilityValue)/20.0));
	const int successBigLimit       = static_cast<int>(ceil(static_cast<float>(totalAbilityValue)/5.0));
	const int successNormalLimit    = static_cast<int>(ceil(static_cast<float>(totalAbilityValue)*4.0/5.0));
	const int successSmallLimit     = totalAbilityValue;
	const int failSmallLimit        = 2 * totalAbilityValue - successNormalLimit;
	const int failNormalLimit       = 2 * totalAbilityValue - successBigLimit;
	const int failBigLimit          = 98;

	if(ROLL <= successCriticalLimit)	return successCritical;
	if(ROLL <= successBigLimit)			return successBig;
	if(ROLL <= successNormalLimit)		return successNormal;
	if(ROLL <= successSmallLimit)		return successSmall;
	if(ROLL <= failSmallLimit)			return failSmall;
	if(ROLL <= failNormalLimit)			return failNormal;
	if(ROLL <= failBigLimit)			return failBig;

	return failCritical;

	/* Example:
	-----------
	Ability = 50

	Roll:
	1  -   3: Critical  success
	4  -  10: Big       success
	11 -  40: Normal    success
	41 -  50: Small     Success
	51 -  60: Small     Fail
	61 -  90: Normal    Fail
	91 -  98: Big       Fail
	99 - 100: Critical  Fail */
}


void AbilityValues::reset() {
	for(unsigned int i = 0; i < endOfAbilities; i++) {
		abilityList[i] = 0;
	}
}

void AbilityValues::setAbilityValue(const Abilities_t devName, const int value) {
	abilityList[devName] = value;
}

void AbilityValues::changeAbilityValue(const Abilities_t devName, const int change) {
	abilityList[devName] += change;
}


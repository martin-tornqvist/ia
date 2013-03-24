#include "AbilityValues.h"

#include <math.h>

#include "Engine.h"
#include "ActorPlayer.h"
#include "PlayerBonuses.h"
#include "BasicUtils.h"

int AbilityValues::getAbilityValue(const Abilities_t ability,
                                   const bool IS_AFFECTED_BY_STATUS_EFFECTS,
                                   Actor& actor) const {
  int val = abilityList[ability];

  if(IS_AFFECTED_BY_STATUS_EFFECTS) {
    val += actor.getStatusEffectsHandler()->getAbilityModifier(ability);
  }

  if(&actor == eng->player) {
    switch(ability) {
    case ability_searching: {
      val += 8;
    } break;

    case ability_accuracyMelee: {
      val += 45;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_adeptMeleeCombatant))
        val += 15;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_masterfulMeleeCombatant))
        val += 15;
    } break;

    case ability_accuracyRanged: {
      val += 55;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_marksman))
        val += 15;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_sharpshooter))
        val += 15;
    } break;

    case ability_dodgeTrap: {
      val += 5;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_dexterous))
        val += 20;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_lithe))
        val += 20;
    } break;

    case ability_dodgeAttack: {
      val += 10;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_dexterous))
        val += 30;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_lithe))
        val += 30;
    } break;

    case ability_resistStatusBody: {
      val += 25;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_tough))
        val += 20;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_rugged))
        val += 20;
    } break;

    case ability_resistStatusMind: {
      val += 25;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_strongMinded))
        val += 20;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_unyielding))
        val += 20;
    } break;

    case ability_stealth: {
      val += 30;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_stealthy))
        val += 45;
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_imperceptible))
        val += 20;
    } break;
    default: {
    } break;
    }

    //Searching must be at least 1
    if(ability == ability_searching) {
      val = max(1, val);
    }
  }

  val = max(0, val);

  return val;
}

AbilityRollResult_t AbilityRoll::roll(const int TOTAL_SKILL_VALUE) const {
  const int ROLL = eng->dice(1, 100);

  const int successCriticalLimit  = static_cast<int>(ceil(static_cast<float>(TOTAL_SKILL_VALUE) / 20.0));
  const int successBigLimit       = static_cast<int>(ceil(static_cast<float>(TOTAL_SKILL_VALUE) / 5.0));
  const int successNormalLimit    = static_cast<int>(ceil(static_cast<float>(TOTAL_SKILL_VALUE) * 4.0 / 5.0));
  const int successSmallLimit     = TOTAL_SKILL_VALUE;
  const int failSmallLimit        = 2 * TOTAL_SKILL_VALUE - successNormalLimit;
  const int failNormalLimit       = 2 * TOTAL_SKILL_VALUE - successBigLimit;
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

void AbilityValues::setAbilityValue(const Abilities_t ability, const int VAL) {
  abilityList[ability] = VAL;
}



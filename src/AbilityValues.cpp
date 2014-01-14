#include "AbilityValues.h"

#include <math.h>

#include "Engine.h"
#include "ActorPlayer.h"
#include "PlayerBonuses.h"
#include "BasicUtils.h"
#include "Properties.h"

int AbilityValues::getVal(const Abilities_t ability,
                          const bool IS_AFFECTED_BY_STATUS_EFFECTS,
                          Actor& actor) const {
  int val = abilityList[ability];

  if(IS_AFFECTED_BY_STATUS_EFFECTS) {
    val += actor.getPropHandler().getAbilityMod(ability);
  }

  PlayerBonHandler& bonHlr = *eng->playerBonHandler;

  if(&actor == eng->player) {
    switch(ability) {
      case ability_searching: {
        val += 8;
      } break;

      case ability_accuracyMelee: {
        val += 45;
        if(bonHlr.hasTrait(traitAdeptMeleeFighter))   val += 10;
        if(bonHlr.hasTrait(traitExpertMeleeFighter))  val += 10;
        if(bonHlr.hasTrait(traitMasterMeleeFighter))  val += 10;
      } break;

      case ability_accuracyRanged: {
        val += 50;
        if(bonHlr.hasTrait(traitAdeptMarksman))   val += 10;
        if(bonHlr.hasTrait(traitExpertMarksman))  val += 10;
        if(bonHlr.hasTrait(traitMasterMarksman))  val += 10;
      } break;

      case ability_dodgeTrap: {
        val += 5;
        if(bonHlr.hasTrait(traitDexterous)) val += 20;
        if(bonHlr.hasTrait(traitLithe))     val += 20;
      } break;

      case ability_dodgeAttack: {
        val += 10;
        if(bonHlr.hasTrait(traitDexterous)) val += 25;
      } break;

      case ability_stealth: {
        val += 10;
        if(bonHlr.hasTrait(traitStealthy))      val += 50;
        if(bonHlr.hasTrait(traitImperceptible)) val += 30;
      } break;

      case ability_empty:
      case endOfAbilities: {} break;
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
  const int ROLL = eng.dice.percentile();

  const int successCriticalLimit  = int(ceil(float(TOTAL_SKILL_VALUE) / 20.0));
  const int successBigLimit       = int(ceil(float(TOTAL_SKILL_VALUE) / 5.0));
  const int successNormalLimit    = int(ceil(float(TOTAL_SKILL_VALUE) * 4.0 / 5.0));
  const int successSmallLimit     = TOTAL_SKILL_VALUE;
  const int failSmallLimit        = 2 * TOTAL_SKILL_VALUE - successNormalLimit;
  const int failNormalLimit       = 2 * TOTAL_SKILL_VALUE - successBigLimit;
  const int failBigLimit          = 98;

  if(ROLL <= successCriticalLimit)  return successCritical;
  if(ROLL <= successBigLimit)     return successBig;
  if(ROLL <= successNormalLimit)    return successNormal;
  if(ROLL <= successSmallLimit)   return successSmall;
  if(ROLL <= failSmallLimit)      return failSmall;
  if(ROLL <= failNormalLimit)     return failNormal;
  if(ROLL <= failBigLimit)      return failBig;

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

void AbilityValues::setVal(const Abilities_t ability, const int VAL) {
  abilityList[ability] = VAL;
}

void AbilityValues::changeVal(const Abilities_t ability, const int CHANGE) {
  abilityList[ability] += CHANGE;
}


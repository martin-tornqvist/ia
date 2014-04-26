#include "AbilityValues.h"

#include <math.h>

#include "ActorPlayer.h"
#include "PlayerBon.h"
#include "Utils.h"
#include "Properties.h"
#include "Map.h"

int AbilityVals::getVal(const AbilityId abilityId,
                        const bool IS_AFFECTED_BY_PROPS, Actor& actor) const {
  int val = abilityList[int(abilityId)];

  if(IS_AFFECTED_BY_PROPS) {
    val += actor.getPropHandler().getAbilityMod(abilityId);
  }

  if(&actor == Map::player) {
    const int HP_PCT  = (actor.getHp() * 100) / actor.getHpMax(true);

    switch(abilityId) {
      case AbilityId::searching: {
        val += 8;
        if(PlayerBon::hasTrait(Trait::observant))   val += 4;
        if(PlayerBon::hasTrait(Trait::perceptive))  val += 4;
      } break;

      case AbilityId::accuracyMelee: {
        val += 45;
        if(PlayerBon::hasTrait(Trait::adeptMeleeFighter))   val += 10;
        if(PlayerBon::hasTrait(Trait::expertMeleeFighter))  val += 10;
        if(PlayerBon::hasTrait(Trait::masterMeleeFighter))  val += 10;
        if(PlayerBon::hasTrait(Trait::perseverant) && HP_PCT <= 25) val += 30;
      } break;

      case AbilityId::accuracyRanged: {
        val += 50;
        if(PlayerBon::hasTrait(Trait::adeptMarksman))   val += 10;
        if(PlayerBon::hasTrait(Trait::expertMarksman))  val += 10;
        if(PlayerBon::hasTrait(Trait::masterMarksman))  val += 10;
        if(PlayerBon::hasTrait(Trait::perseverant) && HP_PCT <= 25) val += 30;
      } break;

      case AbilityId::dodgeTrap: {
        val += 5;
        if(PlayerBon::hasTrait(Trait::dexterous)) val += 20;
        if(PlayerBon::hasTrait(Trait::lithe))     val += 20;
      } break;

      case AbilityId::dodgeAttack: {
        val += 10;
        if(PlayerBon::hasTrait(Trait::dexterous)) val += 20;
        if(PlayerBon::hasTrait(Trait::lithe))     val += 20;
        if(PlayerBon::hasTrait(Trait::perseverant) && HP_PCT <= 25) val += 50;
      } break;

      case AbilityId::stealth: {
        val += 10;
        if(PlayerBon::hasTrait(Trait::stealthy))      val += 50;
        if(PlayerBon::hasTrait(Trait::imperceptible)) val += 30;
      } break;

      case AbilityId::empty:
      case AbilityId::endOfAbilityId: {} break;
    }

    if(abilityId == AbilityId::searching) {
      val = max(val, 1);
    } else if(abilityId == AbilityId::dodgeAttack) {
      val = min(val, 95);
    }
  }

  val = max(0, val);

  return val;
}

void AbilityVals::reset() {
  for(int i = 0; i < int(AbilityId::endOfAbilityId); i++) {
    abilityList[i] = 0;
  }
}

void AbilityVals::setVal(const AbilityId ability, const int VAL) {
  abilityList[int(ability)] = VAL;
}

void AbilityVals::changeVal(const AbilityId ability, const int CHANGE) {
  abilityList[int(ability)] += CHANGE;
}

namespace AbilityRoll {

AbilityRollResult roll(const int TOT_SKILL_VALUE) {
  const int ROLL = Rnd::percentile();

  const int SUCC_CRI_LMT = int(ceil(float(TOT_SKILL_VALUE) / 20.0));
  const int SUCC_BIG_LMT = int(ceil(float(TOT_SKILL_VALUE) / 5.0));
  const int SUCC_NRM_LMT = int(ceil(float(TOT_SKILL_VALUE) * 4.0 / 5.0));
  const int SUCC_SML_LMT = TOT_SKILL_VALUE;
  const int FAIL_SML_LMT = 2 * TOT_SKILL_VALUE - SUCC_NRM_LMT;
  const int FAIL_NRM_LMT = 2 * TOT_SKILL_VALUE - SUCC_BIG_LMT;
  const int FAIL_BIG_LMT = 98;

  if(ROLL <= SUCC_CRI_LMT) return successCritical;
  if(ROLL <= SUCC_BIG_LMT) return successBig;
  if(ROLL <= SUCC_NRM_LMT) return successNormal;
  if(ROLL <= SUCC_SML_LMT) return successSmall;
  if(ROLL <= FAIL_SML_LMT) return failSmall;
  if(ROLL <= FAIL_NRM_LMT) return failNormal;
  if(ROLL <= FAIL_BIG_LMT) return failBig;

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

} //AbilityRoll

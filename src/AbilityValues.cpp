#include "AbilityValues.h"

#include <math.h>

#include "ActorPlayer.h"
#include "PlayerBon.h"
#include "Utils.h"
#include "Properties.h"
#include "Map.h"

using namespace std;

int AbilityVals::getVal(const AbilityId abilityId,
                        const bool IS_AFFECTED_BY_PROPS, Actor& actor) const
{
  int ret = abilityList[int(abilityId)];

  if (IS_AFFECTED_BY_PROPS)
  {
    ret += actor.getPropHandler().getAbilityMod(abilityId);
  }

  if (actor.isPlayer())
  {
    for (const InvSlot& slot : actor.getInv().slots_)
    {
      if (slot.item)
      {
        ret += slot.item->getData().abilityModsWhileEquipped[int(abilityId)];
      }
    }

    const int HP_PCT  = (actor.getHp() * 100) / actor.getHpMax(true);

    switch (abilityId)
    {
      case AbilityId::searching:
        ret += 8;
        if (PlayerBon::traitsPicked[int(Trait::observant)])   ret += 4;
        if (PlayerBon::traitsPicked[int(Trait::perceptive)])  ret += 4;
        break;

      case AbilityId::melee:
        ret += 45;
        if (PlayerBon::traitsPicked[int(Trait::adeptMeleeFighter)])   ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::expertMeleeFighter)])  ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::masterMeleeFighter)])  ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::perseverant)] && HP_PCT <= 25) ret += 30;
        break;

      case AbilityId::ranged:
        ret += 50;
        if (PlayerBon::traitsPicked[int(Trait::adeptMarksman)])   ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::expertMarksman)])  ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::masterMarksman)])  ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::perseverant)] && HP_PCT <= 25) ret += 30;
        break;

      case AbilityId::dodgeTrap:
        ret += 5;
        if (PlayerBon::traitsPicked[int(Trait::dexterous)]) ret += 25;
        if (PlayerBon::traitsPicked[int(Trait::lithe)])     ret += 25;
        break;

      case AbilityId::dodgeAtt:
        ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::dexterous)]) ret += 25;
        if (PlayerBon::traitsPicked[int(Trait::lithe)])     ret += 25;
        if (PlayerBon::traitsPicked[int(Trait::perseverant)] && HP_PCT <= 25) ret += 50;
        break;

      case AbilityId::stealth:
        ret += 10;
        if (PlayerBon::traitsPicked[int(Trait::stealthy)])      ret += 50;
        if (PlayerBon::traitsPicked[int(Trait::imperceptible)]) ret += 30;
        break;

      case AbilityId::empty:
      case AbilityId::END: {} break;
    }

    if (abilityId == AbilityId::searching)
    {
      //Searching must always be at least 1 to avoid trapping the player
      ret = max(ret, 1);
    }
    else if (abilityId == AbilityId::dodgeAtt)
    {
      //It should not be possible to dodge every attack
      ret = min(ret, 95);
    }
  }

  ret = max(0, ret);

  return ret;
}

void AbilityVals::reset()
{
  for (int i = 0; i < int(AbilityId::END); ++i)
  {
    abilityList[i] = 0;
  }
}

void AbilityVals::setVal(const AbilityId ability, const int VAL)
{
  abilityList[int(ability)] = VAL;
}

void AbilityVals::changeVal(const AbilityId ability, const int CHANGE)
{
  abilityList[int(ability)] += CHANGE;
}

namespace AbilityRoll
{

AbilityRollResult roll(const int TOT_SKILL_VALUE)
{
  const int ROLL = Rnd::percentile();

  const int SUCC_CRI_LMT = int(ceil(float(TOT_SKILL_VALUE) / 20.0));
  const int SUCC_BIG_LMT = int(ceil(float(TOT_SKILL_VALUE) / 5.0));
  const int SUCC_NRM_LMT = int(ceil(float(TOT_SKILL_VALUE) * 4.0 / 5.0));
  const int SUCC_SML_LMT = TOT_SKILL_VALUE;
  const int FAIL_SML_LMT = 2 * TOT_SKILL_VALUE - SUCC_NRM_LMT;
  const int FAIL_NRM_LMT = 2 * TOT_SKILL_VALUE - SUCC_BIG_LMT;
  const int FAIL_BIG_LMT = 98;

  if (ROLL <= SUCC_CRI_LMT) return successCritical;
  if (ROLL <= SUCC_BIG_LMT) return successBig;
  if (ROLL <= SUCC_NRM_LMT) return successNormal;
  if (ROLL <= SUCC_SML_LMT) return successSmall;
  if (ROLL <= FAIL_SML_LMT) return failSmall;
  if (ROLL <= FAIL_NRM_LMT) return failNormal;
  if (ROLL <= FAIL_BIG_LMT) return failBig;

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

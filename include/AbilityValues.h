#ifndef ABILITY_VALUES_H
#define ABILITY_VALUES_H

#include <vector>
#include <string>
#include <iostream>

class Actor;

enum class AbilityId
{
  empty,
  searching,
  ranged,
  melee,
  dodgeTrap,
  dodgeAttack,
  stealth,
  END
};

//Each actor has an instance of this class
class AbilityVals
{
public:
  AbilityVals() {reset();}

  AbilityVals& operator=(const AbilityVals& other)
  {
    for (int i = 0; i < int(AbilityId::END); ++i)
    {
      abilityList[i] = other.abilityList[i];
    }
    return *this;
  }

  void reset();

  int getVal(const AbilityId abilityId, const bool IS_AFFECTED_BY_PROPS,
             Actor& actor) const;

  inline int getRawVal(const AbilityId ability)
  {
    return abilityList[int(ability)];
  }

  void setVal(const AbilityId ability, const int VAL);

  void changeVal(const AbilityId ability, const int CHANGE);

private:
  int abilityList[int(AbilityId::END)];
};

//TODO Is this really necessary? Most functionality nowadays just roll their own chances.
//Probably the only case where failSmall and failBig is used is for melee attack messages.
//It seems simpler and more transparent to just use the Rnd functions for rolling,
//together with AbilityVals::getVal() for retrieving abilities to roll against.
enum AbilityRollResult
{
  failCritical,
  failBig,
  failNormal,
  failSmall,
  successSmall,
  successNormal,
  successBig,
  successCritical
};

//TODO See comment above for AbilityRollResult
namespace AbilityRoll
{

AbilityRollResult roll(const int TOT_SKILL_VALUE);

} //AbilityRoll

#endif

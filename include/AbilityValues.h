#ifndef ABILITY_VALUES_H
#define ABILITY_VALUES_H

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Engine;
class Actor;

enum Abilities_t {
  ability_empty,
  ability_searching,
  ability_accuracyRanged,
  ability_accuracyMelee,
  ability_dodgeTrap,
  ability_dodgeAttack,
  ability_stealth,
  endOfAbilities
};

//Each actor has an instance of this class
class AbilityValues {
public:
  AbilityValues() : eng(NULL) {reset();}

  AbilityValues& operator=(const AbilityValues& other) {
    for(unsigned int i = 0; i < endOfAbilities; i++) {
      abilityList[i] = other.abilityList[i];
    }
    return *this;
  }

  void reset();

  int getVal(const Abilities_t ability,
             const bool IS_AFFECTED_BY_STATUS_EFFECTS,
             Actor& actor) const;

  inline int getRawVal(const Abilities_t ability) {
    return abilityList[ability];
  }

  void setVal(const Abilities_t ability, const int VAL);

  void changeVal(const Abilities_t ability, const int CHANGE);

  Engine* eng;
private:
  int abilityList[endOfAbilities];
};

enum AbilityRollResult_t {
  failCritical, failBig, failNormal, failSmall,
  successSmall, successNormal, successBig, successCritical
};

//This is a single global class
class AbilityRoll {
public:
  AbilityRoll(Engine& engine) : eng(engine) {}
  AbilityRollResult_t roll(const int TOTAL_SKILL_VALUE) const;

private:
  Engine& eng;
};

#endif

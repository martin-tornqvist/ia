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
	ability_weaponHandling,
	ability_dodge,
	ability_resistStatusBodyAndSense,
	ability_resistStatusMindAndShock,
	ability_sneaking,
	ability_mobility,
//	ability_backstabbing,
	ability_firstAid,
	ability_language,
	endOfAbilities
};

//Each actor has an instance of this class
class AbilityValues
{
public:
	AbilityValues() : m_actor(NULL) {
		reset();
	}

	AbilityValues& operator=(const AbilityValues& other) {
		for(unsigned int i = 0; i < endOfAbilities; i++) {
			abilityList[i] = other.abilityList[i];
		}
		return *this;
	}

	void reset();
	void setOwningActor(Actor* const actor) {m_actor = actor;}
	int getAbilityValue(const Abilities_t devName, const bool affectedByStatusEffects) const;
	void setAbilityValue(const Abilities_t devName, const int value);
	void changeAbilityValue(const Abilities_t devName, const int change);

private:
	int abilityList[endOfAbilities];
	Actor* m_actor;
};

enum AbilityRollResult_t {
	failCritical, failBig, failNormal, failSmall,
	successSmall, successNormal, successBig, successCritical
};


//This is a single global class
class AbilityRoll
{
public:
	AbilityRoll(Engine* engine) : eng(engine) {}
	AbilityRollResult_t roll(const int totalSkillValue) const;

private:
	Engine* eng;
};

#endif

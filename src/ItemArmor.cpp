#include "ItemArmor.h"

#include "Engine.h"
#include "Log.h"

Armor::Armor(ItemDefinition* const itemDefinition, Engine* engine) :
	Item(itemDefinition), eng(engine), durability(eng->dice.getInRange(60, 100)) {
}

string Armor::getArmorDataLine() const {
	const string apLabelOverRide = m_instanceDefinition.armorData.overRideAbsorptionPointLabel;
	const string absorptionPoints = apLabelOverRide == "" ? intToString(getAbsorptionPoints(damageType_physical)) : apLabelOverRide;
	return "[" + absorptionPoints + "]";
}

int Armor::takeDurabilityHitAndGetReducedDamage(const int DAMAGE_BEFORE, const DamageTypes_t damageType) {
	//Absorption points = damage soaked up by armor instead of hitting the player
	//DDF = Damage (to) Durability Factor, how much damage the durability takes per attack damage point

	const int ABSORPTION_POINTS_BEFORE = getAbsorptionPoints(damageType);

	const double DDF_INTRINSIC = m_instanceDefinition.armorData.damageToDurabilityFactors[damageType];
	const double DDF_RANDOM = static_cast<float> (eng->dice(1, 100)) / 100.0;
	const double DDF_ADJUST = 5.0;

	durability = max(0, durability - static_cast<int> (DAMAGE_BEFORE * DDF_INTRINSIC * DDF_RANDOM * DDF_ADJUST));

	if(getAbsorptionPoints(damageType_physical) < ABSORPTION_POINTS_BEFORE) {
		eng->log->addMessage("Your " + m_instanceDefinition.name.name + " is damaged!");
	}

	return max(1, DAMAGE_BEFORE - ABSORPTION_POINTS_BEFORE);
}

int Armor::getAbsorptionPoints(const DamageTypes_t damageType) const {
	const int ABSORPTION_POINTS_MAX = m_instanceDefinition.armorData.absorptionPoints[damageType];

	if(durability > 60) {
		return ABSORPTION_POINTS_MAX;
	}

	if(durability > 40) {
		return max(0, ABSORPTION_POINTS_MAX - 1);
	}

	if(durability > 25) {
		return max(0, ABSORPTION_POINTS_MAX - 2);
	}

	if(durability > 15) {
		return max(0, ABSORPTION_POINTS_MAX - 3);
	}

	return 0;
}

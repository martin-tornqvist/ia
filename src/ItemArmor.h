#ifndef ITEM_ARMOR_H
#define ITEM_ARMOR_H

#include "Engine.h"

#include "Item.h"

class Armor: public Item {
public:
	Armor(ItemDefinition* const itemDefinition, Engine* engine);
	~Armor() {
	}

	int getDurability() const {
		return durability;
	}

	string getArmorDataLine() const;

	int takeDurabilityHitAndGetReducedDamage(const int DAMAGE_BEFORE, const DamageTypes_t damageType);

private:
	int getAbsorptionPoints(const DamageTypes_t damageType) const;

	Engine* eng;
	int durability;
};

#endif

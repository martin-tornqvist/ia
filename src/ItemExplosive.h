#ifndef ITEM_EXPLOSIVE_H
#define ITEM_EXPLOSIVE_H

#include "StatusEffects.h"
#include "Item.h"

class Engine;

class Explosive : public Item
{
public:
	Explosive(ItemDefinition* const itemDefinition) : Item(itemDefinition) {}

	virtual void setPlayerExplosive(Engine* const engine) const {(void)engine;}

protected:
};

class Dynamite: public Explosive
{
public:
	Dynamite(ItemDefinition* const itemDefinition) : Explosive(itemDefinition) {}

	void setPlayerExplosive(Engine* const engine) const;

private:
};

class Molotov: public Explosive
{
public:
	Molotov(ItemDefinition* const itemDefinition) : Explosive(itemDefinition) {}

	void setPlayerExplosive(Engine* const engine) const;

private:
};

class Flare: public Explosive
{
public:
	Flare(ItemDefinition* const itemDefinition) : Explosive(itemDefinition) {}

	void setPlayerExplosive(Engine* const engine) const;

private:
};

#endif

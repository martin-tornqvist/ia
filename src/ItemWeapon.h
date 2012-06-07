#ifndef WEAPON_H
#define WEAPON_H

#include "Item.h"
#include "Converters.h"

class Engine;

class Weapon: public Item {
public:
	Weapon(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Item(itemDefinition), m_ammoDefinition(ammoDefinition) {
		ammoLoaded = 0;
		ammoCapacity = 0;
		effectiveRangeLimit = 3;
		clip = false;
	}
	virtual ~Weapon() {
	}

	int ammoLoaded;
	int effectiveRangeLimit;
	int ammoCapacity;
	bool clip;

	void setRandomMeleePlus(Engine* const engine);

	void setMeleePlus(const int VALUE) {
      m_instanceDefinition.meleeDmg.plus = VALUE;
	}

	void appplyDropEffects() {
		setColorForAmmoStatus();
	}

	virtual vector<string> itemSpecificWriteToFile() {
		vector<string> lines;
		lines.push_back(intToString(ammoLoaded));
		return lines;
	}

	virtual void itemSpecificReadFromFile(vector<string> lines) {
		ammoLoaded = stringToInt(lines.at(0));
	}

	//actorHit may be NULL
	virtual void weaponSpecific_projectileObstructed(int originX, int originY, Actor* actor, Engine* engine) {
		(void) originX;
		(void) originY;
		(void) actor;
		(void) engine;
	}

	const ItemDefinition& getAmmoDefinition() {
		return *m_ammoDefinition;
	}

	void itemSpecificAddSaveLines(vector<string>& lines) {
		lines.push_back(intToString(m_instanceDefinition.meleeDmg.plus));
		lines.push_back(intToString(ammoLoaded));
	}

	void itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
		m_instanceDefinition.meleeDmg.plus = stringToInt(lines.front());
		lines.erase(lines.begin());
		ammoLoaded = stringToInt(lines.front());
		lines.erase(lines.begin());
	}

protected:
	Weapon& operator=(const Weapon& other) {
		(void) other;
		return *this;
	}

	ItemDefinition* const m_ammoDefinition;

	friend class ItemPickup;
	void setColorForAmmoStatus() {
		resetGlyphAndColor();
		if(ammoLoaded == 0) {
			m_instanceDefinition.color.r /= 2;
			m_instanceDefinition.color.g /= 2;
			m_instanceDefinition.color.b /= 2;
		}
	}
};

class SawedOff: public Weapon {
public:
	SawedOff(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoCapacity = 2;
		ammoLoaded = ammoCapacity;
		effectiveRangeLimit = 3;
	}
	~SawedOff() {
	}

private:
};

class PumpShotgun: public Weapon {
public:
	PumpShotgun(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoCapacity = 8;
		ammoLoaded = ammoCapacity;
		effectiveRangeLimit = 3;
	}
	~PumpShotgun() {
	}

private:
};

class Pistol: public Weapon {
public:
	Pistol(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoCapacity = 7;
		ammoLoaded = ammoCapacity;
		effectiveRangeLimit = 6;
	}
	~Pistol() {
	}

private:
};

class FlareGun: public Weapon {
public:
	FlareGun(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoLoaded = 1;
		ammoCapacity = 1;
		effectiveRangeLimit = 6;
	}
	~FlareGun() {
	}

private:
};

class MachineGun: public Weapon {
public:
	MachineGun(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoCapacity = m_ammoDefinition->ammoContainedInClip;
		ammoLoaded = ammoCapacity;
		effectiveRangeLimit = 8;
		clip = true;
	}
	~MachineGun() {
	}

private:
};

class Incinerator: public Weapon {
public:
	Incinerator(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoCapacity = 1;
		ammoLoaded = ammoCapacity;
		effectiveRangeLimit = 8;
		clip = false;
	}

	void weaponSpecific_projectileObstructed(int originX, int originY, Actor* actorHit, Engine* engine);
	~Incinerator() {
	}
private:
};

class TeslaCanon: public Weapon {
public:
	TeslaCanon(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoCapacity = m_ammoDefinition->ammoContainedInClip;
		ammoLoaded = ammoCapacity;
		effectiveRangeLimit = 8;
		clip = true;
	}
	~TeslaCanon() {
	}
private:
};

class SpikeGun: public Weapon {
public:
	SpikeGun(ItemDefinition* const itemDefinition, ItemDefinition* const ammoDefinition) :
		Weapon(itemDefinition, ammoDefinition) {
		ammoCapacity = 5;
		ammoLoaded = ammoCapacity;
		effectiveRangeLimit = 3;
		clip = true;
	}
	~SpikeGun() {
	}
private:
};

#endif

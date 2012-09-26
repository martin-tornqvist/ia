#ifndef MONSTER_H
#define MONSTER_H

#include "ConstTypes.h"

#include "Actor.h"
#include "ItemWeapon.h"
#include "Sound.h"
#include "Spells.h"

struct BestAttack {
	BestAttack() :
		weapon(NULL), melee(true) {
	}

	Weapon* weapon;
	bool melee;
};

struct AttackOpport {
	AttackOpport() :
		timeToReload(false), melee(true) {
		weapons.resize(0);
	}

	AttackOpport(const AttackOpport& other) :
		weapons(other.weapons), timeToReload(other.timeToReload), melee(other.melee) {
	}

	AttackOpport& operator=(const AttackOpport& other) {
		weapons = other.weapons;
		timeToReload = other.timeToReload;
		melee = other.melee;

		return *this;
	}

	vector<Weapon*> weapons;
	bool timeToReload;
	bool melee;
};

class Weapon;

class Monster: public Actor {
public:
	Monster() :
		playerAwarenessCounter(0), messageMonsterInViewPrinted(false), lastDirectionTraveled(coord(0, 0)), spellCoolDownCurrent(0), shockDecrease(0),
		isRoamingAllowed(true), isStealth(false), leader(NULL), target(NULL) {
	}

	virtual ~Monster() {
		if(deadState == actorDeadState_alive && m_archetypeDefinition->nrLeftAllowedToSpawn == 0) {
			m_archetypeDefinition->nrLeftAllowedToSpawn = 1;
		}

		for(unsigned int i = 0; i < spellsKnown.size(); i++) {
			delete spellsKnown.at(i);
		}
	}

	void moveToCell(const coord& targetCell);

	AttackOpport getAttackOpport(const coord& attackPos);
	BestAttack getBestAttack(const AttackOpport& attackOpport);
	bool attemptAttack(const coord& attackPos);

	virtual void actorSpecific_spawnStartItems() = 0;

	void registerHeardSound(const Sound& sound);

	void act();

	virtual bool actorSpecificAct() {
		return false;
	}

	int playerAwarenessCounter;

	bool messageMonsterInViewPrinted;

	coord lastDirectionTraveled;

	vector<Spell*> spellsKnown;
	int spellCoolDownCurrent;

	int shockDecrease;

	bool isRoamingAllowed;

	void clearHeardSounds() {
		soundsHeard.resize(0);
	}

	bool isStealth;

	Actor* leader;
	Actor* target;

protected:
	void monsterHit();

	vector<Sound> soundsHeard;
};

class Rat: public Monster {
public:
	Rat() :
		Monster() {
	}
	~Rat() {
	}
	virtual void actorSpecific_spawnStartItems();
};

class RatThing: public Rat {
public:
	RatThing() :
		Rat() {
	}
	~RatThing() {
	}
	void actorSpecific_spawnStartItems();
};

class BrownJenkin: public RatThing {
public:
	BrownJenkin() :
		RatThing() {
	}
	~BrownJenkin() {
	}
};

class Spider: public Monster {
public:
	Spider() :
		Monster() {
	}
	virtual ~Spider() {
	}
	bool actorSpecificAct();
};

class GreenSpider: public Spider {
public:
	GreenSpider() :
		Spider() {
	}
	~GreenSpider() {
	}
	void actorSpecific_spawnStartItems();
};

class BlackSpider: public Spider {
public:
	BlackSpider() :
		Spider() {
	}
	~BlackSpider() {
	}
	void actorSpecific_spawnStartItems();
};

class WhiteSpider: public Spider {
public:
	WhiteSpider() :
		Spider() {
	}
	~WhiteSpider() {
	}
	void actorSpecific_spawnStartItems();
};

class LengSpider: public Spider {
public:
	LengSpider() :
		Spider() {
	}
	~LengSpider() {
	}
	void actorSpecific_spawnStartItems();
};

class Zombie: public Monster {
public:
	Zombie() :
		Monster() {
		deadTurnCounter = 0;
		hasResurrected = false;
	}
	virtual ~Zombie() {
	}
	virtual bool actorSpecificAct();
	void actorSpecificDie();
protected:
	bool attemptResurrect();
	int deadTurnCounter;
	bool hasResurrected;
};

class ZombieClaw: public Zombie {
public:
	ZombieClaw() :
		Zombie() {
	}
	~ZombieClaw() {
	}
	void actorSpecific_spawnStartItems();
};

class ZombieAxe: public Zombie {
public:
	ZombieAxe() :
		Zombie() {
	}
	~ZombieAxe() {
	}
	void actorSpecific_spawnStartItems();
};

class BloatedZombie: public Zombie {
public:
	BloatedZombie() :
		Zombie() {
	}
	~BloatedZombie() {
	}

	void actorSpecific_spawnStartItems();
};

class MajorClaphamLee: public ZombieClaw {
public:
	MajorClaphamLee() :
		ZombieClaw(), hasSummonedTombLegions(false) {
	}
	~MajorClaphamLee() {
	}

	bool actorSpecificAct();
private:
	bool hasSummonedTombLegions;
};

class DeanHalsey: public ZombieClaw {
public:
	DeanHalsey() :
		ZombieClaw() {
	}
	~DeanHalsey() {
	}
};

class KeziahMason: public Monster {
public:
	KeziahMason() :
		Monster() {
		hasSummonedJenkin = false;
	}
	~KeziahMason() {
	}

	bool actorSpecificAct();
	void actorSpecific_spawnStartItems();
private:
	bool hasSummonedJenkin;
};

class Cultist: public Monster {
public:
	Cultist() :
		Monster() {
	}
	virtual ~Cultist() {
	}
};

class CultistShotgun: public Cultist {
public:
	CultistShotgun() :
		Cultist() {
	}
	~CultistShotgun() {
	}
	void actorSpecific_spawnStartItems();
};

class CultistMachineGun: public Cultist {
public:
	CultistMachineGun() :
		Cultist() {
	}
	~CultistMachineGun() {
	}
	void actorSpecific_spawnStartItems();
};

class CultistTeslaCannon: public Cultist {
public:
	CultistTeslaCannon() :
		Cultist() {
	}
	~CultistTeslaCannon() {
	}
	void actorSpecific_spawnStartItems();
};

class CultistPistol: public Cultist {
public:
	CultistPistol() :
		Cultist() {
	}
	~CultistPistol() {
	}
	void actorSpecific_spawnStartItems();
};

class CultistSpikeGun: public Cultist {
public:
	CultistSpikeGun() :
		Cultist() {
	}
	~CultistSpikeGun() {
	}
	void actorSpecific_spawnStartItems();
};

class CultistPriest: public Cultist {
public:
	CultistPriest() :
		Cultist() {
	}
	~CultistPriest() {
	}
	void actorSpecific_spawnStartItems();
};

class LordOfShadows: public Monster {
public:
	LordOfShadows() :
		Monster() {
	}
	~LordOfShadows() {
	}
	bool actorSpecificAct();
	void actorSpecific_spawnStartItems();
};

class LordOfSpiders: public Monster {
public:
	LordOfSpiders() :
		Monster() {
	}
	~LordOfSpiders() {
	}
	bool actorSpecificAct();
	void actorSpecific_spawnStartItems();
};

class LordOfSpirits: public Monster {
public:
	LordOfSpirits() :
		Monster() {
	}
	~LordOfSpirits() {
	}
	bool actorSpecificAct();
	void actorSpecific_spawnStartItems();
};

class LordOfPestilence: public Monster {
public:
	LordOfPestilence() :
		Monster() {
	}
	~LordOfPestilence() {
	}
	bool actorSpecificAct();
	void actorSpecific_spawnStartItems();
};

class FireHound: public Monster {
public:
	FireHound() :
		Monster() {
	}
	~FireHound() {
	}
	void actorSpecific_spawnStartItems();
};

class Ghost: public Monster {
public:
	Ghost() :
		Monster() {
	}
	~Ghost() {
	}
	bool actorSpecificAct();
	virtual void actorSpecific_spawnStartItems();
};

class Phantasm: public Ghost {
public:
	Phantasm() :
		Ghost() {
	}
	~Phantasm() {
	}
	void actorSpecific_spawnStartItems();
};

class Wraith: public Ghost {
public:
	Wraith() :
		Ghost() {
	}
	~Wraith() {
	}
	void actorSpecific_spawnStartItems();
};

class GiantBat: public Monster {
public:
	GiantBat() :
		Monster() {
	}
	~GiantBat() {
	}
	void actorSpecific_spawnStartItems();
};

class Byakhee: public GiantBat {
public:
	Byakhee() :
		GiantBat() {
	}
	~Byakhee() {
	}
	void actorSpecific_spawnStartItems();
};

class HuntingHorror: public GiantBat {
public:
	HuntingHorror() :
		GiantBat() {
	}
	~HuntingHorror() {
	}
	void actorSpecific_spawnStartItems();
};

class Wolf: public Monster {
public:
	Wolf() :
		Monster() {
	}
	~Wolf() {
	}
	void actorSpecific_spawnStartItems();
};

class MiGo: public Monster {
public:
	MiGo() :
		Monster() {
	}
	~MiGo() {
	}
	void actorSpecific_spawnStartItems();
};

class Ghoul: public Monster {
public:
	Ghoul() :
		Monster() {
	}
	~Ghoul() {
	}
	virtual void actorSpecific_spawnStartItems();
};

class DeepOne: public Monster {
public:
	DeepOne() :
		Monster() {
	}
	~DeepOne() {
	}
	void actorSpecific_spawnStartItems();
};

class Mummy: public Monster {
public:
	Mummy() :
		Monster() {
	}
	~Mummy() {
	}
	virtual void actorSpecific_spawnStartItems();
};

class MummyUnique: public Mummy {
public:
	MummyUnique() :
		Mummy() {
	}
	~MummyUnique() {
	}
	void actorSpecific_spawnStartItems();
};

class Shadow: public Monster {
public:
	Shadow() :
		Monster() {
	}
	~Shadow() {
	}

	virtual void actorSpecific_spawnStartItems();
};

class WormMass: public Monster {
public:
	WormMass() :
		Monster(), chanceToSpawnNew(35) {
	}
	~WormMass() {
	}
	bool actorSpecificAct();
	virtual void actorSpecific_spawnStartItems();
private:
	int chanceToSpawnNew;
};

class FireVampire: public Monster {
public:
	FireVampire() :
		Monster() {
	}
	~FireVampire() {
	}
	void actorSpecific_spawnStartItems();
	void monsterDeath();
};

class Ooze: public Monster {
public:
	Ooze() :
		Monster() {
	}
	~Ooze() {
	}
	virtual void actorSpecific_spawnStartItems() = 0;
};

class OozeGray: public Ooze {
public:
	OozeGray() :
		Ooze() {
	}
	~OozeGray() {
	}
	void actorSpecific_spawnStartItems();
};

class OozeClear: public Ooze {
public:
	OozeClear() :
		Ooze() {
	}
	~OozeClear() {
	}
	void actorSpecific_spawnStartItems();
};

class OozePutrid: public Ooze {
public:
	OozePutrid() :
		Ooze() {
	}
	~OozePutrid() {
	}
	void actorSpecific_spawnStartItems();
};

#endif

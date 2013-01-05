#ifndef SPELLS_H
#define SPELLS_H

#include <vector>

#include "ConstTypes.h"

using namespace std;

class Engine;
class Actor;
class Monster;

enum Spells_t {
	spell_azathothsBlast,
	spell_blind,
	spell_fear,
	spell_slow,
	spell_disease,
	spell_summonRandom,
	spell_healSelf,
	spell_knockBack,
	spell_teleport,
	spell_confuse,

	endOfSpells
};

class Spell;

//Class for keeping track of all available spells
class SpellHandler {
public:
	SpellHandler(Engine* const engine) :
		eng(engine) {
	}

	Spell* getRandomSpellForMonsters();
	void addAllCommonSpellsForMonsters(vector<Spell*>& knownSpells) const;

private:
	Spell* getSpellFromEnum(const Spells_t spell) const;

	Engine* eng;
};

//Struct for spell casting info
struct SpellData {
	SpellData(Actor* const caster, const coord& targetCell = coord(-1, -1)) :
		caster_(caster), targetCell_(targetCell) {
	}

	Actor* caster_;
	coord targetCell_;
};

//Base spell class
class Spell {
public:
	Spell() {
	}

	virtual ~Spell() {
	}

	void cast(const SpellData& d, Engine* const eng);

	//AI support function. Each individuall spell knows best how to
	//set target cell parameters etc.
	void monsterCast(Monster* const monster, Engine* const eng);

	virtual bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine) = 0;

protected:
	virtual void specificCast(const SpellData& d, Engine* const eng) = 0;
	virtual void specificMonsterCast(Monster* const monster, Engine* const eng) = 0;
};

class SpellAzathothsBlast: public Spell {
public:
	SpellAzathothsBlast() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellKnockBack: public Spell {
public:
	SpellKnockBack() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellTeleport: public Spell {
public:
	SpellTeleport() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellConfuse: public Spell {
public:
	SpellConfuse() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellBlind: public Spell {
public:
	SpellBlind() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellFear: public Spell {
public:
	SpellFear() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellSlow: public Spell {
public:
	SpellSlow() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellDisease: public Spell {
public:
	SpellDisease() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellSummonRandom: public Spell {
public:
	SpellSummonRandom() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

class SpellHealSelf: public Spell {
public:
	SpellHealSelf() :
		Spell() {
	}

	bool isGoodForMonsterNow(const Monster* const monster, Engine* const engine);

private:
	void specificMonsterCast(Monster* const monster, Engine* const eng);
	void specificCast(const SpellData& d, Engine* const eng);
};

#endif

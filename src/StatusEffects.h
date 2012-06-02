#ifndef STATUS_EFFECTS_H
#define STATUS_EFFECTS_H

#include <string>

#include "AbilityValues.h"
#include "ConstTypes.h"
#include "Converters.h"

using namespace std;

class Engine;

class Actor;
class TimedEntity;
class Weapon;

enum StatusEffects_t {
	statusBlind,
	statusBurning,
	statusFlared,
	statusParalyzed,
	statusTerrified,
	statusConfused,
	statusWaiting,
	statusSlowed,
	statusDiseased,
	statusFainted,
	statusPerfectReflexes,
	statusPerfectAim,
	statusPerfectStealth,
	statusPerfectFortitude,
	statusPerfectToughness,
	statusCursed,
	statusBlessed,

	statusNailed,

	//Added when pressing the wait key, gives better aim with ranged weapons
	statusStill,

	//The following are mostly used as AI nerfs
	statusDisabledAttack,
	statusDisabledMelee,
	statusDisabledRanged
};

class StatusEffect {
public:
	StatusEffect(const int turns, const StatusEffects_t effectId) :
		turnsLeft(turns), m_effectId(effectId) {
	}
	StatusEffect(const StatusEffects_t effectId) :
		m_effectId(effectId) {
	}
	StatusEffect(const StatusEffect& other) :
		turnsLeft(other.turnsLeft) {
	}
	virtual ~StatusEffect() {
	}

	void setOwningActor(Actor* owningActor_) {
		owningActor = owningActor_;
	}

	StatusEffects_t getEffectId() {
		return m_effectId;
	}

	virtual bool isFinnished() {
		return turnsLeft <= 0;
	}

	int turnsLeft;

	//Ability used for saving, and how it is modified.
	//(A status can be hard to avoid by having a large negative modifier, like a
	//strong poison, or getting covered in napalm. It can be made impossible to
	//avoid the effect by returning ability_empty, and a huge negative modifier)
	virtual Abilities_t getSaveAbility() = 0;
	virtual int getSaveAbilityModifier() = 0;

	virtual StatusEffect* copy() = 0;

	//Questions for the status to answer. This is how the status ends,
	//how it damages the actor, how it affects ability values, etc.
	virtual string getInterfaceName() = 0;
	virtual string messageWhenStart() = 0;
	virtual string messageWhenStartOther() = 0;
	virtual string messageWhenEnd() = 0;
	virtual string messageWhenEndOther() = 0;
	virtual string messageWhenSaves() = 0;
	virtual string messageWhenSavesOther() = 0;
	virtual string messageWhenMore() = 0;
	virtual string messageWhenMoreOther() = 0;
	virtual void newTurn(Engine* engine) = 0;
	virtual void start() = 0;
	virtual void end() = 0;

	virtual void more() {
	}

	virtual bool allowSee() {
		return true;
	}
	virtual bool allowMove() {
		return true;
	}
	virtual bool allowAct() {
		return true;
	}
	virtual bool isSlowed() {
		return false;
	}
	virtual void isHit() {
	}

	virtual bool allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return true;
	}
	virtual bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return true;
	}

	virtual int getAbilityModifier(const Abilities_t ability) {
		(void)ability;
		return 0;
	}
	virtual coord changeMoveCoord(const coord actorPos, const coord movePos, Engine* engine) {
		(void)actorPos;
		(void)engine;
		return movePos;
	}

	//Less used...
	virtual bool isEthereal() {
		return false;
	}

protected:
	virtual DiceParam getRandomStandardNrTurns() = 0;
	void setTurnsFromRandomStandard(Engine* const engine);

	StatusEffects_t m_effectId;

	Actor* owningActor;
};

class StatusTerrified: public StatusEffect {
public:
	StatusTerrified(Engine* const engine) :
		StatusEffect(statusTerrified) {
		setTurnsFromRandomStandard(engine);
	}
	StatusTerrified(const int turns) :
		StatusEffect(turns, statusTerrified) {
	}
	~StatusTerrified() {
	}

	StatusTerrified* copy() {
		StatusTerrified* cpy = new StatusTerrified(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Terrified";
	}
	string messageWhenStart() {
		return "You are terrified!";
	}
	string messageWhenStartOther() {
		return "is terrified.";
	}
	string messageWhenMore() {
		return "You are more terrified.";
	}
	string messageWhenMoreOther() {
		return "is more terrified.";
	}
	string messageWhenEnd() {
		return "You are no longer terrified!";
	}
	string messageWhenSaves() {
		return "You resist fear.";
	}
	string messageWhenSavesOther() {
		return "resists fear.";
	}
	string messageWhenEndOther() {
		return "is no longer terrified.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusMindAndShock;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_dodge: return 20; break;
		case ability_accuracyRanged: return -20; break;
		default: {} break;
		}
		return 0;
	}

	int getSaveAbilityModifier() {
		return 10;
	}

	void start() {
	}

	void end() {
	}

	bool allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE);

	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE);

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(1, 8);
	}
};

class StatusDiseased: public StatusEffect {
public:
	StatusDiseased(Engine* const engine) :
		StatusEffect(statusDiseased) {
		setTurnsFromRandomStandard(engine);
	}
	~StatusDiseased() {
	}

	StatusDiseased* copy() {
		StatusDiseased* cpy = new StatusDiseased(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Diseased";
	}
	string messageWhenStart() {
		return "You are diseased!";
	}
	string messageWhenStartOther() {
		return "is diseased.";
	}
	string messageWhenMore() {
		return "You are more diseased.";
	}
	string messageWhenMoreOther() {
		return "is more diseased.";
	}
	string messageWhenEnd() {
		return "You are no longer diseased!";
	}
	string messageWhenSaves() {
		return "You resist disease.";
	}
	string messageWhenSavesOther() {
		return "resists disease.";
	}
	string messageWhenEndOther() {
		return "is no longer diseased.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusBodyAndSense;
	}
	int getSaveAbilityModifier() {
		return 10;
	}

	void start() {
	}
	void end() {
	}

	void newTurn(Engine* engine);

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 1500);
	}
	friend class StatusEffectsHandler;
	StatusDiseased(const int turns) :
		StatusEffect(turns, statusDiseased) {
	}
};

class StatusStill: public StatusEffect {
public:
	StatusStill(Engine* const engine) :
		StatusEffect(statusStill) {
		setTurnsFromRandomStandard(engine);
	}
	StatusStill(const int turns) :
		StatusEffect(turns, statusStill) {
	}
	~StatusStill() {
	}

	StatusStill* copy() {
		StatusStill* cpy = new StatusStill(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Still";
	}
	string messageWhenStart() {
		return "";
	}
	string messageWhenMore() {
		return "";
	}
	string messageWhenMoreOther() {
		return "";
	}
	string messageWhenEnd() {
		return "";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "";
	}
	string messageWhenEndOther() {
		return "";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return -9999;
	}

	void start() {
	}

	void end() {
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_accuracyRanged:
			return 10;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 1);
	}
};

class StatusBlind: public StatusEffect {
public:
	StatusBlind(Engine* const engine) :
		StatusEffect(statusBlind) {
		setTurnsFromRandomStandard(engine);
	}
	StatusBlind(const int turns) :
		StatusEffect(turns, statusBlind) {
	}
	~StatusBlind() {
	}

	StatusBlind* copy() {
		StatusBlind* cpy = new StatusBlind(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Blind";
	}
	string messageWhenStart() {
		return "You are blind!";
	}
	string messageWhenMore() {
		return "You are more blind.";
	}
	string messageWhenMoreOther() {
		return "is more blind.";
	}
	string messageWhenEnd() {
		return "You can see again!";
	}
	string messageWhenSaves() {
		return "You resist blindness.";
	}
	string messageWhenStartOther() {
		return "is blinded.";
	}
	string messageWhenEndOther() {
		return "can see again.";
	}
	string messageWhenSavesOther() {
		return "resists blindness.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusBodyAndSense;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	void start() {
	}

	void end() {
	}

	bool allowSee() {
		return false;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_searching:
			return -9999;
			break;
		case ability_dodge:
			return -50;
			break;
		case ability_accuracyRanged:
			return -50;
			break;
		case ability_accuracyMelee:
			return -25;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(1, 8, 8);
	}
};

class StatusBlessed: public StatusEffect {
public:
	StatusBlessed(Engine* const engine) :
		StatusEffect(statusBlessed) {
		setTurnsFromRandomStandard(engine);
	}
	StatusBlessed(const int turns) :
		StatusEffect(turns, statusBlessed) {
	}
	~StatusBlessed() {
	}

	StatusBlessed* copy() {
		StatusBlessed* cpy = new StatusBlessed(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Blessed";
	}
	string messageWhenStart() {
		return "You feel luckier.";
	}
	string messageWhenMore() {
		return "You feel luckier.";
	}
	string messageWhenMoreOther() {
		return "is luckier.";
	}
	string messageWhenEnd() {
		return "Your good luck ends.";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "is more lucky.";
	}
	string messageWhenEndOther() {
		return "has normal luck.";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	void start();

	void end() {
	}

	int getAbilityModifier(const Abilities_t ability) {
		if(ability == ability_loreArcana || ability == ability_loreLanguage) {
			return 0;
		}
		return 10;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 500);
	}
};

class StatusCursed: public StatusEffect {
public:
	StatusCursed(Engine* const engine) :
		StatusEffect(statusCursed) {
		setTurnsFromRandomStandard(engine);
	}
	StatusCursed(const int turns) :
		StatusEffect(turns, statusCursed) {
	}
	~StatusCursed() {
	}

	StatusCursed* copy() {
		StatusCursed* cpy = new StatusCursed(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Cursed";
	}
	string messageWhenStart() {
		return "You feel misfortunate.";
	}
	string messageWhenMore() {
		return "You feel more misfortunate.";
	}
	string messageWhenMoreOther() {
		return "is misfortunate.";
	}
	string messageWhenEnd() {
		return "Your misfortune ends.";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "is more misfortunate.";
	}
	string messageWhenEndOther() {
		return "has normal fortune.";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	void start();

	void end() {
	}

	int getAbilityModifier(const Abilities_t ability) {
		(void)ability;
		return -10;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 500);
	}
};

class StatusBurning: public StatusEffect {
public:
	StatusBurning(Engine* const engine) :
		StatusEffect(statusBurning) {
		setTurnsFromRandomStandard(engine);
	}
	~StatusBurning() {
	}

	StatusBurning* copy() {
		StatusBurning* cpy = new StatusBurning(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Burning";
	}
	string messageWhenStart() {
		return "You are Burning!";
	}
	string messageWhenMore() {
		return "You burn more.";
	}
	string messageWhenMoreOther() {
		return "is burning more.";
	}
	string messageWhenEnd() {
		return "The flames are put out.";
	}
	string messageWhenSaves() {
		return "You resist burning.";
	}
	string messageWhenStartOther() {
		return "is burning.";
	}
	string messageWhenEndOther() {
		return "is no longer burning.";
	}
	string messageWhenSavesOther() {
		return "resists burning.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusBodyAndSense;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	void start();
	void end();
	void newTurn(Engine* engine);

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(1, 3, 3);
	}
	friend class StatusEffectsHandler;
	StatusBurning(const int turns) :
		StatusEffect(turns, statusBurning) {
	}
};

class StatusFlared: public StatusEffect {
public:
	StatusFlared(Engine* const engine) :
		StatusEffect(statusFlared) {
		setTurnsFromRandomStandard(engine);
	}
	~StatusFlared() {
	}

	StatusFlared* copy() {
		StatusFlared* cpy = new StatusFlared(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "";
	}
	string messageWhenStart() {
		return "";
	}
	string messageWhenMore() {
		return ".";
	}
	string messageWhenMoreOther() {
		return "is perforated by another flare.";
	}
	string messageWhenEnd() {
		return "";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "is perforated by the flare.";
	}
	string messageWhenEndOther() {
		return "recovers from the flare.";
	}
	string messageWhenSavesOther() {
		return "resists the flare.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusBodyAndSense;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	void start();
	void end();
	void newTurn(Engine* engine);

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(1, 3, 6);
	}
	friend class StatusEffectsHandler;
	StatusFlared(const int turns) :
		StatusEffect(turns, statusFlared) {
	}
};

class StatusConfused: public StatusEffect {
public:
	StatusConfused(Engine* const engine) :
		StatusEffect(statusConfused) {
		setTurnsFromRandomStandard(engine);
	}
	StatusConfused(const int turns) :
		StatusEffect(turns, statusConfused) {
	}
	~StatusConfused() {
	}

	StatusConfused* copy() {
		StatusConfused* cpy = new StatusConfused(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Confused";
	}
	string messageWhenStart() {
		return "You are confused!";
	}
	string messageWhenStartOther() {
		return "is confused.";
	}
	string messageWhenMore() {
		return "You are more confused.";
	}
	string messageWhenMoreOther() {
		return "is more confused.";
	}
	string messageWhenEnd() {
		return "You are no longer confused";
	}
	string messageWhenEndOther() {
		return "is no longer confused.";
	}
	string messageWhenSaves() {
		return "You resist confusion.";
	}
	string messageWhenSavesOther() {
		return "resists confusion.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusMindAndShock;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	coord changeMoveCoord(const coord actorPos, const coord movePos, Engine* engine);

	bool allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE);
	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE);

	void start() {
	}

	void end() {
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(1, 8, 8);
	}
};

class StatusNailed: public StatusEffect {
public:
	StatusNailed(Engine* const engine) : StatusEffect(statusNailed), nrOfSpikes(1) {
		setTurnsFromRandomStandard(engine);
	}

	StatusNailed(const int turns) : StatusEffect(turns, statusNailed), nrOfSpikes(1) {
	}

	~StatusNailed() {
	}

	StatusNailed* copy() {
		StatusNailed* cpy = new StatusNailed(turnsLeft);
		cpy->nrOfSpikes = nrOfSpikes;
		return cpy;
	}

	string getInterfaceName() {
		return "Nailed(" + intToString(nrOfSpikes) + ")";
	}
	string messageWhenStart() {
		return "You are fastened by a spike!";
	}
	string messageWhenStartOther() {
		return "is fastened by a spike.";
	}
	string messageWhenMore() {
		return "You are fastened by another spike.";
	}
	string messageWhenMoreOther() {
		return "is fastened by another spike.";
	}
	string messageWhenEnd() {
		return "You tear free!";
	}
	string messageWhenEndOther() {
		return "tears free!";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusBodyAndSense;
	}
	int getSaveAbilityModifier() {
		return 27;
	}

	coord changeMoveCoord(const coord actorPos, const coord movePos, Engine* engine);

	void more() {
		nrOfSpikes++;
	}

	void start() {
	}

	void end() {
	}

	bool isFinnished() {
		return nrOfSpikes <= 0;
	}

	void newTurn(Engine* engine) {
		(void)engine;
	}

private:
	int nrOfSpikes;

	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 999);
	}
};



class StatusWaiting: public StatusEffect {
public:
	StatusWaiting(Engine* const engine) :
		StatusEffect(statusWaiting) {
		setTurnsFromRandomStandard(engine);
	}
	StatusWaiting(const int turns) :
		StatusEffect(turns, statusWaiting) {
	}
	~StatusWaiting() {
	}

	StatusWaiting* copy() {
		StatusWaiting* cpy = new StatusWaiting(turnsLeft);
		return cpy;
	}

	virtual Abilities_t getSaveAbility() {
		return ability_empty;
	}
	virtual int getSaveAbilityModifier() {
		return -999;
	}

	string getInterfaceName() {
		return "";
	}
	string messageWhenStart() {
		return "";
	}
	string messageWhenMore() {
		return "";
	}
	string messageWhenMoreOther() {
		return "";
	}
	string messageWhenEnd() {
		return "";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "";
	}
	string messageWhenEndOther() {
		return "";
	}
	string messageWhenSavesOther() {
		return "";
	}
	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}
	void start() {
	}
	void end() {
	}

	bool allowMove() {
		return false;
	}
	bool allowAct() {
		return false;
	}
	bool allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}
	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 1);
	}
};

class StatusDisabledAttack: public StatusEffect {
public:
	StatusDisabledAttack(Engine* const engine) :
		StatusEffect(statusDisabledAttack) {
		setTurnsFromRandomStandard(engine);
	}
	StatusDisabledAttack(const int turns) :
		StatusEffect(turns, statusDisabledAttack) {
	}
	~StatusDisabledAttack() {
	}

	StatusDisabledAttack* copy() {
		StatusDisabledAttack* cpy = new StatusDisabledAttack(turnsLeft);
		return cpy;
	}

	virtual Abilities_t getSaveAbility() {
		return ability_empty;
	}
	virtual int getSaveAbilityModifier() {
		return -999;
	}

	string getInterfaceName() {
		return "";
	}
	string messageWhenStart() {
		return "";
	}
	string messageWhenMore() {
		return "";
	}
	string messageWhenMoreOther() {
		return "";
	}
	string messageWhenEnd() {
		return "";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "";
	}
	string messageWhenEndOther() {
		return "";
	}
	string messageWhenSavesOther() {
		return "";
	}
	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}
	void start() {
	}
	void end() {
	}

	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}
	bool allowAttackMleee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}
private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 1);
	}
};

class StatusDisabledAttackMelee: public StatusEffect {
public:
	StatusDisabledAttackMelee(Engine* const engine) :
		StatusEffect(statusDisabledMelee) {
		setTurnsFromRandomStandard(engine);
	}
	StatusDisabledAttackMelee(const int turns) :
		StatusEffect(turns, statusDisabledMelee) {
	}
	~StatusDisabledAttackMelee() {
	}

	StatusDisabledAttackMelee* copy() {
		StatusDisabledAttackMelee* cpy = new StatusDisabledAttackMelee(turnsLeft);
		return cpy;
	}

	virtual Abilities_t getSaveAbility() {
		return ability_empty;
	}
	virtual int getSaveAbilityModifier() {
		return -999;
	}

	string getInterfaceName() {
		return "";
	}
	string messageWhenStart() {
		return "";
	}
	string messageWhenMore() {
		return "";
	}
	string messageWhenMoreOther() {
		return "";
	}
	string messageWhenEnd() {
		return "";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "";
	}
	string messageWhenEndOther() {
		return "";
	}
	string messageWhenSavesOther() {
		return "";
	}
	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}
	void start() {
	}
	void end() {
	}

	bool allowAttackMleee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}
private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 1);
	}
};

class StatusDisabledAttackRanged: public StatusEffect {
public:
	StatusDisabledAttackRanged(Engine* const engine) :
		StatusEffect(statusDisabledRanged) {
		setTurnsFromRandomStandard(engine);
	}
	StatusDisabledAttackRanged(const int turns) :
		StatusEffect(turns, statusDisabledRanged) {
	}
	~StatusDisabledAttackRanged() {
	}

	StatusDisabledAttackRanged* copy() {
		StatusDisabledAttackRanged* cpy = new StatusDisabledAttackRanged(turnsLeft);
		return cpy;
	}

	virtual Abilities_t getSaveAbility() {
		return ability_empty;
	}
	virtual int getSaveAbilityModifier() {
		return -999;
	}

	string getInterfaceName() {
		return "";
	}
	string messageWhenStart() {
		return "";
	}
	string messageWhenMore() {
		return "";
	}
	string messageWhenMoreOther() {
		return "";
	}
	string messageWhenEnd() {
		return "";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "";
	}
	string messageWhenEndOther() {
		return "";
	}
	string messageWhenSavesOther() {
		return "";
	}
	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}
	void start() {
	}
	void end() {
	}

	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}
private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(0, 0, 1);
	}
};

class StatusParalyzed: public StatusEffect {
public:
	StatusParalyzed(Engine* const engine) :
		StatusEffect(statusParalyzed) {
		setTurnsFromRandomStandard(engine);
	}
	StatusParalyzed(const int turns) :
		StatusEffect(turns, statusParalyzed) {
	}
	~StatusParalyzed() {
	}

	StatusParalyzed* copy() {
		StatusParalyzed* cpy = new StatusParalyzed(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Paralyzed";
	}
	string messageWhenStart() {
		return "You are paralyzed!";
	}
	string messageWhenMore() {
		return "You are more paralyzed.";
	}
	string messageWhenMoreOther() {
		return "is more paralyzed.";
	}
	string messageWhenEnd() {
		return "You can move again!";
	}
	string messageWhenSaves() {
		return "You resist paralyzation.";
	}
	string messageWhenStartOther() {
		return "is paralyzed.";
	}
	string messageWhenEndOther() {
		return "can move again.";
	}
	string messageWhenSavesOther() {
		return "resists paralyzation.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusBodyAndSense;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	void start() {
	}

	void end() {
	}

	bool allowAct() {
		return false;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_dodge:
			return -999;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}
	bool allowAttackMleee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(1, 3, 6);
	}
};

class StatusFainted: public StatusEffect {
public:
	StatusFainted(Engine* const engine) :
		StatusEffect(statusFainted) {
		setTurnsFromRandomStandard(engine);
	}
	~StatusFainted() {
	}

	StatusFainted* copy() {
		StatusFainted* cpy = new StatusFainted(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Fainted";
	}
	string messageWhenStart() {
		return "You faint!";
	}
	string messageWhenMore() {
		return "You faint deeper.";
	}
	string messageWhenMoreOther() {
		return "faints deeper.";
	}
	string messageWhenEnd() {
		return "You are awake.";
	}
	string messageWhenSaves() {
		return "You resist fainting.";
	}
	string messageWhenStartOther() {
		return "faints.";
	}
	string messageWhenEndOther() {
		return "wakes up.";
	}
	string messageWhenSavesOther() {
		return "resists fainting.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusMindAndShock;
	}
	int getSaveAbilityModifier() {
		return -15;
	}

	void start() {
	}

	void end() {
	}

	bool allowAct() {
		return false;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_dodge:
			return -999;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}
	bool allowAttackMleee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		(void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
		return false;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

	void isHit() {
		turnsLeft = 0;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(1, 50, 50);
	}

	friend class StatusEffectsHandler;
	StatusFainted(const int turns) :
		StatusEffect(turns, statusFainted) {
	}
};

class StatusSlowed: public StatusEffect {
public:
	StatusSlowed(Engine* const engine) :
		StatusEffect(statusSlowed) {
		setTurnsFromRandomStandard(engine);
	}
	StatusSlowed(const int turns) :
		StatusEffect(turns, statusSlowed) {
	}
	~StatusSlowed() {
	}

	StatusSlowed* copy() {
		StatusSlowed* cpy = new StatusSlowed(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "Slowed";
	}
	string messageWhenStart() {
		return "Everything around you seems to speed up.";
	}
	string messageWhenMore() {
		return "You are more slowed.";
	}
	string messageWhenMoreOther() {
		return "slows down more.";
	}
	string messageWhenEnd() {
		return "Everything around you seems to slow down.";
	}
	string messageWhenSaves() {
		return "You resist slowness.";
	}
	string messageWhenStartOther() {
		return "slows down.";
	}
	string messageWhenEndOther() {
		return "speeds up.";
	}
	string messageWhenSavesOther() {
		return "resists slowness.";
	}

	Abilities_t getSaveAbility() {
		return ability_resistStatusBodyAndSense;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	void start() {
	}

	void end() {
	}

	bool isSlowed() {
		return true;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(3, 6, 6);
	}
};

class StatusPerfectReflexes: public StatusEffect {
public:
	StatusPerfectReflexes(Engine* const engine) :
		StatusEffect(statusPerfectReflexes) {
		setTurnsFromRandomStandard(engine);
	}
	StatusPerfectReflexes(const int turns) :
		StatusEffect(turns, statusPerfectReflexes) {
	}
	~StatusPerfectReflexes() {
	}

	StatusPerfectReflexes* copy() {
		StatusPerfectReflexes* cpy = new StatusPerfectReflexes(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "PerfReflex";
	}
	string messageWhenStart() {
		return "You have godlike reflexes!";
	}
	string messageWhenMore() {
		return "You have godlike reflexes!";
	}
	string messageWhenMoreOther() {
		return "has godlike reflexes.";
	}
	string messageWhenEnd() {
		return "Your reflexes are normal.";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "has godlike reflexes.";
	}
	string messageWhenEndOther() {
		return "has normal reflexes.";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_dodge:
			return 999;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	void start() {
	}

	void end() {
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(3, 8, 24);
	}
};

class StatusPerfectAim: public StatusEffect {
public:
	StatusPerfectAim(Engine* const engine) :
		StatusEffect(statusPerfectAim) {
		setTurnsFromRandomStandard(engine);
	}
	StatusPerfectAim(const int turns) :
		StatusEffect(turns, statusPerfectAim) {
	}
	~StatusPerfectAim() {
	}

	StatusPerfectAim* copy() {
		StatusPerfectAim* cpy = new StatusPerfectAim(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "PerfAiming";
	}
	string messageWhenStart() {
		return "You have godlike aiming!";
	}
	string messageWhenMore() {
		return "You have godlike aiming!";
	}
	string messageWhenMoreOther() {
		return "has godlike aiming.";
	}
	string messageWhenEnd() {
		return "Your aiming is normal.";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "has godlike aiming.";
	}
	string messageWhenEndOther() {
		return "has normal aiming.";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_accuracyMelee:
			return 999;
			break;
		case ability_accuracyRanged:
			return 999;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	void start() {
	}

	void end() {
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(3, 8, 24);
	}
};

class StatusPerfectStealth: public StatusEffect {
public:
	StatusPerfectStealth(Engine* const engine) :
		StatusEffect(statusPerfectStealth) {
		setTurnsFromRandomStandard(engine);
	}
	StatusPerfectStealth(const int turns) :
		StatusEffect(turns, statusPerfectStealth) {
	}
	~StatusPerfectStealth() {
	}

	StatusPerfectStealth* copy() {
		StatusPerfectStealth* cpy = new StatusPerfectStealth(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "PerfStealth";
	}
	string messageWhenStart() {
		return "You move silent like a ghost!";
	}
	string messageWhenMore() {
		return "You move silent like a ghost!";
	}
	string messageWhenMoreOther() {
		return "";
	}
	string messageWhenEnd() {
		return "Your movements make noises again.";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "";
	}
	string messageWhenEndOther() {
		return "";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_sneaking: return 999; break;
		default: {} break;
		}
		return 0;
	}

	void start() {
	}

	void end() {
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(8, 8, 100);
	}
};

class StatusPerfectFortitude: public StatusEffect {
public:
	StatusPerfectFortitude(Engine* const engine) :
		StatusEffect(statusPerfectFortitude) {
		setTurnsFromRandomStandard(engine);
	}
	StatusPerfectFortitude(const int turns) :
		StatusEffect(turns, statusPerfectFortitude) {
	}
	~StatusPerfectFortitude() {
	}

	StatusPerfectFortitude* copy() {
		StatusPerfectFortitude* cpy = new StatusPerfectFortitude(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "PerfFort";
	}
	string messageWhenStart() {
		return "You have godlike fortitude!";
	}
	string messageWhenMore() {
		return "You have godlike fortitude!";
	}
	string messageWhenMoreOther() {
		return "has godlike fortitude!";
	}
	string messageWhenEnd() {
		return "Your fortitude is normal.";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "has godlike fortitude!";
	}
	string messageWhenEndOther() {
		return "has normal fortitude.";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_resistStatusMindAndShock:
			return 999;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	void start() {
	}

	void end() {
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(3, 8, 24);
	}
};

class StatusPerfectToughness: public StatusEffect {
public:
	StatusPerfectToughness(Engine* const engine) :
		StatusEffect(statusPerfectToughness) {
		setTurnsFromRandomStandard(engine);
	}
	StatusPerfectToughness(const int turns) :
		StatusEffect(turns, statusPerfectToughness) {
	}
	~StatusPerfectToughness() {
	}

	StatusPerfectToughness* copy() {
		StatusPerfectToughness* cpy = new StatusPerfectToughness(turnsLeft);
		return cpy;
	}

	string getInterfaceName() {
		return "PerfTough";
	}
	string messageWhenStart() {
		return "You have godlike toughness!";
	}
	string messageWhenMore() {
		return "You have godlike toughness!";
	}
	string messageWhenMoreOther() {
		return "has godlike toughness!";
	}
	string messageWhenEnd() {
		return "Your toughness is normal.";
	}
	string messageWhenSaves() {
		return "";
	}
	string messageWhenStartOther() {
		return "has godlike toughness!";
	}
	string messageWhenEndOther() {
		return "has normal toughness.";
	}
	string messageWhenSavesOther() {
		return "";
	}

	Abilities_t getSaveAbility() {
		return ability_empty;
	}
	int getSaveAbilityModifier() {
		return 0;
	}

	int getAbilityModifier(const Abilities_t ability) {
		switch(ability) {
		case ability_resistStatusBodyAndSense:
			return 999;
			break;
		default: {
		}
		break;
		}
		return 0;
	}

	void start() {
	}

	void end() {
	}

	bool isSlowed() {
		return true;
	}

	void newTurn(Engine* engine) {
		(void)engine;
		turnsLeft--;
	}

private:
	DiceParam getRandomStandardNrTurns() {
		return DiceParam(3, 8, 24);
	}
};

//Every actor has a StatusEffectHandler member. This class
//contains a vector of StatusEffect. It handles communication
//between the effects and the actor, and recieves attempt
//requests to add new status effects to the actor.
class StatusEffectsHandler {
public:
	StatusEffectsHandler(Actor* const owningActor_, Engine* engine) :
		owningActor(owningActor_), eng(engine) {
		effects.resize(0);
	}
	~StatusEffectsHandler() {
		for(unsigned int i = 0; i < effects.size(); i++) {
			delete effects.at(i);
		}
		effects.resize(0);
	}

	//This function was created so that the id could be saved to file when saving the game,
	//in other cases, a status effect instance should be created simply using "new"
	StatusEffect* makeEffectFromId(const StatusEffects_t id, const int TURNS_LEFT);

	void attemptAddEffect(StatusEffect* const effect, const bool FORCE_EFFECT = false, const bool NO_MESSAGES = false);

	void attemptAddEffectsFromWeapon(Weapon* weapon, const bool IS_MELEE);

	coord changeMoveCoord(const coord actorPos, const coord movePos) {
		coord ret = movePos;
		for(unsigned int i = 0; i < effects.size(); i++) {
			ret = effects.at(i)->changeMoveCoord(actorPos, movePos, eng);
		}
		return ret;
	}

	bool allowAttack(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		for(unsigned int i = 0; i < effects.size(); i++)
			if(effects.at(i)->allowAttackMelee(ALLOW_PRINT_MESSAGE_WHEN_FALSE) == false && effects.at(i)->allowAttackRanged(
			         ALLOW_PRINT_MESSAGE_WHEN_FALSE) == false)
				return false;
		return true;
	}

	bool allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		for(unsigned int i = 0; i < effects.size(); i++)
			if(effects.at(i)->allowAttackMelee(ALLOW_PRINT_MESSAGE_WHEN_FALSE) == false)
				return false;
		return true;
	}

	bool allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
		for(unsigned int i = 0; i < effects.size(); i++) {
			if(effects.at(i)->allowAttackRanged(ALLOW_PRINT_MESSAGE_WHEN_FALSE) == false) {
				return false;
			}
		}
		return true;
	}

	bool isSlowed() {
		for(unsigned int i = 0; i < effects.size(); i++) {
			if(effects.at(i)->isSlowed() == true) {
				return true;
			}
		}
		return false;
	}

	bool allowSee();

	bool allowMove() {
		for(unsigned int i = 0; i < effects.size(); i++)
			if(effects.at(i)->allowMove() == false)
				return false;
		return true;
	}

	void isHit() {
		for(unsigned int i = 0; i < effects.size(); i++) {
			effects.at(i)->isHit();
		}
	}

	bool allowAct() {
		for(unsigned int i = 0; i < effects.size(); i++)
			if(effects.at(i)->allowAct() == false)
				return false;
		return true;
	}

	int getAbilityModifier(const Abilities_t ability) {
		int modifier = 0;
		for(unsigned int i = 0; i < effects.size(); i++)
			modifier += effects.at(i)->getAbilityModifier(ability);
		return modifier;
	}

	bool isEthereal() {
		for(unsigned int i = 0; i < effects.size(); i++)
			if(effects.at(i)->isEthereal() == true)
				return true;
		return false;
	}

	string getStatusLine() {
		string line = "";
		for(unsigned int i = 0; i < effects.size(); i++)
			line += effects.at(i)->getInterfaceName() + " ";
		return line;
	}

	bool hasEffect(const StatusEffects_t effect) {
		for(unsigned int i = 0; i < effects.size(); i++) {
			if(effects.at(i)->getEffectId() == effect) {
				return true;
			}
		}
		return false;
	}

	void endEffect(const StatusEffects_t effect) {
		for(unsigned int i = 0; i < effects.size(); i++) {
			if(effects.at(i)->getEffectId() == effect) {
				effects.at(i)->turnsLeft = 0;
			}
		}
	}

	void endEffectsOfAbility(const Abilities_t ability) {
		for(unsigned int i = 0; i < effects.size(); i++) {
			if(effects.at(i)->getSaveAbility() == ability) {
				effects.at(i)->turnsLeft = 0;
			}
		}
	}

	vector<StatusEffect*> effects;

private:

	Actor* owningActor;
	Engine* eng;

	friend class GameTime;
	void newTurnAllEffects();
};

#endif

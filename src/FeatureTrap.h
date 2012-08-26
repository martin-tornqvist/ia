#ifndef FEATURE_TRAPS_H
#define FEATURE_TRAPS_H

#include "Feature.h"
#include "AbilityValues.h"
#include "Art.h"
#include "Feature.h"
#include "ConstTypes.h"

class Engine;
class SpecificTrapBase;
class TrapSpawnData;

enum Trap_t {
	trap_blinding,
	trap_dart,
	trap_gasConfusion,
	trap_gasFear,
	trap_gasParalyze,
	trap_smoke,
	trap_spear,
	trap_spiderWeb,
	trap_teleport,
	endOfTraps,
	trap_any
};

class Trap: public FeatureStatic {
public:
	~Trap() {
	}

	void bump(Actor* actorBumping);
	SDL_Color getColor() const;
	char getGlyph() const;
	Tile_t getTile() const;
	string getDescription(const bool DEFINITE_ARTICLE) const;
	bool canHaveBlood() const;
	bool canHaveGore() const;
	bool canHaveCorpse() const;
	bool canHaveItem() const;


	void triggerOnPurpose(Actor* actorTriggering);

	void reveal(const bool PRINT_MESSSAGE);

	bool isHidden() const {
		return isHidden_;
	}

	coord actorAttemptLeave(Actor* const actor, const coord& pos, const coord& dest);

protected:
	friend class FeatureFactory;
	Trap(Feature_t id, coord pos, Engine* engine, TrapSpawnData* spawnData);

	void setSpecificTrapFromId(const Trap_t id);

	const FeatureDef* const mimicFeature_;
	bool isHidden_;
	SpecificTrapBase* specificTrap_;

	friend class Player;
	void playerTrySpotHidden();
};

class SpecificTrapBase {
public:

protected:
	friend class Trap;
	SpecificTrapBase(coord pos, Engine* engine) :
		pos_(pos), eng(engine) {
	}
	SpecificTrapBase() {
	}
	~SpecificTrapBase() {
	}

	virtual coord specificTrapActorAttemptLeave(Actor* const actor, const coord& pos, const coord& dest) {
		(void)actor;
		(void)pos;
		return dest;
	}

	virtual void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) = 0;
	virtual string getTrapSpecificTitle() const = 0;
	virtual SDL_Color getTrapSpecificColor() const = 0;
	virtual char getTrapSpecificGlyph() const = 0;
	virtual Tile_t getTrapSpecificTile() const = 0;

	coord pos_;
	Engine* eng;
};

class TrapDart: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapDart(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrWhiteHigh;
	}
	string getTrapSpecificTitle() const {
		return "Dart trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapSpear: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapSpear(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrWhiteHigh;
	}
	string getTrapSpecificTitle() const {
		return "Spear trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapGasConfusion: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapGasConfusion(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrCyan;
	}
	string getTrapSpecificTitle() const {
		return "Gas trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapGasParalyzation: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapGasParalyzation(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrCyan;
	}
	string getTrapSpecificTitle() const {
		return "Gas trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapGasFear: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapGasFear(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrCyan;
	}
	string getTrapSpecificTitle() const {
		return "Gas trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapBlindingFlash: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapBlindingFlash(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrYellow;
	}
	string getTrapSpecificTitle() const {
		return "Blinding trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapTeleport: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapTeleport(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrMagenta;
	}
	string getTrapSpecificTitle() const {
		return "Teleporter trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapSmoke: public SpecificTrapBase {
public:
private:
	friend class Trap;
	TrapSmoke(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrGray;
	}
	string getTrapSpecificTitle() const {
		return "Smoke trap";
	}
	char getTrapSpecificGlyph() const {
		return '^';
	}
	Tile_t getTrapSpecificTile() const {
		return tile_trapGeneral;
	}
};

class TrapSpiderWeb: public SpecificTrapBase {
public:
	coord specificTrapActorAttemptLeave(Actor* const actor, const coord& pos, const coord& dest);
private:
	friend class Trap;
	TrapSpiderWeb(coord pos, Engine* engine) :
		SpecificTrapBase(pos, engine) {
	}
	void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
	SDL_Color getTrapSpecificColor() const {
		return clrWhiteHigh;
	}
	string getTrapSpecificTitle() const {
		return "Spider web";
	}
	char getTrapSpecificGlyph() const {
		return '*';//6;
	}
	Tile_t getTrapSpecificTile() const {
		return tile_spiderWeb;
	}

	bool isHoldingActor;
};

#endif

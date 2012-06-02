#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <vector>

#include "SDL/SDL.h"

#include "ConstTypes.h"

#include "ActorData.h"
#include "SpawnCounter.h"
#include "StatusEffects.h"
#include "Sound.h"
#include "Config.h"
#include "MapTests.h"
#include "Art.h"

using namespace std;

class Engine;

class TimedEntity;
class Inventory;

class Actor: public SpawnCounter<Actor> {
public:
	Actor() {
	}

	StatusEffectsHandler* getStatusEffectsHandler() {
		return m_statusEffectsHandler;
	}

	ActorDefinition* getInstanceDefinition() {
		m_instanceDefinition.nrOfKills = m_archetypeDefinition->nrOfKills;
		return &m_instanceDefinition;
	}

	ActorDefinition* getArchetypeDefinition() {
		m_instanceDefinition.nrOfKills = m_archetypeDefinition->nrOfKills;
		return m_archetypeDefinition;
	}

	virtual ~Actor();

	coord pos;
	ActorDeadState_t deadState;

	Inventory* getInventory() {
		return m_inventory;
	}

	void place(const coord pos_, ActorDefinition* const actorDefinition, Engine* engine);

	bool hit(int dmg, const DamageTypes_t damageType);

	bool restoreHP(int hpRestored, const bool ALLOW_MESSAGE = true);

	//void boostHP(int hpBoosted) {m_instanceDefinition.HP += hpBoosted;}

	void changeMaxHP(const int increase, const bool ALLOW_MESSAGES);

	void die(const bool MANGLED, const bool ALLOW_GORE, const bool ALLOW_DROP_ITEMS);

	void newTurn();

	virtual void act() = 0;

	virtual void registerHeardSound(const Sound& sound) = 0;

	virtual void resetColor();

	//Function taking account FOV, invisibility, status, etc
	//This is the final word on wether an actor can visually percieve another actor.
	bool checkIfSeeActor(const Actor& other, bool visionBlockingCells[MAP_X_CELLS][MAP_Y_CELLS]) const;

	//Various "shortcuts" to the instance definition
	int getHP() const {
		return m_instanceDefinition.HP;
	}
	int getHP_max() const {
		return m_instanceDefinition.HP_max;
	}
	string getNameThe() const {
		return m_instanceDefinition.name_the;
	}
	string getNameA() const {
		return m_instanceDefinition.name_a;
	}
	bool isHumanoid() const {
		return m_instanceDefinition.isHumanoid;
	}
	char getGlyph() const {
		return m_instanceDefinition.glyph;
	}
	const SDL_Color& getColor() const {
		return m_instanceDefinition.color;
	}
	void setColor(const SDL_Color color) {
		m_instanceDefinition.color = color;
	}
	const Tile_t& getTile() const {
		return m_instanceDefinition.tile;
	}

	MoveType_t getMoveType() const {
		return m_instanceDefinition.moveType;
	}

	Engine* eng;

	void teleportToRandom();

protected:
	virtual void actorSpecificDie() {
	}

	virtual void actorSpecific_hit(const int DMG) {
		(void)DMG;
	}

	virtual void actorSpecific_spawnStartItems() = 0;

	//Called from within the normal hit function to set monsters playerAwareness
	virtual void monsterHit() {
	}

	//Monsters may have special stuff happening when they die (such as fire vampire explosion)
	virtual void monsterDeath() {

	}

	coord lairCell;

	StatusEffectsHandler* m_statusEffectsHandler;

	ActorDefinition m_instanceDefinition;
	ActorDefinition* m_archetypeDefinition;

	Inventory* m_inventory;

	friend class AbilityValues;
};

#endif

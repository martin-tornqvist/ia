#include "Actor.h"

#include "Engine.h"
#include "ItemArmor.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Fov.h"
#include "PlayerVisualMemory.h"
#include "Log.h"
#include "Blood.h"
#include "FeatureTrap.h"
#include "ItemDrop.h"
#include "Explosion.h"
#include "DungeonMaster.h"
#include "Inventory.h"

using namespace std;

Actor::~Actor() {
	delete m_statusEffectsHandler;
	delete m_inventory;
}

void Actor::newTurn() {
	if(m_statusEffectsHandler->allowAct() == false) {
		eng->gameTime->letNextAct();
	} else {
		act();
	}
}

bool Actor::checkIfSeeActor(const Actor& other, bool visionBlockingCells[MAP_X_CELLS][MAP_Y_CELLS]) const {
	if(other.deadState == actorDeadState_alive) {
		if(this == &other)
			return true;

		if(this == eng->player) {
			const bool IS_MONSTER_SNEAKING = dynamic_cast<const Monster*>(&other)->isSneaking;
			return eng->map->playerVision[other.pos.x][other.pos.y] && IS_MONSTER_SNEAKING == false;
		}

		if(dynamic_cast<const Monster*>(this)->leader == eng->player && &other != eng->player) {
			const bool IS_MONSTER_SNEAKING = dynamic_cast<const Monster*>(&other)->isSneaking;
			if(IS_MONSTER_SNEAKING) return false;
		}

		if(m_statusEffectsHandler->allowSee() == false) {
			return false;
		}

		if(pos.x - other.pos.x > FOV_RADI_INT) return false;
		if(other.pos.x - pos.x > FOV_RADI_INT)	return false;
		if(other.pos.y - pos.y > FOV_RADI_INT)	return false;
		if(pos.y - other.pos.y > FOV_RADI_INT)	return false;

		if(visionBlockingCells != NULL) {
			return eng->fov->checkOneCell(visionBlockingCells, other.pos.x, other.pos.y, pos.x, pos.y, true);
		}
	}
	return false;
}

void Actor::getSpotedEnemies() {
	spotedEnemies.resize(0);

	const bool IS_SELF_PLAYER = this == eng->player;

	bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];

	if(IS_SELF_PLAYER == false) {
		eng->mapTests->makeVisionBlockerArray(visionBlockers);
	}

	const unsigned int SIZE_OF_LOOP = eng->gameTime->getLoopSize();
	for(unsigned int i = 0; i < SIZE_OF_LOOP; i++) {
		Actor* const actor = eng->gameTime->getActorAt(i);
		if(actor != this && actor->deadState == actorDeadState_alive) {

			if(IS_SELF_PLAYER) {
				if(dynamic_cast<Monster*>(actor)->leader != this) {
					if(checkIfSeeActor(*actor, NULL)) {
						spotedEnemies.push_back(actor);
					}
				}
			} else {
				const bool IS_OTHER_PLAYER = actor == eng->player;
				const bool IS_SELF_HOSTILE_TO_PLAYER = dynamic_cast<Monster*>(this)->leader != eng->player;
				const bool IS_OTHER_HOSTILE_TO_PLAYER = IS_OTHER_PLAYER ? false : dynamic_cast<Monster*>(actor)->leader != eng->player;

				//Note that IS_OTHER_HOSTILE_TO_PLAYER is false if the other IS the player,
				//so there is no need to check if IS_SELF_HOSTILE_TO_PLAYER && IS_OTHER_PLAYER
				if(
				   (IS_SELF_HOSTILE_TO_PLAYER == true && IS_OTHER_HOSTILE_TO_PLAYER == false) ||
				   (IS_SELF_HOSTILE_TO_PLAYER == false && IS_OTHER_HOSTILE_TO_PLAYER == true)) {
					if(checkIfSeeActor(*actor, visionBlockers)) {
						spotedEnemies.push_back(actor);
					}
				}
			}
		}
	}
}

void Actor::getSpotedEnemiesPositions() {
	spotedEnemiesPositions.resize(0);
	getSpotedEnemies();

	const unsigned int SIZE_OF_LOOP = spotedEnemies.size();
	for(unsigned int i = 0; i < SIZE_OF_LOOP; i++) {
		const Actor* const actor = spotedEnemies.at(i);
		if(actor != NULL) {
			spotedEnemiesPositions.push_back(actor->pos);
		}
	}
}

void Actor::place(const coord pos_, ActorDefinition* const actorDefinition, Engine* engine) {
	eng = engine;
	pos = pos_;
	m_archetypeDefinition = actorDefinition;
	m_instanceDefinition = *actorDefinition;
	m_inventory = new Inventory(m_instanceDefinition.isHumanoid);
	m_statusEffectsHandler = new StatusEffectsHandler(this, eng);
	deadState = actorDeadState_alive;
	lairCell = pos;

	m_instanceDefinition.abilityValues.setOwningActor(this);

	if(this != eng->player) {
		actorSpecific_spawnStartItems();
	}
}

void Actor::teleportToRandom() {
	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeMoveBlockerArray(this, blockers);
	eng->basicUtils->reverseBoolArray(blockers);
	vector<coord> freeCells;
	eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
	const coord CELL = freeCells.at(eng->dice(1, freeCells.size()) - 1);

	if(this == eng->player) {
		eng->player->FOVupdate();
		eng->renderer->drawMapAndInterface();
		eng->playerVisualMemory->updateVisualMemory();
	}

	pos = CELL;

	if(this == eng->player) {
		eng->player->FOVupdate();
		eng->renderer->drawMapAndInterface();
		eng->playerVisualMemory->updateVisualMemory();
		eng->log->addMessage("You suddenly find yourself in a different location!");
		m_statusEffectsHandler->attemptAddEffect(new StatusConfused(8 + eng->dice(1, 8)));
	}
}

void Actor::resetColor() {
	if(deadState == actorDeadState_alive) {
		m_instanceDefinition.color = m_archetypeDefinition->color;
	} else {
		m_instanceDefinition.color = clrRedLight;
	}
}

bool Actor::restoreHP(int hpRestored, const bool ALLOW_MESSAGE) {
	bool gainedHP = false;

	int difFromMax = m_instanceDefinition.HP_max - hpRestored;

	//If hp is below limit, but restored hp will push it
	//over the limit - hp is set to max.
	if(getHP() > difFromMax && getHP() < getHP_max()) {
		m_instanceDefinition.HP = getHP_max();
		gainedHP = true;
	}

	//If hp is below limit, and restored hp will NOT push it
	//over the limit - restored hp is added to current.
	if(getHP() <= difFromMax) {
		m_instanceDefinition.HP += hpRestored;
		gainedHP = true;
	}

	if(ALLOW_MESSAGE) {
		if(gainedHP) {
			if(this == eng->player) {
				eng->log->addMessage("You feel healthier!", clrMessageGood);
			} else {
				if(eng->player->checkIfSeeActor(*this, NULL)) {
					eng->log->addMessage(m_instanceDefinition.name_the + " looks healthier.");
				}
			}
			eng->renderer->drawMapAndInterface();
		}
	}

	return gainedHP;
}

void Actor::changeMaxHP(const int change, const bool ALLOW_MESSAGES) {
	m_instanceDefinition.HP_max += change;
	m_instanceDefinition.HP += change;

	m_instanceDefinition.HP_max = max(1, getHP_max());
	m_instanceDefinition.HP = max(1, getHP());

	if(ALLOW_MESSAGES == true) {
		if(this == eng->player) {
			if(change > 0)
				eng->log->addMessage("You feel more vigorous!", clrMessageGood);
			if(change < 0)
				eng->log->addMessage("You feel frailer!", clrMessageBad);
		} else {
			if(eng->map->playerVision[pos.x][pos.y] == true) {
				if(change > 0)
					eng->log->addMessage(getNameThe() + " looks more vigorous.");
				if(change < 0)
					eng->log->addMessage(getNameThe() + " looks frailer.");
			}
		}
	}
}

bool Actor::hit(int dmg, const DamageTypes_t damageType) {
	monsterHit();

	dmg = max(1, dmg);

	//Filter damage through worn armor
	if(isHumanoid() == true) {
		Armor* const armor = dynamic_cast<Armor*>(m_inventory->getItemInSlot(slot_armorBody));
		if(armor != NULL) {
			dmg = armor->takeDurabilityHitAndGetReducedDamage(dmg, damageType);
		}
	}

	//Filter damage through intrinsic armor
	//Stuff goes here***


	m_statusEffectsHandler->isHit();

	actorSpecific_hit(dmg);

	//Damage to corpses
	if(deadState != actorDeadState_alive) {
		if(dmg >= getHP_max() / 2) {
			deadState = actorDeadState_mangled;
			m_instanceDefinition.glyph = ' ';
			if(isHumanoid()) {
				eng->gore->makeGore(pos);
			}
		}
		return false;
	}

	if(this != eng->player || eng->config->BOT_PLAYING == false) {
		m_instanceDefinition.HP -= dmg;
	}

	const bool IS_ON_BOTTOMLESS = eng->map->featuresStatic[pos.x][pos.y]->isBottomless();
	const bool IS_MANGLED = IS_ON_BOTTOMLESS == true ? true : (dmg > ((getHP_max() * 5) / 4) ? true : false);
	if(getHP() <= 0) {
		die(IS_MANGLED, !IS_ON_BOTTOMLESS, !IS_ON_BOTTOMLESS);
		actorSpecificDie();
		return true;
	} else {
		return false;
	}
}

void Actor::die(const bool MANGLED, const bool ALLOW_GORE, const bool ALLOW_DROP_ITEMS) {
	if(this != eng->player) {
		if(isHumanoid() == true) {
			eng->soundEmitter->emitSound(Sound("You hear agonised screaming.", true, pos, 3, false));
		}
	}

	bool diedOnVisibleTrap = false;

	//If died on a visible trap, always destroy the corpse
	const Feature* const f = eng->map->featuresStatic[pos.x][pos.y];
	if(f->getId() == feature_trap) {
		if(dynamic_cast<const Trap*>(f)->isHidden() == false) {
			diedOnVisibleTrap = true;
		}
	}

	//Print death messages
	if(this != eng->player) {
		//Only print if visible
		if(eng->player->checkIfSeeActor(*this, NULL)) {
			const string deathMessageOverride = m_instanceDefinition.deathMessageOverride;
			if(deathMessageOverride != "") {
				eng->log->addMessage(deathMessageOverride);
			} else {
				eng->log->addMessage(getNameThe() + " dies.");
			}
		}
	}

	//If mangled because of damage, or if a monster died on a visible trap, gib the corpse.
	deadState = (MANGLED == true || (diedOnVisibleTrap == true && this != eng->player)) ? actorDeadState_mangled : actorDeadState_corpse;

	if(ALLOW_DROP_ITEMS) {
		eng->itemDrop->dropAllCharactersItems(this, true);
	}

	//Died with a flare inside?
	if(m_instanceDefinition.actorSize > actorSize_floor) {
		if(m_statusEffectsHandler->hasEffect(statusFlared)) {
			eng->explosionMaker->runExplosion(pos, false, new StatusBurning(eng));
			deadState = actorDeadState_mangled;
		}
	}

	if(MANGLED == false) {
		coord newCoord;
		Feature* f = eng->map->featuresStatic[pos.x][pos.y];
		//TODO this should be decided with a floodfill instead
		if(f->canHaveCorpse() == false) {
			for(int dx = -1; dx <= 1; dx++) {
				for(int dy = -1; dy <= 1; dy++) {
					newCoord = pos + coord(dx, dy);
					f = eng->map->featuresStatic[pos.x + dx][pos.y + dy];
					if(f->canHaveCorpse() == true) {
						pos.set(newCoord);
						dx = 9999;
						dy = 9999;
					}
				}
			}
		}
		m_instanceDefinition.glyph = '&';
		m_instanceDefinition.tile = tile_corpse;
	} else {
		m_instanceDefinition.glyph = ' ';
		m_instanceDefinition.tile = tile_empty;
		if(isHumanoid() == true) {
			if(ALLOW_GORE) {
				eng->gore->makeGore(pos);
			}
		}
	}

	m_instanceDefinition.color = clrRedLight;

	monsterDeath();

	//Give exp if monster, and count up nr of kills.
	if(this != eng->player) {
		eng->dungeonMaster->monsterKilled(this);
	}
}


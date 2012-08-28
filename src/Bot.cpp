#include "Bot.h"

#include <cassert>

#include "Engine.h"
#include "StatusEffects.h"
#include "Actor.h"
#include "Feature.h"
#include "Input.h"
#include "Map.h"
#include "ActorPlayer.h"
#include "ActorFactory.h"
#include "Attack.h"
#include "ItemWeapon.h"
#include "FeatureDoor.h"
#include "Pathfinding.h"
#include "Inventory.h"

//TODO reimplement

void Bot::init() {
	currentPath_.resize(0);

	if(eng->config->BOT_PLAYING == true) {
		//Switch to melee weapon
		eng->input->handleKeyPress('z', false, false);
		//Make sure it's my turn
		while(eng->gameTime->getCurrentActor() != eng->player) {
			eng->gameTime->letNextAct();
		}
		runFunctionTests();
	}
}

void Bot::runFunctionTests() {
	//TEST HIT CHANCE
	Item* weaponUsed = eng->player->getInventory()->getItemInSlot(slot_wielded);
	const Abilities_t weaponAbilityUsed = weaponUsed->getInstanceDefinition().meleeAbilityUsed;
	const int HIT_CHANCE_WEAPON = weaponUsed->getInstanceDefinition().meleeBaseAttackSkill;
	const int HIT_CHANCE_SKILL = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(weaponAbilityUsed, true);
	const int HIT_CHANCE_TOTAL = HIT_CHANCE_SKILL + HIT_CHANCE_WEAPON;
	const int NUMBER_OF_ATTACKS = 100;
	double hitChanceReal = 0;
	for(int i = 0; i < NUMBER_OF_ATTACKS; i++) {
		Actor* actor = eng->actorFactory->spawnActor(actor_rat, eng->player->pos + coord(1, 0));
		eng->attack->melee(eng->player->pos.x + 1, eng->player->pos.y, dynamic_cast<Weapon*> (weaponUsed));
		if(actor->getHP() < actor->getHP_max()) {
			hitChanceReal++;
		}
		for(unsigned int ii = 0; ii < eng->gameTime->getLoopSize(); ii++) {
			if(eng->gameTime->getActorAt(ii) == actor) {
				eng->gameTime->eraseElement(ii);
				break;
			}
		}
		delete actor;

		while(eng->gameTime->getCurrentActor() != eng->player) {
			eng->gameTime->letNextAct();
		}
	}
	hitChanceReal /= NUMBER_OF_ATTACKS;
	hitChanceReal *= 100;
	const double RATIO = static_cast<double> (HIT_CHANCE_TOTAL) / hitChanceReal;

	assert( RATIO > 0.95 && RATIO < 1.05 );
}

void Bot::act() {

	const int PLAY_TO_DLVL = LAST_CAVERN_LEVEL;

	if(eng->map->getDungeonLevel() >= PLAY_TO_DLVL) {
		eng->config->BOT_PLAYING = false;
	} else {
		const int ACTION_DELAY = 1;

		//Attempt to use stairs
		if(eng->map->featuresStatic[eng->player->pos.x][eng->player->pos.y]->getId() == feature_stairsDown) {
			eng->input->handleKeyPress('>', false, false);
			return;
		}

		//To simplify things, adjacent doors get revealed
		for(int dx = -1; dx <= 1; dx++) {
			for(int dy = -1; dy <= 1; dy++) {
				FeatureStatic* f = eng->map->featuresStatic[eng->player->pos.x + dx][eng->player->pos.y + dy];
				if(f->getId() == feature_door) {
					dynamic_cast<Door*> (f)->reveal(false);
					//If adjacent door is stuck, bash at it
					if(dynamic_cast<Door*> (f)->isStuck()) {
						dynamic_cast<Door*> (f)->tryBash(eng->player);
						SDL_Delay(ACTION_DELAY);
						return;
					}
				}
			}
		}

		//If afraid, wait it out
		if(eng->player->getStatusEffectsHandler()->hasEffect(statusTerrified) == true) {
			if(walkToAdjacentCell(eng->player->pos) == true) {
				SDL_Delay(ACTION_DELAY);
				return;
			}
		}

		findPathToNextStairs();

		if(currentPath_.empty() == false) {
			const coord nextCell = currentPath_.at(currentPath_.size() - 1);
			if(walkToAdjacentCell(nextCell)) {
				SDL_Delay(ACTION_DELAY);
				return;
			}
		}

		// Try to pick something up
		eng->input->handleKeyPress('g', false, false);
		currentPath_.resize(currentPath_.size() - 1);

		SDL_Delay(botDelay_);
	}
}

bool Bot::walkToAdjacentCell(const coord& cell) {
	bool playerWalkedIntoCell = false;

	coord playerCell(eng->player->pos);

	//Get relative coordinates
	int xRel = 0;
	int yRel = 0;

	if(cell.x > playerCell.x)
		xRel = 1;
	if(cell.x < playerCell.x)
		xRel = -1;

	if(cell.y > playerCell.y)
		yRel = 1;
	if(cell.y < playerCell.y)
		yRel = -1;

	if(xRel == 0 && yRel == 0)
		eng->input->handleKeyPress('5', false, false);
	if(xRel == 1 && yRel == 0)
		eng->input->handleKeyPress('6', false, false);
	if(xRel == 1 && yRel == -1)
		eng->input->handleKeyPress('9', false, false);
	if(xRel == 0 && yRel == -1)
		eng->input->handleKeyPress('8', false, false);
	if(xRel == -1 && yRel == -1)
		eng->input->handleKeyPress('7', false, false);
	if(xRel == -1 && yRel == 0)
		eng->input->handleKeyPress('4', false, false);
	if(xRel == -1 && yRel == 1)
		eng->input->handleKeyPress('1', false, false);
	if(xRel == 0 && yRel == 1)
		eng->input->handleKeyPress('2', false, false);
	if(xRel == 1 && yRel == 1)
		eng->input->handleKeyPress('3', false, false);

	playerCell.x = eng->player->pos.x;
	playerCell.y = eng->player->pos.y;

	if(playerCell.x == cell.x && playerCell.y == cell.y)
		playerWalkedIntoCell = true;

	return playerWalkedIntoCell;
}

coord Bot::findNextStairs() {
	for(int x = 0; x < MAP_X_CELLS; x++) {
		for(int y = 0; y < MAP_Y_CELLS; y++) {
			FeatureStatic* f = eng->map->featuresStatic[x][y];
			if(f->getId() == feature_stairsDown) {
				return coord(x, y);
			}
		}
	}
	return coord(-1, -1);
}

void Bot::findPathToNextStairs() {
	currentPath_.resize(0);

	const coord stairPos = findNextStairs();

	if(stairPos == coord(-1, -1)) {
	} else {
		bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
		eng->mapTests->makeMoveBlockerArray(eng->player, blockers);
		//Consider all doors passable
		for(int x = 0; x < MAP_X_CELLS; x++) {
			for(int y = 0; y < MAP_Y_CELLS; y++) {
				FeatureStatic* f = eng->map->featuresStatic[x][y];
				if(f->getId() == feature_door) {
					blockers[x][y] = false;
				}
			}
		}
		currentPath_ = eng->pathfinder->findPath(eng->player->pos, blockers, stairPos);
	}
}


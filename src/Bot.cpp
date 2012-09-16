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
#include "FeatureFactory.h"
#include "ActorMonster.h"

//TODO reimplement

void Bot::init() {
	currentPath_.resize(0);

	if(eng->config->BOT_PLAYING == true) {
		//Switch to melee weapon
//		eng->input->handleKeyPress('z', false, false);
		//Make sure it's my turn
//		while(eng->gameTime->getCurrentActor() != eng->player) {
//			eng->gameTime->letNextAct();
//		}
//		runFunctionTests();
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

	//Make sure an actor can be spawned next to the player
	eng->featureFactory->spawnFeatureAt(feature_stoneFloor, eng->player->pos + coord(1,0));

	for(int i = 0; i < NUMBER_OF_ATTACKS; i++) {
		Actor* actor = eng->actorFactory->spawnActor(actor_rat, eng->player->pos + coord(1, 0));
		dynamic_cast<Monster*>(actor)->playerAwarenessCounter = 999;
		eng->attack->melee(eng->player->pos.x + 1, eng->player->pos.y, dynamic_cast<Weapon*>(weaponUsed));
		if(actor->getHP() < actor->getHP_max()) {
			hitChanceReal += 1.0;
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
	hitChanceReal /= static_cast<double>(NUMBER_OF_ATTACKS);
	hitChanceReal *= 100;
	const double RATIO = static_cast<double>(HIT_CHANCE_TOTAL) / hitChanceReal;

	assert(RATIO > 0.80 && RATIO < 1.20);
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

		//To simplify things, adjacent doors gets revealed
		for(int dx = -1; dx <= 1; dx++) {
			for(int dy = -1; dy <= 1; dy++) {
				FeatureStatic* f = eng->map->featuresStatic[eng->player->pos.x + dx][eng->player->pos.y + dy];
				if(f->getId() == feature_door) {
					dynamic_cast<Door*>(f)->reveal(false);
					//If adjacent door is stuck, bash at it
					if(dynamic_cast<Door*>(f)->isStuck()) {
						dynamic_cast<Door*>(f)->tryBash(eng->player);
						SDL_Delay(ACTION_DELAY);
						return;
					}
				}
			}
		}

		//If afraid, wait it out
		if(eng->player->getStatusEffectsHandler()->hasEffect(statusTerrified) == true) {
			if(walkToAdjacentCell(eng->player->pos)) {
				SDL_Delay(ACTION_DELAY);
				return;
			}
		}

		findPathToNextStairs();

      const coord nextCell = currentPath_.back();

      walkToAdjacentCell(nextCell);

//      assert(eng->gameTime->getCurrentActor() != eng->player);

		SDL_Delay(botDelay_);
	}
}

bool Bot::walkToAdjacentCell(const coord& cellToGoTo) {

	coord playerCell(eng->player->pos);

	assert(eng->mapTests->isCellsNeighbours(playerCell, cellToGoTo, true));

	//Get relative coordinates
   const int xRel = cellToGoTo.x > playerCell.x ? 1 : cellToGoTo.x < playerCell.x ? -1 : 0;
   const int yRel = cellToGoTo.y > playerCell.y ? 1 : cellToGoTo.y < playerCell.y ? -1 : 0;

   if(cellToGoTo != playerCell) {
      assert(xRel != 0 || yRel != 0);
   }

   char key = ' ';

	if(xRel == 0 && yRel == 0)
      key = '5';
	if(xRel == 1 && yRel == 0)
		key = '6';
	if(xRel == 1 && yRel == -1)
		key = '9';
	if(xRel == 0 && yRel == -1)
		key = '8';
	if(xRel == -1 && yRel == -1)
		key = '7';
	if(xRel == -1 && yRel == 0)
		key = '4';
	if(xRel == -1 && yRel == 1)
		key = '1';
	if(xRel == 0 && yRel == 1)
		key = '2';
	if(xRel == 1 && yRel == 1)
		key = '3';

   assert(key >= '0' && key <= '9');

   eng->input->handleKeyPress(key, false, false);

	return playerCell == cellToGoTo;
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

	assert(stairPos != coord(-1, -1));

	bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
	eng->mapTests->makeMoveBlockerArrayFeaturesOnly(eng->player, blockers);

	//Consider all doors passable
   for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
			FeatureStatic* f = eng->map->featuresStatic[x][y];
			if(f->getId() == feature_door) {
				blockers[x][y] = false;
			}
		}
	}
	currentPath_ = eng->pathfinder->findPath(eng->player->pos, blockers, stairPos);
	assert(currentPath_.size() > 0);
}


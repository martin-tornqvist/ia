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

void Bot::init() {
  currentPath_.resize(0);

  if(eng->config->isBotPlaying == true) {
    //Switch to melee weapon
    //    eng->input->handleKeyPress('z', false, false);
    //Make sure it's my turn
    //    while(eng->gameTime->getCurrentActor() != eng->player) {
    //      eng->gameTime->endTurnOfCurrentActor();
    //    }
    //    runFunctionTests();
  }
}

void Bot::runFunctionTests() {
  //TEST HIT CHANCE
  Item* weaponUsed = eng->player->getInventory()->getItemInSlot(slot_wielded);
  const Abilities_t weaponAbilityUsed = weaponUsed->getDef().meleeAbilityUsed;
  const int HIT_CHANCE_WEAPON = weaponUsed->getDef().meleeHitChanceMod;
  const int HIT_CHANCE_SKILL = eng->player->getDef()->abilityVals.getVal(weaponAbilityUsed, true, *(eng->player));
  const int HIT_CHANCE_TOTAL = HIT_CHANCE_SKILL + HIT_CHANCE_WEAPON;
  const int NUMBER_OF_ATTACKS = 100;
  double hitChanceReal = 0;

  //Make sure an actor can be spawned next to the player
  eng->featureFactory->spawnFeatureAt(feature_stoneFloor, eng->player->pos + coord(1, 0));

  for(int i = 0; i < NUMBER_OF_ATTACKS; i++) {
    Actor* actor = eng->actorFactory->spawnActor(actor_rat, eng->player->pos + coord(1, 0));
    dynamic_cast<Monster*>(actor)->playerAwarenessCounter = 999;
    eng->attack->melee(*eng->player, *dynamic_cast<Weapon*>(weaponUsed), *actor);

    if(actor->getHp() < actor->getHpMax(true)) {
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
      eng->gameTime->endTurnOfCurrentActor();
    }
  }
  hitChanceReal /= static_cast<double>(NUMBER_OF_ATTACKS);
  hitChanceReal *= 100;
  const double RATIO = static_cast<double>(HIT_CHANCE_TOTAL) / hitChanceReal;

  assert(RATIO > 0.80 && RATIO < 1.20);
}

void Bot::act() {
  tracer << "Bot::act()" << endl;

  const int PLAY_TO_DLVL = LAST_CAVERN_LEVEL;
  const int NR_OF_RUNS = 100;
  int runCount = 1;

  assert(eng->player->pos.x > 0);
  assert(eng->player->pos.y > 0);
  assert(eng->player->pos.x < MAP_X_CELLS - 1);
  assert(eng->player->pos.y < MAP_Y_CELLS - 1);

  tracer << "Bot: Checking if can use stairs here" << endl;
  if(eng->map->featuresStatic[eng->player->pos.x][eng->player->pos.y]->getId() == feature_stairsDown) {
    if(eng->map->getDungeonLevel() >= PLAY_TO_DLVL) {
      tracer << "Bot: Run " << runCount << " finished" << endl;
      runCount++;
      if(runCount >= NR_OF_RUNS) {
        tracer << "Bot: All runs finished, stopping" << endl;
        eng->config->isBotPlaying = false;
      } else {
        tracer << "Bot: Starting new run on first dungeon level" << endl;
        eng->map->dungeonLevel_ = 0;
      }
    }
    eng->input->handleKeyPress(KeyboardReadReturnData('>'));
    return;
  }

  tracer << "Bot: Looking for adjacent doors" << endl;
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      FeatureStatic* f = eng->map->featuresStatic[eng->player->pos.x + dx][eng->player->pos.y + dy];
      if(f->getId() == feature_door) {
        tracer << "Bot: Adjacent door found, revealing" << endl;
        dynamic_cast<Door*>(f)->reveal(false);
        //If adjacent door is stuck, bash at it
        if(dynamic_cast<Door*>(f)->isStuck()) {
          tracer << "Bot: Door is stuck, attempting bash" << endl;
          dynamic_cast<Door*>(f)->tryBash(eng->player);
          return;
        }
      }
    }
  }

  tracer << "Bot: Checking if terrified (to ignore stairs-path)" << endl;
  if(eng->player->getStatusEffectsHandler()->hasEffect(statusTerrified)) {
    tracer << "Bot: Is terrified, walking to current pos or randomly" << endl;
    if(walkToAdjacentCell(eng->player->pos)) {
      return;
    }
  }

  findPathToNextStairs();

  const coord nextCell = currentPath_.back();

  walkToAdjacentCell(nextCell);
}

bool Bot::walkToAdjacentCell(const coord& cellToGoTo) {
  tracer << "Bot::walkToAdjacentCell()..." << endl;

  coord playerCell(eng->player->pos);

  assert(eng->mapTests->isCellsNeighbours(playerCell, cellToGoTo, true));

  //Get relative coordinates
  const int xRel = cellToGoTo.x > playerCell.x ? 1 : cellToGoTo.x < playerCell.x ? -1 : 0;
  const int yRel = cellToGoTo.y > playerCell.y ? 1 : cellToGoTo.y < playerCell.y ? -1 : 0;

  if(cellToGoTo != playerCell) {
    assert(xRel != 0 || yRel != 0);
  }

  char key = ' ';

  if(xRel == 0 && yRel == 0) {
    key = '5';
  }
  if(xRel == 1 && yRel == 0) {
    key = '6';
  }
  if(xRel == 1 && yRel == -1) {
    key = '9';
  }
  if(xRel == 0 && yRel == -1) {
    key = '8';
  }
  if(xRel == -1 && yRel == -1) {
    key = '7';
  }
  if(xRel == -1 && yRel == 0) {
    key = '4';
  }
  if(xRel == -1 && yRel == 1) {
    key = '1';
  }
  if(xRel == 0 && yRel == 1) {
    key = '2';
  }
  if(xRel == 1 && yRel == 1) {
    key = '3';
  }

  const int CHANCE_FOR_RANDOM_DIRECTION = 50;
  if(eng->dice(1, 100) < CHANCE_FOR_RANDOM_DIRECTION) {
    key = '0' + eng->dice.getInRange(1, 9);
  }

  assert(key >= '1' && key <= '9');

  tracer << "Bot: Sending walk keypress" << endl;
  eng->input->handleKeyPress(KeyboardReadReturnData(key));

  tracer << "Bot::walkToAdjacentCell() [DONE]" << endl;
  return playerCell == cellToGoTo;
}

coord Bot::findNextStairs() {
  tracer << "Bot::findNextStairs()..." << endl;
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      FeatureStatic* f = eng->map->featuresStatic[x][y];
      if(f->getId() == feature_stairsDown) {
        tracer << "Bot::findNextStairs() [DONE]" << endl;
        return coord(x, y);
      }
    }
  }
  tracer << "[WARNING] Could not find stairs coord, in Bot::findNextStairs()" << endl;
  return coord(-1, -1);
}

void Bot::findPathToNextStairs() {
  tracer << "Bot::findPathToNextStairs()..." << endl;
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
  tracer << "Bot::findPathToNextStairs() [DONE]" << endl;
}


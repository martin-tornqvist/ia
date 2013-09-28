#include "Bot.h"

#include <cassert>

#include "Engine.h"
#include "Properties.h"
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
}

void Bot::act() {
  trace << "Bot::act()" << endl;

  const int PLAY_TO_DLVL = LAST_CAVERN_LEVEL;
  const int NR_OF_RUNS = 100;
  int runCount = 1;

  assert(eng->player->pos.x > 0);
  assert(eng->player->pos.y > 0);
  assert(eng->player->pos.x < MAP_X_CELLS - 1);
  assert(eng->player->pos.y < MAP_Y_CELLS - 1);

  //Occasionally apply a random property for a few turns
  if(eng->dice.oneIn(5)) {
    vector<PropId_t> propCandidates;
    propCandidates.resize(0);
    for(unsigned int i = 0; i < endOfPropIds; i++) {
      PropData& d = eng->propDataHandler->dataList[i];
      if(d.allowTestingOnBot) {
        propCandidates.push_back(PropId_t(i));
      }
    }
    PropId_t propId =
      propCandidates.at(eng->dice.range(0, propCandidates.size() - 1));

    PropHandler* const propHandler = eng->player->getPropHandler();
    Prop* const prop =
      propHandler->makePropFromId(propId, propTurnsSpecified, 5);

    propHandler->tryApplyProp(prop, true);
  }

  trace << "Bot: Checking if can use stairs here" << endl;
  const Pos& pos = eng->player->pos;
  if(eng->map->featuresStatic[pos.x][pos.y]->getId() == feature_stairsDown) {
    if(eng->map->getDLVL() >= PLAY_TO_DLVL) {
      trace << "Bot: Run " << runCount << " finished" << endl;
      runCount++;
      if(runCount >= NR_OF_RUNS) {
        trace << "Bot: All runs finished, stopping" << endl;
        eng->config->isBotPlaying = false;
      } else {
        trace << "Bot: Starting new run on first dungeon level" << endl;
        eng->map->dlvl_ = 0;
      }
    }
    eng->input->handleKeyPress(KeyboardReadReturnData('>'));
    return;
  }

  trace << "Bot: Looking for adjacent doors" << endl;
  const Pos& playerPos = eng->player->pos;
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      FeatureStatic* f =
        eng->map->featuresStatic[playerPos.x + dx][playerPos.y + dy];
      if(f->getId() == feature_door) {
        trace << "Bot: Adjacent door found, revealing" << endl;
        dynamic_cast<Door*>(f)->reveal(false);
        //If adjacent door is stuck, bash at it
        if(dynamic_cast<Door*>(f)->isStuck()) {
          trace << "Bot: Door is stuck, attempting bash" << endl;
          dynamic_cast<Door*>(f)->tryBash(eng->player);
          return;
        }
      }
    }
  }

  trace << "Bot: Checking if terrified (to ignore stairs-path)" << endl;
  if(eng->player->getPropHandler()->hasProp(propTerrified)) {
    trace << "Bot: Is terrified, walking to current pos or randomly" << endl;
    if(walkToAdjacentCell(playerPos)) {
      return;
    }
  }

  findPathToNextStairs();

  const Pos nextCell = currentPath_.back();

  walkToAdjacentCell(nextCell);
}

bool Bot::walkToAdjacentCell(const Pos& cellToGoTo) {
  trace << "Bot::walkToAdjacentCell()..." << endl;

  Pos playerCell(eng->player->pos);

  assert(eng->mapTests->isCellsNeighbours(playerCell, cellToGoTo, true));

  //Get relative positions
  const int xRel =
    cellToGoTo.x > playerCell.x ? 1 : cellToGoTo.x < playerCell.x ? -1 : 0;
  const int yRel =
    cellToGoTo.y > playerCell.y ? 1 : cellToGoTo.y < playerCell.y ? -1 : 0;

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

  //Occasionally randomize movement
  if(eng->dice.oneIn(2)) {
    key = '0' + eng->dice.range(1, 9);
  }

  assert(key >= '1' && key <= '9');

  trace << "Bot: Sending walk keypress" << endl;
  eng->input->handleKeyPress(KeyboardReadReturnData(key));

  trace << "Bot::walkToAdjacentCell() [DONE]" << endl;
  return playerCell == cellToGoTo;
}

Pos Bot::findNextStairs() {
  trace << "Bot::findNextStairs()..." << endl;
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      FeatureStatic* f = eng->map->featuresStatic[x][y];
      if(f->getId() == feature_stairsDown) {
        trace << "Bot::findNextStairs() [DONE]" << endl;
        return Pos(x, y);
      }
    }
  }
  trace << "[WARNING] Could not find stairs Pos, in Bot::findNextStairs()" << endl;
  return Pos(-1, -1);
}

void Bot::findPathToNextStairs() {
  trace << "Bot::findPathToNextStairs()..." << endl;
  currentPath_.resize(0);

  const Pos stairPos = findNextStairs();

  assert(stairPos != Pos(-1, -1));

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(
    actorBodyType_normal, blockers);

  //Consider all doors passable
  for(int y = 0; y < MAP_Y_CELLS; y++) {
    for(int x = 0; x < MAP_X_CELLS; x++) {
      FeatureStatic* f = eng->map->featuresStatic[x][y];
      if(f->getId() == feature_door) {
        blockers[x][y] = false;
      }
    }
  }
  currentPath_ =
    eng->pathfinder->findPath(eng->player->pos, blockers, stairPos);
  assert(currentPath_.size() > 0);
  trace << "Bot::findPathToNextStairs() [DONE]" << endl;
}


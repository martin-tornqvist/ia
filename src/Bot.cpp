#include "Bot.h"

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
  const int PLAY_TO_DLVL = LAST_CAVERN_LEVEL;
  const int NR_OF_RUNS = 500;
  int runCount = 1;

  //=======================================================================
  // TESTS
  //=======================================================================
  const int NR_ACTORS = eng->gameTime->getLoopSize();
  for(int i = 0; i < NR_ACTORS; i++) {
    const Actor* const actor = eng->gameTime->getActorAt(i);
    if(eng->mapTests->isPosInsideMap(actor->pos) == false) {
      throw runtime_error("Actor outside map");
    }
  }
  //=======================================================================

  PropHandler* const propHandler = eng->player->getPropHandler();

  //Occasionally apply RFear
  //(Helps avoiding getting stuck on fear-causing monsters too long)
  if(eng->dice.oneIn(7)) {
    propHandler->tryApplyProp(new PropRFear(eng, propTurnsSpecified, 4), true);
  }

  //Ocassionally send a TAB command to attack nearby monsters
  //(Helps avoiding getting stuck around monsters too long)
  if(eng->dice.coinToss()) {
    eng->input->handleKeyPress(KeyboardReadReturnData(SDLK_TAB));
    return;
  }

  //Occasionally apply a random property to exercise the prop code
  if(eng->dice.oneIn(10)) {
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

    Prop* const prop =
      propHandler->makePropFromId(propId, propTurnsSpecified, 5);

    propHandler->tryApplyProp(prop, true);
  }

  //If we are on the stairs,
  //check if we are finished with the current run or finished with all runs,
  //otherwise descend the stairs
  const Pos& pos = eng->player->pos;
  if(eng->map->featuresStatic[pos.x][pos.y]->getId() == feature_stairsDown) {
    if(eng->map->getDLVL() >= PLAY_TO_DLVL) {
      trace << "Bot: Run " << runCount << " finished" << endl;
      runCount++;
      if(runCount >= NR_OF_RUNS) {
        trace << "Bot: All runs finished, stopping" << endl;
        eng->config->isBotPlaying = false;
        return;
      } else {
        trace << "Bot: Starting new run on first dungeon level" << endl;
        eng->map->dlvl_ = 0;
      }
    }
    eng->input->handleKeyPress(KeyboardReadReturnData('>'));
    return;
  }

  //Handle blocking door
  const Pos& playerPos = eng->player->pos;
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      FeatureStatic* f =
        eng->map->featuresStatic[playerPos.x + dx][playerPos.y + dy];
      if(f->getId() == feature_door) {
        dynamic_cast<Door*>(f)->reveal(false);
        if(dynamic_cast<Door*>(f)->isStuck()) {
          dynamic_cast<Door*>(f)->tryBash(eng->player);
          return;
        }
      }
    }
  }

  //If we are terrified, wait in place
  if(eng->player->getPropHandler()->hasProp(propTerrified)) {
    if(walkToAdjacentCell(playerPos)) {
      return;
    }
  }

  findPathToNextStairs();

  const Pos nextCell = currentPath_.back();

  walkToAdjacentCell(nextCell);
}

bool Bot::walkToAdjacentCell(const Pos& cellToGoTo) {
  Pos playerCell(eng->player->pos);

  if(eng->mapTests->isCellsAdj(playerCell, cellToGoTo, true) == false) {
    throw runtime_error("Bad position parameter");
  }

  //Get relative positions
  const int xRel =
    cellToGoTo.x > playerCell.x ? 1 : cellToGoTo.x < playerCell.x ? -1 : 0;
  const int yRel =
    cellToGoTo.y > playerCell.y ? 1 : cellToGoTo.y < playerCell.y ? -1 : 0;

  if(cellToGoTo != playerCell) {
    if(xRel == 0 && yRel == 0) {throw runtime_error("Expected non zero");}
  }

  char key = ' ';

  if(xRel == 0 && yRel == 0) {key = '5';}
  if(xRel == 1 && yRel == 0) {key = '6';}
  if(xRel == 1 && yRel == -1) {key = '9';}
  if(xRel == 0 && yRel == -1) {key = '8';}
  if(xRel == -1 && yRel == -1) {key = '7';}
  if(xRel == -1 && yRel == 0) {key = '4';}
  if(xRel == -1 && yRel == 1) {key = '1';}
  if(xRel == 0 && yRel == 1) {key = '2';}
  if(xRel == 1 && yRel == 1) {key = '3';}

  //Occasionally randomize movement
  if(eng->dice.oneIn(2)) {
    key = '0' + eng->dice.range(1, 9);
  }

//  assert(key >= '1' && key <= '9');

  eng->input->handleKeyPress(KeyboardReadReturnData(key));

  return playerCell == cellToGoTo;
}

Pos Bot::findNextStairs() {
  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      FeatureStatic* f = eng->map->featuresStatic[x][y];
      if(f->getId() == feature_stairsDown) {
        return Pos(x, y);
      }
    }
  }
  trace << "[WARNING] Could not find stairs Pos, in Bot::findNextStairs()";
  trace << endl;
  return Pos(-1, -1);
}

void Bot::findPathToNextStairs() {
  currentPath_.resize(0);

  const Pos stairPos = findNextStairs();

//  assert(stairPos != Pos(-1, -1));

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForBodyTypeFeaturesOnly(
    bodyType_normal, blockers);

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
//  assert(currentPath_.size() > 0);
}


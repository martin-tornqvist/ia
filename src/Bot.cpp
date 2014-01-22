#include "Bot.h"

#include <assert.h>

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
#include "Inventory.h"
#include "FeatureFactory.h"
#include "ActorMonster.h"
#include "MapParsing.h"

void Bot::init() {
  currentPath_.resize(0);
}

void Bot::act() {
  //=======================================================================
  // TESTS
  //=======================================================================
  const int NR_ACTORS = eng.gameTime->getNrActors();
  for(int i = 0; i < NR_ACTORS; i++) {
    const Actor& actor = eng.gameTime->getActorAtElement(i);
    assert(eng.basicUtils->isPosInsideMap(actor.pos));
  }
  //=======================================================================

  PropHandler& propHandler = eng.player->getPropHandler();

  //Occasionally apply RFear
  //(Helps avoiding getting stuck on fear-causing monsters too long)
  if(eng.dice.oneIn(7)) {
    propHandler.tryApplyProp(new PropRFear(eng, propTurnsSpecified, 4), true);
  }

  //Ocassionally send a TAB command to attack nearby monsters
  //(Helps avoiding getting stuck around monsters too long)
  if(eng.dice.coinToss()) {
    eng.input->handleKeyPress(KeyboardReadReturnData(SDLK_TAB));
    return;
  }

  //Occasionally apply a random property to exercise the prop code
  if(eng.dice.oneIn(10)) {
    vector<PropId_t> propCandidates;
    propCandidates.resize(0);
    for(unsigned int i = 0; i < endOfPropIds; i++) {
      PropData& d = eng.propDataHandler->dataList[i];
      if(d.allowTestingOnBot) {
        propCandidates.push_back(PropId_t(i));
      }
    }
    PropId_t propId =
      propCandidates.at(eng.dice.range(0, propCandidates.size() - 1));

    Prop* const prop =
      propHandler.makePropFromId(propId, propTurnsSpecified, 5);

    propHandler.tryApplyProp(prop, true);
  }

  //If we are on the stairs,
  //check if we are finished with the current run or finished with all runs,
  //otherwise descend the stairs
  const Pos& pos = eng.player->pos;
  const FeatureStatic* const featureHere =
    eng.map->cells[pos.x][pos.y].featureStatic;
  if(featureHere->getId() == feature_stairsDown) {
    if(eng.map->getDlvl() >= LAST_CAVERN_LEVEL) {
      trace << "Bot: Starting new run on first dungeon level" << endl;
      eng.map->dlvl_ = 0;
    }
    eng.input->handleKeyPress(KeyboardReadReturnData('>'));
    return;
  }

  //Handle blocking door
  const Pos& playerPos = eng.player->pos;
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      FeatureStatic* f =
        eng.map->cells[playerPos.x + dx][playerPos.y + dy].featureStatic;
      if(f->getId() == feature_door) {
        Door* const door = dynamic_cast<Door*>(f);
        door->reveal(false);
        if(door->isStuck()) {
          f->bash(*eng.player);
          return;
        }
      }
    }
  }

  //If we are terrified, wait in place
  if(eng.player->getPropHandler().hasProp(propTerrified)) {
    if(walkToAdjacentCell(playerPos)) {
      return;
    }
  }

  findPathToNextStairs();

  const Pos nextCell = currentPath_.back();

  walkToAdjacentCell(nextCell);
}

bool Bot::walkToAdjacentCell(const Pos& cellToGoTo) {
  Pos playerCell(eng.player->pos);

  assert(eng.basicUtils->isPosAdj(playerCell, cellToGoTo, true));

  //Get relative positions
  const int xRel =
    cellToGoTo.x > playerCell.x ? 1 : cellToGoTo.x < playerCell.x ? -1 : 0;
  const int yRel =
    cellToGoTo.y > playerCell.y ? 1 : cellToGoTo.y < playerCell.y ? -1 : 0;

  assert(cellToGoTo == playerCell || xRel != 0 || yRel != 0);

  char key = ' ';

  if(xRel ==  0 && yRel ==  0) {key = '5';}
  if(xRel ==  1 && yRel ==  0) {key = '6';}
  if(xRel ==  1 && yRel == -1) {key = '9';}
  if(xRel ==  0 && yRel == -1) {key = '8';}
  if(xRel == -1 && yRel == -1) {key = '7';}
  if(xRel == -1 && yRel ==  0) {key = '4';}
  if(xRel == -1 && yRel ==  1) {key = '1';}
  if(xRel ==  0 && yRel ==  1) {key = '2';}
  if(xRel ==  1 && yRel ==  1) {key = '3';}

  //Occasionally randomize movement
  if(eng.dice.oneIn(2)) {
    key = '0' + eng.dice.range(1, 9);
  }

//  assert(key >= '1' && key <= '9');

  eng.input->handleKeyPress(KeyboardReadReturnData(key));

  return playerCell == cellToGoTo;
}

Pos Bot::findNextStairs() {
  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      FeatureStatic* f = eng.map->cells[x][y].featureStatic;
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

  bool blockers[MAP_W][MAP_H];
  const BodyType_t playerBodyType = eng.player->getBodyType();
  MapParser::parse(CellPredBlocksBodyType(playerBodyType, false, eng),
                   blockers);

  //Consider all doors passable
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      FeatureStatic* f = eng.map->cells[x][y].featureStatic;
      if(f->getId() == feature_door) {
        blockers[x][y] = false;
      }
    }
  }
  eng.pathFinder->run(eng.player->pos, stairPos, blockers, currentPath_);
//  assert(currentPath_.size() > 0);
}


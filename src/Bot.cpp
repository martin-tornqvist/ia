#include "Bot.h"

#include <assert.h>
#include <algorithm>
#include <vector>

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

using namespace std;

void Bot::init() {
  currentPath_.resize(0);
}

void Bot::act() {
  //=======================================================================
  // TESTS
  //=======================================================================
  for(Actor* actor : eng.gameTime->actors_) {
    assert(eng.basicUtils->isPosInsideMap(actor->pos));
  }
  //=======================================================================

  PropHandler& propHandler = eng.player->getPropHandler();

  //Occasionally apply RFear (to avoid getting stuck on fear-causing monsters)
  if(eng.dice.oneIn(7)) {
    propHandler.tryApplyProp(new PropRFear(eng, propTurnsSpecified, 4), true);
  }

  //Occasionally teleport (to avoid getting stuck)
  if(eng.dice.oneIn(200)) {
    eng.player->teleport(false);
  }

  //Ocassionally send a TAB command to attack nearby monsters
  if(eng.dice.coinToss()) {
    eng.input->handleKeyPress(KeyboardReadReturnData(SDLK_TAB));
    return;
  }

  //Occasionally apply a random property to exercise the prop code
  if(eng.dice.oneIn(10)) {
    vector<PropId> propCandidates;
    propCandidates.resize(0);
    for(unsigned int i = 0; i < endOfPropIds; i++) {
      PropData& d = eng.propDataHandler->dataList[i];
      if(d.allowTestingOnBot) {
        propCandidates.push_back(PropId(i));
      }
    }
    PropId propId =
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
  if(featureHere->getId() == feature_stairs) {
    if(eng.map->getDlvl() >= LAST_CAVERN_LEVEL) {
      trace << "Bot: Starting new run on first dungeon level" << endl;
      eng.map->dlvl_ = 0;
    }
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
  vector<PropId> props;
  eng.player->getPropHandler().getAllActivePropIds(props);

  if(find(props.begin(), props.end(), propTerrified) != props.end()) {
    if(walkToAdjacentCell(playerPos)) {
      return;
    }
  }

  findPathToStairs();

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
  if(eng.dice.oneIn(3)) {
    key = '0' + eng.dice.range(1, 9);
  }

//  assert(key >= '1' && key <= '9');

  eng.input->handleKeyPress(KeyboardReadReturnData(key));

  return playerCell == cellToGoTo;
}

void Bot::findPathToStairs() {
  currentPath_.resize(0);

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false, eng), blockers);

  vector<Pos> bla;
  eng.basicUtils->makeVectorFromBoolMap(false, blockers, bla);

  Pos stairPos(-1, -1);

  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      const FeatureId curId = eng.map->cells[x][y].featureStatic->getId();
      if(curId == feature_stairs) {
        blockers[x][y] = false;
        stairPos.set(x, y);
      } else if(curId == feature_door) {
        blockers[x][y] = false;
      }
    }
  }
  assert(stairPos != Pos(-1, -1));

  PathFind::run(eng.player->pos, stairPos, blockers, currentPath_, eng);
//  assert(currentPath_.size() > 0);
}


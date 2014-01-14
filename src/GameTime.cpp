#include "GameTime.h"

#include <assert.h>

#include "Engine.h"

#include "CommonTypes.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "PlayerVisualMemory.h"
#include "Map.h"
#include "PopulateMonsters.h"
#include "Input.h"
#include "Inventory.h"
#include "InventoryHandler.h"
#include "PlayerBonuses.h"
#include "Audio.h"
#include "MapParsing.h"
#include "Renderer.h"

void GameTime::addSaveLines(vector<string>& lines) const {
  lines.push_back(toString(turn_));
}

void GameTime::setParamsFromSaveLines(vector<string>& lines) {
  turn_ = toInt(lines.front());
  lines.erase(lines.begin());
}

void GameTime::getFeatureMobsAtPos(
  const Pos& pos, vector<FeatureMob*>& vectorRef) {
  vectorRef.resize(0);
  for(FeatureMob * m : featureMobs_) {
    if(m->getPos() == pos) {
      vectorRef.push_back(m);
    }
  }
}

GameTime::~GameTime() {
  for(unsigned int i = 0; i < actors_.size(); i++) {
    delete actors_.at(i);
  }
  actors_.resize(0);

  for(unsigned int i = 0; i < featureMobs_.size(); i++) {
    delete featureMobs_.at(i);
  }
  featureMobs_.resize(0);
}

void GameTime::eraseActorInElement(const unsigned int i) {
  if(actors_.empty() == false) {
    delete actors_.at(i);
    actors_.erase(actors_.begin() + i);
  }
}

void GameTime::insertActorInLoop(Actor* actor) {
  //Sanity check actor inserted
  assert(eng.basicUtils->isPosInsideMap(actor->pos));
  actors_.push_back(actor);
}

//For every turn type step, run through all actors and let those who can act
//during this type of turn act. When all actors who can act on this phase have
//acted, and if this is a normal speed phase - consider it a standard turn;
//update status effects, update timed features, spawn more monsters etc.
void GameTime::actorDidAct(const bool IS_FREE_TURN) {
  runAtomicTurnEvents();

  Actor* currentActor = getCurrentActor();

  if(currentActor == eng.player) {
    eng.player->updateFov();
    eng.renderer->drawMapAndInterface();
    eng.playerVisualMemory->updateVisualMemory();
  } else {
    Monster* monster = dynamic_cast<Monster*>(currentActor);
    if(monster->playerAwarenessCounter > 0) {
      monster->playerAwarenessCounter -= 1;
    }
  }

  //Tick properties running on actor turns
  currentActor->getPropHandler().tick(propTurnModeActor, NULL);

  if(IS_FREE_TURN == false) {
    TurnType_t currentTurnType = TurnType_t(currentTurnTypePos_);

    bool actorWhoCanActThisTurnFound = false;
    while(actorWhoCanActThisTurnFound == false) {
      currentTurnType = (TurnType_t)(currentTurnTypePos_);

      currentActorVectorPos_++;

      if((unsigned int)currentActorVectorPos_ >= actors_.size()) {
        currentActorVectorPos_ = 0;
        currentTurnTypePos_++;
        if(currentTurnTypePos_ == endOfTurnType) {
          currentTurnTypePos_ = 0;
        }

        if(
          currentTurnType != turnType_fast &&
          currentTurnType != turnType_fastest) {
          runStandardTurnEvents();
        }
      }

      currentActor = getCurrentActor();

      const bool IS_SLOWED =
        currentActor->getPropHandler().hasProp(propSlowed);
      const ActorSpeed_t defSpeed = currentActor->getData().speed;
      const ActorSpeed_t realSpeed =
        IS_SLOWED == false || defSpeed == actorSpeed_sluggish ?
        defSpeed : ActorSpeed_t(defSpeed - 1);
      switch(realSpeed) {
        case actorSpeed_sluggish: {
          actorWhoCanActThisTurnFound =
            (currentTurnType == turnType_slow ||
             currentTurnType == turnType_normal_2)
            && eng.dice.fraction(2, 3);
        } break;

        case actorSpeed_slow: {
          actorWhoCanActThisTurnFound =
            currentTurnType == turnType_slow ||
            currentTurnType == turnType_normal_2;
        } break;

        case actorSpeed_normal: {
          actorWhoCanActThisTurnFound =
            currentTurnType != turnType_fast &&
            currentTurnType != turnType_fastest;
        } break;

        case actorSpeed_fast: {
          actorWhoCanActThisTurnFound = currentTurnType != turnType_fastest;
        } break;

        case actorSpeed_fastest: {
          actorWhoCanActThisTurnFound = true;
        } break;
      }
    }
  }

//  traceVerbose << "GameTime::endTurnOfCurrentActor() [DONE]" << endl;
}

void GameTime::runStandardTurnEvents() {
//  traceVerbose << "GameTime::runStandardTurnEvents()..." << endl;

  turn_++;

//  traceVerbose << "GameTime: Current turn: " << turn_ << endl;

  Actor* actor = NULL;
  int loopSize = actors_.size();

  bool visionBlockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);

  int regenSpiEveryNTurns = 11;

  for(int i = 0; i < loopSize; i++) {
    actor = actors_.at(i);

    actor->getPropHandler().tick(propTurnModeStandard, visionBlockers);

    //Do light damage if actor in lit cell
    const Pos& pos = actor->pos;
    if(eng.map->cells[pos.x][pos.y].isLight) {
      actor->hit(1, dmgType_light, false);
    }

    if(actor->deadState == actorDeadState_alive) {
      //Regen Spi
      if(actor == eng.player) {
        if(eng.playerBonHandler->hasTrait(traitStrongSpirited)) {
          regenSpiEveryNTurns -= 2;
        }
        if(eng.playerBonHandler->hasTrait(traitMightySpirited)) {
          regenSpiEveryNTurns -= 2;
        }
      }

      if(isSpiRegenThisTurn(regenSpiEveryNTurns)) {
        actor->restoreSpi(1, false);
      }

      actor->onStandardTurn_();
    }

    //Delete dead, mangled actors
    if(actor->deadState == actorDeadState_mangled) {
      delete actor;
      if(eng.player->target == actor) {eng.player->target = NULL;}
      actors_.erase(actors_.begin() + i);
      i--;
      loopSize--;
      if((unsigned int)currentActorVectorPos_ >= actors_.size()) {
        currentActorVectorPos_ = 0;
      }
    }
  }

  //Update mobile features
  const vector<FeatureMob*> mobsCpy = featureMobs_;
  for(FeatureMob * f : mobsCpy) {f->newTurn();}

  //Update timed features
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      eng.map->cells[x][y].featureStatic->newTurn();
    }
  }

  //Spawn more monsters?
  //(If an unexplored cell is selected, the spawn is aborted)
  const int DLVL = eng.map->getDlvl();
  if(DLVL >= 1 && DLVL <= LAST_CAVERN_LEVEL) {
    const int SPAWN_N_TURN = 125;
    if(turn_ == (turn_ / SPAWN_N_TURN) * SPAWN_N_TURN) {
      eng.populateMonsters->trySpawnDueToTimePassed();
    }
  }

  //Run new turn events on all player items
  Inventory& playerInv = eng.player->getInv();
  vector<Item*>& playerBackpack = playerInv.getGeneral();
  for(Item * const item : playerBackpack) {item->newTurnInInventory();}
  vector<InventorySlot>& playerSlots = playerInv.getSlots();
  for(InventorySlot & slot : playerSlots) {
    if(slot.item != NULL) {
      slot.item->newTurnInInventory();
    }
  }

  eng.soundEmitter->resetNrSoundMsgPrintedCurTurn();

  eng.audio->tryPlayAmb(250);

//  traceVerbose << "GameTime::runStandardTurnEvents() [DONE]" << endl;
}

bool GameTime::isSpiRegenThisTurn(const int REGEN_N_TURNS) {
  return turn_ == (turn_ / REGEN_N_TURNS) * REGEN_N_TURNS;
}

void GameTime::runAtomicTurnEvents() {
  updateLightMap();
}

void GameTime::updateLightMap() {
  bool lightTmp[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      eng.map->cells[x][y].isLight = false;
      lightTmp[x][y] = false;
    }
  }

  eng.player->addLight(lightTmp);

  const int NR_ACTORS = actors_.size();
  for(int i = 0; i < NR_ACTORS; i++) {
    actors_.at(i)->addLight(lightTmp);
  }

  const int NR_FEATURE_MOBS = featureMobs_.size();
  for(int i = 0; i < NR_FEATURE_MOBS; i++) {
    featureMobs_.at(i)->addLight(lightTmp);
  }

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      eng.map->cells[x][y].featureStatic->addLight(lightTmp);

      //Note: Here the temporary values are copied to the map.
      //This must of course be done last!
      eng.map->cells[x][y].isLight = lightTmp[x][y];
    }
  }
}

Actor* GameTime::getCurrentActor() {
  Actor* const actor = actors_.at(currentActorVectorPos_);

  //Sanity check actor retrieved
  assert(eng.basicUtils->isPosInsideMap(actor->pos));
  return actor;
}

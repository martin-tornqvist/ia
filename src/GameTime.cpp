#include "GameTime.h"

#include "CmnTypes.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "PopulateMonsters.h"
#include "Input.h"
#include "Inventory.h"
#include "InventoryHandler.h"
#include "PlayerBon.h"
#include "Audio.h"
#include "MapParsing.h"
#include "Renderer.h"
#include "Utils.h"

void GameTime::storeToSaveLines(vector<string>& lines) const {
  lines.push_back(toStr(turn_));
}

void GameTime::setupFromSaveLines(vector<string>& lines) {
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
  assert(Utils::isPosInsideMap(actor->pos));
  actors_.push_back(actor);
}

//For every turn type step, run through all actors and let those who can act
//during this type of turn act. When all actors who can act on this phase have
//acted, and if this is a normal speed phase - consider it a standard turn;
//update status effects, update timed features, spawn more monsters etc.
void GameTime::actorDidAct(const bool IS_FREE_TURN) {
  runAtomicTurnEvents();

  Actor* currentActor = getCurrentActor();

  if(currentActor == Map::player) {
    Map::player->updateFov();
    Renderer::drawMapAndInterface();
    Map::updateVisualMemory();
  } else {
    Monster* monster = dynamic_cast<Monster*>(currentActor);
    if(monster->awareOfPlayerCounter_ > 0) {
      monster->awareOfPlayerCounter_ -= 1;
    }
  }

  //Tick properties running on actor turns
  currentActor->getPropHandler().tick(propTurnModeActor, NULL);

  if(IS_FREE_TURN == false) {

    bool actorWhoCanActThisTurnFound = false;
    while(actorWhoCanActThisTurnFound == false) {
      TurnType currentTurnType = (TurnType)(currentTurnTypePos_);

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
      vector<PropId> props;
      currentActor->getPropHandler().getAllActivePropIds(props);

      const bool IS_SLOWED =
        find(props.begin(), props.end(), propSlowed) != props.end();
      const ActorSpeed defSpeed = currentActor->getData().speed;
      const ActorSpeed realSpeed =
        IS_SLOWED == false || defSpeed == ActorSpeed::sluggish ?
        defSpeed : ActorSpeed(int(defSpeed) - 1);
      switch(realSpeed) {
        case ActorSpeed::sluggish: {
          actorWhoCanActThisTurnFound =
            (currentTurnType == turnType_slow ||
             currentTurnType == turnType_normal_2)
            && Rnd::fraction(2, 3);
        } break;

        case ActorSpeed::slow: {
          actorWhoCanActThisTurnFound =
            currentTurnType == turnType_slow ||
            currentTurnType == turnType_normal_2;
        } break;

        case ActorSpeed::normal: {
          actorWhoCanActThisTurnFound =
            currentTurnType != turnType_fast &&
            currentTurnType != turnType_fastest;
        } break;

        case ActorSpeed::fast: {
          actorWhoCanActThisTurnFound = currentTurnType != turnType_fastest;
        } break;

        case ActorSpeed::fastest: {
          actorWhoCanActThisTurnFound = true;
        } break;

        case ActorSpeed::endOfActorSpeed: {} break;
      }
    }
  }

//  traceVerbose << "GameTime::endTurnOfCurrentActor() [DONE]" << endl;
}

void GameTime::runStandardTurnEvents() {
//  traceVerbose << "GameTime::runStandardTurnEvents()..." << endl;

  turn_++;

//  traceVerbose << "GameTime: Current turn: " << turn_ << endl;

  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);

  int regenSpiNTurns = 12;

  for(size_t i = 0; i < actors_.size(); i++) {
    Actor* const actor = actors_.at(i);

    actor->getPropHandler().tick(propTurnModeStandard, visionBlockers);

    if(actor != Map::player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      if(monster->playerAwareOfMeCounter_ > 0) {
        monster->playerAwareOfMeCounter_--;
      }
    }

    //Do light damage if actor in lit cell
    const Pos& pos = actor->pos;
    if(Map::cells[pos.x][pos.y].isLight) {
      actor->hit(1, DmgType::light, false);
    }

    if(actor->deadState == ActorDeadState::alive) {
      //Regen Spi
      if(actor == Map::player) {
        if(PlayerBon::hasTrait(Trait::stoutSpirit))   regenSpiNTurns -= 2;
        if(PlayerBon::hasTrait(Trait::strongSpirit))  regenSpiNTurns -= 2;
        if(PlayerBon::hasTrait(Trait::mightySpirit))  regenSpiNTurns -= 2;
      }

      regenSpiNTurns = max(2, regenSpiNTurns);

      if(isSpiRegenThisTurn(regenSpiNTurns)) {
        actor->restoreSpi(1, false);
      }

      actor->onStandardTurn();
    }

    //Delete destroyed actors
    if(actor->deadState == ActorDeadState::destroyed) {
      //Do not delete player if player died, just exit the function
      if(actor == Map::player) {return;}

      delete actor;
      if(Map::player->target == actor) {Map::player->target = NULL;}
      actors_.erase(actors_.begin() + i);
      i--;
      if(currentActorVectorPos_ >= int(actors_.size())) {
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
      Map::cells[x][y].featureStatic->newTurn();
    }
  }

  //Spawn more monsters?
  //(If an unexplored cell is selected, the spawn is aborted)
  const int DLVL = Map::getDlvl();
  if(DLVL >= 1 && DLVL <= LAST_CAVERN_LEVEL) {
    const int SPAWN_N_TURN = 125;
    if(turn_ == (turn_ / SPAWN_N_TURN) * SPAWN_N_TURN) {
      eng.populateMonsters->trySpawnDueToTimePassed();
    }
  }

  //Run new turn events on all player items
  Inventory& playerInv = Map::player->getInv();
  vector<Item*>& playerBackpack = playerInv.getGeneral();
  for(Item * const item : playerBackpack) {item->newTurnInInventory();}
  vector<InvSlot>& playerSlots = playerInv.getSlots();
  for(InvSlot & slot : playerSlots) {
    if(slot.item != NULL) {
      slot.item->newTurnInInventory();
    }
  }

  SndEmit::resetNrSoundMsgPrintedCurTurn();

  Audio::tryPlayAmb(75, eng);

//  traceVerbose << "GameTime::runStandardTurnEvents() [DONE]" << endl;
}

bool GameTime::isSpiRegenThisTurn(const int REGEN_N_TURNS) {
  assert(REGEN_N_TURNS != 0);
  return turn_ == (turn_ / REGEN_N_TURNS) * REGEN_N_TURNS;
}

void GameTime::runAtomicTurnEvents() {
  updateLightMap();
}

void GameTime::updateLightMap() {
  bool lightTmp[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Map::cells[x][y].isLight = false;
      lightTmp[x][y] = false;
    }
  }

  Map::player->addLight(lightTmp);

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
      Map::cells[x][y].featureStatic->addLight(lightTmp);

      //Note: Here the temporary values are copied to the map.
      //This must of course be done last!
      Map::cells[x][y].isLight = lightTmp[x][y];
    }
  }
}

Actor* GameTime::getCurrentActor() {
  Actor* const actor = actors_.at(currentActorVectorPos_);

  //Sanity check actor retrieved
  assert(Utils::isPosInsideMap(actor->pos));
  return actor;
}

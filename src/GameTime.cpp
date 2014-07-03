#include "GameTime.h"

#include <vector>
#include <assert.h>

#include "CmnTypes.h"
#include "Feature.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "PopulateMonsters.h"
#include "Input.h"
#include "Inventory.h"
#include "InventoryHandling.h"
#include "PlayerBon.h"
#include "Audio.h"
#include "MapParsing.h"
#include "Renderer.h"
#include "Utils.h"

using namespace std;

namespace GameTime {

vector<Actor*>      actors_;
vector<FeatureMob*> featureMobs_;

namespace {

vector<ActorSpeed>  turnTypeVector_;
int                 curTurnTypePos_         = 0;
int                 curActorVectorPos_  = 0;
int                 turn_                   = 0;

bool isSpiRegenThisTurn(const int REGEN_N_TURNS) {
  assert(REGEN_N_TURNS != 0);
  return turn_ == (turn_ / REGEN_N_TURNS) * REGEN_N_TURNS;
}

void runStandardTurnEvents() {
  turn_++;

//  TRACE_VERBOSE << "GameTime: Cur turn: " << turn_ << endl;

  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);

  int regenSpiNTurns = 12;

  for(size_t i = 0; i < actors_.size(); ++i) {
    Actor* const actor = actors_[i];

    actor->getPropHandler().tick(propTurnModeStandard, visionBlockers);

    if(actor != Map::player) {
      Monster* const monster = static_cast<Monster*>(actor);
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

      if(isSpiRegenThisTurn(regenSpiNTurns)) {actor->restoreSpi(1, false);}

      actor->onStandardTurn();
    }

    //Delete destroyed actors
    if(actor->deadState == ActorDeadState::destroyed) {
      //Do not delete player if player died, just exit the function
      if(actor == Map::player) {return;}

      delete actor;
      if(Map::player->target == actor) {Map::player->target = nullptr;}
      actors_.erase(actors_.begin() + i);
      i--;
      if(curActorVectorPos_ >= int(actors_.size())) {curActorVectorPos_ = 0;}
    }
  }

  //Update mobile features
  const vector<FeatureMob*> mobsCpy = featureMobs_;
  for(auto* f : mobsCpy) {f->newTurn();}

  //Update timed features
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Map::cells[x][y].featureStatic->newTurn();
    }
  }

  //Spawn more monsters?
  //(If an unexplored cell is selected, the spawn is aborted)

  if(Map::dlvl >= 1 && Map::dlvl <= LAST_CAVERN_LVL) {
    const int SPAWN_N_TURN = 125;
    if(turn_ == (turn_ / SPAWN_N_TURN) * SPAWN_N_TURN) {
      PopulateMonsters::trySpawnDueToTimePassed();
    }
  }

  //Run new turn events on all player items
  Inventory& playerInv = Map::player->getInv();
  auto& playerBackpack = playerInv.getGeneral();
  for(Item* const item : playerBackpack) {item->newTurnInInventory();}

  auto& playerSlots = playerInv.getSlots();
  for(InvSlot& slot : playerSlots) {
    if(slot.item) {slot.item->newTurnInInventory();}
  }

  SndEmit::resetNrSoundMsgPrintedCurTurn();

  Audio::tryPlayAmb(75);
}

void runAtomicTurnEvents() {
  updateLightMap();
}

} //namespace

void init() {
  curTurnTypePos_     = 0;
  curActorVectorPos_  = 0;
  turn_               = 0;
  actors_.resize(0);
  featureMobs_.resize(0);
}

void cleanup() {
  for(Actor* a : actors_) {delete a;}
  actors_.resize(0);

  for(auto* f : featureMobs_) {delete f;}
  featureMobs_.resize(0);
}

void storeToSaveLines(vector<string>& lines) {
  lines.push_back(toStr(turn_));
}

void setupFromSaveLines(vector<string>& lines) {
  turn_ = toInt(lines.front());
  lines.erase(lines.begin());
}

int getTurn() {
  return turn_;
}

void getFeatureMobsAtPos(const Pos& p, vector<FeatureMob*>& vectorRef) {
  vectorRef.resize(0);
  for(auto* m : featureMobs_) {if(m->getPos() == p) {vectorRef.push_back(m);}}
}

void addFeatureMob(FeatureMob* const feature) {
  featureMobs_.push_back(feature);
}

void eraseFeatureMob(FeatureMob* const feature, const bool DESTROY_OBJECT) {
  for(auto it = featureMobs_.begin(); it != featureMobs_.end(); ++it) {
    if(*it == feature) {
      if(DESTROY_OBJECT) {delete feature;}
      featureMobs_.erase(it);
      return;
    }
  }
}

void eraseAllFeatureMobs() {
  for(auto* m : featureMobs_) {delete m;}
  featureMobs_.resize(0);
}

void eraseActorInElement(const size_t i) {
  if(!actors_.empty()) {
    delete actors_.at(i);
    actors_.erase(actors_.begin() + i);
  }
}

void insertActorInLoop(Actor* actor) {
  //Sanity check actor inserted
  assert(Utils::isPosInsideMap(actor->pos));
  actors_.push_back(actor);
}

void resetTurnTypeAndActorCounters() {
  curTurnTypePos_         = 0;
  curActorVectorPos_  = 0;
}

//For every turn type step, run through all actors and let those who can act
//during this type of turn act. When all actors who can act on this phase have
//acted, and if this is a normal speed phase - consider it a standard turn;
//update status effects, update timed features, spawn more monsters etc.
void actorDidAct(const bool IS_FREE_TURN) {
  runAtomicTurnEvents();

  auto* curActor = getCurActor();

  if(curActor == Map::player) {
    Map::player->updateFov();
    Renderer::drawMapAndInterface();
    Map::updateVisualMemory();
  } else {
    auto* monster = static_cast<Monster*>(curActor);
    if(monster->awareOfPlayerCounter_ > 0) {
      monster->awareOfPlayerCounter_ -= 1;
    }
  }

  //Tick properties running on actor turns
  curActor->getPropHandler().tick(propTurnModeActor, nullptr);

  if(!IS_FREE_TURN) {

    bool actorWhoCanActThisTurnFound = false;
    while(!actorWhoCanActThisTurnFound) {
      auto curTurnType = (TurnType)(curTurnTypePos_);

      curActorVectorPos_++;

      if((size_t)curActorVectorPos_ >= actors_.size()) {
        curActorVectorPos_ = 0;
        curTurnTypePos_++;
        if(curTurnTypePos_ == int(TurnType::endOfTurnType)) {
          curTurnTypePos_ = 0;
        }

        if(curTurnType != TurnType::fast && curTurnType != TurnType::fastest) {
          runStandardTurnEvents();
        }
      }

      curActor = getCurActor();
      vector<PropId> props;
      curActor->getPropHandler().getAllActivePropIds(props);

      const bool IS_SLOWED =
        find(props.begin(), props.end(), propSlowed) != props.end();
      const ActorSpeed defSpeed = curActor->getData().speed;
      const ActorSpeed realSpeed =
        !IS_SLOWED || defSpeed == ActorSpeed::sluggish ?
        defSpeed : ActorSpeed(int(defSpeed) - 1);
      switch(realSpeed) {
        case ActorSpeed::sluggish: {
          actorWhoCanActThisTurnFound = (curTurnType == TurnType::slow ||
                                         curTurnType == TurnType::normal2)
                                        && Rnd::fraction(2, 3);
        } break;

        case ActorSpeed::slow: {
          actorWhoCanActThisTurnFound = curTurnType == TurnType::slow ||
                                        curTurnType == TurnType::normal2;
        } break;

        case ActorSpeed::normal: {
          actorWhoCanActThisTurnFound = curTurnType != TurnType::fast &&
                                        curTurnType != TurnType::fastest;
        } break;

        case ActorSpeed::fast: {
          actorWhoCanActThisTurnFound = curTurnType != TurnType::fastest;
        } break;

        case ActorSpeed::fastest: {
          actorWhoCanActThisTurnFound = true;
        } break;

        case ActorSpeed::endOfActorSpeed: {} break;
      }
    }
  }
}

void updateLightMap() {
  bool lightTmp[MAP_W][MAP_H];

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      Map::cells[x][y].isLight = false;
      lightTmp[x][y] = false;
    }
  }

  Map::player->addLight(lightTmp);

  const int NR_ACTORS = actors_.size();
  for(int i = 0; i < NR_ACTORS; ++i) {
    actors_.at(i)->addLight(lightTmp);
  }

  const int NR_FEATURE_MOBS = featureMobs_.size();
  for(int i = 0; i < NR_FEATURE_MOBS; ++i) {
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

Actor* getCurActor() {
  Actor* const actor = actors_.at(curActorVectorPos_);

  //Sanity check actor retrieved
  assert(Utils::isPosInsideMap(actor->pos));
  return actor;
}

} //GameTime

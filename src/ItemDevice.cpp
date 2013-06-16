#include "ItemDevice.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Render.h"
#include "GameTime.h"
#include "Log.h"
#include "Knockback.h"
#include "Inventory.h"
#include "Map.h"

//---------------------------------------------------- BASE CLASS
bool Device::activateDefault(Actor* const actor, Engine* const engine) {
  (void)actor;

  if(def_->isIdentified) {
    bool isDestroyed = toggle(engine);
    engine->gameTime->endTurnOfCurrentActor();
    return isDestroyed;
  } else {
    engine->log->addMessage("I cannot yet use this.");
    return false;
  }
}

bool Device::toggle(Engine* const engine) {
  printToggleMessage(engine);

  if(isActivated_) {
    isActivated_ = false;
    nrTurnsToNextGoodEffect_ = nrTurnsToNextBadEffect_ = -1;
    specificToggle(engine);
  } else {
    isActivated_ = true;
    nrTurnsToNextGoodEffect_ = getRandomNrTurnsToNextGoodEffect(engine);
    nrTurnsToNextBadEffect_ = getRandomNrTurnsToNextBadEffect(engine);
    const string message = getSpecificActivateMessage();
    if(message != "") {
      engine->log->addMessage(message);
    }
    specificToggle(engine);
//    runBadEffect(engine);
  }
  return false;
}

void Device::printToggleMessage(Engine* const engine) {
  const string name_a = engine->itemData->getItemRef(*this, itemRef_a, true);
  engine->log->addMessage((isActivated_ ? "I deactive " : "I activate ") + name_a + ".");
}

int Device::getRandomNrTurnsToNextGoodEffect(Engine* const engine) const {
  return engine->dice.getInRange(6, 9);
}

int Device::getRandomNrTurnsToNextBadEffect(Engine* const engine) const {
  return engine->dice.getInRange(12, 16);
}

void Device::newTurnInInventory(Engine* const engine) {
  if(isActivated_) {

    specificnewTurnInInventory(engine);

    if(--nrTurnsToNextGoodEffect_ <= 0) {
      nrTurnsToNextGoodEffect_ = getRandomNrTurnsToNextGoodEffect(engine);
      runGoodEffect(engine);
    }
    if(--nrTurnsToNextBadEffect_ <= 0) {
      nrTurnsToNextBadEffect_ = getRandomNrTurnsToNextBadEffect(engine);
      runBadEffect(engine);
    }
  }
}

void Device::runBadEffect(Engine* const engine) {
  const string name = engine->itemData->getItemRef(*this, itemRef_plain, true);

  const int RND = engine->dice.percentile();
  if(RND < 2) {
    engine->log->addMessage("The " + name + " breaks!");
    engine->player->getInventory()->removetemInGeneralWithPointer(this, false);
  } else if(RND < 40) {
    engine->log->addMessage("I am hit with a jolt of electricity from the " + name + ".", clrMessageBad, messageInterrupt_force);
    engine->player->getStatusEffectsHandler()->tryAddEffect(new StatusParalyzed(2));
    engine->player->hit(engine->dice.getInRange(1, 2), dmgType_electric);
  } else {
    engine->log->addMessage("The " + name + " hums ominously.");
  }
}

void Device::itemSpecificAddSaveLines(vector<string>& lines) {
  lines.push_back(isActivated_ ? "1" : "0");
  deviceSpecificAddSaveLines(lines);
}

void Device::itemSpecificSetParametersFromSaveLines(vector<string>& lines) {
  isActivated_ = lines.back() == "1";
  lines.erase(lines.begin());
  deviceSpecificSetParametersFromSaveLines(lines);
}

void Device::identify(const bool IS_SILENT_IDENTIFY) {
  (void)IS_SILENT_IDENTIFY;

  def_->isIdentified = true;
}

//---------------------------------------------------- SENTRY
string DeviceSentry::getSpecificActivateMessage() {
  return "It seems to peruse area.";
}

void DeviceSentry::runGoodEffect(Engine* const engine) {
  const int DMG = engine->dice(1, 6) + 2;

  engine->player->getSpotedEnemies();
  vector<Actor*>& targetCandidates = engine->player->spotedEnemies;
  const unsigned int NR_TARGET_CANDIDATES = targetCandidates.size();
  if(NR_TARGET_CANDIDATES > 0) {
    Actor* const actor = targetCandidates.at(engine->dice.getInRange(0, NR_TARGET_CANDIDATES - 1));
    const coord& pos = actor->pos;
    engine->log->addMessage(actor->getNameThe() + " is hit by a bolt of lightning!", clrMessageGood, messageInterrupt_force);
    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
      vector<coord>(1, pos), clrYellow, 1);
    actor->hit(DMG, dmgType_electric);
  }
}

//---------------------------------------------------- REPELLER
string DeviceRepeller::getSpecificActivateMessage() {
  return "I feel a certain tension in the air around me.";
}

void DeviceRepeller::runGoodEffect(Engine* const engine) {
  const coord& playerPos = engine->player->pos;
  const unsigned int NR_ACTORS = engine->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    Actor* const actor = engine->gameTime->getActorAt(i);
    if(actor != engine->player) {
      const coord& otherPos = actor->pos;
      if(engine->mapTests->isCellsNeighbours(playerPos, otherPos, false)) {
        engine->knockBack->tryKnockBack(actor, playerPos, false, true);
      }
    }
  }
}

int DeviceRepeller::getRandomNrTurnsToNextGoodEffect(Engine* const engine) const {
  return engine->dice.getInRange(2, 4);
}

//---------------------------------------------------- REJUVENATOR
string DeviceRejuvenator::getSpecificActivateMessage() {
  return "It seems to attempt repairing my flesh.";
}

void DeviceRejuvenator::runGoodEffect(Engine* const engine) {
//  const string name = engine->itemData->getItemRef(this, itemRef_plain, true);
//  engine->log->addMessage(name + " repairs my wounds.");
  engine->player->restoreHP(1, false);
}

//---------------------------------------------------- TRANSLOCATOR
string DeviceTranslocator::getSpecificActivateMessage() {
  return "";
}

void DeviceTranslocator::runGoodEffect(Engine* const engine) {
  Player* const player = engine->player;
  player->getSpotedEnemiesPositions();
  if(
    player->getHp() <= player->getHpMax(true) / 4 &&
    player->spotedEnemiesPositions.empty() == false) {
    const string name = engine->itemData->getItemRef(*this, itemRef_plain, true);
    engine->log->addMessage("The " + name + " makes a droning noise...");
    player->teleport(true);
  }
}

//---------------------------------------------------- SPELL REFLECTOR
//void DeviceSpellReflector::runGoodEffect(Engine* const engine) {
//
//}

//---------------------------------------------------- ELECTRIC LANTERN
void DeviceElectricLantern::specificnewTurnInInventory(Engine* const engine) {
  if(isActivated_ && malfunctCooldown_ > 0) {
    malfunctCooldown_--;
    if(malfunctCooldown_ <= 0) {
      engine->gameTime->updateLightMap();
      engine->player->updateFov();
      engine->renderer->drawMapAndInterface();
    }
  }
}

void DeviceElectricLantern::printToggleMessage(Engine* const engine) {
  const string toggleStr = isActivated_ ? "I turn off" : "I turn on";
  engine->log->addMessage(toggleStr + " an Electric Lantern.");
}

void DeviceElectricLantern::specificToggle(Engine* const engine) {
  engine->gameTime->updateLightMap();
  engine->player->updateFov();
  engine->renderer->drawMapAndInterface();
}

bool DeviceElectricLantern::isGivingLight() const {
  return isActivated_ && malfunctCooldown_ <= 0;
}

void DeviceElectricLantern::runBadEffect(Engine* const engine) {
  if(malfunctCooldown_ <= 0) {
    bool isVisionUpdateNeeded = false;
    bool isItemDestroyed = false;

    const int RND = engine->dice.percentile();
    if(RND < 4) {
      engine->log->addMessage("My Electric Lantern breaks!");
      engine->player->getInventory()->removetemInGeneralWithPointer(this, false);
      isVisionUpdateNeeded = true;
      isItemDestroyed = true;
    } else if(RND < 20) {
      engine->log->addMessage("My Electric Lantern malfunctions.");
      malfunctCooldown_ = engine->dice.getInRange(3, 4);
      isVisionUpdateNeeded = true;
    } else if(RND < 50) {
      engine->log->addMessage("My Electric Lantern flickers.");
      malfunctCooldown_ = 2;
      isVisionUpdateNeeded = true;
    }

    if(isVisionUpdateNeeded) {
      engine->gameTime->updateLightMap();
      engine->player->updateFov();
      engine->renderer->drawMapAndInterface();
    }
    if(isItemDestroyed) {
      delete this;
    }
  }
}



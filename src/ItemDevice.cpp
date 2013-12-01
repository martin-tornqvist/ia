#include "ItemDevice.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Renderer.h"
#include "GameTime.h"
#include "Log.h"
#include "Knockback.h"
#include "Inventory.h"
#include "Map.h"
#include "Audio.h"

//---------------------------------------------------- BASE CLASS
Device::Device(ItemData* const itemData, Engine* engine) :
  Item(itemData, engine), isActivated_(false), nrTurnsToNextGoodEffect_(-1),
  nrTurnsToNextBadEffect_(-1) {}

bool Device::activateDefault(Actor* const actor) {
  (void)actor;

  if(data_->isIdentified) {
    bool isDestroyed = toggle();
    eng->gameTime->endTurnOfCurrentActor();
    return isDestroyed;
  } else {
    eng->log->addMsg("I cannot yet use this.");
    return false;
  }
}

bool Device::toggle() {
  printToggleMessage();

  if(isActivated_) {
    isActivated_ = false;
    nrTurnsToNextGoodEffect_ = nrTurnsToNextBadEffect_ = -1;
    specificToggle();
  } else {
    isActivated_ = true;
    nrTurnsToNextGoodEffect_ = getRandomNrTurnsToNextGoodEffect();
    nrTurnsToNextBadEffect_ = getRandomNrTurnsToNextBadEffect();
    specificToggle();
    const string message = getSpecificActivateMessage();
    if(message.empty() == false) {
      eng->log->addMsg(message);
    }
  }
  return false;
}

void Device::printToggleMessage() {
  const string name_a =
    eng->itemDataHandler->getItemRef(*this, itemRef_a, true);
  eng->log->addMsg(
    (isActivated_ ? "I deactive " : "I activate ") + name_a + ".");
}

int Device::getRandomNrTurnsToNextGoodEffect() const {
  return eng->dice.range(6, 9);
}

int Device::getRandomNrTurnsToNextBadEffect() const {
  return eng->dice.range(12, 16);
}

void Device::newTurnInInventory() {
  if(isActivated_) {

    specificnewTurnInInventory();

    if(--nrTurnsToNextGoodEffect_ <= 0) {
      nrTurnsToNextGoodEffect_ = getRandomNrTurnsToNextGoodEffect();
      runGoodEffect();
    }
    if(--nrTurnsToNextBadEffect_ <= 0) {
      nrTurnsToNextBadEffect_ = getRandomNrTurnsToNextBadEffect();
      runBadEffect();
    }
  }
}

void Device::runBadEffect() {
  const string name =
    eng->itemDataHandler->getItemRef(*this, itemRef_plain, true);

  const int RND = eng->dice.percentile();
  if(RND < 2) {
    eng->log->addMsg("The " + name + " breaks!");
    eng->player->getInventory()->removetemInGeneralWithPointer(this, false);
  } else if(RND < 40) {
    eng->log->addMsg(
      "I am hit with a jolt of electricity from the " + name +
      ".", clrMessageBad, true);
    eng->player->getPropHandler()->tryApplyProp(
      new PropParalyzed(eng, propTurnsSpecified, 2));
    eng->player->hit(eng->dice.range(1, 2), dmgType_electric, false);
  } else {
    eng->log->addMsg("The " + name + " hums ominously.");
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
  (void)eng;
  (void)IS_SILENT_IDENTIFY;

  data_->isIdentified = true;
}

//---------------------------------------------------- SENTRY
string DeviceSentry::getSpecificActivateMessage() {
  return "It seems to peruse area.";
}

void DeviceSentry::runGoodEffect() {
  const int DMG = eng->dice(1, 6) + 2;

  vector<Actor*> targetCandidates;
  eng->player->getSpotedEnemies(targetCandidates);
  const unsigned int NR_CANDIDATES = targetCandidates.size();
  if(NR_CANDIDATES > 0) {
    const int ELEMENT = eng->dice.range(0, NR_CANDIDATES - 1);
    Actor* const actor = targetCandidates.at(ELEMENT);
    const Pos& pos = actor->pos;
    eng->log->addMsg(
      actor->getNameThe() + " is hit by a bolt of lightning!",
      clrMessageGood, true);
    eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
      vector<Pos>(1, pos), clrYellow);
    actor->hit(DMG, dmgType_electric, false);
  }
}

//---------------------------------------------------- REPELLER
string DeviceRepeller::getSpecificActivateMessage() {
  return "I feel a certain tension in the air around me.";
}

void DeviceRepeller::runGoodEffect() {
  const Pos& playerPos = eng->player->pos;
  const unsigned int NR_ACTORS = eng->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    Actor* const actor = eng->gameTime->getActorAtElement(i);
    if(actor != eng->player) {
      const Pos& otherPos = actor->pos;
      if(eng->basicUtils->isPosAdj(playerPos, otherPos, false)) {
        eng->knockBack->tryKnockBack(actor, playerPos, false, true);
      }
    }
  }
}

int DeviceRepeller::getRandomNrTurnsToNextGoodEffect() const {
  return eng->dice.range(2, 4);
}

//---------------------------------------------------- REJUVENATOR
string DeviceRejuvenator::getSpecificActivateMessage() {
  return "It seems to attempt repairing my flesh.";
}

void DeviceRejuvenator::runGoodEffect() {
//  const string name = eng->itemData->getItemRef(this, itemRef_plain, true);
//  eng->log->addMsg(name + " repairs my wounds.");
  eng->player->restoreHp(1, false);
}

//---------------------------------------------------- TRANSLOCATOR
string DeviceTranslocator::getSpecificActivateMessage() {
  return "";
}

void DeviceTranslocator::runGoodEffect() {
  Player* const player = eng->player;
  vector<Actor*> spotedEnemies;
  player->getSpotedEnemies(spotedEnemies);
  if(
    player->getHp() <= player->getHpMax(true) / 4 &&
    spotedEnemies.empty() == false) {
    const string name = eng->itemDataHandler->getItemRef(
                          *this, itemRef_plain, true);
    eng->log->addMsg("The " + name + " makes a droning noise...");
    player->teleport(true);
  }
}

//---------------------------------------------------- ELECTRIC LANTERN
void DeviceElectricLantern::specificnewTurnInInventory() {
  if(isActivated_ && malfunctCooldown_ > 0) {
    malfunctCooldown_--;
    if(malfunctCooldown_ <= 0) {
      eng->gameTime->updateLightMap();
      eng->player->updateFov();
      eng->renderer->drawMapAndInterface();
    }
  }
}

void DeviceElectricLantern::printToggleMessage() {
  const string toggleStr = isActivated_ ? "I turn off" : "I turn on";
  eng->log->addMsg(toggleStr + " an Electric Lantern.");
}

void DeviceElectricLantern::specificToggle() {
  eng->audio->play(sfxElectricLantern);
  eng->gameTime->updateLightMap();
  eng->player->updateFov();
  eng->renderer->drawMapAndInterface();
}

bool DeviceElectricLantern::isGivingLight() const {
  return isActivated_ && malfunctCooldown_ <= 0;
}

void DeviceElectricLantern::runBadEffect() {
  if(malfunctCooldown_ <= 0) {
    bool isVisionUpdateNeeded = false;
    bool isItemDestroyed = false;

    const int RND = eng->dice.percentile();
    if(RND < 6) {
      eng->log->addMsg("My Electric Lantern breaks!");
      eng->player->getInventory()->removetemInGeneralWithPointer(this, false);
      isVisionUpdateNeeded = true;
      isItemDestroyed = true;
    } else if(RND < 20) {
      eng->log->addMsg("My Electric Lantern malfunctions.");
      malfunctCooldown_ = eng->dice.range(3, 4);
      isVisionUpdateNeeded = true;
    } else if(RND < 50) {
      eng->log->addMsg("My Electric Lantern flickers.");
      malfunctCooldown_ = 2;
      isVisionUpdateNeeded = true;
    }

    if(isVisionUpdateNeeded) {
      eng->gameTime->updateLightMap();
      eng->player->updateFov();
      eng->renderer->drawMapAndInterface();
    }
    if(isItemDestroyed) {
      delete this;
    }
  }
}



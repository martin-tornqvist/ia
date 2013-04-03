#include "ItemDevice.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "Render.h"
#include "GameTime.h"
#include "Log.h"
#include "Knockback.h"
#include "Inventory.h"

//---------------------------------------------------- BASE CLASS
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
    specificToggle(engine);
    runBadEffect(engine);
  }
  return false;
}

void Device::printToggleMessage(Engine* const engine) {
  const string name_a = engine->itemData->getItemRef(this, itemRef_a, true);
  engine->log->addMessage((isActivated_ ? "I deactive " : "I activate ") + name_a + ".");
}

int Device::getRandomNrTurnsToNextGoodEffect(Engine* const engine) const {
  return engine->dice.getInRange(6, 10);
}

int Device::getRandomNrTurnsToNextBadEffect(Engine* const engine) const {
  return engine->dice.getInRange(6, 10);
}

void Device::newTurn(Engine* const engine) {
  if(isActivated_) {

    specificNewTurn(engine);

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

}

//---------------------------------------------------- SENTRY
void DeviceSentry::runGoodEffect(Engine* const engine) {
  const int DMG = engine->dice(1, 6) + 2;

  engine->player->getSpotedEnemies();
  vector<Actor*>& targetCandidates = engine->player->spotedEnemies;
  const unsigned int NR_TARGET_CANDIDATES = targetCandidates.size();
  if(NR_TARGET_CANDIDATES > 0) {
    Actor* const actor = targetCandidates.at(engine->dice.getInRange(0, NR_TARGET_CANDIDATES - 1));
    const coord& pos = actor->pos;
    engine->log->addMessage(actor->getNameThe() + " is hit by a bolt of lightning!", clrMessageGood, messageInterrupt_force);
    engine->renderer->drawBlastAnimationAtPositionsWithPlayerVision(vector<coord>(1, pos), clrYellow, 1, engine);
    actor->hit(DMG, damageType_electric);
  }
}

//---------------------------------------------------- REPELLER
void DeviceRepeller::runGoodEffect(Engine* const engine) {
  const coord& playerPos = engine->player->pos;
  const unsigned int NR_ACTORS = engine->gameTime->getLoopSize();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    Actor* const actor = engine->gameTime->getActorAt(i);
    if(actor != engine->player) {
      const coord& otherPos = actor->pos;
      if(engine->mapTests->isCellsNeighbours(playerPos, otherPos, false)) {
        engine->knockBack->attemptKnockBack(actor, playerPos, false, true);
      }
    }
  }
}

int DeviceRepeller::getRandomNrTurnsToNextGoodEffect(Engine* const engine) const {
  return engine->dice.getInRange(2, 4);
}

//---------------------------------------------------- REJUVENATOR
void DeviceRejuvenator::runGoodEffect(Engine* const engine) {

}

//---------------------------------------------------- TRANSLOCATOR
void DeviceTranslocator::runGoodEffect(Engine* const engine) {

}

//---------------------------------------------------- SPELL REFLECTOR
//void DeviceSpellReflector::runGoodEffect(Engine* const engine) {
//
//}

//---------------------------------------------------- ELECTRIC LANTERN
void DeviceElectricLantern::specificNewTurn(Engine* const engine) {
  if(isActivated_ && malfunctCooldown_ > 0) {
    malfunctCooldown_--;
    if(malfunctCooldown_ <= 0) {
      engine->log->addMessage("The Electric Lantern is casting light.");
      engine->gameTime->updateLightMap();
      engine->player->updateFov();
      engine->renderer->drawMapAndInterface();
    }
  }
}

void DeviceElectricLantern::printToggleMessage(Engine* const engine) {
  const string toggleStr = isActivated_ ? "I turn off" : "I turn on";
  engine->log->addMessage(toggleStr + " the Electric Lantern.");
}

void DeviceElectricLantern::runGoodEffect(Engine* const engine) {
  (void)engine;
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

    const int RND = engine->dice(1, 100);
    if(RND < 3) {
      engine->log->addMessage("The Electric Lantern breaks!");
      engine->player->getInventory()->removetemInGeneralWithPointer(this, false);
      isVisionUpdateNeeded = true;
      isItemDestroyed = true;
    } else {
      if(RND < 20) {
        engine->log->addMessage("The Electric Lantern malfunctions.");
        malfunctCooldown_ = engine->dice.getInRange(2, 4);
        isVisionUpdateNeeded = true;
      } else {
        if(RND < 50) {
          engine->log->addMessage("The Electric Lantern flickers.");
          malfunctCooldown_ = 2;
          isVisionUpdateNeeded = true;
        }
      }
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



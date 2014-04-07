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
#include "Utils.h"

//---------------------------------------------------- BASE CLASS
Device::Device(ItemData* const itemData, Engine& engine) :
  Item(itemData, engine),
  condition_(Rnd::coinToss() ? Condition::fine : Condition::shoddy) {}

void Device::identify(const bool IS_SILENT_IDENTIFY) {
  (void)eng;
  (void)IS_SILENT_IDENTIFY;

  data_->isIdentified = true;
}

void Device::addSaveLines(vector<string>& lines) {
  lines.push_back(toStr(int(condition_)));
}

void Device::setParamsFromSaveLines(vector<string>& lines) {
  condition_ = Condition(toInt(lines.front()));
  lines.erase(lines.begin());
}

//---------------------------------------------------- STRANGE DEVICE BASE
StrangeDevice::StrangeDevice(ItemData* const itemData, Engine& engine) :
  Device(itemData, engine) {}

ConsumeItem StrangeDevice::activateDefault(Actor* const actor) {
  (void)actor;

  if(data_->isIdentified) {
    const string itemName =
      eng.itemDataHandler->getItemRef(*this, ItemRefType::plain, true);
    const string itemNameA =
      eng.itemDataHandler->getItemRef(*this, ItemRefType::a, true);

    eng.log->addMsg("I activate " + itemNameA + "...");

    //Damage user? Fail to run effect? Condition degrade? Warning?
    const string hurtMsg = "It hits me with a jolt of electricity!";
    bool isEffectFailed = false;
    bool isCondDegrade  = false;
    bool isWarning      = false;
    int bon = 0;
    if(actor->getPropHandler().hasProp(propCursed)) {bon -= 2;}
    const int RND = Rnd::range(1, 8 + bon);
    switch(condition_) {
      case Condition::breaking: {
        if(RND == 5 || RND == 6) {
          eng.log->addMsg(hurtMsg, clrMsgBad);
          actor->hit(Rnd::dice(2, 4), DmgType::electric, true);
        }
        isEffectFailed  = RND == 3 || RND == 4;
        isCondDegrade   = RND <= 2;
        isWarning       = RND == 7 || RND == 8;
      } break;

      case Condition::shoddy: {
        if(RND == 4) {
          eng.log->addMsg(hurtMsg, clrMsgBad);
          actor->hit(Rnd::dice(1, 4), DmgType::electric, true);
        }
        isEffectFailed  = RND == 3;
        isCondDegrade   = RND <= 2;
        isWarning       = RND == 5 || RND == 6;
      } break;

      case Condition::fine: {
        isCondDegrade   = RND <= 2;
        isWarning       = RND == 3 || RND == 4;
      } break;
    }

    if(eng.player->deadState != ActorDeadState::alive) {
      return ConsumeItem::no;
    }
    if(isEffectFailed) {
      eng.log->addMsg("It suddenly stops.");
    } else {
      triggerEffect();
    }

    bool isDestroyed = false;
    if(isCondDegrade) {
      if(condition_ == Condition::breaking) {
        eng.log->addMsg("The " + itemName + " breaks!");
        isDestroyed = true;
      } else {
        eng.log->addMsg("The " + itemName + " makes a terrible grinding "
                        "noise.");
        eng.log->addMsg("I seem to have damaged it.");
        condition_ = Condition(int(condition_) - 1);
      }
    }

    if(isWarning) {
      eng.log->addMsg("The " + itemName + " hums ominously.");
    }

    eng.gameTime->actorDidAct();
    return isDestroyed ? ConsumeItem::yes : ConsumeItem::no;
  } else {
    eng.log->addMsg("This device is completely alien to me, ");
    eng.log->addMsg("I could never understand it through normal means.");
    return ConsumeItem::no;
  }
}

//---------------------------------------------------- SENTRY
void DeviceSentry::triggerEffect() {
  vector<Actor*> targetCandidates;
  eng.player->getSpottedEnemies(targetCandidates);
  if(targetCandidates.empty()) {
    eng.log->addMsg("It seems to peruse area.");
  } else {
    Spell* const spell = eng.spellHandler->getSpellFromId(spell_azathothsWrath);
    spell->cast(eng.player, false, eng);
    delete spell;
  }
}

//---------------------------------------------------- REPELLER
void DeviceRepeller::triggerEffect() {
  eng.log->addMsg("It triggers a shockwave around me.");

  const Pos& playerPos = eng.player->pos;
  for(Actor * actor : eng.gameTime->actors_) {
    if(actor != eng.player && actor->deadState == ActorDeadState::alive) {
      const Pos& otherPos = actor->pos;
      if(Utils::isPosAdj(playerPos, otherPos, false)) {
        actor->hit(Rnd::dice(1, 8), DmgType::physical, true);
        if(actor->deadState == ActorDeadState::alive) {
          eng.knockBack->tryKnockBack(*actor, playerPos, false, true);
        }
      }
    }
  }
}

//---------------------------------------------------- REJUVENATOR
void DeviceRejuvenator::triggerEffect() {
  eng.log->addMsg("It repairs my body.");
  eng.player->getPropHandler().endAppliedPropsByMagicHealing();
  eng.player->restoreHp(999, false);
}

//---------------------------------------------------- TRANSLOCATOR
void DeviceTranslocator::triggerEffect() {
  Player* const player = eng.player;
  vector<Actor*> spottedEnemies;
  player->getSpottedEnemies(spottedEnemies);

  if(spottedEnemies.empty()) {
    eng.log->addMsg("It seems to peruse area.");
  } else {
    for(Actor * actor : spottedEnemies) {
      eng.log->addMsg(actor->getNameThe() + " is teleported.");
      Renderer::drawBlastAnimAtPositions(vector<Pos> {actor->pos}, clrYellow);
      actor->teleport(false);
    }
  }
}

//---------------------------------------------------- ELECTRIC LANTERN
DeviceLantern::DeviceLantern(ItemData* const itemData, Engine& engine) :
  Device(itemData, engine),
  malfunctCooldown_(-1),
  malfState_(LanternMalfState::working),
  isActivated_(false),
  nrTurnsToNextBadEffect_(-1) {}

ConsumeItem DeviceLantern::activateDefault(Actor* const actor) {
  (void)actor;
  toggle();
  return ConsumeItem::no;
}

void DeviceLantern::addSaveLines(vector<string>& lines) {
  const int CONDITION_INT = int(condition_);
  lines.push_back(toStr(CONDITION_INT));
  lines.push_back(isActivated_ ? "1" : "0");
}

void DeviceLantern::setParamsFromSaveLines(vector<string>& lines) {
  condition_ = Condition(toInt(lines.front()));
  lines.erase(lines.begin());
  isActivated_ = lines.back() == "1";
  lines.erase(lines.begin());
}

void DeviceLantern::toggle() {
  const string toggleStr = isActivated_ ? "I turn off" : "I turn on";
  eng.log->addMsg(toggleStr + " an Electric Lantern.");

  isActivated_ = !isActivated_;

  Audio::play(SfxId::electricLantern);
  eng.gameTime->updateLightMap();
  eng.player->updateFov();
  Renderer::drawMapAndInterface();
}

void DeviceLantern::newTurnInInventory() {
  if(isActivated_ && malfunctCooldown_ > 0) {
    malfunctCooldown_--;
    if(malfunctCooldown_ <= 0) {
      malfState_ = LanternMalfState::working;
      eng.gameTime->updateLightMap();
      eng.player->updateFov();
      Renderer::drawMapAndInterface();
    }
  }
}

LanternLightSize DeviceLantern::getCurLightSize() const {
  if(isActivated_) {
    switch(malfState_) {
      case LanternMalfState::working:     return LanternLightSize::normal;
      case LanternMalfState::flicker:     return LanternLightSize::small;
      case LanternMalfState::malfunction: return LanternLightSize::none;
      case LanternMalfState::destroyed:   return LanternLightSize::none;
    }
  }
  return LanternLightSize::none;
}

void DeviceLantern::runBadEffect() {
  if(malfunctCooldown_ <= 0) {

    const int RND = Rnd::percentile();

    if(RND < 5) {
      eng.log->addMsg("My Electric Lantern breaks!");
      eng.player->getInv().removetemInGeneralWithPointer(this, false);
      malfState_ = LanternMalfState::destroyed;
    } else if(RND < 20) {
      eng.log->addMsg("My Electric Lantern malfunctions.");
      malfState_        = LanternMalfState::malfunction;
      malfunctCooldown_ = Rnd::range(3, 4);
    } else if(RND < 55) {
      eng.log->addMsg("My Electric Lantern starts to flicker.");
      malfState_        = LanternMalfState::flicker;
      malfunctCooldown_ = Rnd::range(6, 12);
    } else {
      malfState_        = LanternMalfState::working;
    }

    if(malfState_ != LanternMalfState::working) {
      eng.gameTime->updateLightMap();
      eng.player->updateFov();
      Renderer::drawMapAndInterface();
    }
    if(malfState_ == LanternMalfState::destroyed) {delete this;}
  }
}

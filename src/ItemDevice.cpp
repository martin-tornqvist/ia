#include "ItemDevice.h"

#include <algorithm>
#include <vector>

#include "ActorPlayer.h"
#include "Render.h"
#include "GameTime.h"
#include "Log.h"
#include "Knockback.h"
#include "Inventory.h"
#include "Map.h"
#include "Audio.h"
#include "Utils.h"

using namespace std;

//---------------------------------------------------- DEVICE
Device::Device(ItemDataT* const itemData) :
  Item(itemData),
  condition_(Rnd::coinToss() ? Condition::fine : Condition::shoddy) {}

void Device::identify(const bool IS_SILENT_IDENTIFY) {
  (void)IS_SILENT_IDENTIFY;

  data_->isIdentified = true;
}

void Device::storeToSaveLines(vector<string>& lines) {
  lines.push_back(toStr(int(condition_)));
}

void Device::setupFromSaveLines(vector<string>& lines) {
  condition_ = Condition(toInt(lines.front()));
  lines.erase(begin(lines));
}

//---------------------------------------------------- STRANGE DEVICE
StrangeDevice::StrangeDevice(ItemDataT* const itemData) :
  Device(itemData) {}

ConsumeItem StrangeDevice::activateDefault(Actor* const actor) {
  (void)actor;

  if(data_->isIdentified) {
    const string itemName   = getName(ItemRefType::plain);
    const string itemNameA  = getName(ItemRefType::a);

    Log::addMsg("I activate " + itemNameA + "...");

    //Damage user? Fail to run effect? Condition degrade? Warning?
    const string hurtMsg = "It hits me with a jolt of electricity!";
    bool isEffectFailed = false;
    bool isCondDegrade  = false;
    bool isWarning      = false;
    int bon = 0;
    vector<PropId> props;
    actor->getPropHandler().getAllActivePropIds(props);
    if(find(begin(props), end(props), propBlessed) != end(props)) {
      bon += 2;
    }
    if(find(begin(props), end(props), propCursed) != end(props)) {
      bon -= 2;
    }
    const int RND = Rnd::range(1, 8 + bon);
    switch(condition_) {
      case Condition::breaking: {
        if(RND == 5 || RND == 6) {
          Log::addMsg(hurtMsg, clrMsgBad);
          actor->hit(Rnd::dice(2, 4), DmgType::electric);
        }
        isEffectFailed  = RND == 3 || RND == 4;
        isCondDegrade   = RND <= 2;
        isWarning       = RND == 7 || RND == 8;
      } break;

      case Condition::shoddy: {
        if(RND == 4) {
          Log::addMsg(hurtMsg, clrMsgBad);
          actor->hit(Rnd::dice(1, 4), DmgType::electric);
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

    if(Map::player->deadState != ActorDeadState::alive) {
      return ConsumeItem::no;
    }
    if(isEffectFailed) {
      Log::addMsg("It suddenly stops.");
    } else {
      triggerEffect();
    }

    bool isDestroyed = false;
    if(isCondDegrade) {
      if(condition_ == Condition::breaking) {
        Log::addMsg("The " + itemName + " breaks!");
        isDestroyed = true;
      } else {
        Log::addMsg("The " + itemName + " makes a terrible grinding "
                    "noise.");
        Log::addMsg("I seem to have damaged it.");
        condition_ = Condition(int(condition_) - 1);
      }
    }

    if(isWarning) {
      Log::addMsg("The " + itemName + " hums ominously.");
    }

    GameTime::actorDidAct();
    return isDestroyed ? ConsumeItem::yes : ConsumeItem::no;
  } else {
    Log::addMsg("This device is completely alien to me, ");
    Log::addMsg("I could never understand it through normal means.");
    return ConsumeItem::no;
  }
}

std::string StrangeDevice::getNameInf() const {
  if(data_->isIdentified) {
    switch(condition_) {
      case Condition::breaking: return "{breaking}";
      case Condition::shoddy:   return "{shoddy}";
      case Condition::fine:     return "{fine}";
    }
  }
  return "";
}

//---------------------------------------------------- SENTRY
void DeviceSentry::triggerEffect() {
  vector<Actor*> targetBucket;
  Map::player->getSeenFoes(targetBucket);
  if(targetBucket.empty()) {
    Log::addMsg("It seems to peruse area.");
  } else {
    Spell* const spell = SpellHandling::mkSpellFromId(SpellId::azathothsWrath);
    spell->cast(Map::player, false);
    delete spell;
  }
}

//---------------------------------------------------- REPELLER
void DeviceRepeller::triggerEffect() {
  Log::addMsg("It triggers a shockwave around me.");

  const Pos& playerPos = Map::player->pos;
  for(Actor* actor : GameTime::actors_) {
    if(actor != Map::player && actor->deadState == ActorDeadState::alive) {
      const Pos& otherPos = actor->pos;
      if(Utils::isPosAdj(playerPos, otherPos, false)) {
        actor->hit(Rnd::dice(1, 8), DmgType::physical);
        if(actor->deadState == ActorDeadState::alive) {
          KnockBack::tryKnockBack(*actor, playerPos, false, true);
        }
      }
    }
  }
}

//---------------------------------------------------- REJUVENATOR
void DeviceRejuvenator::triggerEffect() {
  Log::addMsg("It repairs my body.");
  Map::player->getPropHandler().endAppliedPropsByMagicHealing();
  Map::player->restoreHp(999, false);
}

//---------------------------------------------------- TRANSLOCATOR
void DeviceTranslocator::triggerEffect() {
  Player* const player = Map::player;
  vector<Actor*> seenFoes;
  player->getSeenFoes(seenFoes);

  if(seenFoes.empty()) {
    Log::addMsg("It seems to peruse area.");
  } else {
    for(Actor* actor : seenFoes) {
      Log::addMsg(actor->getNameThe() + " is teleported.");
      Render::drawBlastAnimAtPositions(vector<Pos> {actor->pos}, clrYellow);
      actor->teleport(false);
    }
  }
}

//---------------------------------------------------- ELECTRIC LANTERN
DeviceLantern::DeviceLantern(ItemDataT* const itemData) :
  Device(itemData),
  malfunctCooldown_(-1),
  malfState_(LanternMalfState::working),
  isActivated_(false),
  nrTurnsToNextBadEffect_(-1) {}

ConsumeItem DeviceLantern::activateDefault(Actor* const actor) {
  (void)actor;
  toggle();
  GameTime::actorDidAct();
  return ConsumeItem::no;
}

void DeviceLantern::storeToSaveLines(vector<string>& lines) {
  lines.push_back(toStr(int(condition_)));
  lines.push_back(isActivated_ ? "1" : "0");
}

void DeviceLantern::setupFromSaveLines(vector<string>& lines) {
  condition_ = Condition(toInt(lines.front()));
  lines.erase(begin(lines));
  isActivated_ = lines.back() == "1";
  lines.erase(begin(lines));
}

void DeviceLantern::toggle() {
  const string toggleStr = isActivated_ ? "I turn off" : "I turn on";
  Log::addMsg(toggleStr + " an Electric Lantern.");

  isActivated_ = !isActivated_;

  Audio::play(SfxId::electricLantern);
  GameTime::updateLightMap();
  Map::player->updateFov();
  Render::drawMapAndInterface();
}

void DeviceLantern::newTurnInInventory() {
  if(isActivated_) {
    if(malfunctCooldown_ > 0) {
      //A malfunction is active, count down on effect
      malfunctCooldown_--;
      if(malfunctCooldown_ <= 0) {
        malfState_ = LanternMalfState::working;
        GameTime::updateLightMap();
        Map::player->updateFov();
        Render::drawMapAndInterface();
      }
    } else {
      //No malfunction active, check if new should be applied

      const int RND = Rnd::range(1, 1000);

      if(RND <= 6) {
        Log::addMsg("My Electric Lantern breaks!", clrMsgWarning,
                    false, true);
        Map::player->getInv().removeItemInGeneralWithPointer(this, false);
        malfState_ = LanternMalfState::destroyed;
      } else if(RND <= 10) {
        Log::addMsg("My Electric Lantern malfunctions.");
        malfState_        = LanternMalfState::malfunction;
        malfunctCooldown_ = Rnd::range(3, 4);
      } else if(RND <= 40) {
        Log::addMsg("My Electric Lantern starts to flicker.");
        malfState_        = LanternMalfState::flicker;
        malfunctCooldown_ = Rnd::range(4, 12);
      } else {
        malfState_        = LanternMalfState::working;
      }

      if(malfState_ != LanternMalfState::working) {
        GameTime::updateLightMap();
        Map::player->updateFov();
        Render::drawMapAndInterface();
      }
      if(malfState_ == LanternMalfState::destroyed) {delete this;}
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

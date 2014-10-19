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
#include "FeatureRigid.h"

using namespace std;

//---------------------------------------------------- DEVICE
Device::Device(ItemDataT* const itemData) : Item(itemData) {}

void Device::identify(const bool IS_SILENT_IDENTIFY) {
  (void)IS_SILENT_IDENTIFY;

  data_->isIdentified = true;
}

//---------------------------------------------------- STRANGE DEVICE
StrangeDevice::StrangeDevice(ItemDataT* const itemData) :
  Device(itemData),
  condition_(Rnd::coinToss() ? Condition::fine : Condition::shoddy) {}

void StrangeDevice::storeToSaveLines(vector<string>& lines) {
  lines.push_back(toStr(int(condition_)));
}

void StrangeDevice::setupFromSaveLines(vector<string>& lines) {
  condition_ = Condition(toInt(lines.front()));
  lines.erase(begin(lines));
}

vector<string> StrangeDevice::getDescr() const {
  if(data_->isIdentified) {
    auto descr = getDescrIdentified();

    string condStr = "It seems ";

    switch(condition_) {
      case Condition::fine:     condStr += "to be in fine condition.";    break;
      case Condition::shoddy:   condStr += "to be in shoddy condition.";  break;
      case Condition::breaking: condStr += "almost broken.";              break;
    }

    descr.push_back(condStr);

    return descr;
  } else {
    return data_->baseDescr;
  }
}

ConsumeItem StrangeDevice::activateDefault(Actor* const actor) {
  (void)actor;

  if(data_->isIdentified) {
    const string itemName   = getName(ItemRefType::plain, ItemRefInf::none);
    const string itemNameA  = getName(ItemRefType::a, ItemRefInf::none);

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
        Log::addMsg("The " + itemName + " makes a terrible grinding noise.");
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

//---------------------------------------------------- BLASTER
void DeviceBlaster::triggerEffect() {
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

//---------------------------------------------------- SHOCK WAVE
void DeviceShockwave::triggerEffect() {
  Log::addMsg("It triggers a shock wave around me.");

  const Pos& playerPos = Map::player->pos;

  for(int dy = -1; dy <= 1; ++dy) {
    for(int dx = -1; dx <= 1; ++dx) {
      const Pos p(playerPos + Pos(dx, dy));
      Rigid* const rigid = Map::cells[p.x][p.y].rigid;
      rigid->hit(DmgType::physical, DmgMethod::explosion);

      //GameTime::updateLightMap();
      Map::player->updateFov();
      Render::drawMapAndInterface();
    }
  }

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
      Render::drawBlastAtCells(vector<Pos> {actor->pos}, clrYellow);
      actor->teleport(false);
    }
  }
}

//---------------------------------------------------- ELECTRIC LANTERN
DeviceLantern::DeviceLantern(ItemDataT* const itemData) :
  Device(itemData),
  nrTurnsLeft_(Rnd::range(150, 175)),
  nrMalfunctTurnsLeft_(-1),
  malfState_(LanternMalfState::working),
  isActivated_(false) {}

std::string DeviceLantern::getNameInf() const {
  string inf = "{" + toStr(nrTurnsLeft_);
  if(isActivated_) {inf += ", Lit";}
  return inf + "}";
}

ConsumeItem DeviceLantern::activateDefault(Actor* const actor) {
  (void)actor;
  toggle();
  GameTime::actorDidAct();
  return ConsumeItem::no;
}

void DeviceLantern::storeToSaveLines(vector<string>& lines) {
  lines.push_back(toStr(nrTurnsLeft_));
  lines.push_back(toStr(nrMalfunctTurnsLeft_));
  lines.push_back(toStr(int(malfState_)));
  lines.push_back(isActivated_ ? "1" : "0");
}

void DeviceLantern::setupFromSaveLines(vector<string>& lines) {
  nrTurnsLeft_          = toInt(lines.front());
  lines.erase(begin(lines));
  nrMalfunctTurnsLeft_  = toInt(lines.front());
  lines.erase(begin(lines));
  malfState_            = LanternMalfState(toInt(lines.front()));
  lines.erase(begin(lines));
  isActivated_          = lines.front() == "1";
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

    if(malfState_ == LanternMalfState::working) {--nrTurnsLeft_;}

    if(nrTurnsLeft_ <= 0) {
      Log::addMsg("My Electric Lantern breaks!", clrMsgWarning, true, true);

      malfState_ = LanternMalfState::malfunction;
      GameTime::updateLightMap();
      Map::player->updateFov();
      Render::drawMapAndInterface();

      //Note: The following line deletes the object
      Map::player->getInv().removeItemInGeneralWithPtr(this, true);

      return;
    } else if(nrTurnsLeft_ <= 3) {
      Log::addMsg("My Electric Lantern is breaking.", clrMsgWarning, true, true);
    }

    //The lantern is not destroyed. Check malfunctions.
    if(nrMalfunctTurnsLeft_ > 0) {
      //A malfunction is already active, count down on effect instead
      nrMalfunctTurnsLeft_--;
      if(nrMalfunctTurnsLeft_ <= 0) {
        malfState_ = LanternMalfState::working;
        GameTime::updateLightMap();
        Map::player->updateFov();
        Render::drawMapAndInterface();
      }
    } else {
      //No malfunction active, check if new should be applied
      const int RND = Rnd::range(1, 1000);

      if(RND <= 7) {
        Log::addMsg("My Electric Lantern malfunctions.");
        malfState_            = LanternMalfState::malfunction;
        nrMalfunctTurnsLeft_  = Rnd::range(3, 4);
      } else if(RND <= 40) {
        Log::addMsg("My Electric Lantern starts to flicker.");
        malfState_            = LanternMalfState::flicker;
        nrMalfunctTurnsLeft_  = Rnd::range(4, 12);
      } else {
        malfState_            = LanternMalfState::working;
      }

      if(malfState_ != LanternMalfState::working) {
        GameTime::updateLightMap();
        Map::player->updateFov();
        Render::drawMapAndInterface();
      }
    }
  }
}

LanternLightSize DeviceLantern::getCurLightSize() const {
  if(isActivated_) {
    switch(malfState_) {
      case LanternMalfState::working:     return LanternLightSize::normal;
      case LanternMalfState::flicker:     return LanternLightSize::small;
      case LanternMalfState::malfunction: return LanternLightSize::none;
    }
  }
  return LanternLightSize::none;
}

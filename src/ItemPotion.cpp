#include "ItemPotion.h"

#include <climits>

#include "Init.h"
#include "Properties.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ActorMonster.h"
#include "PlayerSpellsHandling.h"
#include "ItemScroll.h"
#include "GameTime.h"
#include "Audio.h"
#include "Render.h"
#include "Inventory.h"
#include "MapTravel.h"
#include "MapParsing.h"
#include "Utils.h"
#include "FeatureRigid.h"
#include "ItemFactory.h"

using namespace std;

void Potion::identify(const bool IS_SILENT_IDENTIFY) {
  if(!data_->isIdentified) {
    data_->isIdentified = true;

    if(!IS_SILENT_IDENTIFY) {
      const string name = getName(ItemRefType::a, ItemRefInf::none);
      Log::addMsg("It was " + name + ".");
      Map::player->incrShock(ShockValue::heavy, ShockSrc::useStrangeItem);
    }
  }
}

vector<string> Potion::getDescr() const {
  if(data_->isIdentified) {
    return getDescrIdentified();
  } else {
    return data_->baseDescr;
  }
}

void Potion::collide(const Pos& pos, Actor* const actor) {
  if(!Map::cells[pos.x][pos.y].rigid->isBottomless() || actor) {

    const bool PLAYER_SEE_CELL = Map::cells[pos.x][pos.y].isSeenByPlayer;

    if(PLAYER_SEE_CELL) {
      //TODO Use standard animation
      Render::drawGlyph('*', Panel::map, pos, data_->clr);

      if(actor) {
        if(actor->deadState == ActorDeadState::alive) {
          Log::addMsg("The potion shatters on " + actor->getNameThe() + ".");
        }
      } else {
        Feature* const f = Map::cells[pos.x][pos.y].rigid;
        Log::addMsg("The potion shatters on " + f->getName(Article::the) + ".");
      }
    }
    //If the blow from the bottle didn't kill the actor, apply what's inside
    if(actor) {
      if(actor->deadState == ActorDeadState::alive) {
        collide_(pos, actor);
        if(
          actor->deadState == ActorDeadState::alive &&
          !data_->isIdentified && PLAYER_SEE_CELL) {
          Log::addMsg("It had no apparent effect...");
        }
      }
    }
  }
}

void Potion::quaff(Actor* const actor) {
  if(actor == Map::player) {
    data_->isTried = true;

    Audio::play(SfxId::potionQuaff);

    if(data_->isIdentified) {
      const string name = getName(ItemRefType::a, ItemRefInf::none);
      Log::addMsg("I drink " + name + "...");
    } else {
      const string name = getName(ItemRefType::plain, ItemRefInf::none);
      Log::addMsg("I drink an unknown " + name + "...");
    }
    Map::player->incrShock(ShockValue::heavy,
                           ShockSrc::useStrangeItem);
  }

  quaff_(actor);

  if(Map::player->deadState == ActorDeadState::alive) {
    GameTime::actorDidAct();
  }
}

string Potion::getNameInf() const {
  return (data_->isTried && !data_->isIdentified) ? "{Tried}" : "";
}

void PotionVitality::quaff_(Actor* const actor) {
  actor->getPropHandler().endAppliedPropsByMagicHealing();

  //HP is always restored at least up to maximum HP, but can go beyond
  const int HP          = actor->getHp();
  const int HP_MAX      = actor->getHpMax(true);
  const int HP_RESTORED = max(20, HP_MAX - HP);

  actor->restoreHp(HP_RESTORED, true, true);

  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionVitality::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionSpirit::quaff_(Actor* const actor) {

  //SPI is always restored at least up to maximum SPI, but can go beyond
  const int SPI           = actor->getSpi();
  const int SPI_MAX       = actor->getSpiMax();
  const int SPI_RESTORED  = max(10, SPI_MAX - SPI);

  actor->restoreSpi(SPI_RESTORED, true, true);

  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionSpirit::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionBlindness::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropBlind(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionBlindness::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionParal::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionParal::collide_(const Pos& pos, Actor* const actor) {

  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionDisease::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropDiseased(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionConf::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropConfused(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionConf::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionFrenzy::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropFrenzied(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionFrenzy::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionFortitude::quaff_(Actor* const actor) {
  PropHandler& propHandler = actor->getPropHandler();

  PropRFear*      const rFear   = new PropRFear(PropTurns::std);
  PropRConfusion* const rConf   = new PropRConfusion(
    PropTurns::specific, rFear->turnsLeft_);
  PropRSleep*     const rSleep  = new PropRSleep(
    PropTurns::specific, rFear->turnsLeft_);

  propHandler.tryApplyProp(rFear);
  propHandler.tryApplyProp(rConf);
  propHandler.tryApplyProp(rSleep);

  if(actor == Map::player) {
    bool isPhobiasCured = false;
    for(int i = 0; i < int(Phobia::END); ++i) {
      if(Map::player->phobias[i]) {
        Map::player->phobias[i] = false;
        isPhobiasCured = true;
      }
    }

    if(isPhobiasCured) {Log::addMsg("All my phobias are cured!");}

    bool isObsessionsCured = false;
    for(int i = 0; i < int(Obsession::END); ++i) {
      if(Map::player->obsessions[i]) {
        Map::player->obsessions[i] = false;
        isObsessionsCured = true;
      }
    }
    if(isObsessionsCured) {Log::addMsg("All my obsessions are cured!");}

    Map::player->restoreShock(999, false);
    Log::addMsg("I feel more at ease.");
  }

  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionFortitude::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionPoison::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropPoisoned(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionPoison::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionRFire::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropRFire(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionRFire::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionAntidote::quaff_(Actor* const actor) {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), visionBlockers);
  const bool IS_POISON_ENDED =
    actor->getPropHandler().endAppliedProp(propPoisoned, visionBlockers);

  if(IS_POISON_ENDED && Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionAntidote::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionRElec::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropRElec(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionRElec::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionRAcid::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropRAcid(PropTurns::std));
  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionRAcid::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionInsight::quaff_(Actor* const actor) {
  (void)actor;

  auto& inv = Map::player->getInv();

  vector<Item*> identifyBucket;

  auto& slots = inv.slots_;
  for(InvSlot& slot : slots) {
    Item* const item = slot.item;
    if(item) {
      const ItemDataT& d = item->getData();
      if(!d.isIdentified) {identifyBucket.push_back(item);}
    }
  }
  vector<Item*>& general = inv.general_;
  for(Item* item : general) {
    if(item->getData().id != ItemId::potionInsight) {
      const ItemDataT& d = item->getData();
      if(!d.isIdentified) {identifyBucket.push_back(item);}
    }
  }

  const size_t NR_ELEMENTS = identifyBucket.size();
  if(NR_ELEMENTS > 0) {
    const int ELEMENT = Rnd::range(0, NR_ELEMENTS - 1);

    Item* const item = identifyBucket.at(ELEMENT);

    const string itemNameBefore = item->getName(ItemRefType::a, ItemRefInf::none);

    item->identify(true);

    const string itemNameAfter = item->getName(ItemRefType::a, ItemRefInf::none);

    Log::addMsg("I gain intuitions about " + itemNameBefore + "...");
    Log::addMsg("It is identified as " + itemNameAfter + "!");
  }

  identify(false);
}

void PotionClairv::quaff_(Actor* const actor) {
  if(actor == Map::player) {
    Log::addMsg("I see far and wide!");

    vector<Pos> animCells;
    animCells.clear();

    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), blocked);
    for(int y = 0; y < MAP_H; ++y) {
      for(int x = 0; x < MAP_W; ++x) {
        Cell& cell = Map::cells[x][y];
        if(!blocked[x][y] && !cell.isDark) {
          cell.isExplored = true;
          cell.isSeenByPlayer = true;
          animCells.push_back(Pos(x, y));
        }
      }
    }

    Render::drawMapAndInterface(false);
//    Map::updateVisualMemory();
    Map::player->updateFov();

    Render::drawBlastAtCells(animCells, clrWhite);
  }
  identify(false);
}

void PotionDescent::quaff_(Actor* const actor) {
  (void)actor;
  if(Map::dlvl < LAST_CAVERN_LVL) {
    Log::addMsg("I sink downwards!", clrWhite, false, true);
    MapTravel::goToNxt();
  } else {
    Log::addMsg("I feel a faint sinking sensation.");
  }

  identify(false);
}

namespace PotionNameHandling {

namespace {

vector<PotionLook> potionLooks_;

} //namespace

void init() {
  TRACE_FUNC_BEGIN;

  //Init possible potion colors and fake names
  potionLooks_.clear();
  potionLooks_.push_back(PotionLook {"Golden",   "a Golden",   clrYellow});
  potionLooks_.push_back(PotionLook {"Yellow",   "a Yellow",   clrYellow});
  potionLooks_.push_back(PotionLook {"Dark",     "a Dark",     clrGray});
  potionLooks_.push_back(PotionLook {"Black",    "a Black",    clrGray});
  potionLooks_.push_back(PotionLook {"Oily",     "an Oily",    clrGray});
  potionLooks_.push_back(PotionLook {"Smoky",    "a Smoky",    clrWhite});
  potionLooks_.push_back(PotionLook {"Slimy",    "a Slimy",    clrGreen});
  potionLooks_.push_back(PotionLook {"Green",    "a Green",    clrGreenLgt});
  potionLooks_.push_back(PotionLook {"Fiery",    "a Fiery",    clrRedLgt});
  potionLooks_.push_back(PotionLook {"Murky",    "a Murky",    clrBrownDrk});
  potionLooks_.push_back(PotionLook {"Muddy",    "a Muddy",    clrBrown});
  potionLooks_.push_back(PotionLook {"Violet",   "a Violet",   clrViolet});
  potionLooks_.push_back(PotionLook {"Orange",   "an Orange",  clrOrange});
  potionLooks_.push_back(PotionLook {"Watery",   "a Watery",   clrBlueLgt});
  potionLooks_.push_back(PotionLook {"Metallic", "a Metallic", clrGray});
  potionLooks_.push_back(PotionLook {"Clear",    "a Clear",    clrWhiteHigh});
  potionLooks_.push_back(PotionLook {"Misty",    "a Misty",    clrWhiteHigh});
  potionLooks_.push_back(PotionLook {"Bloody",   "a Bloody",   clrRed});
  potionLooks_.push_back(PotionLook {"Magenta",  "a Magenta",  clrMagenta});
  potionLooks_.push_back(PotionLook {"Clotted",  "a Clotted",  clrGreen});
  potionLooks_.push_back(PotionLook {"Moldy",    "a Moldy",    clrBrown});
  potionLooks_.push_back(PotionLook {"Frothy",   "a Frothy",   clrWhite});

  TRACE << "Init potion names" << endl;
  for(auto* const d : ItemData::data) {
    if(d->isPotion) {
      //Color and false name
      const int ELEMENT = Rnd::range(0, potionLooks_.size() - 1);

      PotionLook& look = potionLooks_.at(ELEMENT);

      d->baseNameUnid.names[int(ItemRefType::plain)]   = look.namePlain + " potion";
      d->baseNameUnid.names[int(ItemRefType::plural)]  = look.namePlain + " potions";
      d->baseNameUnid.names[int(ItemRefType::a)]       = look.nameA     + " potion";
      d->clr = look.clr;

      potionLooks_.erase(potionLooks_.begin() + ELEMENT);

      //True name
      const Potion* const potion =
        static_cast<const Potion*>(ItemFactory::mk(d->id, 1));

      const string REAL_TYPE_NAME = potion->getRealName();

      delete potion;

      const string REAL_NAME        = "Potion of "    + REAL_TYPE_NAME;
      const string REAL_NAME_PLURAL = "Potions of "   + REAL_TYPE_NAME;
      const string REAL_NAME_A      = "a potion of "  + REAL_TYPE_NAME;

      d->baseName.names[int(ItemRefType::plain)]  = REAL_NAME;
      d->baseName.names[int(ItemRefType::plural)] = REAL_NAME_PLURAL;
      d->baseName.names[int(ItemRefType::a)]      = REAL_NAME_A;
    }
  }

  TRACE_FUNC_END;
}

void storeToSaveLines(vector<string>& lines) {
  for(int i = 0; i < int(ItemId::END); ++i) {
    ItemDataT* const d = ItemData::data[i];
    if(d->isPotion) {
      lines.push_back(d->baseNameUnid.names[int(ItemRefType::plain)]);
      lines.push_back(d->baseNameUnid.names[int(ItemRefType::plural)]);
      lines.push_back(d->baseNameUnid.names[int(ItemRefType::a)]);
      lines.push_back(toStr(d->clr.r));
      lines.push_back(toStr(d->clr.g));
      lines.push_back(toStr(d->clr.b));
    }
  }
}

void setupFromSaveLines(vector<string>& lines) {
  for(int i = 0; i < int(ItemId::END); ++i) {
    ItemDataT* const d = ItemData::data[i];
    if(d->isPotion) {
      d->baseNameUnid.names[int(ItemRefType::plain)]  = lines.front();
      lines.erase(begin(lines));
      d->baseNameUnid.names[int(ItemRefType::plural)] = lines.front();
      lines.erase(begin(lines));
      d->baseNameUnid.names[int(ItemRefType::a)]      = lines.front();
      lines.erase(begin(lines));
      d->clr.r = toInt(lines.front());
      lines.erase(begin(lines));
      d->clr.g = toInt(lines.front());
      lines.erase(begin(lines));
      d->clr.b = toInt(lines.front());
      lines.erase(begin(lines));
    }
  }
}

} //PotionNameHandling

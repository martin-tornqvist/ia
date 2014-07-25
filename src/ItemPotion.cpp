#include "ItemPotion.h"

#include <climits>

#include "Properties.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ActorMonster.h"
#include "PlayerSpellsHandling.h"
#include "ItemScroll.h"
#include "GameTime.h"
#include "Audio.h"
#include "Renderer.h"
#include "Inventory.h"
#include "DungeonClimb.h"
#include "MapParsing.h"
#include "Utils.h"
#include "FeatureRigid.h"

using namespace std;

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
  actor->getPropHandler().tryApplyProp(new PropBlind(PropTurns::standard));
  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionBlindness::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionParal::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::standard));
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
    new PropDiseased(PropTurns::standard));
  if(Map::player->isSeeingActor(*actor, nullptr)) {
    identify(false);
  }
}

void PotionConf::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropConfused(PropTurns::standard));
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
    new PropFrenzied(PropTurns::standard));
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

  PropRFear*      const rFear   = new PropRFear(PropTurns::standard);
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
  actor->getPropHandler().tryApplyProp(new PropPoisoned(PropTurns::standard));
  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionPoison::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionRFire::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropRFire(PropTurns::standard));
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
  actor->getPropHandler().tryApplyProp(new PropRElec(PropTurns::standard));
  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionRElec::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionRAcid::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropRAcid(PropTurns::standard));
  if(Map::player->isSeeingActor(*actor, nullptr)) {identify(false);}
}

void PotionRAcid::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor) {quaff_(actor);}
}

void PotionInsight::quaff_(Actor* const actor) {
  (void)actor;

  Inventory& inv = Map::player->getInv();

  vector<Item*> identifyBucket;

  vector<InvSlot>& slots = inv.getSlots();
  for(InvSlot& slot : slots) {
    Item* const item = slot.item;
    if(item) {
      const ItemDataT& d = item->getData();
      if(!d.isIdentified) {identifyBucket.push_back(item);}
    }
  }
  vector<Item*>& general = inv.getGeneral();
  for(Item* item : general) {
    if(item->getData().id != ItemId::potionInsight) {
      const ItemDataT& d = item->getData();
      if(!d.isIdentified) {identifyBucket.push_back(item);}
    }
  }

  const unsigned int NR_ELEMENTS = identifyBucket.size();
  if(NR_ELEMENTS > 0) {
    const int ELEMENT = Rnd::range(0, NR_ELEMENTS - 1);

    Item* const item = identifyBucket.at(ELEMENT);

    const string itemNameBefore =
      ItemData::getItemRef(*item, ItemRefType::a, true);

    item->identify(true);

    const string itemNameAfter =
      ItemData::getItemRef(*item, ItemRefType::a, true);

    Log::addMsg("I gain intuitions about " + itemNameBefore + "...");
    Log::addMsg("It is identified as " + itemNameAfter + "!");
  }

  identify(false);
}

void PotionClairv::quaff_(Actor* const actor) {
  if(actor == Map::player) {
    Log::addMsg("I see far and wide!");

    vector<Pos> animPositions;
    animPositions.resize(0);

    bool blocked[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(), blocked);
    for(int y = 0; y < MAP_H; ++y) {
      for(int x = 0; x < MAP_W; ++x) {
        Cell& cell = Map::cells[x][y];
        if(!blocked[x][y] && !cell.isDark) {
          cell.isExplored = true;
          cell.isSeenByPlayer = true;
          animPositions.push_back(Pos(x, y));
        }
      }
    }

    Renderer::drawMapAndInterface(false);
//    Map::updateVisualMemory();
    Map::player->updateFov();

    Renderer::drawBlastAnimAtPositions(animPositions, clrWhite);
  }
  identify(false);
}

void PotionDescent::quaff_(Actor* const actor) {
  (void)actor;
  if(Map::dlvl < LAST_CAVERN_LVL) {
    DungeonClimb::travelDown(1);
    Log::addMsg("I sink downwards!");
  } else {
    Log::addMsg("I feel a faint sinking sensation.");
  }

  identify(false);
}

void Potion::identify(const bool IS_SILENT_IDENTIFY) {
  if(!data_->isIdentified) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME        = "Potion of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Potions of " + REAL_TYPE_NAME;
    const string REAL_NAME_A      = "a potion of " + REAL_TYPE_NAME;

    data_->baseName.name        = REAL_NAME;
    data_->baseName.name_plural = REAL_NAME_PLURAL;
    data_->baseName.nameA      = REAL_NAME_A;

    if(!IS_SILENT_IDENTIFY) {
      Log::addMsg("It was a " + REAL_NAME + ".");
      Map::player->incrShock(ShockValue::shockValue_heavy,
                             ShockSrc::useStrangeItem);
    }

    data_->isIdentified = true;
  }
}

void Potion::collide(const Pos& pos, Actor* const actor) {
  if(!Map::cells[pos.x][pos.y].rigid->isBottomless() || actor) {

    const bool PLAYER_SEE_CELL = Map::cells[pos.x][pos.y].isSeenByPlayer;

    if(PLAYER_SEE_CELL) {
      //TODO Use standard animation
      Renderer::drawGlyph('*', Panel::map, pos, data_->clr);

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
      Log::addMsg("I drink " + data_->baseName.nameA + "...");
    } else {
      Log::addMsg("I drink an unknown " + data_->baseName.name + "...");
    }
    Map::player->incrShock(ShockValue::shockValue_heavy,
                           ShockSrc::useStrangeItem);
  }

  quaff_(actor);

  if(Map::player->deadState == ActorDeadState::alive) {
    GameTime::actorDidAct();
  }
}

namespace PotionNameHandling {

namespace {

vector<PotionLook> potionLooks_;

} //namespace

void init() {
  potionLooks_.resize(0);

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
}

void setClrAndFalseName(ItemDataT* d) {
  const int ELEMENT = Rnd::range(0, potionLooks_.size() - 1);

  PotionLook& look = potionLooks_.at(ELEMENT);

  d->baseName.name        = look.namePlain + " potion";
  d->baseName.name_plural = look.namePlain + " potions";
  d->baseName.nameA      = look.nameA     + " potion";
  d->clr                  = look.clr;

  potionLooks_.erase(potionLooks_.begin() + ELEMENT);
}

void storeToSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::END); ++i) {
    ItemDataT* const d = ItemData::data[i];
    if(d->isPotion) {
      lines.push_back(d->baseName.name);
      lines.push_back(d->baseName.name_plural);
      lines.push_back(d->baseName.nameA);
      lines.push_back(toStr(d->clr.r));
      lines.push_back(toStr(d->clr.g));
      lines.push_back(toStr(d->clr.b));
    }
  }
}

void setupFromSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::END); ++i) {
    ItemDataT* const d = ItemData::data[i];
    if(d->isPotion) {
      d->baseName.name = lines.front();
      lines.erase(begin(lines));
      d->baseName.name_plural = lines.front();
      lines.erase(begin(lines));
      d->baseName.nameA = lines.front();
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

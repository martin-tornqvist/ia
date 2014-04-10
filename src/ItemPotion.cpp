#include "ItemPotion.h"

#include <climits>

#include "Engine.h"
#include "Properties.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ActorMonster.h"
#include "PlayerSpellsHandler.h"
#include "ItemScroll.h"
#include "GameTime.h"
#include "Audio.h"
#include "Renderer.h"
#include "Inventory.h"
#include "DungeonClimb.h"
#include "MapParsing.h"
#include "PlayerVisualMemory.h"
#include "Utils.h"

void PotionOfVitality::quaff_(Actor* const actor) {
  actor->getPropHandler().endAppliedPropsByMagicHealing();

  //HP is always restored at least up to maximum HP, but can go beyond
  const int HP          = actor->getHp();
  const int HP_MAX      = actor->getHpMax(true);
  const int HP_RESTORED = max(20, HP_MAX - HP);

  actor->restoreHp(HP_RESTORED, true, true);

  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfVitality::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfSpirit::quaff_(Actor* const actor) {

  //SPI is always restored at least up to maximum SPI, but can go beyond
  const int SPI           = actor->getSpi();
  const int SPI_MAX       = actor->getSpiMax();
  const int SPI_RESTORED  = max(10, SPI_MAX - SPI);

  actor->restoreSpi(SPI_RESTORED, true, true);

  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfSpirit::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfBlindness::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropBlind(eng, propTurnsStd));
  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfBlindness::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfParalyzation::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(new PropParalyzed(eng, propTurnsStd));
  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfParalyzation::collide_(const Pos& pos, Actor* const actor) {

  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfDisease::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropDiseased(eng, propTurnsStd));
  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfConfusion::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropConfused(eng, propTurnsStd));
  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfConfusion::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfFrenzy::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropFrenzied(eng, propTurnsStd));
  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfFrenzy::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfFortitude::quaff_(Actor* const actor) {
  PropHandler& propHandler = actor->getPropHandler();

  PropRFear*      const rFear   = new PropRFear(eng, propTurnsStd);
  PropRConfusion* const rConf   = new PropRConfusion(
    eng, propTurnsSpecific, rFear->turnsLeft_);
  PropRSleep*     const rSleep  = new PropRSleep(
    eng, propTurnsSpecific, rFear->turnsLeft_);

  propHandler.tryApplyProp(rFear);
  propHandler.tryApplyProp(rConf);
  propHandler.tryApplyProp(rSleep);

  if(actor == eng.player) {
    bool isPhobiasCured = false;
    for(int i = 0; i < int(Phobia::endOfPhobias); i++) {
      if(eng.player->phobias[i]) {
        eng.player->phobias[i] = false;
        isPhobiasCured = true;
      }
    }
    if(isPhobiasCured) {
      eng.log->addMsg("All my phobias are cured!");
    }

    bool isObsessionsCured = false;
    for(int i = 0; i < int(Obsession::endOfObsessions); i++) {
      if(eng.player->obsessions[i]) {
        eng.player->obsessions[i] = false;
        isObsessionsCured = true;
      }
    }
    if(isObsessionsCured) {
      eng.log->addMsg("All my obsessions are cured!");
    }

    eng.player->restoreShock(999, false);
    eng.log->addMsg("I feel more at ease.");
  }

  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfFortitude::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfPoison::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropPoisoned(eng, propTurnsStd));

  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfPoison::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfRFire::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropRFire(eng, propTurnsStd));

  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfRFire::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfAntidote::quaff_(Actor* const actor) {
  bool visionBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), visionBlockers);
  const bool IS_POISON_ENDED =
    actor->getPropHandler().endAppliedProp(propPoisoned, visionBlockers);

  if(IS_POISON_ENDED && eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfAntidote::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfRElec::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropRElec(eng, propTurnsStd));

  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfRElec::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfRAcid::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropRAcid(eng, propTurnsStd));

  if(eng.player->isSeeingActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfRAcid::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfInsight::quaff_(Actor* const actor) {
  (void)actor;

  Inventory& inv = eng.player->getInv();

  vector<Item*> identifyCandidates;

  vector<InventorySlot>& slots = inv.getSlots();
  for(InventorySlot & slot : slots) {
    Item* const item = slot.item;
    if(item) {
      const ItemData& d = item->getData();
      if(d.isIdentified == false) {identifyCandidates.push_back(item);}
    }
  }
  vector<Item*>& general = inv.getGeneral();
  for(Item * item : general) {
    if(item->getData().id != ItemId::potionOfInsight) {
      const ItemData& d = item->getData();
      if(d.isIdentified == false) {identifyCandidates.push_back(item);}
    }
  }

  const unsigned int NR_ELEMENTS = identifyCandidates.size();
  if(NR_ELEMENTS > 0) {
    const int ELEMENT = Rnd::range(0, NR_ELEMENTS - 1);

    Item* const item = identifyCandidates.at(ELEMENT);

    const string itemNameBefore =
      eng.itemDataHandler->getItemRef(*item, ItemRefType::a, true);

    item->identify(true);

    const string itemNameAfter =
      eng.itemDataHandler->getItemRef(*item, ItemRefType::a, true);

    eng.log->addMsg("I gain intuitions about " + itemNameBefore + "...");
    eng.log->addMsg("It is identified as " + itemNameAfter + "!");
  }

  identify(false);
}

void PotionOfClairvoyance::quaff_(Actor* const actor) {
  if(actor == eng.player) {
    eng.log->addMsg("I see far and wide!");

    vector<Pos> animPositions;
    animPositions.resize(0);

    bool blockers[MAP_W][MAP_H];
    MapParse::parse(CellPred::BlocksVision(eng), blockers);
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        Cell& cell = eng.map->cells[x][y];
        if(blockers[x][y] == false && cell.isDark == false) {
          cell.isExplored = true;
          cell.isSeenByPlayer = true;
          animPositions.push_back(Pos(x, y));
        }
      }
    }

    Renderer::drawMapAndInterface(false);
    eng.playerVisualMemory->updateVisualMemory();
    eng.player->updateFov();

    Renderer::drawBlastAnimAtPositions(animPositions, clrWhite);
  }
  identify(false);
}

void PotionOfDescent::quaff_(Actor* const actor) {
  (void)actor;
  if(eng.map->getDlvl() < LAST_CAVERN_LEVEL) {
    eng.dungeonClimb->travelDown(1);
    eng.log->addMsg("I sink downwards!");
  } else {
    eng.log->addMsg("I feel a faint sinking sensation.");
  }

  identify(false);
}

void PotionNameHandler::setClrAndFalseName(ItemData* d) {
  const int ELEMENT = Rnd::range(0, potionLooks_.size() - 1);

  PotionLook& look = potionLooks_.at(ELEMENT);

  d->name.name        = look.namePlain + " potion";
  d->name.name_plural = look.namePlain + " potions";
  d->name.name_a      = look.nameA     + " potion";
  d->clr              = look.clr;

  potionLooks_.erase(potionLooks_.begin() + ELEMENT);
}

void PotionNameHandler::addSaveLines(vector<string>& lines) const {
  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    ItemData* const d = eng.itemDataHandler->dataList[i];
    if(d->isPotion) {
      lines.push_back(d->name.name);
      lines.push_back(d->name.name_plural);
      lines.push_back(d->name.name_a);
      lines.push_back(toStr(d->clr.r));
      lines.push_back(toStr(d->clr.g));
      lines.push_back(toStr(d->clr.b));
    }
  }
}

void PotionNameHandler::setParamsFromSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    ItemData* const d = eng.itemDataHandler->dataList[i];
    if(d->isPotion) {
      d->name.name = lines.front();
      lines.erase(lines.begin());
      d->name.name_plural = lines.front();
      lines.erase(lines.begin());
      d->name.name_a = lines.front();
      lines.erase(lines.begin());
      d->clr.r = toInt(lines.front());
      lines.erase(lines.begin());
      d->clr.g = toInt(lines.front());
      lines.erase(lines.begin());
      d->clr.b = toInt(lines.front());
      lines.erase(lines.begin());
    }
  }
}

void Potion::identify(const bool IS_SILENT_IDENTIFY) {
  if(data_->isIdentified == false) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME        = "Potion of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Potions of " + REAL_TYPE_NAME;
    const string REAL_NAME_A      = "a potion of " + REAL_TYPE_NAME;

    data_->name.name = REAL_NAME;
    data_->name.name_plural = REAL_NAME_PLURAL;
    data_->name.name_a = REAL_NAME_A;

    if(IS_SILENT_IDENTIFY == false) {
      eng.log->addMsg("It was a " + REAL_NAME + ".");
      eng.player->incrShock(ShockValue::shockValue_heavy,
                            ShockSrc::useStrangeItem);
    }

    data_->isIdentified = true;
  }
}

void Potion::collide(const Pos& pos, Actor* const actor) {
  if(eng.map->cells[pos.x][pos.y].featureStatic->isBottomless() == false ||
      actor != NULL) {
//    ItemData* const potData =
//      eng.itemDataHandler->dataList[d.id];

    const bool PLAYER_SEE_CELL = eng.map->cells[pos.x][pos.y].isSeenByPlayer;

    if(PLAYER_SEE_CELL) {
      // TODO Use standard animation
      Renderer::drawGlyph('*', Panel::map, pos, data_->clr);

      if(actor != NULL) {
        if(actor->deadState == ActorDeadState::alive) {
          eng.log->addMsg(
            "The potion shatters on " +
            actor->getNameThe() + ".");
        }
      } else {
        Feature* const f = eng.map->cells[pos.x][pos.y].featureStatic;
        eng.log->addMsg("The potion shatters on " + f->getDescr(true) + ".");
      }
    }
    //If the blow from the bottle didn't kill the actor, apply what's inside
    if(actor != NULL) {
      if(actor->deadState == ActorDeadState::alive) {
        collide_(pos, actor);
        if(
          actor->deadState == ActorDeadState::alive &&
          data_->isIdentified == false && PLAYER_SEE_CELL) {
          eng.log->addMsg("It had no apparent effect...");
        }
      }
    }
  }
}

void Potion::quaff(Actor* const actor) {
  if(actor == eng.player) {
    data_->isTried = true;

    Audio::play(SfxId::potionQuaff);

    if(data_->isIdentified) {
      eng.log->addMsg("I drink " + data_->name.name_a + "...");
    } else {
      eng.log->addMsg("I drink an unknown " + data_->name.name + "...");
    }
    eng.player->incrShock(ShockValue::shockValue_heavy,
                          ShockSrc::useStrangeItem);
  }

  quaff_(actor);

  if(eng.player->deadState == ActorDeadState::alive) {
    eng.gameTime->actorDidAct();
  }
}

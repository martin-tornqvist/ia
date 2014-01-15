#include "ItemPotion.h"

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

void PotionOfHealing::quaff_(Actor* const actor) {
  actor->getPropHandler().endAppliedPropsByMagicHealing();

  //TODO What about wounds?

  //Attempt to heal the actor. If no hp was healed (already at full hp),
  //boost the hp instead.
  if(actor->restoreHp(999, true) == false) {
    actor->changeMaxHp(1, true);
  }

  if(eng.player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfHealing::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfSpirit::quaff_(Actor* const actor) {
  //Attempt to restore spirit. If no hp was healed (already at full hp),
  //boost the hp instead.
  if(actor->restoreSpi(eng.dice(2, 6) + 12, true) == false) {
    actor->changeMaxSpi(1, true);
  }

  if(eng.player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfSpirit::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfBlindness::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropBlind(eng, propTurnsStd));
  if(eng.player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfBlindness::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfParalyzation::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropParalyzed(eng, propTurnsStd));
  if(eng.player->checkIfSeeActor(*actor, NULL)) {
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
  if(eng.player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfConfusion::quaff_(Actor* const actor) {
  actor->getPropHandler().tryApplyProp(
    new PropConfused(eng, propTurnsStd));
  if(eng.player->checkIfSeeActor(*actor, NULL)) {
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
  if(eng.player->checkIfSeeActor(*actor, NULL)) {
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
    eng, propTurnsSpecified, rFear->turnsLeft_);
  PropRSleep*     const rSleep  = new PropRSleep(
    eng, propTurnsSpecified, rFear->turnsLeft_);

  propHandler.tryApplyProp(rFear);
  propHandler.tryApplyProp(rConf);
  propHandler.tryApplyProp(rSleep);

  if(actor == eng.player) {
    bool isPhobiasCured = false;
    for(unsigned int i = 0; i < endOfInsanityPhobias; i++) {
      if(eng.player->insanityPhobias[i]) {
        eng.player->insanityPhobias[i] = false;
        isPhobiasCured = true;
      }
    }
    if(isPhobiasCured) {
      eng.log->addMsg("All my phobias are cured!");
    }

    bool isObsessionsCured = false;
    for(unsigned int i = 0; i < endOfInsanityObsessions; i++) {
      if(eng.player->insanityObsessions[i]) {
        eng.player->insanityObsessions[i] = false;
        isObsessionsCured = true;
      }
    }
    if(isObsessionsCured) {
      eng.log->addMsg("All my obsessions are cured!");
    }

    eng.player->restoreShock(999, false);
    eng.log->addMsg("I feel more at ease.");
  }

  if(eng.player->checkIfSeeActor(*actor, NULL)) {
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

  if(eng.player->checkIfSeeActor(*actor, NULL)) {
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

  if(eng.player->checkIfSeeActor(*actor, NULL)) {
    identify(false);
  }
}

void PotionOfRFire::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionOfAntidote::quaff_(Actor* const actor) {
  bool visionBlockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
  const bool IS_POISON_ENDED =
    actor->getPropHandler().endAppliedProp(propPoisoned, visionBlockers);

  if(IS_POISON_ENDED && eng.player->checkIfSeeActor(*actor, NULL)) {
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

  if(eng.player->checkIfSeeActor(*actor, NULL)) {
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

  if(eng.player->checkIfSeeActor(*actor, NULL)) {
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

  vector<Item*> itemIdentifyCandidates;

  vector<InventorySlot>& slots = inv.getSlots();
  for(InventorySlot & slot : slots) {
    Item* const item = slot.item;
    if(item) {
      const ItemData& d = item->getData();
      if(d.isIdentified == false) {
        itemIdentifyCandidates.push_back(item);
      }
    }
  }
  vector<Item*>& general = inv.getGeneral();
  for(Item * item : general) {
    if(item->getData().id != item_potionOfInsight) {
      const ItemData& d = item->getData();
      if(d.isIdentified == false) {
        itemIdentifyCandidates.push_back(item);
      }
    }
  }

  const unsigned int NR_ELEMENTS = itemIdentifyCandidates.size();
  if(NR_ELEMENTS > 0) {
    Item* const item =
      itemIdentifyCandidates.at(
        eng.dice.range(0, NR_ELEMENTS - 1));

    const string itemNameBefore =
      eng.itemDataHandler->getItemRef(*item, itemRef_a, true);

    item->identify(true);

    const string itemNameAfter =
      eng.itemDataHandler->getItemRef(*item, itemRef_a, true);

    eng.log->addMsg("I gain intuitions about " + itemNameBefore + "...");
    eng.log->addMsg("It is identified as " + itemNameAfter + "!");
  }

  eng.player->incrMth(4, true);
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

void PotionOfDescent::collide_(const Pos& pos, Actor* const actor) {
  (void)pos;
  if(actor != NULL) {quaff_(actor);}
}

void PotionNameHandler::setColorAndFalseName(ItemData* d) {
  const unsigned int NR_NAMES = m_falseNames.size();

  const unsigned int ELEMENT = (unsigned int)(eng.dice(1, NR_NAMES) - 1);

  const string DESCRIPTION = m_falseNames.at(ELEMENT).str;
  const SDL_Color clr = m_falseNames.at(ELEMENT).clr;

  m_falseNames.erase(m_falseNames.begin() + ELEMENT);

  d->name.name = DESCRIPTION + " potion";
  d->name.name_plural = DESCRIPTION + " potions";
  d->name.name_a = "a " + DESCRIPTION + " potion";
  d->color = clr;
}

void PotionNameHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    ItemData* const d = eng.itemDataHandler->dataList[i];
    if(d->isPotion) {
      lines.push_back(d->name.name);
      lines.push_back(d->name.name_plural);
      lines.push_back(d->name.name_a);
      lines.push_back(toString(d->color.r));
      lines.push_back(toString(d->color.g));
      lines.push_back(toString(d->color.b));
    }
  }
}

void PotionNameHandler::setParamsFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    ItemData* const d = eng.itemDataHandler->dataList[i];
    if(d->isPotion) {
      d->name.name = lines.front();
      lines.erase(lines.begin());
      d->name.name_plural = lines.front();
      lines.erase(lines.begin());
      d->name.name_a = lines.front();
      lines.erase(lines.begin());
      d->color.r = toInt(lines.front());
      lines.erase(lines.begin());
      d->color.g = toInt(lines.front());
      lines.erase(lines.begin());
      d->color.b = toInt(lines.front());
      lines.erase(lines.begin());
    }
  }
}

void Potion::identify(const bool IS_SILENT_IDENTIFY) {
  if(data_->isIdentified == false) {
    const string REAL_TYPE_NAME = getRealTypeName();

    const string REAL_NAME = "Potion of " + REAL_TYPE_NAME;
    const string REAL_NAME_PLURAL = "Potions of " + REAL_TYPE_NAME;
    const string REAL_NAME_A = "a potion of " + REAL_TYPE_NAME;

    data_->name.name = REAL_NAME;
    data_->name.name_plural = REAL_NAME_PLURAL;
    data_->name.name_a = REAL_NAME_A;

    if(IS_SILENT_IDENTIFY == false) {
      eng.log->addMsg("It was a " + REAL_NAME + ".");
      eng.player->incrShock(shockValue_heavy, shockSrc_useStrangeItem);
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
      eng.renderer->drawGlyph('*', panel_map, pos, data_->color);

      if(actor != NULL) {
        if(actor->deadState == actorDeadState_alive) {
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
      if(actor->deadState == actorDeadState_alive) {
        collide_(pos, actor);
        if(
          actor->deadState == actorDeadState_alive &&
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

    eng.audio->play(sfxPotionQuaff);

    if(data_->isIdentified) {
      eng.log->addMsg("I drink " + data_->name.name_a + "...");
    } else {
      eng.log->addMsg("I drink an unknown " + data_->name.name + "...");
    }
    eng.player->incrShock(shockValue_heavy, shockSrc_useStrangeItem);
  }

  quaff_(actor);

  if(eng.player->deadState == actorDeadState_alive) {
    eng.gameTime->actorDidAct();
  }
}

void Potion::failedToLearnRealName(const string overrideFailString) {
  if(data_->isIdentified == false) {
    if(overrideFailString.empty() == false) {
      eng.log->addMsg(overrideFailString);
    } else {
      eng.log->addMsg("It doesn't seem to affect me.");
    }
  }
}

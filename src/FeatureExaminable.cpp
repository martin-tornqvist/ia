#include "FeatureExaminable.h"

#include "Engine.h"
#include "Log.h"
#include "Renderer.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemFactory.h"
#include "ItemDrop.h"
#include "Popup.h"
#include "PlayerBonuses.h"
#include "ActorPlayer.h"
#include "Inventory.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "Explosion.h"
#include "PopulateMonsters.h"
#include "Map.h"
#include "FeatureFactory.h"

//---------------------------------------------------------BASE CLASS
FeatureExaminable::FeatureExaminable(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine) {}

void FeatureExaminable::examine() {
  trace << "FeatureExaminable::examine()..." << endl;

  featureSpecific_examine();
  eng.gameTime->actorDidAct();

  trace << "FeatureExaminable::examine() [DONE]" << endl;
}

EventRegularity_t FeatureExaminable::getEventRegularity() {
  const int TOT =
    eventRegularity_common + eventRegularity_rare + eventRegularity_veryRare;
  const int RND = eng.dice.range(1, TOT);
  if(RND <= eventRegularity_common) {
    return eventRegularity_common;
  } else if(RND <= eventRegularity_rare) {
    return eventRegularity_rare;
  } else {
    return eventRegularity_veryRare;
  }
}

//---------------------------------------------------------ITEM CONTAINER CLASS
ExaminableItemContainer::ExaminableItemContainer() {items_.resize(0);}

ExaminableItemContainer::~ExaminableItemContainer() {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
}

void ExaminableItemContainer::setRandomItemsForFeature(
  const Feature_t featureId,
  const int NR_ITEMS_TO_ATTEMPT, Engine& engine) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
  items_.resize(0);

  if(NR_ITEMS_TO_ATTEMPT > 0) {
    while(items_.empty()) {
      vector<ItemId_t> itemCandidates;
      for(unsigned int i = 1; i < endOfItemIds; i++) {
        ItemData* const curData = engine.itemDataHandler->dataList[i];
        for(
          unsigned int ii = 0;
          ii < curData->featuresCanBeFoundIn.size();
          ii++) {
          pair<Feature_t, int> featuresFoundIn =
            curData->featuresCanBeFoundIn.at(ii);
          if(featuresFoundIn.first == featureId) {
            if(engine.dice.percentile() < featuresFoundIn.second) {
              if(
                engine.dice.percentile() <
                curData->chanceToIncludeInSpawnList) {
                itemCandidates.push_back(static_cast<ItemId_t>(i));
                break;
              }
            }
          }
        }
      }

      const int NR_CANDIDATES = int(itemCandidates.size());
      if(NR_CANDIDATES > 0) {
        for(int i = 0; i < NR_ITEMS_TO_ATTEMPT; i++) {
          const unsigned int ELEMENT =
            engine.dice.range(0, NR_CANDIDATES - 1);
          Item* item =
            engine.itemFactory->spawnItem(itemCandidates.at(ELEMENT));
          engine.itemFactory->setItemRandomizedProperties(item);
          items_.push_back(item);
        }
      }
    }
  }
}

void ExaminableItemContainer::dropItems(const Pos& pos, Engine& engine) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    engine.itemDrop->dropItemOnMap(pos, *items_.at(i));
  }
  items_.resize(0);
}

void ExaminableItemContainer::destroySingleFragile(Engine& engine) {
  //TODO Generalize this function (perhaps isFragile variable in item data)

  for(unsigned int i = 0; i < items_.size(); i++) {
    Item* const item = items_.at(i);
    const ItemData& d = item->getData();
    if(d.isPotion || d.id == item_molotov) {
      delete item;
      items_.erase(items_.begin() + i);
      engine.log->addMsg("I hear a muffled shatter.");
      break;
    }
  }
}

//---------------------------------------------------------SPECIFIC FEATURES
//--------------------------------------------------------- TOMB
Tomb::Tomb(Feature_t id, Pos pos, Engine& engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false),
  isTraitKnown_(false), chanceToPushLid_(100),
  appearance_(tombAppearance_common), trait_(endOfTombTraits) {

  //Contained items
  PlayerBonHandler* const bonHandler = eng.playerBonHandler;
  const int NR_ITEMS_MIN = eng.dice.oneIn(3) ? 0 : 1;
  const int NR_ITEMS_MAX =
    NR_ITEMS_MIN + (bonHandler->hasTrait(traitTreasureHunter) ? 1 : 0);

  itemContainer_.setRandomItemsForFeature(
    feature_tomb, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  //Exterior appearance
  if(engine.dice.oneIn(5)) {
    trace << "Tomb: Setting random appearance" << endl;
    appearance_ =
      TombAppearance_t(engine.dice.range(0, endOfTombAppearance - 1));
  } else {
    trace << "Tomb: Setting appearance according to items contained ";
    trace << "(common if zero items)" << endl;
    for(unsigned int i = 0; i < itemContainer_.items_.size(); i++) {
      const ItemValue_t itemValue =
        itemContainer_.items_.at(i)->getData().itemValue;
      if(itemValue == itemValue_majorTreasure) {
        trace << "Tomb: Contains major treasure" << endl;
        appearance_ = tombAppearance_marvelous;
        break;
      } else if(itemValue == itemValue_minorTreasure) {
        trace << "Tomb: Contains minor treasure" << endl;
        appearance_ = tombAppearance_impressive;
      }
    }
  }

  const bool IS_CONTAINING_ITEMS = itemContainer_.items_.empty() == false;

  chanceToPushLid_ = IS_CONTAINING_ITEMS ? engine.dice.range(0, 75) : 90;

  if(IS_CONTAINING_ITEMS) {
    const int RND = engine.dice.percentile();

    if(RND < 15) {
      trait_ = tombTrait_forebodingCarvedSigns;
    } else if(RND < 45) {
      trait_ = tombTrait_stench;
    } else if(RND < 75) {
      trait_ = tombTrait_auraOfUnrest;
    }
    trace << "Tomb: Set trait (" << trait_ << ")" << endl;
  }
}

void Tomb::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The tomb is empty.");
  } else {
    vector<TombAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng.popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A tomb");
    doAction(possibleActions.at((unsigned int)CHOICE_NR));
  }
}

void Tomb::doAction(const TombAction_t action) {
  PropHandler& propHandler              = eng.player->getPropHandler();
  PlayerBonHandler* const bonusHandler  = eng.playerBonHandler;

  const bool IS_TOUGH     = bonusHandler->hasTrait(traitTough);
  const bool IS_OBSERVANT = bonusHandler->hasTrait(traitObservant);
  const bool IS_CONFUSED  = propHandler.hasProp(propConfused);
  const bool IS_WEAK      = propHandler.hasProp(propWeakened);
  const bool IS_CURSED    = propHandler.hasProp(propCursed);
  const bool IS_BLESSED   = propHandler.hasProp(propBlessed);

  switch(action) {
    case tombAction_carveCurseWard: {
      if(IS_CURSED == false && (IS_BLESSED || eng.dice.fraction(4, 5))) {
        eng.log->addMsg("The curse is cleared.");
      } else {
        eng.log->addMsg("I make a misstake, the curse is doubled!");
        PropCursed* const curse =
          new PropCursed(eng, propTurnsStd);
        curse->turnsLeft_ *= 2;
        propHandler.tryApplyProp(curse, true);
      }
      trait_ = endOfTombTraits;
    } break;

    case tombAction_leave: {
      eng.log->addMsg("I leave the tomb for now.");
    } break;

    case tombAction_pushLid: {
      eng.log->addMsg("I attempt to push the lid.");

      const int SPRAIN_ONE_IN_N   = 5;
      const int PARALYZE_ONE_IN_N = 3;

      if(eng.dice.oneIn(SPRAIN_ONE_IN_N)) {
        eng.log->addMsg("I sprain myself.", clrMsgBad);
        eng.player->hit(eng.dice.range(1, 5), dmgType_pure, false);
      }

      if(eng.player->deadState != actorDeadState_alive) {
        return;
      }

      if(eng.dice.oneIn(PARALYZE_ONE_IN_N)) {
        eng.log->addMsg("I am off-balance.");
        propHandler.tryApplyProp(
          new PropParalyzed(eng, propTurnsSpecified, 2));
      }

      if(IS_WEAK) {
        eng.log->addMsg("It seems futile.");
        return;
      }
      const int BON = IS_TOUGH ? 20 : 0;
      if(eng.dice.percentile() < chanceToPushLid_ + BON) {
        eng.log->addMsg("The lid comes off!");
        openFeature();
      } else {
        eng.log->addMsg("The lid resists.");
      }
    } break;

    case tombAction_searchExterior: {
      const int CHANCE_TO_FIND = 50 + IS_OBSERVANT * 40 - IS_CONFUSED * 10;
      const bool IS_ROLL_SUCCESS = eng.dice.percentile() < CHANCE_TO_FIND;
      if(IS_ROLL_SUCCESS && trait_ != endOfTombTraits) {
        string traitDescr = "";
        getTraitDescr(traitDescr);
        eng.log->addMsg(traitDescr);
        isTraitKnown_ = true;
      } else {
        eng.log->addMsg("I find nothing significant.");
      }
    } break;

    case tombAction_smashLidWithSledgehammer: {
      eng.log->addMsg("I strike at the lid with a Sledgehammer.");
      const int BREAK_N_IN_10 = IS_WEAK ? 1 : 8;
      if(eng.dice.fraction(BREAK_N_IN_10, 10)) {
        eng.log->addMsg("The lid cracks open!");
        if(IS_BLESSED == false && (IS_CURSED || eng.dice.oneIn(3))) {
          itemContainer_.destroySingleFragile(eng);
        }
        openFeature();
      } else {
        eng.log->addMsg("The lid resists.");
      }
    } break;
  }
}

bool Tomb::openFeature() {
  eng.log->addMsg("The tomb opens.");
  triggerTrap();
  if(itemContainer_.items_.size() > 0) {
    eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    eng.log->addMsg("There is nothing of value inside.");
  }
  eng.renderer->drawMapAndInterface(true);
  isContentKnown_ = isTraitKnown_ = true;

  return true;
}

void Tomb::triggerTrap() {
  vector<ActorId_t> actorCandidates;

  switch(trait_) {
    case tombTrait_auraOfUnrest: {
      for(unsigned int i = 1; i < endOfActorIds; i++) {
        const ActorData& d = eng.actorDataHandler->dataList[i];
        if(d.isGhost && d.isAutoSpawnAllowed && d.isUnique == false) {
          actorCandidates.push_back(static_cast<ActorId_t>(i));
        }
      }
      eng.log->addMsg("Something rises from the tomb!");
    } break;

    case tombTrait_forebodingCarvedSigns: {
      eng.player->getPropHandler().tryApplyProp(
        new PropCursed(eng, propTurnsStd));
    } break;

    case tombTrait_stench: {
      if(eng.dice.coinToss()) {
        eng.log->addMsg("Fumes burst out from the tomb!");
        Prop* prop = NULL;
        SDL_Color fumeClr = clrMagenta;
        const int RND = eng.dice.percentile();
        if(RND < 20) {
          prop = new PropPoisoned(eng, propTurnsStd);
          fumeClr = clrGreenLgt;
        } else if(RND < 40) {
          prop = new PropDiseased(eng, propTurnsStd);
          fumeClr = clrGreen;
        } else {
          prop = new PropParalyzed(eng, propTurnsStd);
          prop->turnsLeft_ *= 2;
        }
        Explosion::runExplosionAt(
          pos_, eng, 0, endOfSfx, false, prop, true, fumeClr);
      } else {
        for(unsigned int i = 1; i < endOfActorIds; i++) {
          const ActorData& d = eng.actorDataHandler->dataList[i];
          if(
            d.bodyType == bodyType_ooze &&
            d.isAutoSpawnAllowed &&
            d.isUnique == false) {
            actorCandidates.push_back(static_cast<ActorId_t>(i));
          }
        }
        eng.log->addMsg("Something creeps up from the tomb!");
      }

    } break;

    default: {} break;
  }

  if(actorCandidates.size() > 0) {
    const unsigned int ELEM = eng.dice.range(0, actorCandidates.size() - 1);
    const ActorId_t actorIdToSpawn = actorCandidates.at(ELEM);
    Actor* const actor = eng.actorFactory->spawnActor(actorIdToSpawn, pos_);
    dynamic_cast<Monster*>(actor)->becomeAware();
  }
}

void Tomb::getPossibleActions(vector<TombAction_t>& possibleActions) const {
  const bool IS_OCCULTIST = eng.playerBonHandler->getBg() == bgOccultist;

  if(isTraitKnown_) {
    if(IS_OCCULTIST) {
      if(trait_ == tombTrait_forebodingCarvedSigns) {
        possibleActions.push_back(tombAction_carveCurseWard);
      }
    }
  } else {
    possibleActions.push_back(tombAction_searchExterior);
  }

  possibleActions.push_back(tombAction_pushLid);

  const Inventory& inv = eng.player->getInv();
  bool hasSledgehammer = false;
  Item* item = inv.getItemInSlot(slot_wielded);
  if(item != NULL) {
    hasSledgehammer = item->getData().id == item_sledgeHammer;
  }
  if(hasSledgehammer == false) {
    item = inv.getItemInSlot(slot_wieldedAlt);
    hasSledgehammer = item->getData().id == item_sledgeHammer;
  }
  if(hasSledgehammer == false) {
    hasSledgehammer = inv.hasItemInGeneral(item_sledgeHammer);
  }
  if(hasSledgehammer) {
    possibleActions.push_back(tombAction_smashLidWithSledgehammer);
  }

  possibleActions.push_back(tombAction_leave);
}

void Tomb::getChoiceLabels(const vector<TombAction_t>& possibleActions,
                           vector<string>& actionLabels) const {
  actionLabels.resize(0);

  for(unsigned int i = 0; i < possibleActions.size(); i++) {
    const TombAction_t action = possibleActions.at(i);
    switch(action) {
      case tombAction_carveCurseWard: {
        actionLabels.push_back("Carve a curse ward");
      } break;
      case tombAction_leave: {
        actionLabels.push_back("Leave it");
      } break;
      case tombAction_pushLid: {
        actionLabels.push_back("Try pushing the lid");
      } break;
      case tombAction_searchExterior: {
        actionLabels.push_back("Search the exterior");
      } break;
      case tombAction_smashLidWithSledgehammer: {
        actionLabels.push_back("Smash the lid with a sledgehammer");
      } break;
    }
  }
}

void Tomb::getTraitDescr(string& descr) const {
  const bool IS_OCCULTIST = eng.playerBonHandler->getBg() == bgOccultist;

  switch(trait_) {
    case tombTrait_auraOfUnrest: {
      descr = "It has a certain aura of unrest about it.";
    } break;

    case tombTrait_forebodingCarvedSigns: {
      if(IS_OCCULTIST) {
        descr = "There is a curse carved on the box.";
      } else {
        descr = "There are some ominous runes carved on the box.";
      }
    } break;

    case tombTrait_stench: {
      descr = "There is a pungent stench.";
    } break;

    default: {} break;
  }
}

void Tomb::getDescr(string& descr) const {
  switch(appearance_) {
    case tombAppearance_common:     {descr = "It looks ordinary.";} break;
    case tombAppearance_impressive: {descr = "It looks impressive.";} break;
    case tombAppearance_marvelous:  {descr = "It looks marvelous!";} break;
    default: {} break;
  }

  if(isTraitKnown_) {
    string traitDescr = "";
    getTraitDescr(traitDescr);
    descr += " " + traitDescr;
  }

  const bool IS_WEAK = eng.player->getPropHandler().hasProp(propWeakened);

  if(chanceToPushLid_ < 10 || IS_WEAK) {
    descr += " The lid seems very heavy.";
  } else if(chanceToPushLid_ < 50) {
    descr += " The lid does not seem too heavy.";
  } else {
    descr += " I think I could move the lid with small effort.";
  }
}

//--------------------------------------------------------- CHEST
Chest::Chest(Feature_t id, Pos pos, Engine& engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false),
  isLocked_(false), isTrapped_(false), isTrapStatusKnown_(false) {

  PlayerBonHandler* const bonHandler = eng.playerBonHandler;
  const bool IS_TREASURE_HUNTER =
    bonHandler->hasTrait(traitTreasureHunter);
  const int NR_ITEMS_MIN = eng.dice.oneIn(10) ? 0 : 1;
  const int NR_ITEMS_MAX = IS_TREASURE_HUNTER ? 4 : 3;
  itemContainer_.setRandomItemsForFeature(
    feature_chest, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  if(itemContainer_.items_.empty() == false) {
    const int IS_LOCKED_N_IN_10 = 8;
    isLocked_ = eng.dice.fraction(IS_LOCKED_N_IN_10, 10);

    const int IS_TRAPPED_N_IN_10 = 6;
    isTrapped_ = eng.dice.fraction(IS_TRAPPED_N_IN_10, 10);
  }
}

void Chest::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The chest is empty.");
  } else {
    vector<ChestAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng.popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A chest");

    doAction(possibleActions.at((unsigned int)CHOICE_NR));
  }
}

void Chest::doAction(const ChestAction_t action) {
  PropHandler& propHandler = eng.player->getPropHandler();
  PlayerBonHandler* const bonHandler = eng.playerBonHandler;

  const bool IS_OBSERVANT = bonHandler->hasTrait(traitObservant);
  const bool IS_TOUGH     = bonHandler->hasTrait(traitTough);
  const bool IS_CONFUSED  = propHandler.hasProp(propConfused);
  const bool IS_WEAK      = propHandler.hasProp(propWeakened);
  const bool IS_CURSED    = propHandler.hasProp(propCursed);
  const bool IS_BLESSED   = propHandler.hasProp(propBlessed);

  switch(action) {
    case chestAction_open: {
      openFeature();
    } break;

    case chestAction_searchForTrap: {
      const int CHANCE_TO_FIND = 20 + IS_OBSERVANT * 40 - IS_CONFUSED * 10;
      const bool IS_ROLL_SUCCESS = eng.dice.percentile() < CHANCE_TO_FIND;
      if(IS_ROLL_SUCCESS && isTrapped_) {
        eng.log->addMsg("The chest has a hidden trap mechanism!");
        isTrapStatusKnown_ = true;
      } else {
        eng.log->addMsg("I find no indication that the chest is trapped.");
      }
    } break;

    case chestAction_disarmTrap: {
      eng.log->addMsg("I attempt to disarm the trap.");

      const int CHANCE_TO_TRIGGER = 20;
      if(eng.dice.percentile() < CHANCE_TO_TRIGGER) {
        eng.log->addMsg("I accidentally trigger it!");
        openFeature();
      } else {
        const int CHANCE_TO_DISARM = 50;
        if(eng.dice.percentile() < CHANCE_TO_DISARM) {
          eng.log->addMsg("I successfully disarm it!");
          isTrapped_ = false;
        } else {
          eng.log->addMsg("I failed to disarm it.");
        }
      }
    } break;

    case chestAction_forceLock: {
      Inventory& inv    = eng.player->getInv();
      Item* const item  = inv.getItemInSlot(slot_wielded);

      if(item == NULL) {
        eng.log->addMsg(
          "I attempt to punch the lock open, nearly breaking my hand.",
          clrMsgBad);
        eng.player->hit(1, dmgType_pure, false);
      } else {
        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);

        if(eng.dice.percentile() < CHANCE_TO_DMG_WPN) {
          const string wpnName = eng.itemDataHandler->getItemRef(
                                   *item, itemRef_plain, true);

          Weapon* const wpn = dynamic_cast<Weapon*>(item);

          if(wpn->meleeDmgPlus == 0) {
            eng.log->addMsg("My " + wpnName + " breaks!");
            delete wpn;
            inv.getSlot(slot_wielded)->item = NULL;
          } else {
            eng.log->addMsg("My " + wpnName + " is damaged!");
            wpn->meleeDmgPlus--;
          }
          return;
        }

        if(IS_WEAK) {
          eng.log->addMsg("It seems futile.");
        } else {
          const int CHANCE_TO_OPEN = 40;
          if(eng.dice.percentile() < CHANCE_TO_OPEN) {
            eng.log->addMsg("I force the lock open!");
            openFeature();
          } else {
            eng.log->addMsg("The lock resists.");
          }
        }
      }
    } break;

    case chestAction_kick: {
      const int SPRAIN_ONE_IN_N = 5;

      if(eng.dice.oneIn(SPRAIN_ONE_IN_N)) {
        eng.log->addMsg("I sprain myself.", clrMsgBad);
        eng.player->hit(1, dmgType_pure, false);
      }

      if(eng.player->deadState == actorDeadState_alive) {
        if(IS_WEAK) {
          eng.log->addMsg("It seems futile.");
          return;
        }

        if(IS_BLESSED == false && (IS_CURSED || eng.dice.oneIn(3))) {
          itemContainer_.destroySingleFragile(eng);
        }
        const int CHANCE_TO_OPEN = 20 + (IS_TOUGH ? 30 : 0);
        if(eng.dice.percentile() < CHANCE_TO_OPEN) {
          eng.log->addMsg("I kick the lid open!");
          openFeature();
        } else {
          eng.log->addMsg("The lock resists.");
        }
      }
    } break;

    case chestAction_leave: {
      eng.log->addMsg("I leave the chest for now.");
    } break;
  }
}

bool Chest::openFeature() {
  eng.log->addMsg("The chest opens.");
  if(isTrapped_) {
    triggerTrap();
  }
  if(eng.player->deadState == actorDeadState_alive) {
    if(itemContainer_.items_.empty()) {
      eng.log->addMsg("There is nothing of value inside.");
    } else {
      eng.log->addMsg("There are some items inside.");
      itemContainer_.dropItems(pos_, eng);
      eng.renderer->drawMapAndInterface(true);
    }
    isContentKnown_ = true;
    isTrapStatusKnown_ = true;
  }
  return true;
}

void Chest::getPossibleActions(vector<ChestAction_t>& possibleActions) const {
  if(isTrapStatusKnown_ == false) {
    possibleActions.push_back(chestAction_searchForTrap);
  } else if(isTrapped_) {
    possibleActions.push_back(chestAction_disarmTrap);
  }

  if(isLocked_) {
    possibleActions.push_back(chestAction_kick);

    Item* const wpn =
      eng.player->getInv().getItemInSlot(slot_wielded);
    bool canTryForce = false;
    if(wpn == NULL) {
      canTryForce = true;
    } else {
      const ItemData d = wpn->getData();
      canTryForce = d.isRangedWeapon == false;
    }
    if(canTryForce) {
      possibleActions.push_back(chestAction_forceLock);
    }
  } else {
    possibleActions.push_back(chestAction_open);
  }

  possibleActions.push_back(chestAction_leave);
}

void Chest::getChoiceLabels(const vector<ChestAction_t>& possibleActions,
                            vector<string>& actionLabels) const {
  actionLabels.resize(0);

  for(unsigned int i = 0; i < possibleActions.size(); i++) {
    ChestAction_t action = possibleActions.at(i);
    switch(action) {
      case chestAction_open: {
        actionLabels.push_back("Open it");
      } break;
      case chestAction_searchForTrap: {
        actionLabels.push_back("Search it for traps");
      } break;
      case chestAction_disarmTrap: {
        actionLabels.push_back("Disarm the trap");
      } break;
      case chestAction_forceLock: {
        actionLabels.push_back("Force the lock with weapon");
      } break;
      case chestAction_kick: {
        actionLabels.push_back("Kick the lid");
      } break;
      case chestAction_leave: {
        actionLabels.push_back("Leave it");
      } break;
    }
  }
}

void Chest::getDescr(string& descr) const {
  const string lockDescr = isLocked_ ? "locked" : "not locked";
  const string trapDescr = isTrapped_ && isTrapStatusKnown_ ?
                           " There appears to be a trap mechanism." : "";
  descr = "It is " + lockDescr + "." + trapDescr;
}

void Chest::triggerTrap() {
  isTrapStatusKnown_ = true;

  if(isTrapped_) {

    isTrapped_ = false;

    const int EXPLODE_ONE_IN_N = 7;
    if(
      eng.map->getDlvl() >= MIN_DLVL_NASTY_TRAPS &&
      eng.dice.oneIn(EXPLODE_ONE_IN_N)) {
      eng.log->addMsg("The trap explodes!");
      Explosion::runExplosionAt(pos_, eng, 0, sfxExplosion, true);
      if(eng.player->deadState == actorDeadState_alive) {
        eng.featureFactory->spawnFeatureAt(feature_rubbleLow, pos_);
      }
    } else {
      eng.log->addMsg("Fumes burst out from the chest!");
      Prop* prop = NULL;
      SDL_Color fumeClr = clrMagenta;
      const int RND = eng.dice.percentile();
      if(RND < 20) {
        prop = new PropPoisoned(eng, propTurnsStd);
        fumeClr = clrGreenLgt;
      } else if(RND < 40) {
        prop = new PropDiseased(eng, propTurnsStd);
        fumeClr = clrGreen;
      } else {
        prop = new PropParalyzed(eng, propTurnsStd);
        prop->turnsLeft_ *= 2;
      }
      Explosion::runExplosionAt(
        pos_, eng, 0, endOfSfx, false, prop, true, fumeClr);
    }
  }
}

//--------------------------------------------------------- FOUNTAIN
Fountain::Fountain(Feature_t id, Pos pos, Engine& engine) :
  FeatureExaminable(id, pos, engine), fountainType(fountainTypeTepid) {

  fountainType = FountainType_t(eng.dice.range(1, endOfFountainTypes - 1));
}

void Fountain::featureSpecific_examine() {
  if(fountainType == fountainTypeDry) {
    eng.log->addMsg("The fountain is dried out.");
  } else {
    vector<string> actionLabels;
    actionLabels.push_back("Drink from it");
    actionLabels.push_back("Leave it");
    string descr = "";
    const int CHOICE_NR = eng.popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A fountain");
    switch(CHOICE_NR) {
      case 0: {
        drink();
      } break;

      case 1: {
        eng.log->addMsg("I leave the fountain for now.");
      } break;
    }
  }
}

void Fountain::drink() {
  PropHandler& propHandler = eng.player->getPropHandler();

  eng.log->addMsg("I drink from the fountain.");

  switch(fountainType) {
    case fountainTypeDry: {} break;

    case fountainTypeTepid: {
      eng.log->addMsg("The water is tepid.");
    } break;

    case fountainTypeRefreshing: {
      eng.log->addMsg("It's very refreshing.");
      eng.player->restoreHp(1, false);
      eng.player->restoreSpi(1, false);
      eng.player->restoreShock(5, false);
    } break;

    case fountainTypeBlessed: {
      propHandler.tryApplyProp(
        new PropBlessed(eng, propTurnsStd));
    } break;

    case fountainTypeCursed: {
      propHandler.tryApplyProp(
        new PropCursed(eng, propTurnsStd));
    } break;

    case fountainTypeSpirited: {
      eng.player->restoreSpi(2, true, true);
    } break;

    case fountainTypeVitality: {
      eng.player->restoreHp(2, true, true);
    } break;

    case fountainTypeDiseased: {
      propHandler.tryApplyProp(
        new PropDiseased(eng, propTurnsStd));
    } break;

    case fountainTypePoisoned: {
      propHandler.tryApplyProp(
        new PropPoisoned(eng, propTurnsStd));
    } break;

    case fountainTypeFrenzy: {
      propHandler.tryApplyProp(
        new PropFrenzied(eng, propTurnsStd));
    } break;

    case endOfFountainTypes: {} break;
  }

  if(eng.dice.oneIn(5)) {
    eng.log->addMsg("The fountain dries out.");
    fountainType = fountainTypeDry;
  }
}

//--------------------------------------------------------- CABINET
Cabinet::Cabinet(Feature_t id, Pos pos, Engine& engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false) {

  PlayerBonHandler* const bonHandler = eng.playerBonHandler;
  const bool IS_TREASURE_HUNTER =
    bonHandler->hasTrait(traitTreasureHunter);
  const int IS_EMPTY_N_IN_10 = 5;
  const int NR_ITEMS_MIN = eng.dice.fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX = IS_TREASURE_HUNTER ? 2 : 1;
  itemContainer_.setRandomItemsForFeature(
    feature_cabinet, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cabinet::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The cabinet is empty.");
  } else {
    vector<CabinetAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng.popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A cabinet");
    doAction(possibleActions.at((unsigned int)CHOICE_NR));
  }
}

void Cabinet::triggerTrap() {

}

bool Cabinet::openFeature() {
  eng.log->addMsg("The cabinet opens.");
  triggerTrap();
  if(itemContainer_.items_.size() > 0) {
    eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    eng.log->addMsg("There is nothing of value inside.");
  }
  eng.renderer->drawMapAndInterface(true);
  isContentKnown_ = true;

  return true;
}

void Cabinet::getChoiceLabels(const vector<CabinetAction_t>& possibleActions,
                              vector<string>& actionLabels) const {
  for(unsigned int i = 0; i < possibleActions.size(); i++) {
    const CabinetAction_t action = possibleActions.at(i);
    switch(action) {

      case cabinetAction_open: {
        actionLabels.push_back("Open it");
      } break;

      case cabinetAction_leave: {
        actionLabels.push_back("Leave it");
      }
    }
  }
}

void Cabinet::getPossibleActions(
  vector<CabinetAction_t>& possibleActions) const {
  possibleActions.resize(0);
  possibleActions.push_back(cabinetAction_open);
  possibleActions.push_back(cabinetAction_leave);
}

void Cabinet::doAction(const CabinetAction_t action) {
  switch(action) {
    case cabinetAction_open: {
      openFeature();
    } break;

    case cabinetAction_leave: {
      eng.log->addMsg("I leave the cabinet for now.");
    } break;
  }
}

void Cabinet::getDescr(string& descr) const {
  descr = "";
}

//--------------------------------------------------------- COCOON
Cocoon::Cocoon(Feature_t id, Pos pos, Engine& engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false) {

  PlayerBonHandler* const bonHandler = eng.playerBonHandler;
  const bool IS_TREASURE_HUNTER =
    bonHandler->hasTrait(traitTreasureHunter);
  const int IS_EMPTY_N_IN_10 = 6;
  const int NR_ITEMS_MIN = eng.dice.fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX = NR_ITEMS_MIN + (IS_TREASURE_HUNTER ? 1 : 0);
  itemContainer_.setRandomItemsForFeature(
    feature_cocoon, eng.dice.range(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cocoon::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng.log->addMsg("The cocoon is empty.");
  } else {
    vector<CocoonAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng.popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A cocoon");
    doAction(possibleActions.at((unsigned int)CHOICE_NR));
  }
}

void Cocoon::triggerTrap() {
  const int RND = eng.dice.percentile();

  if(RND < 15) {
    eng.log->addMsg("There is a half-dissolved human body inside!");
    eng.player->incrShock(shockValue_heavy, shockSrc_misc);
  } else if(RND < 50) {
    trace << "Cocoon: Attempting to spawn spiders" << endl;
    vector<ActorId_t> spawnCandidates;
    for(unsigned int i = 1; i < endOfActorIds; i++) {
      const ActorData& d = eng.actorDataHandler->dataList[i];
      if(d.isSpider && d.actorSize == actorSize_floor &&
          d.isAutoSpawnAllowed && d.isUnique == false) {
        spawnCandidates.push_back(d.id);
      }
    }

    const int NR_CANDIDATES = spawnCandidates.size();
    if(NR_CANDIDATES > 0) {
      trace << "Cocoon: Spawn candidates found, attempting to place" << endl;
      eng.log->addMsg("There are spiders inside!");
      const int NR_SPIDERS = eng.dice.range(2, 5);
      const int ELEMENT = eng.dice.range(0, NR_CANDIDATES - 1);
      const ActorId_t actorIdToSummon = spawnCandidates.at(ELEMENT);
      eng.actorFactory->summonMonsters(
        pos_, vector<ActorId_t>(NR_SPIDERS, actorIdToSummon), true);
    }
  }
}

bool Cocoon::openFeature() {
  eng.log->addMsg("The cocoon opens.");
  triggerTrap();
  if(itemContainer_.items_.size() > 0) {
    eng.log->addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    eng.log->addMsg("There is nothing of value inside.");
  }
  eng.renderer->drawMapAndInterface(true);
  isContentKnown_ = true;

  return true;
}

void Cocoon::getChoiceLabels(const vector<CocoonAction_t>& possibleActions,
                             vector<string>& actionLabels) const {
  for(unsigned int i = 0; i < possibleActions.size(); i++) {
    const CocoonAction_t action = possibleActions.at(i);
    switch(action) {

      case cocoonAction_open: {
        actionLabels.push_back("Break it open");
      } break;

      case cocoonAction_leave: {
        actionLabels.push_back("Leave it");
      }
    }
  }
}

void Cocoon::getPossibleActions(
  vector<CocoonAction_t>& possibleActions) const {

  possibleActions.resize(0);
  possibleActions.push_back(cocoonAction_open);
  possibleActions.push_back(cocoonAction_leave);
}

void Cocoon::doAction(const CocoonAction_t action) {
  switch(action) {
    case cocoonAction_open: {
      openFeature();
    } break;

    case cocoonAction_leave: {
      eng.log->addMsg("I leave the cocoon for now.");
    } break;
  }
}

void Cocoon::getDescr(string& descr) const {
  descr = "A cocoon spun from spider web.";
}

//--------------------------------------------------------- ALTAR
//Altar::Altar(Feature_t id, Pos pos, Engine& engine) :
//  FeatureExaminable(id, pos, engine) {}
//
//void Altar::featureSpecific_examine() {
//}

//--------------------------------------------------------- CARVED PILLAR
//CarvedPillar::CarvedPillar(Feature_t id, Pos pos, Engine& engine) :
//  FeatureExaminable(id, pos, engine) {}
//
//void CarvedPillar::featureSpecific_examine() {
//}

//--------------------------------------------------------- BARREL
//Barrel::Barrel(Feature_t id, Pos pos, Engine& engine) :
//  FeatureExaminable(id, pos, engine) {}
//
//void Barrel::featureSpecific_examine() {
//}

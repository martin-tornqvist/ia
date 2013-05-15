#include "FeatureExaminable.h"

#include "Engine.h"
#include "Log.h"
#include "Render.h"
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

//------------------------------------------------------------------ BASE CLASS
FeatureExaminable::FeatureExaminable(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {}

void FeatureExaminable::examine() {
  tracer << "FeatureExaminable::examine()..." << endl;

  featureSpecific_examine();
  eng->gameTime->letNextAct();

  tracer << "FeatureExaminable::examine() [DONE]" << endl;
}

EventRegularity_t FeatureExaminable::getEventRegularity() {
  const int TOT = eventRegularity_common + eventRegularity_rare + eventRegularity_veryRare;
  const int RND = eng->dice.getInRange(1, TOT);
  if(RND <= eventRegularity_common) {
    return eventRegularity_common;
  } else if(RND <= eventRegularity_rare) {
    return eventRegularity_rare;
  } else {
    return eventRegularity_veryRare;
  }
}

//------------------------------------------------------------------ ITEM CONTAINER CLASS
ExaminableItemContainer::ExaminableItemContainer() {items_.resize(0);}

ExaminableItemContainer::~ExaminableItemContainer() {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
}

void ExaminableItemContainer::setRandomItemsForFeature(const Feature_t featureId,
    const int NR_ITEMS_TO_ATTEMPT, Engine* const engine) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    delete items_.at(i);
  }
  items_.resize(0);

  if(NR_ITEMS_TO_ATTEMPT > 0) {
    while(items_.empty()) {
      vector<ItemId_t> itemCandidates;
      for(unsigned int i = 1; i < endOfItemIds; i++) {
        ItemDefinition* const currentDef = engine->itemData->itemDefinitions[i];
        for(unsigned int ii = 0; ii < currentDef->featuresCanBeFoundIn.size(); ii++) {
          pair<Feature_t, int> featuresFoundIn = currentDef->featuresCanBeFoundIn.at(ii);
          if(featuresFoundIn.first == featureId) {
            if(engine->dice.percentile() < featuresFoundIn.second) {
              if(engine->dice.percentile() < currentDef->chanceToIncludeInSpawnList) {
                itemCandidates.push_back(static_cast<ItemId_t>(i));
                break;
              }
            }
          }
        }
      }

      const int NR_CANDIDATES = static_cast<int>(itemCandidates.size());
      if(NR_CANDIDATES > 0) {
        for(int i = 0; i < NR_ITEMS_TO_ATTEMPT; i++) {
          const unsigned int ELEMENT = engine->dice.getInRange(0, NR_CANDIDATES - 1);
          Item* item = engine->itemFactory->spawnItem(itemCandidates.at(ELEMENT));
          engine->itemFactory->setItemRandomizedProperties(item);
          items_.push_back(item);
        }
      }
    }
  }
}

void ExaminableItemContainer::dropItems(const coord& pos, Engine* const engine) {
  for(unsigned int i = 0; i < items_.size(); i++) {
    engine->itemDrop->dropItemOnMap(pos, &(items_.at(i)));
  }
  items_.resize(0);
}

void ExaminableItemContainer::destroySingleFragile(Engine* const engine) {
  //TODO Generalize this function (perhaps isFragile variable for item defs)

  for(unsigned int i = 0; i < items_.size(); i++) {
    Item* const item = items_.at(i);
    const ItemDefinition& d = item->getDef();
    if(d.isQuaffable || d.id == item_molotov) {
      delete item;
      items_.erase(items_.begin() + i);
      engine->log->addMessage("I hear a muffled shatter.");
    }
  }
}

//------------------------------------------------------------------ SPECIFIC FEATURES
//--------------------------------------------------------- TOMB
Tomb::Tomb(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false),
  isTraitKnown_(false), chanceToPushLid_(100), appearance_(tombAppearance_common),
  trait_(endOfTombTraits) {

  //Contained items
  PlayerBonusHandler* const bonHandler = eng->playerBonusHandler;
  const int NR_ITEMS_MIN = eng->dice.percentile() < 30 ? 0 : 1;
  const int NR_ITEMS_MAX =
    NR_ITEMS_MIN + (bonHandler->isBonusPicked(playerBonus_treasureHunter) ? 1 : 0);

  itemContainer_.setRandomItemsForFeature(
    feature_tomb, eng->dice.getInRange(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  //Exterior appearance
  if(engine->dice.percentile() < 20) {
    tracer << "Tomb: Setting random appearance" << endl;
    appearance_ = static_cast<TombAppearance_t>(engine->dice.getInRange(0, endOfTombAppearance - 1));
  } else {
    tracer << "Tomb: Setting appearance according to items contained (common if zero items)" << endl;
    for(unsigned int i = 0; i < itemContainer_.items_.size(); i++) {
      const ItemValue_t itemValue = itemContainer_.items_.at(i)->getDef().itemValue;
      if(itemValue == itemValue_majorTreasure) {
        tracer << "Tomb: Contains major treasure" << endl;
        appearance_ = tombAppearance_marvelous;
        break;
      } else if(itemValue == itemValue_minorTreasure) {
        tracer << "Tomb: Contains minor treasure" << endl;
        appearance_ = tombAppearance_impressive;
      }
    }
  }

  const bool IS_CONTAINING_ITEMS = itemContainer_.items_.empty() == false;

  chanceToPushLid_ = IS_CONTAINING_ITEMS ? engine->dice.getInRange(0, 75) : 90;

  if(IS_CONTAINING_ITEMS) {
    const int RND = engine->dice.percentile();

    if(RND < 15) {
      trait_ = tombTrait_forebodingCarvedSigns;
    } else if(RND < 45) {
      trait_ = tombTrait_stench;
    } else if(RND < 75) {
      trait_ = tombTrait_auraOfUnrest;
    }
    tracer << "Tomb: Set trait (" << trait_ << ")" << endl;
  }
}

void Tomb::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng->log->addMessage("The tomb is empty.");
  } else {
    vector<TombAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng->popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A tomb");
    doAction(possibleActions.at(static_cast<unsigned int>(CHOICE_NR)));
  }
}

void Tomb::doAction(const TombAction_t action) {
  StatusEffectsHandler* const statusHandler = eng->player->getStatusEffectsHandler();
  PlayerBonusHandler* const bonusHandler = eng->playerBonusHandler;

  const bool IS_TOUGH     = bonusHandler->isBonusPicked(playerBonus_tough);
  const bool IS_RUGGED    = bonusHandler->isBonusPicked(playerBonus_rugged);
  const bool IS_OBSERVANT = bonusHandler->isBonusPicked(playerBonus_observant);
  const bool IS_CONFUSED  = statusHandler->hasEffect(statusConfused);
  const bool IS_WEAK      = statusHandler->hasEffect(statusWeak);
  const bool IS_CURSED    = statusHandler->hasEffect(statusCursed);
  const bool IS_BLESSED   = statusHandler->hasEffect(statusBlessed);

  switch(action) {
    case tombAction_carveCurseWard: {
      const int CHANCE_TO_CARVE = 80;
      if(IS_CURSED == false && (IS_BLESSED || eng->dice.percentile() < CHANCE_TO_CARVE)) {
        eng->log->addMessage("The curse is cleared.");
      } else {
        eng->log->addMessage("I make a misstake, the curse is doubled!");
        StatusCursed* const curse = new StatusCursed(eng);
        curse->turnsLeft *= 2;
        statusHandler->tryAddEffect(curse, true);
      }
      trait_ = endOfTombTraits;
    } break;

    case tombAction_leave: {
      eng->log->addMessage("I leave the tomb for now.");
    } break;

    case tombAction_pushLid: {
      eng->log->addMessage("I attempt to push the lid.");

      const int CHANCE_TO_SPRAIN    = 15;
      const int CHANCE_TO_PARALYZE  = 30;

      if(eng->dice.percentile() < CHANCE_TO_SPRAIN) {
        eng->log->addMessage("I sprain myself.", clrMessageBad);
        eng->player->hit(1, damageType_pure);
      }

      if(eng->player->deadState != actorDeadState_alive) {
        return;
      }

      if(eng->dice.percentile() < CHANCE_TO_PARALYZE) {
        eng->log->addMessage("I am off-balance.");
        statusHandler->tryAddEffect(new StatusParalyzed(2));
      }

      if(IS_WEAK) {
        eng->log->addMessage("It seems futile.");
        return;
      }
      const int BON = IS_RUGGED ? 20 : (IS_TOUGH ? 10 : 0);
      if(eng->dice.percentile() < chanceToPushLid_ + BON) {
        eng->log->addMessage("The lid comes off!");
        openFeature();
      } else {
        eng->log->addMessage("The lid resists.");
      }
    } break;

    case tombAction_searchExterior: {
      const int CHANCE_TO_FIND = 50 + IS_OBSERVANT * 40 - IS_CONFUSED * 10;
      const bool IS_ROLL_SUCCESS = eng->dice.percentile() < CHANCE_TO_FIND;
      if(IS_ROLL_SUCCESS && trait_ != endOfTombTraits) {
        string traitDescr = "";
        getTraitDescr(traitDescr);
        eng->log->addMessage(traitDescr);
        isTraitKnown_ = true;
      } else {
        eng->log->addMessage("I find nothing significant.");
      }
    } break;

    case tombAction_smashLidWithSledgehammer: {
      eng->log->addMessage("I strike at the lid with a Sledgehammer.");
      const int CHANCE_TO_BREAK = IS_WEAK ? 10 : 90;
      if(eng->dice.percentile() < CHANCE_TO_BREAK) {
        eng->log->addMessage("The lid cracks open!");
        if(IS_BLESSED == false && (IS_CURSED || eng->dice.percentile() < 33)) {
          itemContainer_.destroySingleFragile(eng);
        }
        openFeature();
      } else {
        eng->log->addMessage("The lid resists.");
      }
    } break;
  }
}

bool Tomb::openFeature() {
  eng->log->addMessage("The tomb opens.");
  triggerTrap();
  if(itemContainer_.items_.size() > 0) {
    eng->log->addMessage("There are some items in the tomb.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    eng->log->addMessage("There is nothing of value inside the tomb.");
  }
  eng->renderer->drawMapAndInterface(true);
  isContentKnown_ = isTraitKnown_ = true;

  return true;
}

void Tomb::triggerTrap() {
  vector<ActorId_t> actorCandidates;

  switch(trait_) {
    case tombTrait_auraOfUnrest: {
      for(unsigned int i = 1; i < endOfActorIds; i++) {
        const ActorDefinition& d = eng->actorData->actorDefinitions[i];
        if(d.isGhost && d.isAutoSpawnAllowed && d.isUnique == false) {
          actorCandidates.push_back(static_cast<ActorId_t>(i));
        }
      }
      eng->log->addMessage("Something rises from the tomb!");
    } break;

    case tombTrait_forebodingCarvedSigns: {
      eng->player->getStatusEffectsHandler()->tryAddEffect(new StatusCursed(eng));
    } break;

    case tombTrait_stench: {
      if(eng->dice.coinToss()) {
        eng->log->addMessage("Fumes burst out from the tomb!");
        StatusEffect* effect = NULL;
        SDL_Color fumeClr = clrMagenta;
        const int RND = eng->dice.percentile();
        if(RND < 20) {
          effect = new StatusPoisoned(eng);
          fumeClr = clrGreenLight;
        } else if(RND < 40) {
          effect = new StatusDiseased(eng);
          fumeClr = clrGreen;
        } else {
          effect = new StatusParalyzed(eng);
          effect->turnsLeft *= 2;
        }
        eng->explosionMaker->runExplosion(pos_, false, effect, true, fumeClr);
      } else {
        for(unsigned int i = 1; i < endOfActorIds; i++) {
          const ActorDefinition& d = eng->actorData->actorDefinitions[i];
          if(d.moveType == moveType_ooze && d.isAutoSpawnAllowed && d.isUnique == false) {
            actorCandidates.push_back(static_cast<ActorId_t>(i));
          }
        }
        eng->log->addMessage("Something creeps up from the tomb!");
      }

    } break;

    default: {} break;
  }

  if(actorCandidates.size() > 0) {
    const unsigned int ELEM = eng->dice.getInRange(0, actorCandidates.size() - 1);
    Actor* const actor = eng->actorFactory->spawnActor(actorCandidates.at(ELEM), pos_);
    dynamic_cast<Monster*>(actor)->becomeAware();
  }
}

void Tomb::getPossibleActions(vector<TombAction_t>& possibleActions) const {
  const bool IS_WARLOCK = eng->playerBonusHandler->isBonusPicked(playerBonus_warlock);

  if(isTraitKnown_) {
    if(IS_WARLOCK) {
      if(trait_ == tombTrait_forebodingCarvedSigns) {
        possibleActions.push_back(tombAction_carveCurseWard);
      }
    }
  } else {
    possibleActions.push_back(tombAction_searchExterior);
  }

  possibleActions.push_back(tombAction_pushLid);

  const Inventory* const inv = eng->player->getInventory();
  bool hasSledgehammer = false;
  Item* item = inv->getItemInSlot(slot_wielded);
  if(item != NULL) {
    hasSledgehammer = item->getDef().id == item_sledgeHammer;
  }
  if(hasSledgehammer == false) {
    item = inv->getItemInSlot(slot_wieldedAlt);
    hasSledgehammer = item->getDef().id == item_sledgeHammer;
  }
  if(hasSledgehammer == false) {
    hasSledgehammer = inv->hasItemInGeneral(item_sledgeHammer);
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
  const bool IS_WARLOCK   = eng->playerBonusHandler->isBonusPicked(playerBonus_warlock);

  switch(trait_) {
    case tombTrait_auraOfUnrest: {
      descr = "It has a certain aura of unrest about it.";
    } break;

    case tombTrait_forebodingCarvedSigns: {
      if(IS_WARLOCK) {
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

  const bool IS_WEAK = eng->player->getStatusEffectsHandler()->hasEffect(statusWeak);

  if(chanceToPushLid_ < 10 || IS_WEAK) {
    descr += " The lid seems very heavy.";
  } else if(chanceToPushLid_ < 50) {
    descr += " The lid does not seem too heavy.";
  } else {
    descr += " I think I could move the lid with small effort.";
  }
}

//--------------------------------------------------------- CHEST
Chest::Chest(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false),
  isLocked_(false), isTrapped_(false), isTrapStatusKnown_(false) {

  PlayerBonusHandler* const bonHandler = eng->playerBonusHandler;
  const int CHANCE_FOR_EMPTY = 10;
  const int NR_ITEMS_MIN = eng->dice.percentile() < CHANCE_FOR_EMPTY ? 0 : 1;
  const int NR_ITEMS_MAX = bonHandler->isBonusPicked(playerBonus_treasureHunter) ? 4 : 3;
  itemContainer_.setRandomItemsForFeature(
    feature_chest, eng->dice.getInRange(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  if(itemContainer_.items_.empty() == false) {
    const int CHANCE_FOR_LOCKED = 80;
    isLocked_ = eng->dice.percentile() < CHANCE_FOR_LOCKED;

    const int CHANCE_FOR_TRAPPED = 60;
    isTrapped_ = eng->dice.percentile() < CHANCE_FOR_TRAPPED ? true : false;
  }
}

void Chest::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng->log->addMessage("The chest is empty.");
  } else {
    vector<ChestAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng->popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A chest");

    doAction(possibleActions.at(static_cast<unsigned int>(CHOICE_NR)));
  }
}

void Chest::doAction(const ChestAction_t action) {
  StatusEffectsHandler* const statusHandler = eng->player->getStatusEffectsHandler();
  PlayerBonusHandler* const bonHandler      = eng->playerBonusHandler;

  const bool IS_OBSERVANT = bonHandler->isBonusPicked(playerBonus_observant);
  const bool IS_TOUGH     = bonHandler->isBonusPicked(playerBonus_tough);
  const bool IS_RUGGED    = bonHandler->isBonusPicked(playerBonus_rugged);
//  const bool IS_NIMBLE    = bonHandler->isBonusPicked(playerBonus_nimbleHanded);
  const bool IS_CONFUSED  = statusHandler->hasEffect(statusConfused);
  const bool IS_WEAK      = statusHandler->hasEffect(statusWeak);
  const bool IS_CURSED    = statusHandler->hasEffect(statusCursed);
  const bool IS_BLESSED   = statusHandler->hasEffect(statusBlessed);

  switch(action) {
    case chestAction_open: {
      openFeature();
    }
    break;
    case chestAction_searchForTrap: {
      const int CHANCE_TO_FIND = 20 + IS_OBSERVANT * 40 - IS_CONFUSED * 10;
      const bool IS_ROLL_SUCCESS = eng->dice.percentile() < CHANCE_TO_FIND;
      if(IS_ROLL_SUCCESS && isTrapped_) {
        eng->log->addMessage("The chest has a hidden trap mechanism!");
        isTrapStatusKnown_ = true;
      } else {
        eng->log->addMessage("I find no indication that the chest is trapped.");
      }
    }
    break;
    case chestAction_disarmTrap: {
      eng->log->addMessage("I attempt to disarm the trap.");

      const int CHANCE_TO_TRIGGER = 20;
      if(eng->dice.percentile() < CHANCE_TO_TRIGGER) {
        eng->log->addMessage("I accidentally trigger it!");
        openFeature();
      } else {
        const int CHANCE_TO_DISARM = 50;
        if(eng->dice.percentile() < CHANCE_TO_DISARM) {
          eng->log->addMessage("I successfully disarm it!");
          isTrapped_ = false;
        } else {
          eng->log->addMessage("I failed to disarm it.");
        }
      }
    }
    break;
    case chestAction_forceLock: {
      Item* const wpn = eng->player->getInventory()->getItemInSlot(slot_wielded);

      if(wpn == NULL) {
        eng->log->addMessage("I attempt to punch the lock open, nearly breaking my hand.", clrMessageBad);
        eng->player->hit(1, damageType_pure);
      } else {
        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);

        if(eng->dice.percentile() < CHANCE_TO_DMG_WPN) {
          const string wpnName = eng->itemData->getItemRef(
                                   *wpn, itemRef_plain, true);
          eng->log->addMessage("My " + wpnName + " is damaged!");
          dynamic_cast<Weapon*>(wpn)->meleeDmgPlus--;
        }

        if(IS_WEAK) {
          eng->log->addMessage("It seems futile.");
        } else {
          const int CHANCE_TO_OPEN = 40;
          if(eng->dice.percentile() < CHANCE_TO_OPEN) {
            eng->log->addMessage("I force the lock open!");
            openFeature();
          } else {
            eng->log->addMessage("The lock resists.");
          }
        }
      }
    }
    break;
    case chestAction_kick: {
      const int CHANCE_TO_SPRAIN    = 20;

      if(eng->dice.percentile() < CHANCE_TO_SPRAIN) {
        eng->log->addMessage("I sprain myself.", clrMessageBad);
        eng->player->hit(1, damageType_pure);
      }

      if(eng->player->deadState == actorDeadState_alive) {
        if(IS_WEAK) {
          eng->log->addMessage("It seems futile.");
          return;
        }

        if(IS_BLESSED == false && (IS_CURSED || eng->dice.percentile() < 33)) {
          itemContainer_.destroySingleFragile(eng);
        }
        const int CHANCE_TO_OPEN = 20 + (IS_RUGGED ? 20 : (IS_TOUGH ? 10 : 0));
        if(eng->dice.percentile() < CHANCE_TO_OPEN) {
          eng->log->addMessage("I kick the lid open!");
          openFeature();
        } else {
          eng->log->addMessage("The lock resists.");
        }
      }
    }
    break;
    case chestAction_leave: {
      eng->log->addMessage("I leave the chest for now.");
    }
    break;
  }
}

bool Chest::openFeature() {
  eng->log->addMessage("The chest opens.");
  if(isTrapped_) {
    triggerTrap();
  }
  if(eng->player->deadState == actorDeadState_alive) {
    if(itemContainer_.items_.empty()) {
      eng->log->addMessage("There is nothing of value in the chest.");
    } else {
      eng->log->addMessage("There are some items in the chest.");
      itemContainer_.dropItems(pos_, eng);
      eng->renderer->drawMapAndInterface(true);
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
    possibleActions.push_back(chestAction_forceLock);
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

    const int CHANCE_FOR_EXPLODING = 20;
    if(
      eng->map->getDungeonLevel() >= MIN_DLVL_NASTY_TRAPS &&
      eng->dice.percentile() < CHANCE_FOR_EXPLODING) {
      eng->log->addMessage("The trap explodes!");
      eng->explosionMaker->runExplosion(pos_, true);
      if(eng->player->deadState == actorDeadState_alive) {
        eng->featureFactory->spawnFeatureAt(feature_rubbleLow, pos_);
      }
    } else {
      eng->log->addMessage("Fumes burst out from the chest!");
      StatusEffect* effect = NULL;
      SDL_Color fumeClr = clrMagenta;
      const int RND = eng->dice.percentile();
      if(RND < 20) {
        effect = new StatusPoisoned(eng);
        fumeClr = clrGreenLight;
      } else if(RND < 40) {
        effect = new StatusDiseased(eng);
        fumeClr = clrGreen;
      } else {
        effect = new StatusParalyzed(eng);
        effect->turnsLeft *= 2;
      }
      eng->explosionMaker->runExplosion(pos_, false, effect, true, fumeClr);
    }
  }
}

//--------------------------------------------------------- CABINET
Cabinet::Cabinet(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false) {

  PlayerBonusHandler* const bonHandler = eng->playerBonusHandler;
  const int CHANCE_FOR_EMPTY = 50;
  const int NR_ITEMS_MIN = eng->dice.percentile() < CHANCE_FOR_EMPTY ? 0 : 1;
  const int NR_ITEMS_MAX = bonHandler->isBonusPicked(playerBonus_treasureHunter) ? 2 : 1;
  itemContainer_.setRandomItemsForFeature(
    feature_cabinet, eng->dice.getInRange(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cabinet::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng->log->addMessage("The cabinet is empty.");
  } else {
    vector<CabinetAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng->popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A cabinet");
    doAction(possibleActions.at(static_cast<unsigned int>(CHOICE_NR)));
  }
}

void Cabinet::triggerTrap() {

}

bool Cabinet::openFeature() {
  eng->log->addMessage("The cabinet opens.");
  triggerTrap();
  if(itemContainer_.items_.size() > 0) {
    eng->log->addMessage("There are some items in the cabinet.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    eng->log->addMessage("There is nothing of value inside.");
  }
  eng->renderer->drawMapAndInterface(true);
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

void Cabinet::getPossibleActions(vector<CabinetAction_t>& possibleActions) const {
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
      eng->log->addMessage("I leave the cabinet for now.");
    } break;
  }
}

void Cabinet::getDescr(string& descr) const {
  descr = "";
}

//--------------------------------------------------------- COCOON
Cocoon::Cocoon(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false) {

  PlayerBonusHandler* const bonHandler = eng->playerBonusHandler;
  const int CHANCE_FOR_EMPTY = 60;
  const int NR_ITEMS_MIN = eng->dice.percentile() < CHANCE_FOR_EMPTY ? 0 : 1;
  const int NR_ITEMS_MAX = NR_ITEMS_MIN +
                           bonHandler->isBonusPicked(playerBonus_treasureHunter) ? 1 : 0;
  itemContainer_.setRandomItemsForFeature(
    feature_cocoon, eng->dice.getInRange(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);
}

void Cocoon::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng->log->addMessage("The cocoon is empty.");
  } else {
    vector<CocoonAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string descr = "";
    getDescr(descr);

    const int CHOICE_NR = eng->popup->showMultiChoiceMessage(
                            descr, true, actionLabels, "A cocoon");
    doAction(possibleActions.at(static_cast<unsigned int>(CHOICE_NR)));
  }
}

void Cocoon::triggerTrap() {
  const int RND = eng->dice.percentile();

  if(RND < 15) {
    eng->log->addMessage("There is a half-dissolved human body inside!");
    eng->player->incrShock(shockValue_heavy);
  } else if(RND < 50) {
    tracer << "Cocoon: Attempting to spawn spiders" << endl;
    vector<ActorId_t> spawnCandidates;
    for(unsigned int i = 1; i < endOfActorIds; i++) {
      const ActorDefinition& d = eng->actorData->actorDefinitions[i];
      if(d.isSpider && d.actorSize == actorSize_floor &&
          d.isAutoSpawnAllowed && d.isUnique == false) {
        spawnCandidates.push_back(d.id);
      }
    }

    const int NR_CANDIDATES = spawnCandidates.size();
    if(NR_CANDIDATES != 0) {
      tracer << "Cocoon: Spawn candidates found, attempting to place" << endl;
      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
      vector<coord> freeCells;
      eng->populateMonsters->makeSortedFreeCellsVector(pos_, blockers, freeCells);

      const int NR_SPIDERS_MAX = min(eng->dice.getInRange(2, 4),
                                     static_cast<int>(freeCells.size()));

      if(NR_SPIDERS_MAX > 0) {
        tracer << "Cocoon: Found positions, spawning spiders" << endl;
        eng->log->addMessage("There are spiders inside!");
        const ActorId_t id = spawnCandidates.at(
                               eng->dice.getInRange(0, NR_CANDIDATES - 1));
        for(int i = 0; i < NR_SPIDERS_MAX; i++) {
          Actor* const actor = eng->actorFactory->spawnActor(id, freeCells.front());
          dynamic_cast<Monster*>(actor)->becomeAware();
          freeCells.erase(freeCells.begin());
        }
      }
    }
  }
}

bool Cocoon::openFeature() {
  eng->log->addMessage("The cocoon opens.");
  triggerTrap();
  if(itemContainer_.items_.size() > 0) {
    eng->log->addMessage("There are some items in the cocoon.");
    itemContainer_.dropItems(pos_, eng);
  } else {
    eng->log->addMessage("There is nothing of value inside.");
  }
  eng->renderer->drawMapAndInterface(true);
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

void Cocoon::getPossibleActions(vector<CocoonAction_t>& possibleActions) const {
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
      eng->log->addMessage("I leave the cocoon for now.");
    } break;
  }
}

void Cocoon::getDescr(string& descr) const {
  descr = "A cocoon spun from spider web.";
}

//--------------------------------------------------------- ALTAR
//Altar::Altar(Feature_t id, coord pos, Engine* engine) :
//  FeatureExaminable(id, pos, engine) {}
//
//void Altar::featureSpecific_examine() {
//}

//--------------------------------------------------------- CARVED PILLAR
//CarvedPillar::CarvedPillar(Feature_t id, coord pos, Engine* engine) :
//  FeatureExaminable(id, pos, engine) {}
//
//void CarvedPillar::featureSpecific_examine() {
//}

//--------------------------------------------------------- BARREL
//Barrel::Barrel(Feature_t id, coord pos, Engine* engine) :
//  FeatureExaminable(id, pos, engine) {}
//
//void Barrel::featureSpecific_examine() {
//}

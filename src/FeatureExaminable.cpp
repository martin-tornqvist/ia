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

void ExaminableItemContainer::setRandomItemsForFeature(const Feature_t featureId, const int NR_ITEMS_TO_ATTEMPT, Engine* const engine) {
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


//------------------------------------------------------------------ SPECIFIC FEATURES
//--------------------------------------------------------- TOMB
Tomb::Tomb(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false),
  isTraitKnown_(false), chanceToPushLid_(100), appearance_(tombAppearance_common),
  trait_(endOfTombTraits) {

  //Contained items
  const int NR_ITEMS_MIN = eng->dice.percentile() < 75 ? 0 : 1;
  const int NR_ITEMS_MAX = NR_ITEMS_MIN + eng->playerBonusHandler->isBonusPicked(playerBonus_treasureHunter);
  itemContainer_.setRandomItemsForFeature(feature_tomb, eng->dice.getInRange(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  //Exterior appearance
  if(engine->dice.percentile() < 20) {
    //Random appearance
    appearance_ = static_cast<TombAppearance_t>(engine->dice.getInRange(0, endOfTombAppearance - 1));
  } else {
    //True appearance
    for(unsigned int i = 0; i < itemContainer_.items_.size(); i++) {
      const ItemValue_t itemValue = itemContainer_.items_.at(i)->getDef().itemValue;
      if(itemValue == itemValue_majorTreasure) {
        appearance_ = tombAppearance_marvelous;
        break;
      } else if(itemValue == itemValue_minorTreasure) {
        appearance_ = tombAppearance_impressive;
      }
    }
  }

  const bool IS_CONTAINING_ITEMS = itemContainer_.items_.empty() == false;

  chanceToPushLid_ = IS_CONTAINING_ITEMS ? 90 : engine->dice.getInRange(1, 75);

  if(IS_CONTAINING_ITEMS) {
    if(engine->dice.percentile() < 11) {
      trait_ = tombTrait_forebodingCarvedSigns;
    }

    const int RND = engine->dice.percentile();
    if(RND < 25) {
      trait_ = tombTrait_stench;
    } else if(RND < 45) {
      trait_ = tombTrait_auraOfUnrest;
    }
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

    string description = "";
    getDescription(description);

    const int CHOICE_NR = eng->popup->showMultiChoiceMessage(description, true, actionLabels, "A tomb");
    doAction(possibleActions.at(static_cast<unsigned int>(CHOICE_NR)));
  }
}

void Tomb::doAction(const TombAction_t action) {
  switch(action) {
    case tombAction_carveCurseWard: {
    } break;
    case tombAction_leave: {
    } break;
    case tombAction_pushLid: {
    } break;
    case tombAction_searchExterior: {
    } break;
    case tombAction_smashLidWithSledgehammer: {
    } break;
  }
}

void Tomb::getPossibleActions(vector<TombAction_t>& possibleActions) const {

}

void Tomb::getChoiceLabels(const vector<TombAction_t>& possibleActions,
                           vector<string>& actionLabels) const {

}

void Tomb::getDescription(string& description) const {
  switch(appearance_) {
    case tombAppearance_common:     {description = "It looks ordinary.";} break;
    case tombAppearance_impressive: {description = "It looks impressive.";} break;
    case tombAppearance_marvelous:  {description = "It looks marvelous!";} break;
  }

  const bool IS_OCCULTIST = eng->playerBonusHandler->isBonusPicked(playerBonus_occultist);
  const bool IS_WARLOCK   = eng->playerBonusHandler->isBonusPicked(playerBonus_warlock);

  if(isTraitKnown_) {
    switch(trait_) {
      case tombTrait_auraOfUnrest: {
        description += " It has a certain aura of unrest about it.";
      } break;
      case tombTrait_forebodingCarvedSigns: {
        if(IS_WARLOCK) {
          description += " There is a curse carved on the box.";
        } else {
          description += " There are foreboding runes carved on the box.";
        }
      } break;
      case tombTrait_stench: {
        description += " There is a pungent stench.";
      } break;
    }
  }

  if(chanceToPushLid_ < 10) {
    description += " The lid seems very heavy.";
  } else if(chanceToPushLid_ < 50) {
    description += " The lid doesn't seem too heavy.";
  } else {
    description += " I think I could move the lid with small effort.";
  }
}

//--------------------------------------------------------- CHEST
Chest::Chest(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine), isContentKnown_(false), isLocked_(false), isTrapped_(false), isTrapStatusKnown_(false) {

  //EventRegularity_t eventRegularity = getEventRegularity();
  const int CHANCE_FOR_EMPTY = 10;
  const int NR_ITEMS_MIN = eng->dice.percentile() < CHANCE_FOR_EMPTY ? 0 : 1;
  const int NR_ITEMS_MAX = eng->playerBonusHandler->isBonusPicked(playerBonus_treasureHunter) ? 4 : 3;
  itemContainer_.setRandomItemsForFeature(feature_chest, eng->dice.getInRange(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  if(itemContainer_.items_.empty() == false) {
    const int CHANCE_FOR_LOCKED = 80;
    isLocked_ = eng->dice.percentile() < CHANCE_FOR_LOCKED;

    const int CHANCE_FOR_TRAPPED = 25;
    isTrapped_ = eng->dice.percentile() < CHANCE_FOR_TRAPPED ? true : false;
  }
}

//sf::Color Chest::getColor() const {
//  return material_ == doorMaterial_wood ? clrBrownDark : clrGray;
//}

//string Chest::getDescription(const bool DEFINITE_ARTICLE) const {
//}

void Chest::featureSpecific_examine() {
  if(itemContainer_.items_.empty() && isContentKnown_) {
    eng->log->addMessage("There is nothing of value in the chest.");
  } else {
    vector<ChestAction_t> possibleActions;
    getPossibleActions(possibleActions);

    vector<string> actionLabels;
    getChoiceLabels(possibleActions, actionLabels);

    string description = "";
    getDescription(description);

    const int CHOICE_NR = eng->popup->showMultiChoiceMessage(description, true, actionLabels, "A chest");

    doAction(possibleActions.at(static_cast<unsigned int>(CHOICE_NR)));
  }
}

void Chest::doAction(const ChestAction_t action) {
  const bool IS_NIMBLE = eng->playerBonusHandler->isBonusPicked(playerBonus_nimbleHanded);
  const bool IS_OBSERVANT = eng->playerBonusHandler->isBonusPicked(playerBonus_observant);
  const bool IS_CONFUSED = eng->player->getStatusEffectsHandler()->hasEffect(statusConfused);

  switch(action) {
    case chestAction_open: {
      if(isTrapped_) {
        triggerTrap();
      } else {
        if(itemContainer_.items_.empty()) {
          eng->log->addMessage("There is nothing of value in the chest.");
        } else {
          eng->log->addMessage("I find some items in the chest.");
          itemContainer_.dropItems(pos_, eng);
          eng->renderer->drawMapAndInterface(true);
        }
        isContentKnown_ = true;
        isTrapStatusKnown_ = true;
      }
    }
    break;
    case chestAction_searchForTrap: {
      const int CHANCE_TO_FIND_TRAP = 20 + IS_OBSERVANT * 40 - IS_CONFUSED * 10;
      const bool IS_ROLL_SUCCESS = eng->dice.percentile() < CHANCE_TO_FIND_TRAP;
      if(IS_ROLL_SUCCESS && isTrapped_) {
        eng->log->addMessage("The chest has a hidden trap mechanism!");
        isTrapStatusKnown_ = true;
      } else {
        eng->log->addMessage("I find no indication that the chest is trapped.");
      }
    }
    break;
    case chestAction_disarmTrap: {

    }
    break;
    case chestAction_forceLock: {

    }
    break;
    case chestAction_kick: {

    }
    break;
    case chestAction_leave: {
      eng->log->addMessage("I leave the chest for now.");
    }
    break;
  }
}

void Chest::getPossibleActions(vector<ChestAction_t>& possibleActions) const {
  if(isTrapStatusKnown_ == false) {
    possibleActions.push_back(chestAction_searchForTrap);
  } else if(isTrapped_) {
    possibleActions.push_back(chestAction_disarmTrap);
  }

  if(isLocked_) {
    possibleActions.push_back(chestAction_kick);

    //TODO check wielded weapon
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
      case chestAction_open:          {actionLabels.push_back("Open it");} break;
      case chestAction_searchForTrap: {actionLabels.push_back("Search it for traps");} break;
      case chestAction_disarmTrap:    {actionLabels.push_back("Disarm the trap");} break;
      case chestAction_forceLock:     {actionLabels.push_back("Force the lock");} break;
      case chestAction_kick:          {actionLabels.push_back("Kick the lid");} break;
      case chestAction_leave:         {actionLabels.push_back("Leave it");} break;
    }
  }
}

void Chest::getDescription(string& description) const {
  const string lockDescr = isLocked_ ? "locked" : "not locked";
  const string trapDescr = isTrapped_ && isTrapStatusKnown_ ? " There appears to be a trap mechanism." : "";
  description = "It is " + lockDescr + "." + trapDescr;
}

void Chest::triggerTrap() {
  isTrapped_ = false;
  isTrapStatusKnown_ = true;
}

//--------------------------------------------------------- CABINET
Cabinet::Cabinet(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Cabinet::featureSpecific_examine() {
}

//--------------------------------------------------------- COCOON
Cocoon::Cocoon(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {}

void Cocoon::featureSpecific_examine() {
}

//--------------------------------------------------------- ALTAR
Altar::Altar(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {}

void Altar::featureSpecific_examine() {
}

//--------------------------------------------------------- CARVED PILLAR
CarvedPillar::CarvedPillar(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {}

void CarvedPillar::featureSpecific_examine() {
}

//--------------------------------------------------------- BARREL
Barrel::Barrel(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {}

void Barrel::featureSpecific_examine() {
}

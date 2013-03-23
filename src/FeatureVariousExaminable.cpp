#include "FeatureVariousExaminable.h"

#include "Engine.h"
#include "Log.h"
#include "Render.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemFactory.h"
#include "ItemDrop.h"
#include "Popup.h"
#include "PlayerBonuses.h"

//------------------------------------------------------------------ BASE CLASS
FeatureExaminable::FeatureExaminable(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {

}

//sf::Color FeatureExaminable::getColorBg() const {
//  return isExaminableFurther_ ? clrBlue : clrBlack;
//}

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
ExaminableItemContainer::ExaminableItemContainer() {
  items_.resize(0);
}

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
          if(currentDef->featuresCanBeFoundIn.at(ii) == featureId) {
            if(engine->dice(1, 100) < currentDef->chanceToIncludeInSpawnList) {
              itemCandidates.push_back(static_cast<ItemId_t>(i));
              break;
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
  FeatureExaminable(id, pos, engine) {

}

void Tomb::featureSpecific_examine() {
}

//--------------------------------------------------------- CABINET
Cabinet::Cabinet(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Cabinet::featureSpecific_examine() {
}

//--------------------------------------------------------- CHEST
Chest::Chest(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine), isLocked_(false), isSmashed_(false), material_(doorMaterial_wood) {

  material_ = engine->dice.coinToss() ? doorMaterial_wood : doorMaterial_metal;

  //EventRegularity_t eventRegularity = getEventRegularity();
  const int CHANCE_FOR_EMPTY = 20;
  const int NR_ITEMS_MIN = eng->dice(1, 100) < CHANCE_FOR_EMPTY ? 0 : 1;
  const int NR_ITEMS_MAX = eng->playerBonusHandler->isBonusPicked(playerBonus_treasureHunter) ? 3 : 2;
  itemContainer_.setRandomItemsForFeature(feature_chest, eng->dice.getInRange(NR_ITEMS_MIN, NR_ITEMS_MAX), eng);

  if(itemContainer_.items_.empty() == false) {
    const int CHANCE_FOR_LOCKED_WHEN_CONTAINING_ITEMS = 80;
    isLocked_ = eng->dice(1, 100) < CHANCE_FOR_LOCKED_WHEN_CONTAINING_ITEMS;
  }
}

sf::Color Chest::getColor() const {
  return material_ == doorMaterial_wood ? clrBrownDark : clrGray;
}

string Chest::getDescription(const bool DEFINITE_ARTICLE) const {
  if(material_ == doorMaterial_wood) {
    if(isSmashed_) {
      return DEFINITE_ARTICLE ? "the smashed wooden chest" : "a smashed wooden chest";
    } else {
      return DEFINITE_ARTICLE ? "the wooden chest" : "a wooden chest";
    }
  } else if(material_ == doorMaterial_metal) {
    if(isSmashed_) {
      return DEFINITE_ARTICLE ? "the smashed iron chest" : "a smashed iron chest";
    } else {
      return DEFINITE_ARTICLE ? "the iron chest" : "a iron chest";
    }
  }
  return "[WARNING] Chest lacks description?";
}

void Chest::featureSpecific_examine() {
  string nameThe = getDescription(true);
  nameThe[0] = toupper(nameThe[0]);

  if(isLocked_) {
    vector<string> choicesOnLocked;
    choicesOnLocked.push_back("Alt 1");
    choicesOnLocked.push_back("Alt 2");
    choicesOnLocked.push_back("Alt 3");
    eng->popup->showMultiChoiceMessage(nameThe + " is locked.", true, true, choicesOnLocked, "Locked!");
  } else if(itemContainer_.items_.empty()) {
    eng->log->addMessage("I find nothing of value in the chest.");
  } else {
    eng->log->addMessage("I find some items in the chest.");
    itemContainer_.dropItems(pos_, eng);
  }
  eng->renderer->drawMapAndInterface(true);
}

//--------------------------------------------------------- COCOON
Cocoon::Cocoon(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Cocoon::featureSpecific_examine() {
}

//--------------------------------------------------------- ALTAR
Altar::Altar(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Altar::featureSpecific_examine() {
}

//--------------------------------------------------------- CARVED PILLAR
CarvedPillar::CarvedPillar(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void CarvedPillar::featureSpecific_examine() {
}

//--------------------------------------------------------- BARREL
Barrel::Barrel(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Barrel::featureSpecific_examine() {
}

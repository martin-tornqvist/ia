#include "InventoryHandler.h"

#include "Engine.h"

#include "ItemScroll.h"
#include "ItemExplosive.h"
#include "ActorPlayer.h"
#include "ItemPotion.h"
#include "InventoryIndexes.h"
#include "MenuBrowser.h"
#include "Log.h"
#include "RenderInventory.h"
#include "MenuInputHandler.h"
#include "Render.h"
#include "ItemDrop.h"
#include "Query.h"
#include "ItemFactory.h"

InventoryHandler::InventoryHandler(Engine* engine) : eng(engine) {
}

void InventoryHandler::activateDefault(const unsigned int GENERAL_ITEMS_ELEMENT) {
  Inventory* const playerInv = eng->player->getInventory();
  Item* item = playerInv->getGeneral()->at(GENERAL_ITEMS_ELEMENT);
  if(item->activateDefault(eng->player, eng)) {
    playerInv->decreaseItemInGeneral(GENERAL_ITEMS_ELEMENT);
  }
}

void InventoryHandler::filterPlayerGeneralSlotButtonsEquip(const SlotTypes_t slotToEquip) {
  vector<Item*>* general = eng->player->getInventory()->getGeneral();
  generalItemsToShow.resize(0);

  for(unsigned int i = 0; i < general->size(); i++) {
    const Item* const item = general->at(i);
    const ItemDefinition& def = item->getDef();

    switch(slotToEquip) {
    case slot_armorBody: {
      if(def.isArmor) {
        generalItemsToShow.push_back(i);
      }
    }
    break;
    case slot_wielded: {
      if(def.isMeleeWeapon || def.isRangedWeapon) {
        generalItemsToShow.push_back(i);
      }
    }
    break;
    case slot_wieldedAlt: {
      if(def.isMeleeWeapon || def.isRangedWeapon) {
        generalItemsToShow.push_back(i);
      }
    }
    break;
    case slot_missiles: {
      if(def.isMissileWeapon) {
        generalItemsToShow.push_back(i);
      }
    }
    break;
    }
  }
}

void InventoryHandler::filterPlayerGeneralSlotButtonsUsable() {
  vector<Item*>* general = eng->player->getInventory()->getGeneral();

  vector< vector<unsigned int> > groups;

  for(unsigned int i = 0; i < general->size(); i++) {
    const Item* const item = general->at(i);
    const string& label = item->getDefaultActivationLabel();
    if(label != "") {
      bool isExistingGroupFound = false;
      for(unsigned int ii = 0; ii < groups.size(); ii++) {
        if(label == general->at(groups.at(ii).front())->getDefaultActivationLabel()) {
          groups.at(ii).push_back(i);
          isExistingGroupFound = true;
          break;
        }
      }
      if(isExistingGroupFound == false) {
        groups.resize(groups.size() + 1);
        groups.back().resize(0);
        groups.back().push_back(i);
      }
    }
  }

  generalItemsToShow.resize(0);

  for(unsigned int i = 0; i < groups.size(); i++) {
    for(unsigned int ii = 0; ii < groups.at(i).size(); ii++) {
      generalItemsToShow.push_back(groups.at(i).at(ii));
    }
  }
}

void InventoryHandler::filterPlayerGeneralSlotButtonsShowAll() {
  vector<Item*>* general = eng->player->getInventory()->getGeneral();
  generalItemsToShow.resize(0);

  for(unsigned int i = 0; i < general->size(); i++) {
    generalItemsToShow.push_back(i);
  }
}

void InventoryHandler::runSlotsScreen() {
  eng->renderer->drawMapAndInterface();
  sf::Texture bgTexture = eng->renderer->getScreenTextureCopy();

  Inventory* const inv = eng->player->getInventory();
  vector<InventorySlot>* invSlots = inv->getSlots();

  inv->sortGeneralInventory(eng);

  equipmentSlotButtons.resize(0);
  char key = 'a';
  InventorySlotButton inventorySlotButton;
  for(unsigned int i = 0; i < invSlots->size(); i++) {
    inventorySlotButton.inventorySlot = &invSlots->at(i);
    inventorySlotButton.key = key;
    key++;
    equipmentSlotButtons.push_back(inventorySlotButton);
  }

  MenuBrowser browser(invSlots->size() + 1, 0);
  eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons, bgTexture);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons, bgTexture);
    }
    break;
    case menuAction_selectedWithShift: {
      if(runDropScreen(browser.getPos().y)) {
        eng->renderer->drawMapAndInterface();
        return;
      }
      eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons, bgTexture);
    }
    break;
    case menuAction_selected: {
      const char charIndex = 'a' + browser.getPos().y;
      if(charIndex >= 'a' && charIndex <= equipmentSlotButtons.back().key) {
        InventorySlot* const slot = &(invSlots->at(charIndex - 'a'));
        if(slot->item == NULL) {
          if(runEquipScreen(slot)) {
            eng->renderer->drawMapAndInterface();
            return;
          } else {
            eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons, bgTexture);
          }
        } else {
          const bool IS_ARMOR = slot->devName == slot_armorBody;
          const string itemName = eng->itemData->getItemRef(slot->item, itemRef_plain);
          inv->moveItemToGeneral(slot);
          if(IS_ARMOR) {
            eng->log->addMessage("I take off my " + itemName + ".");
            eng->renderer->drawMapAndInterface();
            eng->gameTime->letNextAct();
            return;
          } else {
            eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons, bgTexture);
          }
        }
      } else {
        if(runBrowseInventoryMode()) {
          return;
        }
        eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons, bgTexture);
      }
    }
    break;
    case menuAction_canceled: {
      eng->renderer->drawMapAndInterface();
      return;
    }
    break;
    }
  }
}

bool InventoryHandler::runUseScreen() {
  eng->renderer->drawMapAndInterface();
  sf::Texture bgTexture = eng->renderer->getScreenTextureCopy();

  eng->player->getInventory()->sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsUsable();
  MenuBrowser browser(generalItemsToShow.size(), 0);
  eng->renderInventory->drawUseMode(browser, generalItemsToShow, bgTexture);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      eng->renderInventory->drawUseMode(browser, generalItemsToShow, bgTexture);
    }
    break;
    case menuAction_selected: {
      const int INV_ELEM = generalItemsToShow.at(browser.getPos().y);
      activateDefault(INV_ELEM);
      eng->renderer->drawMapAndInterface();
      return true;
    }
    break;
    case menuAction_selectedWithShift: {
      const int SLOTS_SIZE = eng->player->getInventory()->getSlots()->size();
      if(runDropScreen(SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
        eng->renderer->drawMapAndInterface();
        return true;
      }
    }
    break;
    case menuAction_canceled: {
      eng->renderer->drawMapAndInterface();
      return false;
    }
    break;
    }
  }
}

bool InventoryHandler::runDropScreen(const int GLOBAL_ELEMENT_NR) {
  tracer << "InventoryHandler::runDropScreen()" << endl;
  Inventory* const inv = eng->player->getInventory();
  Item* const item = inv->getItemInElement(GLOBAL_ELEMENT_NR);
  const ItemDefinition& def = item->getDef();

  eng->log->clearLog();
  if(def.isStackable && item->numberOfItems > 1) {
    tracer << "InventoryHandler: item is stackable and more than one" << endl;
    eng->renderer->drawMapAndInterface(false);
    const string nrStr = "1-" + intToString(item->numberOfItems);
    eng->renderer->drawText("Drop how many (" + nrStr + ")?:      [enter] drop  [space/esc] cancel", renderArea_screen, 1, 1, clrWhiteHigh);
    eng->renderer->updateWindow();
    const int NR_TO_DROP = eng->query->number(coord(20 + nrStr.size(), 1), clrWhiteHigh, 0, 3, item->numberOfItems, false);
    if(NR_TO_DROP <= 0) {
      tracer << "InventoryHandler: nr to drop <= 0, nothing to be done" << endl;
      return false;
    } else {
      eng->itemDrop->dropItemFromInventory(eng->player, GLOBAL_ELEMENT_NR, NR_TO_DROP);
      return true;
    }
  } else {
    tracer << "InventoryHandler: item not stackable, or only one item" << endl;
    eng->itemDrop->dropItemFromInventory(eng->player, GLOBAL_ELEMENT_NR);
    return true;
  }
  return false;
}

bool InventoryHandler::runEquipScreen(InventorySlot* const slotToEquip) {
  eng->renderer->drawMapAndInterface();
  sf::Texture bgTexture = eng->renderer->getScreenTextureCopy();

  eng->player->getInventory()->sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsEquip(slotToEquip->devName);

  MenuBrowser browser(generalItemsToShow.size(), 0);
  eng->renderInventory->drawEquipMode(browser, slotToEquip->devName, generalItemsToShow, bgTexture);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      eng->renderInventory->drawEquipMode(browser, slotToEquip->devName, generalItemsToShow, bgTexture);
    }
    break;
    case menuAction_selected: {
      const int INV_ELEM = generalItemsToShow.at(browser.getPos().y);
      eng->player->getInventory()->equipGeneralItemAndPossiblyEndTurn(INV_ELEM, slotToEquip->devName, eng);
      return true;
    }
    break;
    case menuAction_selectedWithShift: {
      const int SLOTS_SIZE = eng->player->getInventory()->getSlots()->size();
      if(runDropScreen(SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
        eng->renderer->drawMapAndInterface();
        return true;
      }
    }
    break;
    case menuAction_canceled: {
      return false;
    }
    break;
    }
  }
}

bool InventoryHandler::runBrowseInventoryMode() {
  eng->renderer->drawMapAndInterface();
  sf::Texture bgTexture = eng->renderer->getScreenTextureCopy();

  eng->player->getInventory()->sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsShowAll();
  MenuBrowser browser(generalItemsToShow.size(), 0);
  eng->renderInventory->drawBrowseInventoryMode(browser, generalItemsToShow, bgTexture);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      eng->renderInventory->drawBrowseInventoryMode(browser, generalItemsToShow, bgTexture);
    }
    break;
    case menuAction_selected: {
    }
    break;
    case menuAction_selectedWithShift: {
      const int SLOTS_SIZE = eng->player->getInventory()->getSlots()->size();
      if(runDropScreen(SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
        eng->renderer->drawMapAndInterface();
        return true;
      }
    }
    break;
    case menuAction_canceled: {
      return false;
    }
    break;
    }
  }
  return false;
}

void InventoryHandler::swapItems(Item** item1, Item** item2) {
  Item* buffer = *item1;
  *item1 = *item2;
  *item2 = buffer;
}


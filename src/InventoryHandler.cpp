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

InventoryHandler::InventoryHandler(Engine* engine) :
  screenToOpenAfterDrop(endOfInventoryScreens),
  equipSlotToOpenAfterDrop(NULL), eng(engine) {

}

void InventoryHandler::activateDefault(
  const unsigned int GENERAL_ITEMS_ELEMENT) {

  Inventory* const playerInv = eng->player->getInventory();
  Item* item = playerInv->getGeneral()->at(GENERAL_ITEMS_ELEMENT);
  if(item->activateDefault(eng->player)) {
    playerInv->decreaseItemInGeneral(GENERAL_ITEMS_ELEMENT);
  }
}

void InventoryHandler::filterPlayerGeneralSlotButtonsEquip(
  const SlotTypes_t slotToEquip) {

  vector<Item*>* general = eng->player->getInventory()->getGeneral();
  generalItemsToShow.resize(0);

  for(unsigned int i = 0; i < general->size(); i++) {
    const Item* const item = general->at(i);
    const ItemData& data = item->getData();

    switch(slotToEquip) {
      case slot_armorBody: {
        if(data.isArmor) {
          generalItemsToShow.push_back(i);
        }
      }
      break;
      case slot_wielded: {
        if(data.isMeleeWeapon || data.isRangedWeapon) {
          generalItemsToShow.push_back(i);
        }
      }
      break;
      case slot_wieldedAlt: {
        if(data.isMeleeWeapon || data.isRangedWeapon) {
          generalItemsToShow.push_back(i);
        }
      }
      break;
      case slot_missiles: {
        if(data.isMissileWeapon) {
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
        if(
          label ==
          general->at(groups.at(ii).front())->getDefaultActivationLabel()) {
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
  screenToOpenAfterDrop = endOfInventoryScreens;
  eng->renderer->drawMapAndInterface();

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
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;
  eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons);
      }
      break;
      case menuAction_selectedWithShift: {
        if(runDropScreen(browser.getPos().y)) {
          screenToOpenAfterDrop = inventoryScreen_slots;
          browserPosToSetAfterDrop = browser.getPos().y;
          return;
        }
        eng->renderInventory->drawBrowseSlotsMode(
          browser, equipmentSlotButtons);
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
              eng->renderInventory->drawBrowseSlotsMode(
                browser, equipmentSlotButtons);
            }
          } else {
            Item* const item = slot->item;

            const string itemName =
              eng->itemDataHandler->getItemRef(*item, itemRef_plain);

            inv->moveItemToGeneral(slot);

            if(slot->id == slot_armorBody) {
              screenToOpenAfterDrop = inventoryScreen_slots;
              browserPosToSetAfterDrop = browser.getPos().y;

              eng->log->addMsg(
                "I take off my " + itemName + ".", clrWhite, true, true);
              item->onTakeOff();
              eng->renderer->drawMapAndInterface();
              eng->gameTime->endTurnOfCurrentActor();
              return;
            } else {
              eng->renderInventory->drawBrowseSlotsMode(
                browser, equipmentSlotButtons);
            }

          }
        } else {
          runBrowseInventoryMode();
          return;
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
  screenToOpenAfterDrop = endOfInventoryScreens;
  eng->renderer->drawMapAndInterface();

  eng->player->getInventory()->sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsUsable();
  MenuBrowser browser(generalItemsToShow.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;
  eng->renderInventory->drawUseMode(browser, generalItemsToShow);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        eng->renderInventory->drawUseMode(browser, generalItemsToShow);
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
          screenToOpenAfterDrop = inventoryScreen_use;
          browserPosToSetAfterDrop = browser.getPos().y;
          return true;
        }
        eng->renderInventory->drawUseMode(browser, generalItemsToShow);
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
  trace << "InventoryHandler::runDropScreen()" << endl;
  Inventory* const inv = eng->player->getInventory();
  Item* const item = inv->getItemInElement(GLOBAL_ELEMENT_NR);
  const ItemData& data = item->getData();

  eng->log->clearLog();
  if(data.isStackable && item->nrItems > 1) {
    trace << "InventoryHandler: item is stackable and more than one" << endl;
    eng->renderer->drawMapAndInterface(false);
    const string nrStr = "1-" + toString(item->nrItems);
    const string dropStr = "Drop how many (" + nrStr + ")?:      " +
                           "| enter to drop | space/esc to cancel";
    eng->renderer->drawText(dropStr, panel_screen, Pos(1, 1), clrWhiteHigh);
    eng->renderer->updateScreen();
    const int NR_TO_DROP = eng->query->number(
                             Pos(20 + nrStr.size(), 1),
                             clrWhiteHigh, 0, 3, item->nrItems, false);
    if(NR_TO_DROP <= 0) {
      trace << "InventoryHandler: nr to drop <= 0, nothing to be done" << endl;
      return false;
    } else {
      eng->itemDrop->dropItemFromInventory(
        eng->player, GLOBAL_ELEMENT_NR, NR_TO_DROP);
      return true;
    }
  } else {
    trace << "InventoryHandler: item not stackable, or only one item" << endl;
    eng->itemDrop->dropItemFromInventory(eng->player, GLOBAL_ELEMENT_NR);
    return true;
  }
  return false;
}

bool InventoryHandler::runEquipScreen(InventorySlot* const slotToEquip) {
  screenToOpenAfterDrop = endOfInventoryScreens;
  equipSlotToOpenAfterDrop = slotToEquip;
  eng->renderer->drawMapAndInterface();

  eng->player->getInventory()->sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsEquip(slotToEquip->id);

  MenuBrowser browser(generalItemsToShow.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;
  eng->renderInventory->drawEquipMode(
    browser, slotToEquip->id, generalItemsToShow);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        eng->renderInventory->drawEquipMode(
          browser, slotToEquip->id, generalItemsToShow);
      }
      break;
      case menuAction_selected: {
        const int INV_ELEM = generalItemsToShow.at(browser.getPos().y);
        eng->player->getInventory()->equipGeneralItemAndPossiblyEndTurn(
          INV_ELEM, slotToEquip->id, eng);
        if(slotToEquip->id == slot_armorBody) {
          slotToEquip->item->onWear();
        }
        return true;
      }
      break;
      case menuAction_selectedWithShift: {
        const int SLOTS_SIZE = eng->player->getInventory()->getSlots()->size();
        if(runDropScreen(SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
          screenToOpenAfterDrop = inventoryScreen_equip;
          browserPosToSetAfterDrop = browser.getPos().y;
          return true;
        }
        eng->renderInventory->drawEquipMode(
          browser, slotToEquip->id, generalItemsToShow);
      }
      break;
      case menuAction_canceled: {
        return false;
      }
      break;
    }
  }
}

void InventoryHandler::runBrowseInventoryMode() {
  screenToOpenAfterDrop = endOfInventoryScreens;
  eng->renderer->drawMapAndInterface();

  eng->player->getInventory()->sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsShowAll();
  MenuBrowser browser(generalItemsToShow.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;
  eng->renderInventory->drawBrowseInventoryMode(browser, generalItemsToShow);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        eng->renderInventory->drawBrowseInventoryMode(
          browser, generalItemsToShow);
      }
      break;
      case menuAction_selected: {
      }
      break;
      case menuAction_selectedWithShift: {
        const int SLOTS_SIZE = eng->player->getInventory()->getSlots()->size();
        if(runDropScreen(SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
          screenToOpenAfterDrop = inventoryScreen_backpack;
          browserPosToSetAfterDrop = browser.getPos().y;
          return;
        }
        eng->renderInventory->drawBrowseInventoryMode(browser, generalItemsToShow);
      }
      break;
      case menuAction_canceled: {
        return;
      }
      break;
    }
  }
}

void InventoryHandler::swapItems(Item** item1, Item** item2) {
  Item* buffer = *item1;
  *item1 = *item2;
  *item2 = buffer;
}


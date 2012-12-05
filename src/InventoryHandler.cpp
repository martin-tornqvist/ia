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
  Inventory* const inv = eng->player->getInventory();
  vector<InventorySlot>* invSlots = inv->getSlots();

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
  eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons);
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
            eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons);
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
            eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons);
          }
        }
      } else {
        runBrowseInventoryMode();
        eng->renderInventory->drawBrowseSlotsMode(browser, equipmentSlotButtons);
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
  filterPlayerGeneralSlotButtonsUsable();
  MenuBrowser browser(generalItemsToShow.size(), 0);
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
    case menuAction_canceled: {
      eng->renderer->drawMapAndInterface();
      return false;
    }
    break;
    }
  }
}

void InventoryHandler::runDropScreen(Item* const itemToDrop) {

  eng->renderInventory->drawDropMode(itemToDrop);
  while(true) {
  }
}

bool InventoryHandler::runEquipScreen(InventorySlot* const slotToEquip) {
  filterPlayerGeneralSlotButtonsEquip(slotToEquip->devName);

  MenuBrowser browser(generalItemsToShow.size(), 0);
  eng->renderInventory->drawEquipMode(browser, slotToEquip->devName, generalItemsToShow);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      eng->renderInventory->drawEquipMode(browser, slotToEquip->devName, generalItemsToShow);
    }
    break;
    case menuAction_selected: {
      const int INV_ELEM = generalItemsToShow.at(browser.getPos().y);
      eng->player->getInventory()->equipGeneralItemAndPossiblyEndTurn(INV_ELEM, slotToEquip->devName, eng);
      return true;
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
  filterPlayerGeneralSlotButtonsShowAll();
  MenuBrowser browser(generalItemsToShow.size(), 0);
  eng->renderInventory->drawBrowseInventoryMode(browser, generalItemsToShow);

  while(true) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
    case menuAction_browsed: {
      eng->renderInventory->drawBrowseInventoryMode(browser, generalItemsToShow);
    }
    break;
    case menuAction_selected: {
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

//void InventoryHandler::runPlayerInventory(InventoryPurpose_t purpose) {
//  updatePlayerGeneralSlotButtons(purpose);
//  const unsigned int NR_OF_SLOTS = static_cast<int>(playerSlotButtons.size());
//
//  bool done = false;
//
//  const int NR_ITEMS_FIRST = purpose == inventoryPurpose_selectDrop ? NR_OF_SLOTS : generalItemsToShow.size();
//  const int NR_ITEMS_SECOND = purpose == inventoryPurpose_selectDrop ? generalItemsToShow.size() : 0;
//  MenuBrowser browser(NR_ITEMS_FIRST, NR_ITEMS_SECOND);
//
//  const bool DRAW_BROWSER = purpose != inventoryPurpose_look;
//
//  if(generalItemsToShow.size() == 0 && (
//        purpose == inventoryPurpose_use ||
//        purpose == inventoryPurpose_quaff ||
//        purpose == inventoryPurpose_eat ||
//        purpose == inventoryPurpose_wieldWear ||
//        purpose == inventoryPurpose_wieldAlt ||
//        purpose == inventoryPurpose_readyExplosive ||
//        purpose == inventoryPurpose_missileSelect)) {
//    done = true;
//
//    if(purpose == inventoryPurpose_use)
//      eng->log->addMessage("I have nothing to use.");
//    if(purpose == inventoryPurpose_quaff)
//      eng->log->addMessage("I have nothing to drink.");
//    if(purpose == inventoryPurpose_eat)
//      eng->log->addMessage("I have nothing to eat.");
//    if(purpose == inventoryPurpose_wieldWear || purpose == inventoryPurpose_wieldAlt)
//      eng->log->addMessage("I have nothing to wield or wear.");
//    if(purpose == inventoryPurpose_readyExplosive)
//      eng->log->addMessage("I have no explosives.");
//    if(purpose == inventoryPurpose_missileSelect)
//      eng->log->addMessage("I have nothing to use as a thrown weapon.");
//  } else {
//    eng->renderInventory->draw(purpose, browser, true, DRAW_BROWSER);
//  }
//
//  while(done == false) {
//    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
//
//    if(action == menuAction_browsed) {
//      eng->renderInventory->draw(purpose, browser, false, DRAW_BROWSER);
//    }
//
//    if(action == menuAction_canceled) {
//      eng->log->clearLog();
//      eng->renderer->drawMapAndInterface();
//      done = true;
//      return;
//    }
//
//    if(action == menuAction_selected) {
//      switch(purpose) {
//      case inventoryPurpose_selectDrop: {
//        //Equipment slot?
//        if(browser.getPos().x == 0) {
//          Item* item = eng->player->getInventory()->getSlots()->at(browser.getPos().y).item;
//          if(item != NULL) {
//            eng->log->clearLog();
//            eng->itemDrop->dropItemFromInventory(eng->player, browser.getPos().y);
//            eng->renderer->drawMapAndInterface();
//            return;
//          }
//        }
//        //General slot?
//        if(browser.getPos().x == 1) {
//          const unsigned int DROP_ELEMENT = NR_OF_SLOTS + browser.getPos().y;
//          eng->log->clearLog();
//          eng->itemDrop->dropItemFromInventory(eng->player, DROP_ELEMENT);
//          eng->renderer->drawMapAndInterface();
//          return;
//        }
//      }
//      break;
//      case inventoryPurpose_eat:
//      case inventoryPurpose_use:
//      case inventoryPurpose_quaff:
//      case inventoryPurpose_readyExplosive: {
//        eng->log->clearLog();
//        activateItem(purpose, generalItemsToShow.at(browser.getPos().y));
//        return;
//      }
//      break;
//      case inventoryPurpose_wieldWear:
//      case inventoryPurpose_missileSelect: {
//        eng->log->clearLog();
//        eng->player->getInventory()->equipGeneralItemAndPossiblyEndTurn(generalItemsToShow.at(browser.getPos().y), purpose, eng);
//        return;
//      }
//      break;
//      case inventoryPurpose_wieldAlt: {
//        eng->log->clearLog();
//        eng->player->getInventory()->equipGeneralItemToAltAndPossiblyEndTurn(generalItemsToShow.at(browser.getPos().y), eng);
//        return;
//      }
//      break;
//      default: {
//      }
//      break;
//      }
//    }
//  }
//
//  eng->renderer->drawMapAndInterface();
//}



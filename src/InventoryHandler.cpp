#include "InventoryHandler.h"

#include "Engine.h"

#include "ItemScroll.h"
#include "ItemExplosive.h"
#include "ActorPlayer.h"
#include "ItemPotion.h"
#include "MenuBrowser.h"
#include "Log.h"
#include "RenderInventory.h"
#include "MenuInputHandler.h"
#include "Renderer.h"
#include "ItemDrop.h"
#include "Query.h"
#include "ItemFactory.h"
#include "GameTime.h"
#include "Audio.h"

InventoryHandler::InventoryHandler(Engine& engine) :
  screenToOpenAfterDrop(endOfInventoryScreens),
  equipSlotToOpenAfterDrop(NULL),
  browserPosToSetAfterDrop(0),
  eng(engine) {}

void InventoryHandler::activateDefault(
  const unsigned int GENERAL_ITEMS_ELEMENT) {

  Inventory& playerInv = eng.player->getInv();
  Item* item = playerInv.getGeneral().at(GENERAL_ITEMS_ELEMENT);
  if(item->activateDefault(eng.player)) {
    playerInv.decrItemInGeneral(GENERAL_ITEMS_ELEMENT);
  }
}

void InventoryHandler::filterPlayerGeneralSlotButtonsEquip(
  const SlotId slotToEquip) {

  vector<Item*>& general = eng.player->getInv().getGeneral();
  generalItemsToShow.resize(0);

  for(unsigned int i = 0; i < general.size(); i++) {
    const Item* const item = general.at(i);
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
  vector<Item*>& general = eng.player->getInv().getGeneral();

  vector< vector<unsigned int> > groups;

  for(unsigned int i = 0; i < general.size(); i++) {
    const Item* const item = general.at(i);
    const string& label = item->getDefaultActivationLabel();
    if(label.empty() == false) {
      bool isExistingGroupFound = false;
      for(vector<unsigned int>& group : groups) {
        if(label == general.at(group.front())->getDefaultActivationLabel()) {
          group.push_back(i);
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

  for(vector<unsigned int>& group : groups) {
    for(unsigned int itemIndex : group) {
      generalItemsToShow.push_back(itemIndex);
    }
  }
}

void InventoryHandler::filterPlayerGeneralSlotButtonsShowAll() {
  vector<Item*>& general = eng.player->getInv().getGeneral();
  generalItemsToShow.resize(0);
  const int NR_GEN = general.size();
  for(int i = 0; i < NR_GEN; i++) {generalItemsToShow.push_back(i);}
}

void InventoryHandler::runSlotsScreen() {
  screenToOpenAfterDrop = endOfInventoryScreens;
  Renderer::drawMapAndInterface();

  Inventory& inv                = eng.player->getInv();
  vector<InventorySlot>& slots  = inv.getSlots();

  inv.sortGeneralInventory(eng);

  MenuBrowser browser(slots.size() + 1, 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;
  RenderInventory::drawBrowseSlots(browser, eng);

  while(true) {
    const MenuAction action = eng.menuInputHandler->getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawBrowseSlots(browser, eng);
      } break;

      case MenuAction::selectedShift: {
        if(runDropScreen(browser.getPos().y)) {
          screenToOpenAfterDrop = inventoryScreen_slots;
          browserPosToSetAfterDrop = browser.getPos().y;
          return;
        }
        RenderInventory::drawBrowseSlots(browser, eng);
      } break;

      case MenuAction::selected: {
        const int elementSelected = browser.getElement();
        if(elementSelected < int(slots.size())) {
          InventorySlot& slot = slots.at(elementSelected);
          if(slot.item == NULL) {
            if(runEquipScreen(&slot)) {
              Renderer::drawMapAndInterface();
              return;
            } else {
              RenderInventory::drawBrowseSlots(browser, eng);
            }
          } else {
            Item* const item = slot.item;

            const string itemName =
              eng.itemDataHandler->getItemRef(*item, itemRef_plain);

            inv.moveItemToGeneral(&slot);

            if(slot.id == slot_armorBody) {
              screenToOpenAfterDrop = inventoryScreen_slots;
              browserPosToSetAfterDrop = browser.getPos().y;

              eng.log->addMsg(
                "I take off my " + itemName + ".", clrWhite, true, true);
              item->onTakeOff();
              Renderer::drawMapAndInterface();
              eng.gameTime->actorDidAct();
              return;
            } else {
              RenderInventory::drawBrowseSlots(browser, eng);
            }

          }
        } else {
          runBrowseInventory();
          return;
        }
      } break;

      case MenuAction::esc:
      case MenuAction::space: {
        Renderer::drawMapAndInterface();
        return;
      } break;
    }
  }
}

bool InventoryHandler::runUseScreen() {
  screenToOpenAfterDrop = endOfInventoryScreens;
  Renderer::drawMapAndInterface();

  eng.player->getInv().sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsUsable();
  MenuBrowser browser(generalItemsToShow.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;

  Audio::play(SfxId::backpack);

  RenderInventory::drawUse(browser, generalItemsToShow, eng);

  while(true) {
    const MenuAction action = eng.menuInputHandler->getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawUse(browser, generalItemsToShow, eng);
      } break;

      case MenuAction::selected: {
        const int INV_ELEM = generalItemsToShow.at(browser.getPos().y);
        activateDefault(INV_ELEM);
        Renderer::drawMapAndInterface();
        return true;
      } break;

      case MenuAction::selectedShift: {
        const int SLOTS_SIZE = eng.player->getInv().getSlots().size();
        if(
          runDropScreen(
            SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
          screenToOpenAfterDrop = inventoryScreen_use;
          browserPosToSetAfterDrop = browser.getPos().y;
          return true;
        }
        RenderInventory::drawUse(browser, generalItemsToShow, eng);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {
        Renderer::drawMapAndInterface();
        return false;
      } break;
    }
  }
}

bool InventoryHandler::runDropScreen(const int GLOBAL_ELEMENT_NR) {
  trace << "InventoryHandler::runDropScreen()" << endl;
  Inventory& inv = eng.player->getInv();
  Item* const item = inv.getItemInElement(GLOBAL_ELEMENT_NR);
  const ItemData& data = item->getData();

  eng.log->clearLog();
  if(data.isStackable && item->nrItems > 1) {
    trace << "InventoryHandler: item is stackable and more than one" << endl;
    Renderer::drawMapAndInterface(false);
    const string nrStr = "1-" + toString(item->nrItems);
    const string dropStr = "Drop how many (" + nrStr + ")?:      " +
                           "| enter to drop" + cancelInfoStr;
    Renderer::drawText(dropStr, Panel::screen, Pos(0, 0), clrWhiteHigh);
    Renderer::updateScreen();
    const Pos nrQueryPos(20 + nrStr.size(), 0);
    const int NR_TO_DROP = eng.query->number(nrQueryPos, clrWhiteHigh, 0, 3,
                           item->nrItems, false);
    if(NR_TO_DROP <= 0) {
      trace << "InventoryHandler: nr to drop <= 0, nothing to be done" << endl;
      return false;
    } else {
      eng.itemDrop->dropItemFromInventory(eng.player, GLOBAL_ELEMENT_NR,
                                          NR_TO_DROP);
      return true;
    }
  } else {
    trace << "InventoryHandler: item not stackable, or only one item" << endl;
    eng.itemDrop->dropItemFromInventory(eng.player, GLOBAL_ELEMENT_NR);
    return true;
  }
  return false;
}

bool InventoryHandler::runEquipScreen(InventorySlot* const slotToEquip) {
  screenToOpenAfterDrop = endOfInventoryScreens;
  equipSlotToOpenAfterDrop = slotToEquip;
  Renderer::drawMapAndInterface();

  eng.player->getInv().sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsEquip(slotToEquip->id);

  MenuBrowser browser(generalItemsToShow.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;

  Audio::play(SfxId::backpack);

  RenderInventory::drawEquip(
    browser, slotToEquip->id, generalItemsToShow, eng);

  while(true) {
    const MenuAction action = eng.menuInputHandler->getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawEquip(
          browser, slotToEquip->id, generalItemsToShow, eng);
      } break;

      case MenuAction::selected: {
        const int INV_ELEM = generalItemsToShow.at(browser.getPos().y);
        eng.player->getInv().equipGeneralItemAndPossiblyEndTurn(
          INV_ELEM, slotToEquip->id, eng);
        if(slotToEquip->id == slot_armorBody) {
          slotToEquip->item->onWear();
        }
        return true;
      } break;

      case MenuAction::selectedShift: {
        const int SLOTS_SIZE = eng.player->getInv().getSlots().size();
        if(
          runDropScreen(
            SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
          screenToOpenAfterDrop = inventoryScreen_equip;
          browserPosToSetAfterDrop = browser.getPos().y;
          return true;
        }
        RenderInventory::drawEquip(
          browser, slotToEquip->id, generalItemsToShow, eng);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {return false;} break;
    }
  }
}

void InventoryHandler::runBrowseInventory() {
  screenToOpenAfterDrop = endOfInventoryScreens;
  Renderer::drawMapAndInterface();

  eng.player->getInv().sortGeneralInventory(eng);

  filterPlayerGeneralSlotButtonsShowAll();
  MenuBrowser browser(generalItemsToShow.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;

  Audio::play(SfxId::backpack);

  RenderInventory::drawBrowseInventory(browser, generalItemsToShow, eng);

  while(true) {
    const MenuAction action = eng.menuInputHandler->getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawBrowseInventory(browser, generalItemsToShow, eng);
      } break;

      case MenuAction::selected: {} break;

      case MenuAction::selectedShift: {
        const int SLOTS_SIZE = eng.player->getInv().getSlots().size();
        if(
          runDropScreen(
            SLOTS_SIZE + generalItemsToShow.at(browser.getPos().y))) {
          screenToOpenAfterDrop = inventoryScreen_backpack;
          browserPosToSetAfterDrop = browser.getPos().y;
          return;
        }
        RenderInventory::drawBrowseInventory(browser, generalItemsToShow, eng);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {
        return;
      } break;
    }
  }
}

void InventoryHandler::swapItems(Item** item1, Item** item2) {
  Item* buffer = *item1;
  *item1 = *item2;
  *item2 = buffer;
}


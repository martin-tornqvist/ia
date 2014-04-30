#include "InventoryHandling.h"

#include "ItemScroll.h"
#include "ItemExplosive.h"
#include "ActorPlayer.h"
#include "ItemPotion.h"
#include "MenuBrowser.h"
#include "Log.h"
#include "RenderInventory.h"
#include "MenuInputHandling.h"
#include "Renderer.h"
#include "ItemDrop.h"
#include "Query.h"
#include "ItemFactory.h"
#include "GameTime.h"
#include "Audio.h"

namespace InvHandling {

InvScrId  screenToOpenAfterDrop     = InvScrId::endOfInventoryScreens;
InvSlot*  equipSlotToOpenAfterDrop  = NULL;
int       browserPosToSetAfterDrop  = 0;

namespace {

//The values in this vector refer to general inventory elements
vector<unsigned int> generalItemsToShow_;

bool runDropScreen(const int GLOBAL_ELEMENT_NR) {
  trace << "InventoryHandler::runDropScreen()" << endl;
  Inventory& inv = Map::player->getInv();
  Item* const item = inv.getItemInElement(GLOBAL_ELEMENT_NR);
  const ItemData& data = item->getData();

  Log::clearLog();
  if(data.isStackable && item->nrItems > 1) {
    trace << "InventoryHandler: item is stackable and more than one" << endl;
    Renderer::drawMapAndInterface(false);
    const string nrStr = "1-" + toStr(item->nrItems);
    const string dropStr = "Drop how many (" + nrStr + ")?:      " +
                           "| enter to drop" + cancelInfoStr;
    Renderer::drawText(dropStr, Panel::screen, Pos(0, 0), clrWhiteHigh);
    Renderer::updateScreen();
    const Pos nrQueryPos(20 + nrStr.size(), 0);
    const int NR_TO_DROP = Query::number(nrQueryPos, clrWhiteHigh, 0, 3,
                           item->nrItems, false);
    if(NR_TO_DROP <= 0) {
      trace << "InventoryHandler: nr to drop <= 0, nothing to be done" << endl;
      return false;
    } else {
      ItemDrop::dropItemFromInv(Map::player, GLOBAL_ELEMENT_NR,
                                NR_TO_DROP);
      return true;
    }
  } else {
    trace << "InventoryHandler: item not stackable, or only one item" << endl;
    ItemDrop::dropItemFromInv(Map::player, GLOBAL_ELEMENT_NR);
    return true;
  }
  return false;
}

void filterPlayerGeneralSlotButtonsEquip(
  const SlotId slotToEquip) {

  vector<Item*>& general = Map::player->getInv().getGeneral();
  generalItemsToShow_.resize(0);

  for(unsigned int i = 0; i < general.size(); i++) {
    const Item* const item = general.at(i);
    const ItemData& data = item->getData();

    switch(slotToEquip) {
      case SlotId::armorBody: {
        if(data.isArmor) {
          generalItemsToShow_.push_back(i);
        }
      }
      break;
      case SlotId::wielded: {
        if(data.isMeleeWeapon || data.isRangedWeapon) {
          generalItemsToShow_.push_back(i);
        }
      }
      break;
      case SlotId::wieldedAlt: {
        if(data.isMeleeWeapon || data.isRangedWeapon) {
          generalItemsToShow_.push_back(i);
        }
      }
      break;
      case SlotId::missiles: {
        if(data.isMissileWeapon) {
          generalItemsToShow_.push_back(i);
        }
      }
      break;
    }
  }
}

void filterPlayerGeneralSlotButtonsUsable() {
  vector<Item*>& general = Map::player->getInv().getGeneral();

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

  generalItemsToShow_.resize(0);

  for(vector<unsigned int>& group : groups) {
    for(unsigned int itemIndex : group) {
      generalItemsToShow_.push_back(itemIndex);
    }
  }
}

void filterPlayerGeneralSlotButtonsShowAll() {
  vector<Item*>& general = Map::player->getInv().getGeneral();
  generalItemsToShow_.resize(0);
  const int NR_GEN = general.size();
  for(int i = 0; i < NR_GEN; i++) {generalItemsToShow_.push_back(i);}
}

void swapItems(Item** item1, Item** item2) {
  Item* buffer = *item1;
  *item1 = *item2;
  *item2 = buffer;
}

} //namespace

void init() {
  screenToOpenAfterDrop = InvScrId::endOfInventoryScreens;
  equipSlotToOpenAfterDrop = NULL;
  browserPosToSetAfterDrop = 0;
}

void activateDefault(
  const unsigned int GENERAL_ITEMS_ELEMENT) {

  Inventory& playerInv = Map::player->getInv();
  Item* item = playerInv.getGeneral().at(GENERAL_ITEMS_ELEMENT);
  if(item->activateDefault(Map::player) == ConsumeItem::yes) {
    playerInv.decrItemInGeneral(GENERAL_ITEMS_ELEMENT);
  }
}

void runSlotsScreen() {
  screenToOpenAfterDrop = endOfInventoryScreens;
  Renderer::drawMapAndInterface();

  Inventory& inv                = Map::player->getInv();
  vector<InvSlot>& slots  = inv.getSlots();

  inv.sortGeneralInventory();

  MenuBrowser browser(slots.size() + 1, 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;
  RenderInventory::drawBrowseSlots(browser);

  while(true) {
    const MenuAction action = MenuInputHandling::getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawBrowseSlots(browser);
      } break;

      case MenuAction::selectedShift: {
        if(runDropScreen(browser.getPos().y)) {
          screenToOpenAfterDrop = InvScrId::slots;
          browserPosToSetAfterDrop = browser.getPos().y;
          return;
        }
        RenderInventory::drawBrowseSlots(browser);
      } break;

      case MenuAction::selected: {
        const int elementSelected = browser.getElement();
        if(elementSelected < int(slots.size())) {
          InvSlot& slot = slots.at(elementSelected);
          if(slot.item == NULL) {
            if(runEquipScreen(&slot)) {
              Renderer::drawMapAndInterface();
              return;
            } else {
              RenderInventory::drawBrowseSlots(browser);
            }
          } else {
            Item* const item = slot.item;

            const string itemName =
              ItemData::getItemRef(*item, ItemRefType::plain);

            inv.moveToGeneral(&slot);

            if(slot.id == SlotId::armorBody) {
              screenToOpenAfterDrop = InvScrId::slots;
              browserPosToSetAfterDrop = browser.getPos().y;

              Log::addMsg(
                "I take off my " + itemName + ".", clrWhite, true, true);
              item->onTakeOff();
              Renderer::drawMapAndInterface();
              GameTime::actorDidAct();
              return;
            } else {
              RenderInventory::drawBrowseSlots(browser);
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

bool runUseScreen() {
  screenToOpenAfterDrop = endOfInventoryScreens;
  Renderer::drawMapAndInterface();

  Map::player->getInv().sortGeneralInventory();

  filterPlayerGeneralSlotButtonsUsable();
  MenuBrowser browser(generalItemsToShow_.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;

  Audio::play(SfxId::backpack);

  RenderInventory::drawUse(browser, generalItemsToShow_);

  while(true) {
    const MenuAction action = MenuInputHandling::getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawUse(browser, generalItemsToShow_);
      } break;

      case MenuAction::selected: {
        const int INV_ELEM = generalItemsToShow_.at(browser.getPos().y);
        activateDefault(INV_ELEM);
        Renderer::drawMapAndInterface();
        return true;
      } break;

      case MenuAction::selectedShift: {
        const int SLOTS_SIZE = Map::player->getInv().getSlots().size();
        if(
          runDropScreen(
            SLOTS_SIZE + generalItemsToShow_.at(browser.getPos().y))) {
          screenToOpenAfterDrop = InvScrId::use;
          browserPosToSetAfterDrop = browser.getPos().y;
          return true;
        }
        RenderInventory::drawUse(browser, generalItemsToShow_);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {
        Renderer::drawMapAndInterface();
        return false;
      } break;
    }
  }
}

bool runEquipScreen(InvSlot* const slotToEquip) {
  screenToOpenAfterDrop = endOfInventoryScreens;
  equipSlotToOpenAfterDrop = slotToEquip;
  Renderer::drawMapAndInterface();

  Map::player->getInv().sortGeneralInventory();

  filterPlayerGeneralSlotButtonsEquip(slotToEquip->id);

  MenuBrowser browser(generalItemsToShow_.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;

  Audio::play(SfxId::backpack);

  RenderInventory::drawEquip(
    browser, slotToEquip->id, generalItemsToShow_);

  while(true) {
    const MenuAction action = MenuInputHandling::getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawEquip(
          browser, slotToEquip->id, generalItemsToShow_);
      } break;

      case MenuAction::selected: {
        const int INV_ELEM = generalItemsToShow_.at(browser.getPos().y);
        Map::player->getInv().equipGeneralItemAndPossiblyEndTurn(
          INV_ELEM, slotToEquip->id);
        if(slotToEquip->id == SlotId::armorBody) {
          slotToEquip->item->onWear();
        }
        return true;
      } break;

      case MenuAction::selectedShift: {
        const int SLOTS_SIZE = Map::player->getInv().getSlots().size();
        if(
          runDropScreen(
            SLOTS_SIZE + generalItemsToShow_.at(browser.getPos().y))) {
          screenToOpenAfterDrop = InvScrId::equip;
          browserPosToSetAfterDrop = browser.getPos().y;
          return true;
        }
        RenderInventory::drawEquip(
          browser, slotToEquip->id, generalItemsToShow_);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {return false;} break;
    }
  }
}

void runBrowseInventory() {
  screenToOpenAfterDrop = endOfInventoryScreens;
  Renderer::drawMapAndInterface();

  Map::player->getInv().sortGeneralInventory();

  filterPlayerGeneralSlotButtonsShowAll();
  MenuBrowser browser(generalItemsToShow_.size(), 0);
  browser.setY(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = 0;

  Audio::play(SfxId::backpack);

  RenderInventory::drawBrowseInventory(browser, generalItemsToShow_);

  while(true) {
    const MenuAction action = MenuInputHandling::getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawBrowseInventory(browser, generalItemsToShow_);
      } break;

      case MenuAction::selected: {} break;

      case MenuAction::selectedShift: {
        const int SLOTS_SIZE = Map::player->getInv().getSlots().size();
        if(
          runDropScreen(
            SLOTS_SIZE + generalItemsToShow_.at(browser.getPos().y))) {
          screenToOpenAfterDrop = InvScrId::backpack;
          browserPosToSetAfterDrop = browser.getPos().y;
          return;
        }
        RenderInventory::drawBrowseInventory(browser, generalItemsToShow_);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {
        return;
      } break;
    }
  }
}

} //InvHandling

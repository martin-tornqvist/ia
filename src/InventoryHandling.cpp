#include "InventoryHandling.h"

#include <vector>

#include "Init.h"
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
#include "Map.h"

using namespace std;

namespace InvHandling {

InvScrId  screenToOpenAfterDrop     = InvScrId::END;
InvSlot*  equipSlotToOpenAfterDrop  = nullptr;
Pos       browserPosToSetAfterDrop  = Pos(0, 0);

namespace {

//The values in this vector refer to general inventory elements
vector<size_t> generalItemsToShow_;

bool runDropScreen(const InvList invList, const size_t ELEMENT) {
  TRACE_FUNC_BEGIN;

  Inventory& inv  = Map::player->getInv();
  Item* item      = nullptr;

  if(invList == InvList::slots) {
    assert(ELEMENT < inv.slots_.size());
    item = inv.slots_.at(ELEMENT).item;
  } else {
    assert(ELEMENT < inv.general_.size());
    item = inv.general_.at(ELEMENT);
  }

  if(!item) {
    return false;
    TRACE_FUNC_END;
  }

  const ItemDataT& data = item->getData();

  Log::clearLog();
  if(data.isStackable && item->nrItems > 1) {
    TRACE << "Item is stackable and more than one" << endl;
    Renderer::drawMapAndInterface(false);
    const string nrStr = "1-" + toStr(item->nrItems);
    const string dropStr = "Drop how many (" + nrStr + ")?:      " +
                           "[enter] to drop" + cancelInfoStr;
    Renderer::drawText(dropStr, Panel::screen, Pos(0, 0), clrWhiteHigh);
    Renderer::updateScreen();
    const Pos nrQueryPos(20 + nrStr.size(), 0);
    const int NR_TO_DROP = Query::number(nrQueryPos, clrWhiteHigh, 0, 3,
                                         item->nrItems, false);
    if(NR_TO_DROP <= 0) {
      TRACE << "Nr to drop <= 0, nothing to be done" << endl;
      TRACE_FUNC_END;
      return false;
    } else {
      ItemDrop::dropItemFromInv(*Map::player, invList, ELEMENT, NR_TO_DROP);
      TRACE_FUNC_END;
      return true;
    }
  } else {
    TRACE << "Item not stackable, or only one item" << endl;
    ItemDrop::dropItemFromInv(*Map::player, invList, ELEMENT);
    TRACE_FUNC_END;
    return true;
  }
  TRACE_FUNC_END;
  return false;
}

void filterPlayerGeneralEquip(
  const SlotId slotToEquip) {

  auto& general = Map::player->getInv().general_;
  generalItemsToShow_.resize(0);

  for(size_t i = 0; i < general.size(); ++i) {
    const Item* const item = general.at(i);
    const ItemDataT& data = item->getData();

    switch(slotToEquip) {
      case SlotId::wielded: {
        if(data.melee.isMeleeWpn || data.ranged.isRangedWpn) {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::wieldedAlt: {
        if(data.melee.isMeleeWpn || data.ranged.isRangedWpn) {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::thrown: {
        if(data.ranged.isThrowingWpn) {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::body: {
        if(data.isArmor) {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::head: {
        if(data.isHeadwear) {
          generalItemsToShow_.push_back(i);
        }
      } break;
    }
  }
}

void filterPlayerGeneralUsable() {
  auto& general = Map::player->getInv().general_;

  vector< vector<size_t> > groups;

  for(size_t i = 0; i < general.size(); ++i) {
    const Item* const item = general.at(i);
    const string& label = item->getDefaultActivationLabel();
    if(!label.empty()) {
      bool isExistingGroupFound = false;
      for(vector<size_t>& group : groups) {
        if(label == general.at(group.front())->getDefaultActivationLabel()) {
          group.push_back(i);
          isExistingGroupFound = true;
          break;
        }
      }
      if(!isExistingGroupFound) {
        groups.resize(groups.size() + 1);
        groups.back().resize(0);
        groups.back().push_back(i);
      }
    }
  }

  generalItemsToShow_.resize(0);

  for(vector<size_t>& group : groups) {
    for(size_t itemIndex : group) {
      generalItemsToShow_.push_back(itemIndex);
    }
  }
}

void filterPlayerGeneralShowAll() {
  auto& general = Map::player->getInv().general_;
  generalItemsToShow_.resize(0);
  const int NR_GEN = general.size();
  for(int i = 0; i < NR_GEN; ++i) {generalItemsToShow_.push_back(i);}
}

void swapItems(Item** item1, Item** item2) {
  Item* buffer  = *item1;
  *item1        = *item2;
  *item2        = buffer;
}

} //namespace

void init() {
  screenToOpenAfterDrop     = InvScrId::END;
  equipSlotToOpenAfterDrop  = nullptr;
  browserPosToSetAfterDrop  = Pos(0, 0);
}

void activateDefault(const size_t GENERAL_ITEMS_ELEMENT) {
  Inventory& playerInv = Map::player->getInv();
  Item* item = playerInv.general_.at(GENERAL_ITEMS_ELEMENT);
  if(item->activateDefault(Map::player) == ConsumeItem::yes) {
    playerInv.decrItemInGeneral(GENERAL_ITEMS_ELEMENT);
  }
}

void runInvScreen() {
  screenToOpenAfterDrop = InvScrId::END;
  Renderer::drawMapAndInterface();

  Inventory& inv = Map::player->getInv();

  inv.sortGeneralInventory();

  MenuBrowser browser(inv.general_.size(), inv.slots_.size());

  browser.setPos(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = Pos(0, 0);
  RenderInventory::drawBrowseInv(browser);

  while(true) {
    inv.sortGeneralInventory();

    const InvList invList = browser.getPos().x == 0 ?
                            InvList::general : InvList::slots;

    const MenuAction action = MenuInputHandling::getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawBrowseInv(browser);
      } break;

      case MenuAction::selectedShift: {
        if(runDropScreen(invList, browser.getPos().y)) {
          const Pos browserPos      = browser.getPos();
          screenToOpenAfterDrop     = InvScrId::inv;
          browserPosToSetAfterDrop  = browserPos;
          if(browserPos.x == 0 && browserPos.y >= int(inv.general_.size())) {
            if(browserPosToSetAfterDrop.y == 0) {     //No more items in inventory
              browserPosToSetAfterDrop = Pos(1, 0);   //Go to equipped items instead
            } else {
              --browserPosToSetAfterDrop.y;
            }
          }
          return;
        }
        RenderInventory::drawBrowseInv(browser);
      } break;

      case MenuAction::selected: {
        const size_t ELEMENT = browser.getY();
        if(invList == InvList::slots) {
          InvSlot& slot = inv.slots_.at(ELEMENT);
          if(slot.item) {
            Item* const item = slot.item;

            const string itemName = ItemData::getItemRef(*item, ItemRefType::plain);

            inv.moveToGeneral(&slot);

            switch(slot.id) {
              case SlotId::wielded:
              case SlotId::wieldedAlt:
              case SlotId::thrown: {
                RenderInventory::drawBrowseInv(browser);
              } break;
              case SlotId::body: {
                screenToOpenAfterDrop     = InvScrId::inv;
                browserPosToSetAfterDrop  = browser.getPos();

                Log::addMsg("I take off my " + itemName + ".", clrWhite, true, true);
                item->onTakeOff();
                Renderer::drawMapAndInterface();
                GameTime::actorDidAct();
                return;
              }
              case SlotId::head: {
                item->onTakeOff();
                RenderInventory::drawBrowseInv(browser);
              } break;
            }
          } else { //No item in slot
            if(runEquipScreen(slot)) {
              Renderer::drawMapAndInterface();
              return;
            } else {
              RenderInventory::drawBrowseInv(browser);
            }
          }
        } else { //In general inventory
          activateDefault(ELEMENT);
          Renderer::drawMapAndInterface();
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

//bool runUseScreen() {
//  screenToOpenAfterDrop = InvScrId::END;
//  Renderer::drawMapAndInterface();
//
//  Map::player->getInv().sortGeneralInventory();
//
//  filterPlayerGeneralUsable();
//  MenuBrowser browser(generalItemsToShow_.size(), 0);
//  browser.setY(browserPosToSetAfterDrop);
//  browserPosToSetAfterDrop = 0;
//
//  Audio::play(SfxId::backpack);
//
//  RenderInventory::drawUse(browser, generalItemsToShow_);
//
//  while(true) {
//    const MenuAction action = MenuInputHandling::getAction(browser);
//    switch(action) {
//      case MenuAction::browsed: {
//        RenderInventory::drawUse(browser, generalItemsToShow_);
//      } break;
//
//      case MenuAction::selected: {
//        const int INV_ELEM = generalItemsToShow_.at(browser.getPos().y);
//        activateDefault(INV_ELEM);
//        Renderer::drawMapAndInterface();
//        return true;
//      } break;
//
//      case MenuAction::selectedShift: {
//        const int SLOTS_SIZE = Map::player->getInv().slots_.size();
//        if(
//          runDropScreen(
//            SLOTS_SIZE + generalItemsToShow_.at(browser.getPos().y))) {
//          screenToOpenAfterDrop = InvScrId::use;
//          browserPosToSetAfterDrop = browser.getPos().y;
//          return true;
//        }
//        RenderInventory::drawUse(browser, generalItemsToShow_);
//      } break;
//
//      case MenuAction::esc:
//      case MenuAction::space: {
//        Renderer::drawMapAndInterface();
//        return false;
//      } break;
//    }
//  }
//}

bool runEquipScreen(InvSlot& slotToEquip) {
  screenToOpenAfterDrop     = InvScrId::END;
  equipSlotToOpenAfterDrop  = &slotToEquip;
  Renderer::drawMapAndInterface();

  auto& inv = Map::player->getInv();

  inv.sortGeneralInventory();

  filterPlayerGeneralEquip(slotToEquip.id);

  MenuBrowser browser(generalItemsToShow_.size(), 0);
  browser.setPos(browserPosToSetAfterDrop);
  browserPosToSetAfterDrop = Pos(0, 0);

  Audio::play(SfxId::backpack);

  RenderInventory::drawEquip(browser, slotToEquip.id, generalItemsToShow_);

  while(true) {
    const MenuAction action = MenuInputHandling::getAction(browser);
    switch(action) {
      case MenuAction::browsed: {
        RenderInventory::drawEquip(browser, slotToEquip.id, generalItemsToShow_);
      } break;

      case MenuAction::selected: {
        if(!generalItemsToShow_.empty()) {
          const int ELEMENT = generalItemsToShow_.at(browser.getY());
          inv.equipGeneralItemAndPossiblyEndTurn(ELEMENT, slotToEquip.id);
          bool applyWearEffect = false;
          switch(slotToEquip.id) {
            case SlotId::wielded:
            case SlotId::wieldedAlt:
            case SlotId::thrown: {} break;
            case SlotId::body:
            case SlotId::head: applyWearEffect = true; break;
          }
          if(applyWearEffect) {slotToEquip.item->onWear();}
          return true;
        }
      } break;

      case MenuAction::selectedShift: {
        if(runDropScreen(InvList::general, generalItemsToShow_.at(browser.getY()))) {
          screenToOpenAfterDrop     = InvScrId::equip;
          browserPosToSetAfterDrop  = browser.getPos();
          return true;
        }
        RenderInventory::drawEquip(browser, slotToEquip.id, generalItemsToShow_);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {return false;} break;
    }
  }
}

//void runBrowseInventory() {
//  screenToOpenAfterDrop = InvScrId::END;
//  Renderer::drawMapAndInterface();
//
//  Map::player->getInv().sortGeneralInventory();
//
//  filterPlayerGeneralShowAll();
//  MenuBrowser browser(generalItemsToShow_.size(), 0);
//  browser.setY(browserPosToSetAfterDrop);
//  browserPosToSetAfterDrop = 0;
//
//  Audio::play(SfxId::backpack);
//
//  RenderInventory::drawBrowseInventory(browser, generalItemsToShow_);
//
//  while(true) {
//    const MenuAction action = MenuInputHandling::getAction(browser);
//    switch(action) {
//      case MenuAction::browsed: {
//        RenderInventory::drawBrowseInventory(browser, generalItemsToShow_);
//      } break;
//
//      case MenuAction::selected: {} break;
//
//      case MenuAction::selectedShift: {
//        const int SLOTS_SIZE = Map::player->getInv().slots_.size();
//        if(
//          runDropScreen(SLOTS_SIZE + generalItemsToShow_.at(browser.getPos().y))) {
//          screenToOpenAfterDrop     = InvScrId::backpack;
//          browserPosToSetAfterDrop  = browser.getPos().y;
//          return;
//        }
//        RenderInventory::drawBrowseInventory(browser, generalItemsToShow_);
//      } break;
//
//      case MenuAction::esc:
//      case MenuAction::space: {
//        return;
//      } break;
//    }
//  }
//}

} //InvHandling

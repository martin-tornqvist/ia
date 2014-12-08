#include "InventoryHandling.h"

#include "Init.h"

#include <vector>

#include "ItemScroll.h"
#include "ActorPlayer.h"
#include "ItemPotion.h"
#include "MenuBrowser.h"
#include "Log.h"
#include "RenderInventory.h"
#include "MenuInputHandling.h"
#include "Render.h"
#include "ItemDrop.h"
#include "Query.h"
#include "ItemFactory.h"
#include "Audio.h"
#include "Map.h"

using namespace std;

namespace InvHandling
{

InvScrId  screenToOpenAfterDrop     = InvScrId::END;
InvSlot*  equipSlotToOpenAfterDrop  = nullptr;
int       browserIdxToSetAfterDrop  = 0;

namespace
{

//The values in this vector refer to general inventory elements
vector<size_t> generalItemsToShow_;

bool runDropScreen(const InvList invList, const size_t ELEMENT)
{
  TRACE_FUNC_BEGIN;

  Inventory& inv  = Map::player->getInv();
  Item* item      = nullptr;

  if (invList == InvList::slots)
  {
    assert(ELEMENT != int(SlotId::END));
    item = inv.slots_[ELEMENT].item;
  }
  else
  {
    assert(ELEMENT < inv.general_.size());
    item = inv.general_[ELEMENT];
  }

  if (!item)
  {
    return false;
    TRACE_FUNC_END;
  }

  const ItemDataT& data = item->getData();

  Log::clearLog();
  if (data.isStackable && item->nrItems_ > 1)
  {
    TRACE << "Item is stackable and more than one" << endl;
    Render::drawMapAndInterface(false);
    const string nrStr = "1-" + toStr(item->nrItems_);
    string dropStr = "Drop how many (" + nrStr + ")?:";
    Render::drawText(dropStr, Panel::screen, Pos(0, 0), clrWhiteHigh);
    Render::updateScreen();
    const Pos nrQueryPos(dropStr.size() + 1, 0);
    const int MAX_DIGITS = 3;
    const Pos doneInfPos = nrQueryPos + Pos(MAX_DIGITS + 2, 0);
    Render::drawText("[enter] to drop" + cancelInfoStr, Panel::screen, doneInfPos,
                     clrWhiteHigh);
    const int NR_TO_DROP = Query::number(nrQueryPos, clrWhiteHigh, 0, 3,
                                         item->nrItems_, false);
    if (NR_TO_DROP <= 0)
    {
      TRACE << "Nr to drop <= 0, nothing to be done" << endl;
      TRACE_FUNC_END;
      return false;
    }
    else
    {
      ItemDrop::dropItemFromInv(*Map::player, invList, ELEMENT, NR_TO_DROP);
      TRACE_FUNC_END;
      return true;
    }
  }
  else
  {
    TRACE << "Item not stackable, or only one item" << endl;
    ItemDrop::dropItemFromInv(*Map::player, invList, ELEMENT);
    TRACE_FUNC_END;
    return true;
  }
  TRACE_FUNC_END;
  return false;
}

void filterPlayerGeneralEquip(const SlotId slotToEquip)
{
  auto& general = Map::player->getInv().general_;
  generalItemsToShow_.clear();

  for (size_t i = 0; i < general.size(); ++i)
  {
    const Item* const item = general[i];
    const ItemDataT& data = item->getData();

    switch (slotToEquip)
    {
      case SlotId::wielded:
      {
        if (data.melee.isMeleeWpn || data.ranged.isRangedWpn)
        {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::wieldedAlt:
      {
        if (data.melee.isMeleeWpn || data.ranged.isRangedWpn)
        {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::thrown:
      {
        if (data.ranged.isThrowingWpn)
        {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::body:
      {
        if (data.isArmor)
        {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::head:
      {
        if (data.isHeadwear)
        {
          generalItemsToShow_.push_back(i);
        }
      } break;

      case SlotId::END:
      {
        TRACE << "Illegal slot id: " << int(slotToEquip) << endl;
        assert(false);
      } break;
    }
  }
}

void filterPlayerGeneralShowAll()
{
  auto& general = Map::player->getInv().general_;
  generalItemsToShow_.clear();
  const int NR_GEN = general.size();
  for (int i = 0; i < NR_GEN; ++i) {generalItemsToShow_.push_back(i);}
}

void swapItems(Item** item1, Item** item2)
{
  Item* buffer  = *item1;
  *item1        = *item2;
  *item2        = buffer;
}

} //namespace

void init()
{
  screenToOpenAfterDrop     = InvScrId::END;
  equipSlotToOpenAfterDrop  = nullptr;
  browserIdxToSetAfterDrop  = 0;
}

void activate(const size_t GENERAL_ITEMS_ELEMENT)
{
  Inventory& playerInv  = Map::player->getInv();
  Item* item            = playerInv.general_[GENERAL_ITEMS_ELEMENT];

  if (item->activate(Map::player) == ConsumeItem::yes)
  {
    playerInv.decrItemInGeneral(GENERAL_ITEMS_ELEMENT);
  }
}

void runInvScreen()
{
  screenToOpenAfterDrop = InvScrId::END;
  Render::drawMapAndInterface();

  Inventory& inv = Map::player->getInv();

  inv.sortGeneralInventory();

  const int SLOTS_SIZE  = int(SlotId::END);
  const int INV_H       = RenderInventory::INV_H;

  auto getBrowser = [](const Inventory & inventory)
  {
    const int GEN_SIZE        = int(inventory.general_.size());
    const int ELEM_ON_WRAP_UP = GEN_SIZE > INV_H ? (SLOTS_SIZE + INV_H - 2) : -1;
    return MenuBrowser(int(SlotId::END) + GEN_SIZE, 0, ELEM_ON_WRAP_UP);
  };

  MenuBrowser browser = getBrowser(inv);

  browser.setPos(Pos(0, browserIdxToSetAfterDrop));
  browserIdxToSetAfterDrop = 0;
  RenderInventory::drawBrowseInv(browser);

  while (true)
  {
    inv.sortGeneralInventory();

    const InvList invList = browser.getPos().y < int(SlotId::END) ?
                            InvList::slots : InvList::general;

    const MenuAction action = MenuInputHandling::getAction(browser);
    switch (action)
    {
      case MenuAction::browsed:
      {
        RenderInventory::drawBrowseInv(browser);
      } break;

      case MenuAction::selectedShift:
      {
        const int BROWSER_Y = browser.getPos().y;
        const size_t ELEMENT =
          invList == InvList::slots ? BROWSER_Y : (BROWSER_Y - int(SlotId::END));
        if (runDropScreen(invList, ELEMENT))
        {
          browser.setGoodPos();
          browserIdxToSetAfterDrop  = browser.getY();
          screenToOpenAfterDrop     = InvScrId::inv;
          return;
        }
        RenderInventory::drawBrowseInv(browser);
      } break;

      case MenuAction::selected:
      {
        if (invList == InvList::slots)
        {
          const size_t ELEMENT = browser.getY();
          InvSlot& slot = inv.slots_[ELEMENT];
          if (slot.item)
          {
            Log::clearLog();

            Item* const           item            = slot.item;
            const UnequipAllowed  unequipAllowed  = item->onUnequip();

            if (unequipAllowed == UnequipAllowed::yes)
            {
              inv.moveToGeneral(slot);
            }

            switch (SlotId(ELEMENT))
            {
              case SlotId::wielded:
              case SlotId::wieldedAlt:
              case SlotId::thrown:
              case SlotId::head:
                RenderInventory::drawBrowseInv(browser);
                break;

              case SlotId::body:
                screenToOpenAfterDrop     = InvScrId::inv;
                browserIdxToSetAfterDrop  = browser.getY();
                return;

              case SlotId::END:
                TRACE << "Illegal slot id: " << ELEMENT << endl;
                assert(false);
                break;
            }
            //Create a new browser to adjust for changed inventory size
            const Pos p = browser.getPos();
            browser = getBrowser(inv);
            browser.setPos(p);
          }
          else //No item in slot
          {
            if (runEquipScreen(slot))
            {
              Render::drawMapAndInterface();
              return;
            }
            else
            {
              RenderInventory::drawBrowseInv(browser);
            }
          }
        }
        else //In general inventory
        {
          const size_t ELEMENT = browser.getY() - int(SlotId::END);
          activate(ELEMENT);
          Render::drawMapAndInterface();
          return;
        }
      } break;

      case MenuAction::esc:
      case MenuAction::space:
      {
        Render::drawMapAndInterface();
        return;
      } break;
    }
  }
}

bool runEquipScreen(InvSlot& slotToEquip)
{
  screenToOpenAfterDrop     = InvScrId::END;
  equipSlotToOpenAfterDrop  = &slotToEquip;
  Render::drawMapAndInterface();

  auto& inv = Map::player->getInv();

  inv.sortGeneralInventory();

  filterPlayerGeneralEquip(slotToEquip.id);

  MenuBrowser browser(generalItemsToShow_.size(), 0);
  browser.setPos(Pos(0, browserIdxToSetAfterDrop));
  browserIdxToSetAfterDrop = 0;

  Audio::play(SfxId::backpack);

  RenderInventory::drawEquip(browser, slotToEquip.id, generalItemsToShow_);

  while (true)
  {
    const MenuAction action = MenuInputHandling::getAction(browser);
    switch (action)
    {
      case MenuAction::browsed:
      {
        RenderInventory::drawEquip(browser, slotToEquip.id, generalItemsToShow_);
      } break;

      case MenuAction::selected:
      {
        if (!generalItemsToShow_.empty())
        {
          const int ELEMENT = generalItemsToShow_[browser.getY()];
          Render::drawMapAndInterface();
          inv.equipGeneralItemAndEndTurn(ELEMENT, slotToEquip.id);

          slotToEquip.item->onEquip();

          browserIdxToSetAfterDrop  = int(slotToEquip.id);
          screenToOpenAfterDrop     = InvScrId::inv;
          return true;
        }
      } break;

      case MenuAction::selectedShift:
      {
        if (runDropScreen(InvList::general, generalItemsToShow_[browser.getY()]))
        {
          browser.setGoodPos();
          browserIdxToSetAfterDrop  = browser.getY();
          screenToOpenAfterDrop     = InvScrId::equip;
          return true;
        }
        RenderInventory::drawEquip(browser, slotToEquip.id, generalItemsToShow_);
      } break;

      case MenuAction::esc:
      case MenuAction::space: {return false;} break;
    }
  }
}

} //InvHandling

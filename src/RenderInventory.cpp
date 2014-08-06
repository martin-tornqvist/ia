#include "RenderInventory.h"

#include <string>

#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "Map.h"

using namespace std;

const int X_POS_WEIGHT = 62;

namespace {

void drawDots(const int X_PREV, const int W_PREV, const int X_NEW, const int Y,
              const Clr& clr) {

  const int X_DOTS = X_PREV + W_PREV;
  const int W_DOTS = X_NEW - X_DOTS;
  const string dots(W_DOTS, '.');
  Clr realColorDots = clr;
  realColorDots.r /= 3; realColorDots.g /= 3; realColorDots.b /= 3;
  Renderer::drawText(dots, Panel::screen, Pos(X_DOTS, Y), realColorDots);
}

void drawItemSymbol(const Item& item, const Pos& pos) {
  const Clr itemClr = item.getClr();
  if(Config::isTilesMode()) {
    Renderer::drawTile(item.getTile(), Panel::screen, pos, itemClr);
  } else {
    Renderer::drawGlyph(item.getGlyph(), Panel::screen, pos, itemClr);
  }
}

} //Namespace

namespace RenderInventory {

void drawBrowseSlots(const MenuBrowser& browser) {
  Pos pos(0, 0);

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  Renderer::coverArea(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 2));

  string str =
    "Select slot to equip/unequip. [shift+select] to drop [space/esc] to exit";
  Renderer::drawText(str, Panel::screen, pos, clrWhiteHigh);

  const int X_POS_ITEM_NAME = 12;

  pos.y++;

  Inventory& inv = Map::player->getInv();
  vector<InvSlot>& slots = inv.getSlots();

  for(size_t i = 0; i < slots.size(); ++i) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    const InvSlot& slot = slots.at(i);
    str = slot.name;
    pos.x = 0;
    Renderer::drawText(
      str, Panel::screen, pos, IS_CUR_POS ? clrWhiteHigh : clrNosfTealDrk);
    pos.x = X_POS_ITEM_NAME;
    Item* const item = slot.item;
    if(!item) {
      pos.x += 2;
      Renderer::drawText(
        "<empty>", Panel::screen, pos, IS_CUR_POS ? clrWhite : clrNosfTealDrk);
    } else {
      drawItemSymbol(*item, pos);
      pos.x += 2;

      const Clr itemInterfClr =
        IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();

      const ItemDataT& d = item->getData();
      PrimaryAttMode attackMode = PrimaryAttMode::none;
      if(slot.id == SlotId::wielded || slot.id == SlotId::wieldedAlt) {
        attackMode =
          d.primaryAttackMode == PrimaryAttMode::missile ?
          PrimaryAttMode::melee : d.primaryAttackMode;
      } else if(slot.id == SlotId::thrown) {
        attackMode = PrimaryAttMode::missile;
      }

      str = ItemData::getItemInterfaceRef(
              *item, false, attackMode);
      Renderer::drawText(str, Panel::screen, pos, itemInterfClr);
      drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
      Renderer::drawText(
        item->getWeightLabel(), Panel::screen, Pos(X_POS_WEIGHT, pos.y),
        clrGray);
    }

    pos.y++;
  }

  str = "Browse backpack";
  pos.x = 0;
  pos.y += 1;
  const bool IS_CUR_POS = browser.getPos().y == int(slots.size());
  Renderer::drawText(str, Panel::screen, pos,
                     IS_CUR_POS ? clrWhiteHigh : clrNosfTealDrk);

  Renderer::updateScreen();
}

void drawBrowseInventory(const MenuBrowser& browser,
                         const vector<unsigned int>& genInvIndexes) {

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();

  Renderer::coverArea(Panel::screen, Pos(0, 0), Pos(MAP_W, NR_ITEMS + 1));

  string str = NR_ITEMS > 0 ?
               "Browsing backpack. [shift+select] to drop" :
               "I carry no items.";
  str += " [space/esc] to exit";

  Pos pos(0, 0);
  Renderer::drawText(str, Panel::screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory& inv = Map::player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; ++i) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    Item* const item = inv.getGeneral().at(genInvIndexes.at(i));

    const Clr itemInterfClr = IS_CUR_POS ?
                              clrWhiteHigh :
                              item->getInterfaceClr();
    pos.x = 0;

    drawItemSymbol(*item, pos);
    pos.x += 2;

    str = ItemData::getItemInterfaceRef(*item, false);
    Renderer::drawText(str, Panel::screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
    Renderer::drawText(
      item->getWeightLabel(), Panel::screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  Renderer::updateScreen();
}

void drawEquip(const MenuBrowser& browser, const SlotId slotToEquip,
               const vector<unsigned int>& genInvIndexes) {

  Pos pos(0, 0);

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  Renderer::coverArea(
    Panel::screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));

  const bool HAS_ITEM = !genInvIndexes.empty();

  string str = "";
  switch(slotToEquip) {
    case SlotId::wielded: {
      str = HAS_ITEM ? "Wield which item?"            : "I carry no weapon to wield.";
    } break;

    case SlotId::wieldedAlt: {
      str = HAS_ITEM ? "Prepare which weapon?"        : "I carry no weapon to wield.";
    } break;

    case SlotId::thrown: {
      str = HAS_ITEM ? "Use which item as missiles?"  : "I carry no weapon to throw." ;
    } break;

    case SlotId::body: {
      str = HAS_ITEM ? "Wear which armor?"            : "I carry no armor.";
    } break;

    case SlotId::head: {
      str = HAS_ITEM ? "Wear what on head?"           : "I carry no headwear.";
    } break;
  }
  if(HAS_ITEM) {str += " [shift+select] to drop";}
  str += cancelInfoStr;
  Renderer::drawText(str, Panel::screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory& inv = Map::player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; ++i) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    pos.x = 0;

    Item* const item = inv.getGeneral().at(genInvIndexes.at(i));

    drawItemSymbol(*item, pos);
    pos.x += 2;

    const Clr itemInterfClr = IS_CUR_POS ?
                              clrWhiteHigh :
                              item->getInterfaceClr();

    const ItemDataT& d = item->getData();
    PrimaryAttMode attackMode = PrimaryAttMode::none;
    if(slotToEquip == SlotId::wielded || slotToEquip == SlotId::wieldedAlt) {
      attackMode =
        d.primaryAttackMode == PrimaryAttMode::missile ?
        PrimaryAttMode::melee : d.primaryAttackMode;
    } else if(slotToEquip == SlotId::thrown) {
      attackMode = PrimaryAttMode::missile;
    }

    str = ItemData::getItemInterfaceRef(*item, false, attackMode);
    Renderer::drawText(str, Panel::screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
    Renderer::drawText(
      item->getWeightLabel(), Panel::screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  Renderer::updateScreen();
}

void drawUse(const MenuBrowser& browser,
             const vector<unsigned int>& genInvIndexes) {
  Pos pos(0, 0);

//  Renderer::clearScreen();
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  Renderer::coverArea(
    Panel::screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));

  const bool IS_ANY_ITEM_AVAILABLE = !genInvIndexes.empty();
  string str =
    IS_ANY_ITEM_AVAILABLE ? "Use which item? [shift+select] to drop" :
    "I carry no item to use.";
  str += cancelInfoStr;

  Renderer::drawText(str, Panel::screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory& inv = Map::player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; ++i) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    Item* const item = inv.getGeneral().at(genInvIndexes.at(i));

    const Clr itemInterfClr = IS_CUR_POS ?
                              clrWhiteHigh :
                              item->getInterfaceClr();

    //Draw label
    const string& label = item->getDefaultActivationLabel();
    bool isNewLabel = false;
    if(i == 0) {
      isNewLabel = true;
    } else {
      Item* const itemPrev = inv.getGeneral().at(genInvIndexes.at(i - 1));
      const string& labelPrev = itemPrev->getDefaultActivationLabel();
      isNewLabel = label != labelPrev;
    }
    if(isNewLabel) {
      pos.x = 0;
      Renderer::drawText(label, Panel::screen, pos, clrNosfTealDrk);
    }

    pos.x = 11;

    //Draw item symbol
    drawItemSymbol(*item, pos);
    pos.x += 2;

    str = ItemData::getItemRef(*item, ItemRefType::plain, false);
    if(item->nrItems > 1 && item->getData().isStackable) {
      str += " (" + toStr(item->nrItems) + ")";
    }

    Renderer::drawText(str, Panel::screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
    Renderer::drawText(
      item->getWeightLabel(), Panel::screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  Renderer::updateScreen();
}

} //RenderInventory

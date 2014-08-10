#include "RenderInventory.h"

#include <string>

#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "Map.h"

using namespace std;

const int EQ_BOX_X0 = MAP_W_HALF - 1;

namespace {

void drawItemSymbol(const Item& item, const Pos& p) {
  const Clr itemClr = item.getClr();
  if(Config::isTilesMode()) {
    Renderer::drawTile(item.getTile(), Panel::screen, p, itemClr);
  } else {
    Renderer::drawGlyph(item.getGlyph(), Panel::screen, p, itemClr);
  }
}

void drawDetailedItemDescr(const Item* const item, const int BOX_Y0) {
  const Rect descrRect(EQ_BOX_X0, BOX_Y0, MAP_W - 1, CHAR_LINES_OFFSET_H - 1);

  Renderer::drawPopupBox(descrRect, Panel::screen, clrGray, true);

  const Pos p(descrRect.p0 + 1);
  Renderer::drawText("Stuff goes here", Panel::screen, p, clrWhite);
}

} //Namespace

namespace RenderInventory {

void drawBrowseInv(const MenuBrowser& browser) {

  const Pos     browserPos  = browser.getPos();
  const size_t  BROWSER_Y   = browser.getPos().y;
  const auto&   inv         = Map::player->getInv();
  const size_t  NR_SLOTS    = inv.slots_.size();

  Renderer::coverArea(Panel::screen, Pos(0, 0), Pos(MAP_W, CHAR_LINES_OFFSET_H - 1));

  const bool IS_IN_INV = browserPos.x == 0;

  const auto* const curSlotItem = IS_IN_INV ? nullptr : inv.slots_.at(BROWSER_Y).item;

  const string queryEqStr   = curSlotItem ? "unequip" : "equip";
  const string queryBaseStr = "[enter] to " + (IS_IN_INV ? "use item" : queryEqStr);

  const string queryDropStr = IS_IN_INV || curSlotItem ? " [shift+enter] to drop" : "";

  string str                = queryBaseStr + queryDropStr + " [space/esc] to exit";
  Renderer::drawText(str, Panel::screen, Pos(0, 0), clrWhiteHigh);

  const int EQ_BOX_Y1 = inv.slots_.size() + 2;

  const Rect invRect(Pos(0,         1), Pos(EQ_BOX_X0,  CHAR_LINES_OFFSET_H - 1));
  const Rect eqpRect(Pos(EQ_BOX_X0, 1), Pos(MAP_W - 1,  EQ_BOX_Y1));

  for(size_t i = 0; i < NR_SLOTS; ++i) {
    const bool IS_CUR_POS = !IS_IN_INV && BROWSER_Y == i;
    const InvSlot& slot   = inv.slots_.at(i);
    str                   = slot.name;

    Pos p(eqpRect.p0 + Pos(1, i + 1));

    Renderer::drawText(str, Panel::screen, p,
                       IS_CUR_POS ? clrWhiteHigh : clrNosfTealDrk);

    p.x += 9; //Offset to leave room for slot label

    const auto* const item = slot.item;
    if(item) {
      drawItemSymbol(*item, p);
      p.x += 2;

      const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();

      const ItemDataT& d = item->getData();
      PrimaryAttMode attackMode = PrimaryAttMode::none;
      if(slot.id == SlotId::wielded || slot.id == SlotId::wieldedAlt) {
        attackMode = d.primaryAttackMode == PrimaryAttMode::missile ?
                     PrimaryAttMode::melee : d.primaryAttackMode;
      } else if(slot.id == SlotId::thrown) {
        attackMode = PrimaryAttMode::missile;
      }

      str = ItemData::getItemInterfaceRef(*item, false, attackMode);
      Renderer::drawText(str, Panel::screen, p, itemInterfClr);
    } else {
      p.x += 2;
      Renderer::drawText("<empty>", Panel::screen, p,
                         IS_CUR_POS ? clrWhite : clrNosfTealDrk);
    }

    ++p.y;
  }

  for(size_t i = 0; i < inv.general_.size(); ++i) {
    const bool IS_CUR_POS = IS_IN_INV && BROWSER_Y == i;
    Item* const item = inv.general_.at(i);

    const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();

    Pos p(invRect.p0 + Pos(1, i + 1));

    drawItemSymbol(*item, p);

    p.x += 2;

    str = ItemData::getItemInterfaceRef(*item, false);
    Renderer::drawText(str, Panel::screen, p, itemInterfClr);
    p.y++;
  }

  Renderer::drawPopupBox(invRect, Panel::screen, clrGray, false);
  Renderer::drawText("Inventory", Panel::screen, invRect.p0 + Pos(1, 0), clrWhite);

  Renderer::drawPopupBox(eqpRect, Panel::screen, clrGray, false);
  Renderer::drawText("Equiped items", Panel::screen, eqpRect.p0 + Pos(1, 0), clrWhite);

  drawDetailedItemDescr(nullptr, eqpRect.p1.y);

  Renderer::updateScreen();
}

//void drawBrowseInventory(const MenuBrowser& browser,
//                         const vector<size_t>& genInvIndexes) {
//
//  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
//
//  Renderer::coverArea(Panel::screen, Pos(0, 0), Pos(MAP_W, NR_ITEMS + 1));
//
//  string str = NR_ITEMS > 0 ? "Browsing backpack. [shift+enter] to drop" :
//               "I carry no items.";
//
//  str += " [space/esc] to exit";
//
//  Pos p(0, 0);
//  Renderer::drawText(str, Panel::screen, p, clrWhiteHigh);
//  p.y++;
//
//  Inventory& inv = Map::player->getInv();
//  const int NR_INDEXES = genInvIndexes.size();
//  for(int i = 0; i < NR_INDEXES; ++i) {
//    const bool IS_CUR_POS = browser.getPos().y == int(i);
//    Item* const item = inv.general_.at(genInvIndexes.at(i));
//
//    const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();
//    p.x = 0;
//
//    drawItemSymbol(*item, p);
//    p.x += 2;
//
//    str = ItemData::getItemInterfaceRef(*item, false);
//    Renderer::drawText(str, Panel::screen, p, itemInterfClr);
//    p.y++;
//  }
//
//  Renderer::updateScreen();
//}

void drawEquip(const MenuBrowser& browser, const SlotId slotToEquip,
               const vector<size_t>& genInvIndexes) {

  Pos p(0, 0);

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  Renderer::coverArea(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));

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
  if(HAS_ITEM) {str += " [shift+enter] to drop";}
  str += cancelInfoStr;
  Renderer::drawText(str, Panel::screen, p, clrWhiteHigh);
  p.y++;

  Inventory& inv = Map::player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; ++i) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    p.x = 0;

    Item* const item = inv.general_.at(genInvIndexes.at(i));

    drawItemSymbol(*item, p);
    p.x += 2;

    const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();

    const ItemDataT& d = item->getData();
    PrimaryAttMode attackMode = PrimaryAttMode::none;
    if(slotToEquip == SlotId::wielded || slotToEquip == SlotId::wieldedAlt) {
      attackMode = d.primaryAttackMode == PrimaryAttMode::missile ?
                   PrimaryAttMode::melee : d.primaryAttackMode;
    } else if(slotToEquip == SlotId::thrown) {
      attackMode = PrimaryAttMode::missile;
    }

    str = ItemData::getItemInterfaceRef(*item, false, attackMode);
    Renderer::drawText(str, Panel::screen, p, itemInterfClr);
    p.y++;
  }

  Renderer::updateScreen();
}

//void drawUse(const MenuBrowser& browser, const vector<size_t>& genInvIndexes) {
//  Pos p(0, 0);
//
////  Renderer::clearScreen();
//  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
//  Renderer::coverArea(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));
//
//  const bool IS_ANY_ITEM_AVAILABLE = !genInvIndexes.empty();
//  string str = IS_ANY_ITEM_AVAILABLE ? "Use which item? [shift+enter] to drop" :
//               "I carry no item to use.";
//  str += cancelInfoStr;
//
//  Renderer::drawText(str, Panel::screen, p, clrWhiteHigh);
//  p.y++;
//
//  Inventory& inv = Map::player->getInv();
//  const int NR_INDEXES = genInvIndexes.size();
//  for(int i = 0; i < NR_INDEXES; ++i) {
//    const bool IS_CUR_POS = browser.getPos().y == int(i);
//    Item* const item = inv.general_.at(genInvIndexes.at(i));
//
//    const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();
//
//    //Draw label
//    const string& label = item->getDefaultActivationLabel();
//    bool isNewLabel = false;
//    if(i == 0) {
//      isNewLabel = true;
//    } else {
//      Item* const itemPrev = inv.general_.at(genInvIndexes.at(i - 1));
//      const string& labelPrev = itemPrev->getDefaultActivationLabel();
//      isNewLabel = label != labelPrev;
//    }
//    if(isNewLabel) {
//      p.x = 0;
//      Renderer::drawText(label, Panel::screen, p, clrNosfTealDrk);
//    }
//
//    p.x = 11;
//
//    //Draw item symbol
//    drawItemSymbol(*item, p);
//    p.x += 2;
//
//    str = ItemData::getItemRef(*item, ItemRefType::plain, false);
//    if(item->nrItems > 1 && item->getData().isStackable) {
//      str += " (" + toStr(item->nrItems) + ")";
//    }
//
//    Renderer::drawText(str, Panel::screen, p, itemInterfClr);
//    p.y++;
//  }
//
//  Renderer::updateScreen();
//}

} //RenderInventory

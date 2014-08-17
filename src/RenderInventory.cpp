#include "RenderInventory.h"

#include <string>

#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "Map.h"
#include "Item.h"

using namespace std;

const int INV_BOX_X1 = MAP_W_HALF + 6;

namespace {

void drawItemSymbol(const Item& item, const Pos& p) {
  const Clr itemClr = item.getClr();
  if(Config::isTilesMode()) {
    Renderer::drawTile(item.getTile(), Panel::screen, p, itemClr);
  } else {
    Renderer::drawGlyph(item.getGlyph(), Panel::screen, p, itemClr);
  }
}

void drawDetailedItemDescr(const Item* const item) {
  const Rect box(INV_BOX_X1, 1, MAP_W - 1, SCREEN_H - 1);

  const Panel panel = Panel::screen;

  Renderer::coverArea(panel, box);

  const Pos p(box.p0 + 1);
  if(item) {
    Renderer::drawText(item->getName(ItemRefType::a), Panel::screen, p, clrWhite);
  }

  Renderer::drawPopupBox(box, panel, clrPopupBox, false);

  if(Config::isTilesMode()) {
    Renderer::drawTile(TileId::popupHorDown, panel, box.p0, clrPopupBox);
    Renderer::drawTile(TileId::popupHorUp, panel, Pos(box.p0.x, box.p1.y), clrPopupBox);
  }
}

} //Namespace

namespace RenderInventory {

void drawBrowseInv(const MenuBrowser& browser) {

  const int     BROWSER_Y   = browser.getY();
  const auto&   inv         = Map::player->getInv();
  const size_t  NR_SLOTS    = inv.slots_.size();

  Renderer::clearScreen();

  const bool    IS_IN_EQP   = BROWSER_Y < int(NR_SLOTS);
  const size_t  INV_ELEMENT = IS_IN_EQP ? 0 : (size_t(BROWSER_Y) - NR_SLOTS);

  const auto* const item =
    IS_IN_EQP ? inv.slots_.at(BROWSER_Y).item : inv.general_.at(INV_ELEMENT);

  const string queryEqStr   = item ? "unequip" : "equip";
  const string queryBaseStr = "[enter] to " + (IS_IN_EQP ? queryEqStr : "Apply item");

  const string queryDropStr = item ? " [shift+enter] to drop" : "";

  string str                = queryBaseStr + queryDropStr + " [space/esc] to exit";
  Renderer::drawText(str, Panel::screen, Pos(0, 0), clrWhiteHigh);

  const int EQP_BOX_Y1 = inv.slots_.size() + 2;

  const Rect eqpRect(Pos(0, 1),           Pos(INV_BOX_X1, EQP_BOX_Y1));
  const Rect invRect(Pos(0, EQP_BOX_Y1),  Pos(INV_BOX_X1, SCREEN_H - 1));

  const size_t MAX_NR_ITEMS_ON_SCR  = invRect.p1.y - invRect.p0.y - 1;
  const size_t NR_INV_ITEMS         = inv.general_.size();

  size_t invTopIdx = 0;

  if(!IS_IN_EQP && NR_INV_ITEMS > 0) {

    auto isBrowserPosOnScr = [&](const bool IS_FIRST_SCR) {
      const int MORE_LABEL_H = IS_FIRST_SCR ? 1 : 2;
      return int(INV_ELEMENT) < (int(invTopIdx + MAX_NR_ITEMS_ON_SCR) - MORE_LABEL_H);
    };

    if(NR_INV_ITEMS > MAX_NR_ITEMS_ON_SCR && !isBrowserPosOnScr(true)) {

      invTopIdx = MAX_NR_ITEMS_ON_SCR - 1;

      while(true) {
        //Check if this is the bottom screen
        if(NR_INV_ITEMS - invTopIdx + 1 <= MAX_NR_ITEMS_ON_SCR) {
          break;
        }

        //Check if current browser pos is currently on screen
        if(isBrowserPosOnScr(false)) {break;}

        invTopIdx += MAX_NR_ITEMS_ON_SCR - 2;
      }
    }
  }

  Pos p(invRect.p0 + Pos(1, 1));

  const int INV_ITEM_SYM_X  = invRect.p0.x + 1;
  const int INV_ITEM_NAME_X = INV_ITEM_SYM_X + 2;

  p.x = invRect.p0.x + 1;

  for(size_t i = invTopIdx; i < NR_INV_ITEMS; ++i) {
    const bool IS_CUR_POS = !IS_IN_EQP && INV_ELEMENT == i;
    Item* const curItem   = inv.general_.at(i);

    const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

    p.x = INV_ITEM_NAME_X;

    if(i == invTopIdx && invTopIdx > 0) {
      Renderer::drawText("(more)", Panel::screen, p, clrBlack, clrGray);
      ++p.y;
    }

    p.x = INV_ITEM_SYM_X;

    drawItemSymbol(*curItem, p);

    p.x = INV_ITEM_NAME_X;

    str = curItem->getName(ItemRefType::plural, ItemRefInf::yes,
                           ItemRefAttInf::wpnContext);
    Renderer::drawText(str, Panel::screen, p, itemInterfClr);

    p.y++;

    if(p.y == invRect.p1.y - 1 && i + 1 < NR_INV_ITEMS - 1) {
      Renderer::drawText("(more)", Panel::screen, p, clrBlack, clrGray);
      break;
    }
  }

  p = Pos(eqpRect.p0 + Pos(1, 1));

  for(size_t i = 0; i < NR_SLOTS; ++i) {
    const bool IS_CUR_POS = IS_IN_EQP && BROWSER_Y == int(i);
    const InvSlot& slot   = inv.slots_.at(i);
    str                   = slot.name;

    p.x = eqpRect.p0.x + 1;

    Renderer::drawText(str, Panel::screen, p,
                       IS_CUR_POS ? clrWhiteHigh : clrMenuDrk);

    p.x += 9; //Offset to leave room for slot label

    const auto* const curItem = slot.item;
    if(curItem) {
      drawItemSymbol(*curItem, p);
      p.x += 2;

      const Clr itemInterfClr =
        IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

      const ItemDataT& d    = curItem->getData();
      ItemRefAttInf attInf  = ItemRefAttInf::none;
      if(slot.id == SlotId::wielded || slot.id == SlotId::wieldedAlt) {
        //Thrown weapons are forced to show melee info instead
        attInf = d.mainAttMode == MainAttMode::thrown ? ItemRefAttInf::melee :
                 ItemRefAttInf::wpnContext;
      } else if(slot.id == SlotId::thrown) {
        attInf = ItemRefAttInf::thrown;
      }

      ItemRefType refType = ItemRefType::plain;
      if(slot.id == SlotId::thrown) {
        refType = ItemRefType::plural;
      }

      str = curItem->getName(refType, ItemRefInf::yes, attInf);
      Renderer::drawText(str, Panel::screen, p, itemInterfClr);
    } else {
      p.x += 2;
      Renderer::drawText("<empty>", Panel::screen, p,
                         IS_CUR_POS ? clrWhiteHigh : clrMenuDrk);
    }

    ++p.y;
  }

  Renderer::drawPopupBox(eqpRect, Panel::screen, clrPopupBox, false);
  Renderer::drawPopupBox(invRect, Panel::screen, clrPopupBox, false);
  Renderer::drawText("Equiped items", Panel::screen, eqpRect.p0 + Pos(1, 0), clrWhite);
  Renderer::drawText("Inventory", Panel::screen, invRect.p0 + Pos(1, 0), clrWhite);

  drawDetailedItemDescr(item);

  if(Config::isTilesMode()) {
    Renderer::drawTile(TileId::popupVerL, Panel::screen, eqpRect.p1, clrPopupBox);
    Renderer::drawTile(TileId::popupVerR, Panel::screen, invRect.p0, clrPopupBox);
  }

  Renderer::updateScreen();
}

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

    const ItemDataT& d    = item->getData();
    ItemRefAttInf attInf  = ItemRefAttInf::none;
    if(slotToEquip == SlotId::wielded || slotToEquip == SlotId::wieldedAlt) {
      //Thrown weapons are forced to show melee info instead
      attInf = d.mainAttMode == MainAttMode::thrown ? ItemRefAttInf::melee :
               ItemRefAttInf::wpnContext;
    } else if(slotToEquip == SlotId::thrown) {
      attInf = ItemRefAttInf::thrown;
    }

    str = item->getName(ItemRefType::plural, ItemRefInf::yes, attInf);
    Renderer::drawText(str, Panel::screen, p, itemInterfClr);
    p.y++;
  }

  Renderer::updateScreen();
}

} //RenderInventory

#include "RenderInventory.h"

#include <string>

#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "Map.h"
#include "Item.h"

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
  const Rect box(EQ_BOX_X0, BOX_Y0, MAP_W - 1, CHAR_LINES_OFFSET_H - 1);

  const Panel panel = Panel::screen;

  Renderer::drawPopupBox(box, panel, clrGray, true);

  if(Config::isTilesMode()) {
    Renderer::drawTile(TileId::popupVerR,   panel, box.p0, clrGray);
    Renderer::drawTile(TileId::popupVerL,   panel, Pos(box.p1.x, box.p0.y), clrGray);
    Renderer::drawTile(TileId::popupHorUp,  panel, Pos(box.p0.x, box.p1.y), clrGray);
  }

  const Pos p(box.p0 + 1);
  if(item) {
    Renderer::drawText(item->getName(ItemRefType::a), Panel::screen, p, clrWhite);
  }
}

} //Namespace

namespace RenderInventory {

void drawBrowseInv(const MenuBrowser& browser) {

  const Pos     browserPos  = browser.getPos();
  const size_t  BROWSER_Y   = browser.getPos().y;
  const auto&   inv         = Map::player->getInv();
  const size_t  NR_SLOTS    = inv.slots_.size();

  Renderer::coverArea(Panel::screen, Pos(0, 0), Pos(MAP_W, CHAR_LINES_OFFSET_H - 1));

  const bool        IS_IN_INV   = browserPos.x == 0;
  const auto* const item        = IS_IN_INV ?
                                  inv.general_.at(BROWSER_Y) :
                                  inv.slots_.at(BROWSER_Y).item;

  const string queryEqStr   = item ? "unequip" : "equip";
  const string queryBaseStr = "[enter] to " + (IS_IN_INV ? "use item" : queryEqStr);

  const string queryDropStr = item ? " [shift+enter] to drop" : "";

  string str                = queryBaseStr + queryDropStr + " [space/esc] to exit";
  Renderer::drawText(str, Panel::screen, Pos(0, 0), clrWhiteHigh);

  const int EQ_BOX_Y1 = inv.slots_.size() + 2;

  const Rect invRect(Pos(0,         1), Pos(EQ_BOX_X0,  CHAR_LINES_OFFSET_H - 1));
  const Rect eqpRect(Pos(EQ_BOX_X0, 1), Pos(MAP_W - 1,  EQ_BOX_Y1));

  const size_t MAX_NR_ITEMS_ON_SCR  = invRect.p1.y - invRect.p0.y - 1;
  const size_t NR_INV_ITEMS         = inv.general_.size();

  size_t invTopIdx = 0;

  if(browserPos.x == 0 && NR_INV_ITEMS > 0) {

    const int BROWESR_Y = browserPos.y;

    auto isBrowserPosOnScr = [&](const bool IS_FIRST_SCR) {
      return BROWESR_Y < int(invTopIdx + MAX_NR_ITEMS_ON_SCR) - (IS_FIRST_SCR ? 1 : 2);
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

  for(size_t i = invTopIdx; i < NR_INV_ITEMS; ++i) {
    const bool IS_CUR_POS = IS_IN_INV && BROWSER_Y == i;
    Item* const curItem = inv.general_.at(i);

    const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

    p.x = INV_ITEM_NAME_X;

    if(i == invTopIdx && invTopIdx > 0) {
      Renderer::drawText("(more)", Panel::screen, p, clrBlack, clrGray);
      ++p.y;
    }

    p.x = INV_ITEM_SYM_X;

    drawItemSymbol(*curItem, p);

    p.x = INV_ITEM_NAME_X;

    str = curItem->getName(ItemRefType::plural);
    Renderer::drawText(str, Panel::screen, p, itemInterfClr);

    p.y++;

    if(p.y == invRect.p1.y - 1 && i + 1 < NR_INV_ITEMS - 1) {
      Renderer::drawText("(more)", Panel::screen, p, clrBlack, clrGray);
      break;
    }
  }

  p = Pos(eqpRect.p0 + Pos(1, 1));

  for(size_t i = 0; i < NR_SLOTS; ++i) {
    const bool IS_CUR_POS = !IS_IN_INV && BROWSER_Y == i;
    const InvSlot& slot   = inv.slots_.at(i);
    str                   = slot.name;

    p.x = eqpRect.p0.x + 1;

    Renderer::drawText(str, Panel::screen, p,
                       IS_CUR_POS ? clrWhiteHigh : clrNosfTealDrk);

    p.x += 9; //Offset to leave room for slot label

    const auto* const curItem = slot.item;
    if(curItem) {
      drawItemSymbol(*curItem, p);
      p.x += 2;

      const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

      const ItemDataT& d    = curItem->getData();
      ItemRefAttInf attInf  = ItemRefAttInf::none;
      if(slot.id == SlotId::wielded || slot.id == SlotId::wieldedAlt) {
        //Thrown weapons are forced to show melee info instead
        attInf = d.mainAttMode == MainAttMode::thrown ? ItemRefAttInf::melee :
                 ItemRefAttInf::wpnContext;
      } else if(slot.id == SlotId::thrown) {
        attInf = ItemRefAttInf::thrown;
      }

      str = curItem->getName(ItemRefType::plain, ItemRefInf::yes, attInf);
      Renderer::drawText(str, Panel::screen, p, itemInterfClr);
    } else {
      p.x += 2;
      Renderer::drawText("<empty>", Panel::screen, p,
                         IS_CUR_POS ? clrWhite : clrNosfTealDrk);
    }

    ++p.y;
  }

  Renderer::drawPopupBox(invRect, Panel::screen, clrGray, false);
  Renderer::drawText("Inventory", Panel::screen, invRect.p0 + Pos(1, 0), clrWhite);

  Renderer::drawPopupBox(eqpRect, Panel::screen, clrGray, false);
  Renderer::drawText("Equiped items", Panel::screen, eqpRect.p0 + Pos(1, 0), clrWhite);

  if(Config::isTilesMode()) {
    Renderer::drawTile(TileId::popupHorDown, Panel::screen, eqpRect.p0, clrGray);
  }

  drawDetailedItemDescr(item, eqpRect.p1.y);

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

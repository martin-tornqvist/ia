#include "RenderInventory.h"

#include <string>

#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "Map.h"
#include "Item.h"
#include "TextFormatting.h"

using namespace std;

const int DESCR_X0 = MAP_W - 30;

namespace {

void drawItemSymbol(const Item& item, const Pos& p) {
  const Clr itemClr = item.getClr();
  if(Config::isTilesMode()) {
    Renderer::drawTile(item.getTile(), Panel::screen, p, itemClr);
  } else {
    Renderer::drawGlyph(item.getGlyph(), Panel::screen, p, itemClr);
  }
}

void drawWeightPct(const int Y, const int ITEM_NAME_X, const size_t ITEM_NAME_LEN,
                   const Item& item, const Clr& itemNameClr, const bool IS_SELECTED) {
  const int WEIGHT_CARRIED_TOT = Map::player->getInv().getTotalItemWeight();
  const int WEIGHT_PCT         = (item.getWeight() * 100) / WEIGHT_CARRIED_TOT;

  if(WEIGHT_PCT > 0 && WEIGHT_PCT < 100) {
    const string  weightStr = toStr(WEIGHT_PCT) + "%";
    const int     WEIGHT_X = DESCR_X0 - weightStr.size() - 1;
    const Pos     weightPos(WEIGHT_X, Y);
    const Clr     weightClr = IS_SELECTED ? clrWhite : clrGrayDrk;
    Renderer::drawText(weightStr, Panel::screen, weightPos, weightClr);

    const int DOTS_X  = ITEM_NAME_X + ITEM_NAME_LEN;
    const int DOTS_W  = WEIGHT_X - DOTS_X;
    const string dotsStr(DOTS_W, '.');
    Clr dotsClr       = IS_SELECTED ? clrWhite : itemNameClr;
    if(!IS_SELECTED) {dotsClr.r /= 2; dotsClr.g /= 2; dotsClr.b /= 2;}
    Renderer::drawText(dotsStr, Panel::screen, Pos(DOTS_X, Y), dotsClr);
  }
}

void drawDetailedItemDescr(const Item* const item) {
  const Panel panel   = Panel::screen;
  const int DESCR_Y0  = 1;
  const int DESCR_X1  = MAP_W - 1;
  Renderer::coverArea(panel, Rect(DESCR_X0, DESCR_Y0, DESCR_X1, SCREEN_H - 1));

  const int MAX_W = DESCR_X1 - DESCR_X0 + 1;

  Pos p(DESCR_X0, DESCR_Y0);

  if(item) {
    auto printLineAndIncrY = [&](const string & line, const Clr & clr) {
      Renderer::drawText(line, panel, p, clr);
      ++p.y;
    };

    vector<string> lines;

    const auto& data = item->getData();

    const auto baseDescr = data.baseDescr;

    if(!baseDescr.empty()) {
      for(const string& paragraph : baseDescr) {
        TextFormatting::lineToLines(paragraph, MAX_W, lines);
        for(const string& line : lines) {printLineAndIncrY(line, clrWhiteHigh);}
        ++p.y;
      }
    }

//    p.y = max(p.y, 10);
//    p.y = SCREEN_H - 2;

    const bool  IS_PLURAL = item->nrItems_ > 1 && data.isStackable;
    const string weightStr =
      (IS_PLURAL ? "They are " : "It is ") + item->getWeightStr() + " to carry.";

    printLineAndIncrY(weightStr, clrGreen);

    const int WEIGHT_CARRIED_TOT = Map::player->getInv().getTotalItemWeight();
    const int WEIGHT_PCT         = (item->getWeight() * 100) / WEIGHT_CARRIED_TOT;

    if(WEIGHT_PCT > 0 && WEIGHT_PCT < 100) {
      const string pctStr = "(" + toStr(WEIGHT_PCT) + "% of total carried weight)";
      printLineAndIncrY(pctStr, clrGreen);
    }
  }

//  Renderer::drawPopupBox(box, panel, clrPopupBox, false);

//  Renderer::drawText("Description", panel, box.p0 + Pos(1, 0), clrWhite);

//  if(Config::isTilesMode()) {
//    Renderer::drawTile(TileId::popupHorDown, panel, box.p0, clrPopupBox);
//    Renderer::drawTile(TileId::popupHorUp, panel, Pos(box.p0.x, box.p1.y), clrPopupBox);
//  }
}

} //Namespace

namespace RenderInventory {

void drawBrowseInv(const MenuBrowser& browser) {

  Renderer::clearScreen();

  const int     BROWSER_Y   = browser.getY();
  const auto&   inv         = Map::player->getInv();
  const size_t  NR_SLOTS    = inv.slots_.size();

  const bool    IS_IN_EQP   = BROWSER_Y < int(NR_SLOTS);
  const size_t  INV_ELEMENT = IS_IN_EQP ? 0 : (size_t(BROWSER_Y) - NR_SLOTS);

  const auto* const item =
    IS_IN_EQP ? inv.slots_.at(BROWSER_Y).item : inv.general_.at(INV_ELEMENT);

  const string queryEqStr   = item ? "unequip" : "equip";
  const string queryBaseStr = "[enter] to " + (IS_IN_EQP ? queryEqStr : "apply item");

  const string queryDropStr = item ? " [shift+enter] to drop" : "";

  string str                = queryBaseStr + queryDropStr + " [space/esc] to exit";
  Renderer::drawText(str, Panel::screen, Pos(0, 0), clrWhiteHigh);

  const int EQP_Y0  = 1;

  Pos p(0, EQP_Y0);

  const Panel panel = Panel::screen;

  for(size_t i = 0; i < NR_SLOTS; ++i) {
    const bool IS_CUR_POS = IS_IN_EQP && BROWSER_Y == int(i);
    const InvSlot& slot   = inv.slots_.at(i);
    const string slotName = slot.name;

    p.x = 0;

    Renderer::drawText(slotName, panel, p, IS_CUR_POS ? clrWhiteHigh : clrMenuDrk);

    p.x += 9; //Offset to leave room for slot label

    const auto* const curItem = slot.item;
    if(curItem) {
      drawItemSymbol(*curItem, p);
      p.x += 2;

      const Clr clr = IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

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
      if(slot.id == SlotId::thrown) {refType = ItemRefType::plural;}

      const string itemName = curItem->getName(refType, ItemRefInf::yes, attInf);
      Renderer::drawText(itemName, panel, p, clr);

      drawWeightPct(p.y, p.x, itemName.size(), *curItem, clr, IS_CUR_POS);
    } else {
      p.x += 2;
      Renderer::drawText("<empty>", panel, p, IS_CUR_POS ? clrWhiteHigh : clrMenuDrk);
    }

    ++p.y;
  }

  const int INV_Y0          = EQP_Y0 + inv.slots_.size();
  const int INV_Y1          = SCREEN_H - 1;
  const int INV_H           = INV_Y1 - INV_Y0 + 1;
  const size_t NR_INV_ITEMS = inv.general_.size();

  size_t invTopIdx = 0;

  if(!IS_IN_EQP && NR_INV_ITEMS > 0) {

    auto isBrowserPosOnScr = [&](const bool IS_FIRST_SCR) {
      const int MORE_LABEL_H = IS_FIRST_SCR ? 1 : 2;
      return int(INV_ELEMENT) < (int(invTopIdx + INV_H) - MORE_LABEL_H);
    };

    if(int(NR_INV_ITEMS) > INV_H && !isBrowserPosOnScr(true)) {

      invTopIdx = INV_H - 1;

      while(true) {
        //Check if this is the bottom screen
        if(int(NR_INV_ITEMS - invTopIdx) + 1 <= INV_H) {break;}

        //Check if current browser pos is currently on screen
        if(isBrowserPosOnScr(false)) {break;}

        invTopIdx += INV_H - 2;
      }
    }
  }

  p = Pos(0, INV_Y0);

  const int INV_ITEM_NAME_X = 2;

  for(size_t i = invTopIdx; i < NR_INV_ITEMS; ++i) {
    const bool IS_CUR_POS = !IS_IN_EQP && INV_ELEMENT == i;
    Item* const curItem   = inv.general_.at(i);

    const Clr clr = IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

    if(i == invTopIdx && invTopIdx > 0) {
      p.x = INV_ITEM_NAME_X;
      Renderer::drawText("(more)", panel, p, clrBlack, clrGray);
      ++p.y;
    }

    p.x = 0;

    drawItemSymbol(*curItem, p);

    p.x = INV_ITEM_NAME_X;

    const string itemName = curItem->getName(ItemRefType::plural, ItemRefInf::yes,
                            ItemRefAttInf::wpnContext);
    Renderer::drawText(itemName, panel, p, clr);

    drawWeightPct(p.y, INV_ITEM_NAME_X, itemName.size(), *curItem, clr, IS_CUR_POS);

    ++p.y;

    if(p.y == INV_Y1 && ((i + 1) < (NR_INV_ITEMS - 1))) {
      Renderer::drawText("(more)", panel, p, clrBlack, clrGray);
      break;
    }
  }

//  Renderer::drawPopupBox(eqpRect, panel, clrPopupBox, false);
//  Renderer::drawPopupBox(invRect, panel, clrPopupBox, false);

//  for(int x = 0; x < eqpRect.p1.x; x++) {
//    if(Config::isTilesMode()) {
//      Renderer::drawTile(TileId::popupHor, panel, Pos(x, eqpRect.p0.y), clrPopupBox);
//      Renderer::drawTile(TileId::popupHor, panel, Pos(x, invRect.p0.y), clrPopupBox);
//    } else {
  //TODO
//    }
//  }

//  Renderer::drawText("Equiped items", panel, eqpRect.p0 + Pos(1, 0), clrWhite);
//  Renderer::drawText("Inventory", panel, invRect.p0 + Pos(1, 0), clrWhite);

  drawDetailedItemDescr(item);

//  if(Config::isTilesMode()) {
//    Renderer::drawTile(TileId::popupVerL, panel, eqpRect.p1, clrPopupBox);
//    Renderer::drawTile(TileId::popupVerR, panel, invRect.p0, clrPopupBox);
//  }

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
  ++p.y;

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
    ++p.y;
  }

  Renderer::updateScreen();
}

} //RenderInventory

#include "RenderInventory.h"

#include <string>

#include "Init.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Render.h"
#include "Map.h"
#include "Item.h"
#include "TextFormatting.h"

using namespace std;

namespace
{

void drawItemSymbol(const Item& item, const Pos& p)
{
  const Clr itemClr = item.getClr();
  if(Config::isTilesMode())
  {
    Render::drawTile(item.getTile(), Panel::screen, p, itemClr);
  }
  else
  {
    Render::drawGlyph(item.getGlyph(), Panel::screen, p, itemClr);
  }
}

void drawWeightPct(const int Y, const int ITEM_NAME_X, const size_t ITEM_NAME_LEN,
                   const Item& item, const Clr& itemNameClr, const bool IS_SELECTED)
{
  const int WEIGHT_CARRIED_TOT = Map::player->getInv().getTotalItemWeight();
  const int WEIGHT_PCT         = (item.getWeight() * 100) / WEIGHT_CARRIED_TOT;

  if(WEIGHT_PCT > 0 && WEIGHT_PCT < 100)
  {
    const string  weightStr = toStr(WEIGHT_PCT) + "%";
    const int     WEIGHT_X = DESCR_X0 - weightStr.size() - 1;
    const Pos     weightPos(WEIGHT_X, Y);
    const Clr     weightClr = IS_SELECTED ? clrWhite : clrGrayDrk;
    Render::drawText(weightStr, Panel::screen, weightPos, weightClr);

    const int DOTS_X  = ITEM_NAME_X + ITEM_NAME_LEN;
    const int DOTS_W  = WEIGHT_X - DOTS_X;
    const string dotsStr(DOTS_W, '.');
    Clr dotsClr       = IS_SELECTED ? clrWhite : itemNameClr;
    if(!IS_SELECTED) {dotsClr.r /= 2; dotsClr.g /= 2; dotsClr.b /= 2;}
    Render::drawText(dotsStr, Panel::screen, Pos(DOTS_X, Y), dotsClr);
  }
}

void drawDetailedItemDescr(const Item* const item)
{
  if(item)
  {
    vector<StrAndClr> lines;

    const auto baseDescr = item->getDescr();

    if(!baseDescr.empty())
    {
      for(const string& paragraph : baseDescr)
      {
        lines.push_back({paragraph, clrWhiteHigh});
      }
    }

    const bool  IS_PLURAL = item->nrItems_ > 1 && item->getData().isStackable;
    const string weightStr =
      (IS_PLURAL ? "They are " : "It is ") + item->getWeightStr() + " to carry.";

    lines.push_back({weightStr, clrGreen});

    const int WEIGHT_CARRIED_TOT = Map::player->getInv().getTotalItemWeight();
    const int WEIGHT_PCT         = (item->getWeight() * 100) / WEIGHT_CARRIED_TOT;

    if(WEIGHT_PCT > 0 && WEIGHT_PCT < 100)
    {
      const string pctStr = "(" + toStr(WEIGHT_PCT) + "% of total carried weight)";
      lines.push_back({pctStr, clrGreen});
    }

    Render::drawDescrBox(lines);
  }
}

} //Namespace

namespace RenderInventory
{

void drawBrowseInv(const MenuBrowser& browser)
{

  Render::clearScreen();

  const int     BROWSER_Y   = browser.getY();
  const auto&   inv         = Map::player->getInv();
  const size_t  NR_SLOTS    = size_t(SlotId::END);

  const bool    IS_IN_EQP   = BROWSER_Y < int(NR_SLOTS);
  const size_t  INV_ELEMENT = IS_IN_EQP ? 0 : (size_t(BROWSER_Y) - NR_SLOTS);

  const auto* const item =
    IS_IN_EQP ? inv.slots_[BROWSER_Y].item : inv.general_.at(INV_ELEMENT);

  const string queryEqStr   = item ? "unequip" : "equip";
  const string queryBaseStr = "[enter] to " + (IS_IN_EQP ? queryEqStr : "apply item");

  const string queryDropStr = item ? " [shift+enter] to drop" : "";

  string str                = queryBaseStr + queryDropStr + " [space/esc] to exit";
  Render::drawText(str, Panel::screen, Pos(0, 0), clrWhiteHigh);

  Pos p(1, EQP_Y0);

  const Panel panel = Panel::screen;

  for(size_t i = 0; i < NR_SLOTS; ++i)
  {
    const bool IS_CUR_POS = IS_IN_EQP && BROWSER_Y == int(i);
    const InvSlot& slot   = inv.slots_[i];
    const string slotName = slot.name;

    p.x = 1;

    Render::drawText(slotName, panel, p, IS_CUR_POS ? clrWhiteHigh : clrMenuDrk);

    p.x += 9; //Offset to leave room for slot label

    const auto* const curItem = slot.item;
    if(curItem)
    {
      drawItemSymbol(*curItem, p);
      p.x += 2;

      const Clr clr = IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

      const ItemDataT& d    = curItem->getData();
      ItemRefAttInf attInf  = ItemRefAttInf::none;
      if(slot.id == SlotId::wielded || slot.id == SlotId::wieldedAlt)
      {
        //Thrown weapons are forced to show melee info instead
        attInf = d.mainAttMode == MainAttMode::thrown ? ItemRefAttInf::melee :
                 ItemRefAttInf::wpnContext;
      }
      else if(slot.id == SlotId::thrown)
      {
        attInf = ItemRefAttInf::thrown;
      }

      ItemRefType refType = ItemRefType::plain;
      if(slot.id == SlotId::thrown) {refType = ItemRefType::plural;}

      const string itemName = curItem->getName(refType, ItemRefInf::yes, attInf);
      Render::drawText(itemName, panel, p, clr);

      drawWeightPct(p.y, p.x, itemName.size(), *curItem, clr, IS_CUR_POS);
    }
    else
    {
      p.x += 2;
      Render::drawText("<empty>", panel, p, IS_CUR_POS ? clrWhiteHigh : clrMenuDrk);
    }

    ++p.y;
  }


  const size_t  NR_INV_ITEMS  = inv.general_.size();

  size_t invTopIdx = 0;

  if(!IS_IN_EQP && NR_INV_ITEMS > 0)
  {

    auto isBrowserPosOnScr = [&](const bool IS_FIRST_SCR)
    {
      const int MORE_LABEL_H = IS_FIRST_SCR ? 1 : 2;
      return int(INV_ELEMENT) < (int(invTopIdx + INV_H) - MORE_LABEL_H);
    };

    if(int(NR_INV_ITEMS) > INV_H && !isBrowserPosOnScr(true))
    {

      invTopIdx = INV_H - 1;

      while(true)
      {
        //Check if this is the bottom screen
        if(int(NR_INV_ITEMS - invTopIdx) + 1 <= INV_H) {break;}

        //Check if current browser pos is currently on screen
        if(isBrowserPosOnScr(false)) {break;}

        invTopIdx += INV_H - 2;
      }
    }
  }

  const int INV_X0  = 1;

  p = Pos(INV_X0, INV_Y0);

  const int INV_ITEM_NAME_X = INV_X0 + 2;

  for(size_t i = invTopIdx; i < NR_INV_ITEMS; ++i)
  {
    const bool IS_CUR_POS = !IS_IN_EQP && INV_ELEMENT == i;
    Item* const curItem   = inv.general_.at(i);

    const Clr clr = IS_CUR_POS ? clrWhiteHigh : curItem->getInterfaceClr();

    if(i == invTopIdx && invTopIdx > 0)
    {
      p.x = INV_ITEM_NAME_X;
      Render::drawText("(more)", panel, p, clrBlack, clrGray);
      ++p.y;
    }

    p.x = INV_X0;

    drawItemSymbol(*curItem, p);

    p.x = INV_ITEM_NAME_X;

    const string itemName = curItem->getName(ItemRefType::plural, ItemRefInf::yes,
                            ItemRefAttInf::wpnContext);
    Render::drawText(itemName, panel, p, clr);

    drawWeightPct(p.y, INV_ITEM_NAME_X, itemName.size(), *curItem, clr, IS_CUR_POS);

    ++p.y;

    if(p.y == INV_Y1 && ((i + 1) < (NR_INV_ITEMS - 1)))
    {
      Render::drawText("(more)", panel, p, clrBlack, clrGray);
      break;
    }
  }

//  Render::drawPopupBox(eqpRect, panel, clrPopupBox, false);

  const Rect eqpRect(0, EQP_Y0 - 1, DESCR_X0 - 1, EQP_Y1 + 1);
  const Rect invRect(0, INV_Y0 - 1, DESCR_X0 - 1, INV_Y1 + 1);

  Render::drawPopupBox(eqpRect, panel, clrPopupBox, false);
  Render::drawPopupBox(invRect, panel, clrPopupBox, false);

  if(Config::isTilesMode())
  {
    Render::drawTile(TileId::popupVerR, panel, invRect.p0, clrPopupBox);
    Render::drawTile(TileId::popupVerL, panel, Pos(invRect.p1.x, invRect.p0.y),
                     clrPopupBox);
  }

//  Render::drawText("Equiped items", panel, eqpRect.p0 + Pos(1, 0), clrWhite);
//  Render::drawText("Inventory",     panel, invRect.p0 + Pos(1, 0), clrWhite);

  drawDetailedItemDescr(item);

  Render::updateScreen();
}

void drawEquip(const MenuBrowser& browser, const SlotId slotIdToEquip,
               const vector<size_t>& genInvIndexes)
{

  Pos p(0, 0);

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  Render::coverArea(Panel::screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));

  const bool HAS_ITEM = !genInvIndexes.empty();

  string str = "";
  switch(slotIdToEquip)
  {
    case SlotId::wielded:
    {
      str = HAS_ITEM ? "Wield which item?"            : "I carry no weapon to wield.";
    } break;

    case SlotId::wieldedAlt:
    {
      str = HAS_ITEM ? "Prepare which weapon?"        : "I carry no weapon to wield.";
    } break;

    case SlotId::thrown:
    {
      str = HAS_ITEM ? "Use which item as missiles?"  : "I carry no weapon to throw." ;
    } break;

    case SlotId::body:
    {
      str = HAS_ITEM ? "Wear which armor?"            : "I carry no armor.";
    } break;

    case SlotId::head:
    {
      str = HAS_ITEM ? "Wear what on head?"           : "I carry no headwear.";
    } break;

    case SlotId::END:
    {
      TRACE << "Illegal slot id: " << int(slotIdToEquip) << endl;
      assert(false);
    }
  }
  if(HAS_ITEM) {str += " [shift+enter] to drop";}
  str += cancelInfoStr;
  Render::drawText(str, Panel::screen, p, clrWhiteHigh);
  ++p.y;

  Inventory& inv = Map::player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; ++i)
  {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    p.x = 0;

    Item* const item = inv.general_.at(genInvIndexes.at(i));

    drawItemSymbol(*item, p);
    p.x += 2;

    const Clr itemInterfClr = IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();

    const ItemDataT& d    = item->getData();
    ItemRefAttInf attInf  = ItemRefAttInf::none;
    if(slotIdToEquip == SlotId::wielded || slotIdToEquip == SlotId::wieldedAlt)
    {
      //Thrown weapons are forced to show melee info instead
      attInf = d.mainAttMode == MainAttMode::thrown ? ItemRefAttInf::melee :
               ItemRefAttInf::wpnContext;
    }
    else if(slotIdToEquip == SlotId::thrown)
    {
      attInf = ItemRefAttInf::thrown;
    }

    str = item->getName(ItemRefType::plural, ItemRefInf::yes, attInf);
    Render::drawText(str, Panel::screen, p, itemInterfClr);
    ++p.y;
  }

  Render::updateScreen();
}

} //RenderInventory

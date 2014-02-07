#include "RenderInventory.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "InventoryIndexes.h"
#include "Log.h"
#include "Renderer.h"

const int X_POS_WEIGHT = 62;

namespace {

void drawDots(const int X_PREV, const int W_PREV, const int X_NEW, const int Y,
              const SDL_Color& clr, Engine& eng) {

  const int X_DOTS = X_PREV + W_PREV;
  const int W_DOTS = X_NEW - X_DOTS;
  const string dots(W_DOTS, '.');
  SDL_Color realColorDots = clr;
  realColorDots.r /= 3; realColorDots.g /= 3; realColorDots.b /= 3;
  eng.renderer->drawText(dots, panel_screen, Pos(X_DOTS, Y), realColorDots);
}

void drawItemSymbol(const Item& item, const Pos& pos, Engine& eng) {
  const SDL_Color itemClr = item.getColor();
  if(eng.config->isTilesMode) {
    eng.renderer->drawTile(item.getTile(), panel_screen, pos, itemClr);
  } else {
    eng.renderer->drawGlyph(item.getGlyph(), panel_screen, pos, itemClr);
  }
}

} //Namespace

namespace RenderInventory {

void drawBrowseSlots(const MenuBrowser& browser,
                     const vector<InventorySlotButton>& invSlotButtons,
                     Engine& eng) {
  Pos pos(0, 0);

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng.renderer->coverArea(panel_screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 2));

  string str =
    "Select slot to equip/unequip. | shift+select to drop | space/esc to exit";
  eng.renderer->drawText(str, panel_screen, pos, clrWhiteHigh);

  const int X_POS_ITEM_NAME = 15;

  pos.y++;

  for(unsigned int i = 0; i < invSlotButtons.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    str = "x) ";
    str.at(0) = 'a' + i;
    InventorySlot* const slot = invSlotButtons.at(i).inventorySlot;
    str += slot->interfaceName;
    pos.x = 0;
    eng.renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x = X_POS_ITEM_NAME;
    Item* const item = slot->item;
    if(item == NULL) {
      pos.x += 2;
      eng.renderer->drawText(
        "<empty>", panel_screen, pos, IS_CUR_POS ? clrWhite : clrRedLgt);
    } else {
      drawItemSymbol(*item, pos, eng);
      pos.x += 2;

      const SDL_Color itemInterfClr =
        IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();

      const ItemData& d = item->getData();
      PrimaryAttackMode attackMode = primaryAttackMode_none;
      if(slot->id == slot_wielded || slot->id == slot_wieldedAlt) {
        attackMode =
          d.primaryAttackMode == primaryAttackMode_missile ?
          primaryAttackMode_melee : d.primaryAttackMode;
      } else if(slot->id == slot_missiles) {
        attackMode = primaryAttackMode_missile;
      }

      str = eng.itemDataHandler->getItemInterfaceRef(
              *item, false, attackMode);
      eng.renderer->drawText(str, panel_screen, pos, itemInterfClr);
      drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr, eng);
      eng.renderer->drawText(
        item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y),
        clrGray);
    }

    pos.y++;
  }

  str = "x) Browse backpack";
  str.at(0) = invSlotButtons.back().key + 1;
  pos.x = 0;
  pos.y += 1;
  const bool IS_CUR_POS = browser.getPos().y == int(invSlotButtons.size());
  eng.renderer->drawText(str, panel_screen, pos,
                         IS_CUR_POS ? clrWhiteHigh : clrRedLgt);

  eng.renderer->updateScreen();
}

void drawBrowseInventory(const MenuBrowser& browser,
                         const vector<unsigned int>& genInvIndexes,
                         Engine& eng) {

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();

  eng.renderer->coverArea(panel_screen, Pos(0, 0), Pos(MAP_W, NR_ITEMS + 1));

  string str = NR_ITEMS > 0 ?
               "Browsing backpack. | shift+select to drop" :
               "I carry no items.";
  str += " | space/esc to exit";

  Pos pos(0, 0);
  eng.renderer->drawText(str, panel_screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory& inv = eng.player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    Item* const item = inv.getGeneral().at(genInvIndexes.at(i));

    const SDL_Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();
    str = "x) ";
    str.at(0) = 'a' + i;
    pos.x = 0;
    eng.renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x += 3;

    drawItemSymbol(*item, pos, eng);
    pos.x += 2;

    str = eng.itemDataHandler->getItemInterfaceRef(*item, false);
    eng.renderer->drawText(str, panel_screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr, eng);
    eng.renderer->drawText(
      item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  eng.renderer->updateScreen();
}

void drawEquip(const MenuBrowser& browser, const SlotId slotToEquip,
               const vector<unsigned int>& genInvIndexes, Engine& eng) {

  Pos pos(0, 0);

  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng.renderer->coverArea(
    panel_screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));

  const bool IS_ANY_ITEM_AVAILABLE = genInvIndexes.empty() == false;

  string str = "";
  switch(slotToEquip) {
    case slot_armorBody:
      str = IS_ANY_ITEM_AVAILABLE ? "Wear which armor?" :
            "I carry no armor.";
      break;
    case slot_missiles:
      str = IS_ANY_ITEM_AVAILABLE ? "Use which item as missiles?" :
            "I carry no weapon to throw." ;
      break;
    case slot_wielded:
      str = IS_ANY_ITEM_AVAILABLE ? "Wield which item?" :
            "I carry no weapon to wield.";
      break;
    case slot_wieldedAlt:
      str = IS_ANY_ITEM_AVAILABLE ? "Prepare which weapon?" :
            "I carry no weapon to wield.";
      break;
  }
  if(IS_ANY_ITEM_AVAILABLE) {
    str += " | shift+select to drop";
  }
  str += cancelInfoStr;
  eng.renderer->drawText(str, panel_screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory& inv = eng.player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    str = "x) ";
    str.at(0) = 'a' + i;
    pos.x = 0;
    eng.renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x += 3;

    Item* const item = inv.getGeneral().at(genInvIndexes.at(i));

    drawItemSymbol(*item, pos, eng);
    pos.x += 2;

    const SDL_Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();

    const ItemData& d = item->getData();
    PrimaryAttackMode attackMode = primaryAttackMode_none;
    if(slotToEquip == slot_wielded || slotToEquip == slot_wieldedAlt) {
      attackMode =
        d.primaryAttackMode == primaryAttackMode_missile ?
        primaryAttackMode_melee : d.primaryAttackMode;
    } else if(slotToEquip == slot_missiles) {
      attackMode = primaryAttackMode_missile;
    }

    str = eng.itemDataHandler->getItemInterfaceRef(*item, false, attackMode);
    eng.renderer->drawText(str, panel_screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr, eng);
    eng.renderer->drawText(
      item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  eng.renderer->updateScreen();
}

void drawUse(const MenuBrowser& browser,
             const vector<unsigned int>& genInvIndexes, Engine& eng) {
  Pos pos(0, 0);

//  eng.renderer->clearScreen();
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng.renderer->coverArea(
    panel_screen, Pos(0, 1), Pos(MAP_W, NR_ITEMS + 1));

  const bool IS_ANY_ITEM_AVAILABLE = genInvIndexes.empty() == false;
  string str =
    IS_ANY_ITEM_AVAILABLE ? "Use which item? | shift+select to drop" :
    "I carry no item to use.";
  str += cancelInfoStr;

  eng.renderer->drawText(str, panel_screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory& inv = eng.player->getInv();
  const int NR_INDEXES = genInvIndexes.size();
  for(int i = 0; i < NR_INDEXES; i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    Item* const item = inv.getGeneral().at(genInvIndexes.at(i));

    const SDL_Color itemInterfClr = IS_CUR_POS ?
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
      eng.renderer->drawText(label, panel_screen, pos, clrYellow);
    }

    pos.x = 10;
    str = "x) ";
    str.at(0) = 'a' + i;
    eng.renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x += 3;

    //Draw item symbol
    drawItemSymbol(*item, pos, eng);
    pos.x += 2;

    str = eng.itemDataHandler->getItemRef(*item, itemRef_plain, false);
    if(item->nrItems > 1 && item->getData().isStackable) {
      str += " (" + toString(item->nrItems) + ")";
    }

    eng.renderer->drawText(str, panel_screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr, eng);
    eng.renderer->drawText(
      item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  eng.renderer->updateScreen();
}

} //RenderInventory

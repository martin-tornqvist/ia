#include "RenderInventory.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "InventoryIndexes.h"
#include "Log.h"
#include "Render.h"

RenderInventory::RenderInventory(Engine* engine) :
  eng(engine), X_POS_LEFT(1), X_POS_WEIGHT(X_POS_LEFT + 60) {
}

void RenderInventory::drawDots(const int X_PREV, const int W_PREV,
                               const int X_NEW, const int Y,
                               const SDL_Color& clr) {
  const int X_DOTS = X_PREV + W_PREV;
  const int W_DOTS = X_NEW - X_DOTS;
  const string dots(W_DOTS, '.');
  SDL_Color realColorDots = clr;
  realColorDots.r /= 3; realColorDots.g /= 3; realColorDots.b /= 3;
  eng->renderer->drawText(dots, panel_screen, Pos(X_DOTS, Y), realColorDots);
}

void RenderInventory::drawBrowseSlotsMode(const MenuBrowser& browser,
    const vector<InventorySlotButton>& invSlotButtons) {
  Pos pos(X_POS_LEFT, 1);

//  eng->renderer->clearScreen();
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(
    panel_screen, Pos(0, 1), Pos(MAP_X_CELLS, NR_ITEMS + 2));

  string str =
    "Select slot to equip/unequip. | shift+select to drop | space/esc to exit";
  eng->renderer->drawText(str, panel_screen, pos, clrWhiteHigh);

  const int X_POS_ITEM_NAME = X_POS_LEFT + 14;

  pos.y++;

  for(unsigned int i = 0; i < invSlotButtons.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    str = "x) ";
    str.at(0) = 'a' + i;
    InventorySlot* const slot = invSlotButtons.at(i).inventorySlot;
    str += slot->interfaceName;
    pos.x = X_POS_LEFT;
    eng->renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x = X_POS_ITEM_NAME;
    str = ": ";
    Item* const item = slot->item;
    if(item == NULL) {
      str += "<empty>";
      eng->renderer->drawText(
        str, panel_screen, pos, IS_CUR_POS ? clrWhite : clrRedLgt);
    } else {
      const SDL_Color itemInterfClr =
        IS_CUR_POS ? clrWhiteHigh : item->getInterfaceClr();

      const ItemData& d = item->getData();
      PrimaryAttackMode_t attackMode = primaryAttackMode_none;
      if(slot->id == slot_wielded || slot->id == slot_wieldedAlt) {
        attackMode =
          d.primaryAttackMode == primaryAttackMode_missile ?
          primaryAttackMode_melee : d.primaryAttackMode;
      }
      else if(slot->id == slot_missiles) {
        attackMode = primaryAttackMode_missile;
      }

      str += eng->itemDataHandler->getItemInterfaceRef(
               *item, false, attackMode);
      eng->renderer->drawText(str, panel_screen, pos, itemInterfClr);
      drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
      eng->renderer->drawText(
        item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y),
        clrGray);
    }

    pos.y++;
  }

  str = "x) Browse backpack";
  str.at(0) = invSlotButtons.back().key + 1;
  pos.x = X_POS_LEFT;
  pos.y += 1;
  const bool IS_CUR_POS = browser.getPos().y == int(invSlotButtons.size());
  eng->renderer->drawText(str, panel_screen, pos,
                          IS_CUR_POS ? clrWhiteHigh : clrRedLgt);

  eng->renderer->updateScreen();
}

void RenderInventory::drawBrowseInventoryMode(const MenuBrowser& browser,
    const vector<unsigned int>& genInvIndexes) {

  Pos pos(X_POS_LEFT, 1);

//  eng->renderer->clearScreen();
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(panel_screen, Pos(0, 1),
                           Pos(MAP_X_CELLS, NR_ITEMS + 1));

  const bool HAS_ANY_ITEM = genInvIndexes.empty() == false;

  string str =
    HAS_ANY_ITEM ? "Browsing backpack. | shift+select to drop" :
    "I carry no items.";
  str += " | space/esc to exit";
  eng->renderer->drawText(str, panel_screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory* const inv = eng->player->getInventory();

  for(unsigned int i = 0; i < genInvIndexes.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    Item* const item = inv->getGeneral()->at(genInvIndexes.at(i));

    const SDL_Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();
    str = "x) ";
    str.at(0) = 'a' + i;
    pos.x = X_POS_LEFT;
    eng->renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x += 2;

    str = eng->itemDataHandler->getItemInterfaceRef(*item, false);
    eng->renderer->drawText(str, panel_screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
    eng->renderer->drawText(
      item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  eng->renderer->updateScreen();
}

void RenderInventory::drawEquipMode(
  const MenuBrowser& browser, const SlotTypes_t slotToEquip,
  const vector<unsigned int>& genInvIndexes) {

  Pos pos(X_POS_LEFT, 1);

//  eng->renderer->clearScreen();
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(
    panel_screen, Pos(0, 1), Pos(MAP_X_CELLS, NR_ITEMS + 1));

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
  str += " | space/esc to cancel";
  eng->renderer->drawText(str, panel_screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory* const inv = eng->player->getInventory();

  for(unsigned int i = 0; i < genInvIndexes.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    str = "x) ";
    str.at(0) = 'a' + i;
    pos.x = X_POS_LEFT;
    eng->renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x += 2;

    Item* const item = inv->getGeneral()->at(genInvIndexes.at(i));

    const SDL_Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();

    const ItemData& d = item->getData();
    PrimaryAttackMode_t attackMode = primaryAttackMode_none;
    if(slotToEquip == slot_wielded || slotToEquip == slot_wieldedAlt) {
      attackMode =
        d.primaryAttackMode == primaryAttackMode_missile ?
        primaryAttackMode_melee : d.primaryAttackMode;
    }
    else if(slotToEquip == slot_missiles) {
      attackMode = primaryAttackMode_missile;
    }

    str = eng->itemDataHandler->getItemInterfaceRef(*item, false, attackMode);
    eng->renderer->drawText(str, panel_screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
    eng->renderer->drawText(
      item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  eng->renderer->updateScreen();
}

void RenderInventory::drawUseMode(const MenuBrowser& browser,
                                  const vector<unsigned int>& genInvIndexes) {
  const int X_POS_CMD = X_POS_LEFT + 11;

  Pos pos(X_POS_LEFT, 1);

//  eng->renderer->clearScreen();
  const int NR_ITEMS = browser.getNrOfItemsInFirstList();
  eng->renderer->coverArea(
    panel_screen, Pos(0, 1), Pos(MAP_X_CELLS, NR_ITEMS + 1));

  const bool IS_ANY_ITEM_AVAILABLE = genInvIndexes.empty() == false;
  string str =
    IS_ANY_ITEM_AVAILABLE ? "Use which item? | shift+select to drop" :
    "I carry no item to use.";
  str += " | space/esc to cancel";

  eng->renderer->drawText(str, panel_screen, pos, clrWhiteHigh);
  pos.y++;

  Inventory* const inv = eng->player->getInventory();

  for(unsigned int i = 0; i < genInvIndexes.size(); i++) {
    const bool IS_CUR_POS = browser.getPos().y == int(i);
    Item* const item = inv->getGeneral()->at(genInvIndexes.at(i));

    const SDL_Color itemInterfClr = IS_CUR_POS ?
                                    clrWhiteHigh :
                                    item->getInterfaceClr();

    //Draw label
    const string& label = item->getDefaultActivationLabel();
    bool isNewLabel = false;
    if(i == 0) {
      isNewLabel = true;
    } else {
      Item* const itemPrev = inv->getGeneral()->at(genInvIndexes.at(i - 1));
      const string& labelPrev = itemPrev->getDefaultActivationLabel();
      isNewLabel = label != labelPrev;
    }
    if(isNewLabel) {
      pos.x = X_POS_LEFT;
      eng->renderer->drawText(label, panel_screen, pos, clrYellow);
    }

    pos.x = X_POS_CMD;
    str = "x) ";
    str.at(0) = 'a' + i;
    pos.x = X_POS_CMD;
    eng->renderer->drawText(
      str, panel_screen, pos, IS_CUR_POS ? clrWhiteHigh : clrRedLgt);
    pos.x += 2;

    str = eng->itemDataHandler->getItemRef(*item, itemRef_plain, false);
    if(item->nrItems > 1 && item->getData().isStackable) {
      str += " (" + toString(item->nrItems) + ")";
    }

    eng->renderer->drawText(str, panel_screen, pos, itemInterfClr);
    drawDots(pos.x, int(str.size()), X_POS_WEIGHT, pos.y, itemInterfClr);
    eng->renderer->drawText(
      item->getWeightLabel(), panel_screen, Pos(X_POS_WEIGHT, pos.y), clrGray);
    pos.y++;
  }

  eng->renderer->updateScreen();
}


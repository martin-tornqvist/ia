#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "InventoryHandler.h"

#include "ItemData.h"
#include "MenuBrowser.h"

class Engine;

class RenderInventory {
public:
  RenderInventory(Engine* engine);

  void drawBrowseSlotsMode(const MenuBrowser& browser, const vector<InventorySlotButton>& invSlotButtons);
  void drawBrowseInventoryMode(const MenuBrowser& browser, const vector<unsigned int>& genInvIndexes);
  void drawEquipMode(const MenuBrowser& browser, const SlotTypes_t slotToEquip, const vector<unsigned int>& genInvIndexes);
  void drawUseMode(const MenuBrowser& browser, const vector<unsigned int>& genInvIndexes);
  void drawDropMode(const Item* const itemToDrop);

//  string getWeaponDataLine(Weapon* const weapon) const;

private:
  RenderInventory& operator=(const RenderInventory& other) {
    (void)other;
    return *this;
  }

//  void drawSlots(vector<InventorySlotButton>* slotButtons, const MenuBrowser& browser, const bool DRAW_BROWSER);

//  void drawGeneralItems(const int xPosOffset, const InventoryPurpose_t purpose,
//                       const MenuBrowser& browser, const bool DRAW_BROWSER);

  Engine* eng;

  friend class Examine;
//  const int xPosListsLeft1;
//  const int xPosListsLeft2;
//  const int xPosListsLeft3;
//  const int xPosListsRight1;
//  const int xPosListsRight2;
//  const int xPosListsRightStandardOffset;
//  const int yPosLists;
};

#endif

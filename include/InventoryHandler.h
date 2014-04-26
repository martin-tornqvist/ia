#ifndef INVENTORY_HANDLER
#define INVENTORY_HANDLER

#include "Inventory.h"

enum InventoryScreenId {
  inventoryScreen_slots,
  inventoryScreen_equip,
  inventoryScreen_use,
  inventoryScreen_backpack,
  endOfInventoryScreens
};

enum class ConsumeItem {yes, no};

class InventoryHandler {
public:
  InventoryHandler();

  void runSlotsScreen();
  bool runUseScreen();

  void runBrowseInventory();
  bool runEquipScreen(InventorySlot* const slotToEquip);

  void activateDefault(const unsigned int GENERAL_ITEMS_ELEMENT);

  InventoryScreenId screenToOpenAfterDrop;
  InventorySlot* equipSlotToOpenAfterDrop;
  int browserPosToSetAfterDrop;

private:
  //The values in this vector refer to general inventory elements
  vector<unsigned int> generalItemsToShow;

  bool runDropScreen(const int GLOBAL_ELEMENT_NR);

  void filterPlayerGeneralSlotButtonsEquip(const SlotId slotToEquip);
  void filterPlayerGeneralSlotButtonsUsable();
  void filterPlayerGeneralSlotButtonsShowAll();

  void swapItems(Item** item1, Item** item2);


};

#endif

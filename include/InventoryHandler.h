#ifndef INVENTORY_HANDLER
#define INVENTORY_HANDLER

#include "Inventory.h"

class Engine;

enum InventoryScreen_t {
  inventoryScreen_slots,
  inventoryScreen_equip,
  inventoryScreen_use,
  inventoryScreen_backpack,
  endOfInventoryScreens
};

struct InventorySlotButton {
  InventorySlot* inventorySlot;
  char key;
};

class InventoryHandler {
public:
  InventoryHandler(Engine& engine);

  void runSlotsScreen();
  bool runUseScreen();

  void runBrowseInventoryMode();
  bool runEquipScreen(InventorySlot* const slotToEquip);

  void activateDefault(const unsigned int GENERAL_ITEMS_ELEMENT);

  InventoryScreen_t screenToOpenAfterDrop;
  InventorySlot* equipSlotToOpenAfterDrop;
  int browserPosToSetAfterDrop;

private:
  vector<InventorySlotButton> equipmentSlotButtons;

  //The values in this vector refer to general inventory elements
  vector<unsigned int> generalItemsToShow;

  bool runDropScreen(const int GLOBAL_ELEMENT_NR);

  void filterPlayerGeneralSlotButtonsEquip(const SlotTypes_t slotToEquip);
  void filterPlayerGeneralSlotButtonsUsable();
  void filterPlayerGeneralSlotButtonsShowAll();

  void swapItems(Item** item1, Item** item2);

  Engine& eng;
};

#endif

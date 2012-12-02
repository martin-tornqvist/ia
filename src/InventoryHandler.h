#ifndef INVENTORY_HANDLER
#define INVENTORY_HANDLER

#include "Inventory.h"

class Engine;

struct InventorySlotButton {
  InventorySlot* inventorySlot;
  char key;
};

class InventoryHandler {
public:
  InventoryHandler(Engine* engine);

  void runSlotsScreen();
  void runUseScreen();

  void activateDefault(const unsigned int GENERAL_ITEMS_ELEMENT);

private:
  vector<InventorySlotButton> equipmentSlotButtons;

  //The values in this vector refer to general inventory elements
  vector<unsigned int> generalItemsToShow;

  void runDropScreen(Item* const itemToDrop);
  void runEquipScreen(const SlotTypes_t slotToEquip);
  void runBrowseInventoryMode();

  void filterPlayerGeneralSlotButtonsEquip(const SlotTypes_t slotToEquip);
  void filterPlayerGeneralSlotButtonsUsable();
  void filterPlayerGeneralSlotButtonsShowAll();

  void swapItems(Item** item1, Item** item2);

  Engine* eng;
};

#endif

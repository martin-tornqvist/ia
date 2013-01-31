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
  bool runUseScreen();

  void activateDefault(const unsigned int GENERAL_ITEMS_ELEMENT);

private:
  vector<InventorySlotButton> equipmentSlotButtons;

  //The values in this vector refer to general inventory elements
  vector<unsigned int> generalItemsToShow;

  bool runDropScreen(const int GLOBAL_ELEMENT_NR);
  bool runEquipScreen(InventorySlot* const slotToEquip);
  bool runBrowseInventoryMode();

  void filterPlayerGeneralSlotButtonsEquip(const SlotTypes_t slotToEquip);
  void filterPlayerGeneralSlotButtonsUsable();
  void filterPlayerGeneralSlotButtonsShowAll();

  void swapItems(Item** item1, Item** item2);

  Engine* eng;
};

#endif

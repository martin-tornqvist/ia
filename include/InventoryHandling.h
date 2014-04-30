#ifndef INV_HANDLING
#define INV_HANDLING

#include "Inventory.h"

enum class InvScrId {
  slots,
  equip,
  use,
  backpack,
  endOfInventoryScreens
};

enum class ConsumeItem {
  yes, no
};

namespace InvHandling {

extern InvScrId screenToOpenAfterDrop;
extern InvSlot* equipSlotToOpenAfterDrop;
extern int      browserPosToSetAfterDrop;

void init();

void runSlotsScreen();
bool runUseScreen();

void runBrowseInventory();
bool runEquipScreen(InvSlot* const slotToEquip);

void activateDefault(const unsigned int GENERAL_ITEMS_ELEMENT);

} //InvHandling

#endif

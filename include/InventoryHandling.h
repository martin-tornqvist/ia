#ifndef INV_HANDLING
#define INV_HANDLING

#include "Inventory.h"

enum class InvScrId {inv, equip, END};

enum class ConsumeItem {yes, no};

namespace InvHandling
{

extern InvScrId screenToOpenAfterDrop;
extern InvSlot* equipSlotToOpenAfterDrop;
extern int      browserIdxToSetAfterDrop;

void init();

void runInvScreen();
bool runEquipScreen(InvSlot& slotToEquip);

void activateDefault(const size_t GENERAL_ITEMS_ELEMENT);

} //InvHandling

#endif

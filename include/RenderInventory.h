#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "InventoryHandling.h"

#include "ItemData.h"
#include "MenuBrowser.h"

namespace RenderInventory {

void drawBrowseInv(const MenuBrowser& browser);

void drawEquip(const MenuBrowser& browser, const SlotId slotToEquip,
               const std::vector<size_t>& genInvIndexes);

} //RenderInventory

#endif

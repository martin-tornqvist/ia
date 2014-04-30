#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "InventoryHandling.h"

#include "ItemData.h"
#include "MenuBrowser.h"

namespace RenderInventory {

void drawBrowseSlots(const MenuBrowser& browser);

void drawBrowseInventory(const MenuBrowser& browser,
                         const std::vector<unsigned int>& genInvIndexes);

void drawEquip(const MenuBrowser& browser, const SlotId slotToEquip,
               const std::vector<unsigned int>& genInvIndexes);

void drawUse(const MenuBrowser& browser,
             const std::vector<unsigned int>& genInvIndexes);

} //RenderInventory

#endif

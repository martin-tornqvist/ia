#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "InventoryHandler.h"

#include "ItemData.h"
#include "MenuBrowser.h"

class Engine;

namespace RenderInventory {

void drawBrowseSlots(const MenuBrowser& browser, Engine& eng);

void drawBrowseInventory(const MenuBrowser& browser,
                         const vector<unsigned int>& genInvIndexes,
                         Engine& eng);

void drawEquip(const MenuBrowser& browser, const SlotId slotToEquip,
               const vector<unsigned int>& genInvIndexes, Engine& eng);

void drawUse(const MenuBrowser& browser,
             const vector<unsigned int>& genInvIndexes, Engine& eng);

} //RenderInventory

#endif

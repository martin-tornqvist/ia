#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "InventoryHandling.h"

#include "ItemData.h"
#include "MenuBrowser.h"

namespace RenderInventory
{

const int EQP_Y0  = 2;
const int EQP_Y1  = EQP_Y0 + int(SlotId::END) - 1;
const int INV_Y0  = EQP_Y1 + 2;
const int INV_Y1  = SCREEN_H - 2;
const int INV_H   = INV_Y1 - INV_Y0 + 1;

void drawBrowseInv(const MenuBrowser& browser);

void drawEquip(const MenuBrowser& browser, const SlotId slotIdToEquip,
               const std::vector<size_t>& genInvIndexes);

} //RenderInventory

#endif

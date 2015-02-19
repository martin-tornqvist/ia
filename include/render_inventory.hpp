#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "inventory_handling.hpp"

#include "item_data.hpp"
#include "menu_browser.hpp"

namespace Render_inventory
{

const int EQP_Y0  = 2;
const int EQP_Y1  = EQP_Y0 + int(Slot_id::END) - 1;
const int INV_Y0  = EQP_Y1 + 2;
const int INV_Y1  = SCREEN_H - 2;
const int INV_H   = INV_Y1 - INV_Y0 + 1;

void draw_browse_inv(const Menu_browser& browser);

void draw_equip(const Menu_browser& browser, const Slot_id slot_id_to_equip,
               const std::vector<size_t>& gen_inv_indexes);

} //Render_inventory

#endif

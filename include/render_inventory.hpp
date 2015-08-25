#ifndef RENDER_INVENTORY_H
#define RENDER_INVENTORY_H

#include "inventory_handling.hpp"

#include "item_data.hpp"

class Menu_browser;

namespace render_inv
{

extern const int INV_H;

void draw_inv(const Menu_browser& browser);

void draw_apply(const Menu_browser& browser,
                const std::vector<size_t>& gen_inv_indexes);

void draw_equip(const Menu_browser& browser, const Slot_id slot_id_to_equip,
                const std::vector<size_t>& gen_inv_indexes);

} //render_inv

#endif

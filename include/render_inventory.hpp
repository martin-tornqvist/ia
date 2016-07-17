#ifndef RENDER_INVENTORY_HPP
#define RENDER_INVENTORY_HPP

#include "inventory_handling.hpp"

#include "item_data.hpp"

class MenuBrowser;

namespace render_inv
{

extern const int inv_h;

void draw_inv(const MenuBrowser& browser);

void draw_apply(const MenuBrowser& browser,
                const std::vector<size_t>& gen_inv_indexes);

void draw_equip(const MenuBrowser& browser, const SlotId slot_id_to_equip,
                const std::vector<size_t>& gen_inv_indexes);

} //render_inv

#endif

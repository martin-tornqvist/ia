#ifndef INV_HANDLING_HPP
#define INV_HANDLING_HPP

#include "inventory.hpp"

enum class InvScr
{
    inv,
    equip,
    apply,
    none
};

namespace inv_handling
{

extern InvScr      scr_to_open_on_new_turn;
extern InvSlot*    equip_slot_to_open_on_new_turn;
extern int          browser_idx_to_set_on_new_turn;

void init();

void run_inv_screen();

void run_apply_screen();

bool run_equip_screen(InvSlot& slot_to_equip);

void activate(const size_t GENERAL_ITEMS_element);

} //inv_handling

#endif

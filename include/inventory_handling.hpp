#ifndef INV_HANDLING
#define INV_HANDLING

#include "inventory.hpp"

enum class Inv_scr_id       {inv, equip, END};

enum class Consume_item    {yes, no};

enum class Unequip_allowed {yes, no};

namespace inv_handling
{

extern Inv_scr_id screen_to_open_after_drop;
extern Inv_slot* equip_slot_to_open_after_drop;
extern int      browser_idx_to_set_after_drop;

void init();

void run_inv_screen();
bool run_equip_screen(Inv_slot& slot_to_equip);

void activate(const size_t GENERAL_ITEMS_ELEMENT);

} //Inv_handling

#endif

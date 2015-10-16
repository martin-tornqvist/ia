#ifndef DROP_H
#define DROP_H

#include "cmn_types.hpp"

class Item;
class Actor;

namespace item_drop
{

void drop_all_characters_items(Actor& actor);

//This function places the item as close to the origin as possible, but never on top of
//other items, unless they can be stacked.
Item* drop_item_on_map(const P& intended_pos, Item& item);

void try_drop_item_from_inv(Actor& actor, const Inv_type inv_type, const size_t IDX,
                            const int NR_ITEMS_TO_DROP = -1);

} //Item_drop

#endif

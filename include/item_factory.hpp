#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include <string>

#include "item.hpp"

namespace item_factory
{

Item* mk(const Item_id item_id, const int NR_ITEMS = 1);

void set_item_randomized_properties(Item* item);

//TODO: Terrible function, refactor. Perhaps use a vector of Item_type as parameter?
Item* mk_random_scroll_or_potion(const bool ALLOW_SCROLLS, const bool ALLOW_POTIONS);

Item* mk_item_on_floor(const Item_id item_id, const Pos& pos);

Item* copy_item(Item* old_item);

} //Item_factory

#endif

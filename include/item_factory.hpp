#ifndef ITEM_FACTORY_HPP
#define ITEM_FACTORY_HPP

#include <string>

#include "item.hpp"

namespace item_factory
{

Item* mk(const Item_id item_id, const int nr_items = 1);

void set_item_randomized_properties(Item* item);

//TODO: Terrible function, refactor!! Perhaps use a vector of Item_type as parameter?
Item* mk_random_scroll_or_potion(const bool allow_scrolls, const bool allow_potions);

Item* mk_item_on_floor(const Item_id item_id, const P& pos);

Item* copy_item(const Item& item_to_copy);

} //item_factory

#endif

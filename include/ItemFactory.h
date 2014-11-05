#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include <string>

#include "Item.h"

namespace ItemFactory
{

Item* mk(const ItemId itemId, const int NR_ITEMS = 1);

void setItemRandomizedProperties(Item* item);

//TODO Ugly function, refactor. Perhaps use a vector of ItemType as parameter?
//(Then ItemDataT must store ItemType)
Item* mkRandomScrollOrPotion(const bool ALLOW_SCROLLS, const bool ALLOW_POTIONS);

Item* mkItemOnMap(const ItemId itemId, const Pos& pos);

Item* copyItem(Item* oldItem);

} //ItemFactory

#endif

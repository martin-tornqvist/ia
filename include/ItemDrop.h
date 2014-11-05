#ifndef ITEM_DROP_H
#define ITEM_DROP_H

#include <iostream>

#include "CmnTypes.h"

class Item;
class Actor;

namespace ItemDrop
{

void dropAllCharactersItems(Actor& actor);

//This function places the item as close to the origin as possible, but never on top of
//other items, unless they can be stacked.
Item* dropItemOnMap(const Pos& intendedPos, Item& item);

void dropItemFromInv(Actor& actor, const InvList invList, const size_t ELEMENT,
                     const int NR_ITEMS_TO_DROP = -1);

} //ItemDrop

#endif

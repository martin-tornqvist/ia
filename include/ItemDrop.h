#ifndef ITEM_DROP_H
#define ITEM_DROP_H

#include <iostream>

#include "CmnTypes.h"

class Item;
class Actor;

namespace ItemDrop {

void dropAllCharactersItems(Actor* actor, bool died);

//This function places the item as close to the origin as possible, but not
//on top of other items, unless they can be stacked.
Item* dropItemOnMap(const Pos& intendedPos, Item& item);

void dropItemFromInv(Actor* actorDropping, const int ELEMENT,
                     const int NR_ITEMS_TO_DROP = -1);

} //ItemDrop

#endif

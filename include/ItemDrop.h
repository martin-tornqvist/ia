#ifndef ITEM_DROP_H
#define ITEM_DROP_H

#include <iostream>

#include "CommonTypes.h"

class Engine;

class Item;
class Actor;

class ItemDrop {
public:
  ItemDrop(Engine* engine) {
    eng = engine;
  }

  void dropAllCharactersItems(Actor* actor, bool died);

  //This function places the item as close to the origin as possible, but not
  //on top of other items, unless they can be stacked, in which case the
  //parameter item is destroyed.
  Item* dropItemOnMap(const Pos& intendedPos, Item& item);

  void dropItemFromInventory(Actor* actorDropping, const int ELEMENT,
                             const int NR_ITEMS_TO_DROP = -1);

private:
  Engine* eng;
};

#endif

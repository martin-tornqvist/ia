#ifndef ITEM_DROP_H
#define ITEM_DROP_H

#include <iostream>

#include "ConstTypes.h"

class Engine;

class Item;
class Actor;

class ItemDrop {
public:
  ItemDrop(Engine* engine) {
    eng = engine;
  }

  void dropAllCharactersItems(Actor* actor, bool died);

  // This function takes a pointer to an Item pointer, and places the item pointer on the map.
  // It places the item pointer as close to the origin as possible, but not on top of other items,
  // unless they can be stacked, in which case the parameter item is destroyed.
  void dropItemOnMap(const coord pos, Item** item);

  void dropItemFromInventory(Actor* actorDropping, const int ELEMENT, const int NR_ITEMS_TO_DROP = -1);

private:
  Engine* eng;
};

#endif

#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include <string>

#include "Item.h"

using namespace std;

class Engine;

class ItemFactory {
public:
  ItemFactory(Engine& engine) : eng(engine) {}

  Item* spawnItem(const ItemId itemId, const int NR_ITEMS = 1);

  void setItemRandomizedProperties(Item* item);

  Item* spawnRandomScrollOrPotion(const bool ALLOW_SCROLLS,
                                  const bool ALLOW_POTIONS);

  Item* spawnItemOnMap(const ItemId itemId, const Pos& pos);

  Item* copyItem(Item* oldItem);

private:
  Engine& eng;
};

#endif

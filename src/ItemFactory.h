#ifndef ITEM_FACTORY_H
#define ITEM_FACTORY_H

#include <string>

#include "Item.h"

using namespace std;

class Engine;

class ItemFactory {
public:
  ItemFactory(Engine* engine) {
    eng = engine;
  }

  Item* spawnItem(ItemDevNames_t devName);

  Item* spawnRandomScrollOrPotion(const bool ALLOW_SCROLLS, const bool ALLOW_POTIONS);

//  Item* spawnRandomItemRelatedToSpecialRoom(const SpecialRoom_t roomType);

  Item* spawnItemOnMap(ItemDevNames_t devName, const coord pos);

  Item* copyItem(Item* oldItem);

  //void deleteAllItemsOnGround();

private:
  Engine* eng;
};

#endif

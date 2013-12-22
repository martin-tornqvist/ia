#ifndef ITEM_PICKUP_H
#define ITEM_PICKUP_H

class Engine;

class Item;
class Actor;

#include "Inventory.h"

class ItemPickup {
public:
  ItemPickup(Engine& engine) : eng(engine) {}

  void tryPick();

  void tryUnloadWeaponOrPickupAmmoFromGround();

private:
  void pickupEffects(Actor* actor, Item* item);

  bool isInvFull(Inventory& inv, Item& item) const;

  Engine& eng;
};

#endif

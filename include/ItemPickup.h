#ifndef ITEM_PICKUP_H
#define ITEM_PICKUP_H



class Item;
class Actor;

#include "Inventory.h"

class ItemPickup {
public:
  ItemPickup() {}

  void tryPick();

  void tryUnloadWeaponOrPickupAmmoFromGround();

private:
  void pickupEffects(Actor* actor, Item* item);

  bool isInvFull(Inventory& inv, Item& item) const;


};

#endif

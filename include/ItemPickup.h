#ifndef ITEM_PICKUP_H
#define ITEM_PICKUP_H

#include "Inventory.h"

class Item;
class Actor;

namespace ItemPickup
{

void tryPick();

void tryUnloadWpnOrPickupAmmo();

} //ItemPickup

#endif

#ifndef PICKUP_H
#define PICKUP_H

class Item;
class Actor;
class Ammo;
class Wpn;

namespace ItemPickup
{

void tryPick();

void tryUnloadWpnOrPickupAmmo();

Ammo* unloadRangedWpn(Wpn& wpn);

} //ItemPickup

#endif

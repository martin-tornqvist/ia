#ifndef RELOAD_H
#define RELOAD_H

class Actor;
class Item;
class Weapon;

enum ReloadResult {
  reloadResult_success,
  reloadResult_noAmmo,
  reloadResult_notCarryingWpn,
  reloadResult_alreadyFull,
  reloadResult_wpnNotUsingAmmo,
  reloadResult_fumble
};

class Reload {
public:
  Reload() {}

  bool reloadWieldedWpn(Actor& actorReloading);

private:
  void printMsgAndPlaySfx(Actor& actorReloading, Weapon* const wpn,
                          Item* const ammo, const ReloadResult result,
                          const bool IS_SWIFT_RELOAD);
};


#endif

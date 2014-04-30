#ifndef RELOAD_H
#define RELOAD_H

class Actor;

enum class ReloadResult {
  success,
  noAmmo,
  notCarryingWpn,
  alreadyFull,
  wpnNotUsingAmmo,
  fumble
};

namespace Reload {

bool reloadWieldedWpn(Actor& actorReloading);

} //Reload

#endif

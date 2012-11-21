#ifndef RELOAD_H
#define RELOAD_H

class Engine;

class Actor;
class Item;
class Weapon;

enum ReloadResult_t {
  reloadResult_success,
  reloadResult_noAmmo,
  reloadResult_alreadyFull,
  reloadResult_weaponNotUsingAmmo,
  reloadResult_fumble
};

class Reload
{
public:
  Reload(Engine* engine) : eng(engine) {}

  bool reloadWeapon(Actor* actorReloading);

private:
  void printReloadMessages(Actor* actorReloading, Weapon* weapon, Item* ammoItem, ReloadResult_t result, bool isSwift);

  Engine* eng;
};


#endif

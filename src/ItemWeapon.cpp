#include "ItemWeapon.h"

#include "Explosion.h"
#include "Utils.h"

Weapon::Weapon(ItemDataT* const itemData, ItemDataT* const ammoData) :
  Item(itemData), ammoData_(ammoData) {
  nrAmmoLoaded = 0;
  ammoCapacity = 0;
  effectiveRangeLimit = 3;
  clip = false;
  meleeDmgPlus = 0;
}

void Weapon::setRandomMeleePlus() {
  meleeDmgPlus = 0;

  int chance = 45;
  while(Rnd::percentile() < chance && meleeDmgPlus < 3) {
    meleeDmgPlus++;
    chance -= 5;
  }
}

void Incinerator::weaponSpecific_projectileObstructed(
  const Pos& pos, Actor* actorHit) {
  (void)actorHit;
  Explosion::runExplosionAt(pos, ExplType::expl);
}

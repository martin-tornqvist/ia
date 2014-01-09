#include "ItemWeapon.h"
#include "Engine.h"

#include "Explosion.h"

Weapon::Weapon(ItemData* const itemData, ItemData* const ammoData,
               Engine& engine) :
  Item(itemData, engine), ammoData_(ammoData) {
  nrAmmoLoaded = 0;
  ammoCapacity = 0;
  effectiveRangeLimit = 3;
  clip = false;
  meleeDmgPlus = 0;
}

void Weapon::setRandomMeleePlus() {
  meleeDmgPlus = 0;

  int chance = 45;
  while(eng.dice.percentile() < chance && meleeDmgPlus < 3) {
    meleeDmgPlus++;
    chance -= 5;
  }
}

void Incinerator::weaponSpecific_projectileObstructed(
  const Pos& pos, Actor* actorHit) {
  (void)actorHit;
  Explosion::runExplosionAt(
    pos, eng, 0, sfxExplosion, true, new PropBurning(eng, propTurnsStd));
}

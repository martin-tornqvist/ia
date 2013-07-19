#include "ItemWeapon.h"
#include "Engine.h"

#include "Explosion.h"

void Weapon::setRandomMeleePlus(Engine* const engine) {
  int plus = 0;

  int chance = 45;
  while(engine->dice.percentile() < chance) {
    plus++;
    chance -= 5;
  }

  meleeDmgPlus = plus;
}

void Incinerator::weaponSpecific_projectileObstructed(int originX, int originY, Actor* actorHit, Engine* engine) {
  (void)actorHit;
  engine->explosionMaker->runExplosion(Pos(originX, originY), true, new StatusBurning(engine));
}

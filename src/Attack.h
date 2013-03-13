#ifndef ATTACK_H
#define ATTACK_H

#include <vector>
#include <math.h>
#include <iostream>

#include "AbilityValues.h"
#include "ItemData.h"
#include "ActorData.h"
#include "Art.h"

class Engine;

class Actor;
class Weapon;

enum ProjectileHitType_t {
  projectileHitType_miss,
  projectileHitType_strayHit,
  projectileHitType_cleanHit
};

struct AttackData {
  AttackData() : aimLevel(actorSize_none) {
  }

  bool isBackStab, isWeakAttack;
  string weaponName_a;
  string verbPlayerAttacksMissile;
  string verbOtherAttacksMissile;
  Actor* attacker;
  Actor* currentDefender;
  int attackerX, attackerY;
  Abilities_t abilityUsed;
  bool isIntrinsic;
  bool isMelee;
  int wpnBaseSkill, attackSkill, totalSkill;
  AbilityRollResult_t attackResult;
  int dmgRolls, dmgSides, dmgPlus;
  int dmgRoll, dmg;
  string dmgDescript;
  bool isPlayerAttacking;
  bool isDefenderDodging;
  ActorSizes_t aimLevel;
  ActorSizes_t currentDefenderSize;
  bool isTargetEthereal;
};

struct Projectile {
  Projectile() : pos(coord(-1, -1)), isObstructed(false), isVisibleToPlayer(true),
    actorHit(NULL), obstructedInElement(-1), isDoneRendering(false),
    glyph(-1), tile(tile_empty), clr(clrWhite) {
  }

  void setTile(const Tile_t tileToRender, const sf::Color clrToRender) {
    tile = tileToRender;
    clr = clrToRender;
  }

  void setGlyph(const char GLYPH_TO_RENDER, const sf::Color clrToRender) {
    glyph = GLYPH_TO_RENDER;
    clr = clrToRender;
  }

  coord pos;
  bool isObstructed;
  bool isVisibleToPlayer;
  Actor* actorHit;
  int obstructedInElement;
  bool isDoneRendering;
  char glyph;
  Tile_t tile;
  sf::Color clr;
  AttackData data;
};

class Attack {
public:
  Attack(Engine* engine) :
    shotgunSpreadAngleHalf(asin(0.5 / sqrt(12.5))),
    eng(engine) {}

  bool ranged(int attackX, int attackY, Weapon* weapon);
  void melee(const coord& defenderPos, Weapon* weapon);

private:
  double shotgunSpreadAngleHalf;

  void getAttackData(AttackData& data, const coord& target, const coord& currentPos, Weapon* const weapon, const bool IS_MELEE);

  void printMeleeMessages(AttackData data, Weapon* weapon);

  void printRangedInitiateMessages(AttackData data);
  void printProjectileAtActorMessages(AttackData data, ProjectileHitType_t hitType);

  void projectileFire(const coord& origin, coord target, Weapon* const weapon, const unsigned int NR_OF_PROJECTILES);

  void shotgun(const coord& origin, const coord& target, Weapon* const weapon);

  bool isCellOnLine(vector<coord> line, int x, int y);

  Engine* eng;
};

#endif


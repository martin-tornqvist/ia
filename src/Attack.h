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

class AttackData {
public:
  Actor* attacker;
  Actor* currentDefender;
  AbilityRollResult_t attackResult;
  int dmgRolls, dmgSides, dmgPlus;
  int dmgRoll, dmg;
  bool isDefenderDodging;

protected:
  AttackData(Engine* engine) : eng(engine) {}

  Engine* const eng;

private:
  AttackData() {}
};

class MeleeAttackData: public AttackData {
public:
  MeleeAttackData() : AttackData() {}

  bool isBackStab, isWeakAttack;
};

class RangedAttackData: public AttackData {
public:
  RangedAttackData() : AttackData(), aimLevel(actorSize_none) {
  }

  coord         attackPos;
  ActorSizes_t  aimLevel;
  ActorSizes_t  currentDefenderSize;
  string        verbPlayerAttacks;
  string        verbOtherAttacks;
};

struct Projectile {
  Projectile() : pos(coord(-1, -1)), isObstructed(false), isVisibleToPlayer(true),
    actorHit(NULL), obstructedInElement(-1), isDoneRendering(false),
    glyph(-1), tile(tile_empty), clr(clrWhite) {
  }

  void setTile(const Tile_t tileToRender, const SDL_Color clrToRender) {
    tile = tileToRender;
    clr = clrToRender;
  }

  void setGlyph(const char GLYPH_TO_RENDER, const SDL_Color clrToRender) {
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
  SDL_Color clr;
  RangedAttackData data;
};

class Attack {
public:
  Attack(Engine* engine) : eng(engine) {}

  bool ranged(const Actor& attacker, const coord& attackPos, Weapon& wpn);

  void melee(const Actor& attacker, Actor& defender, const Weapon& wpn);

  void getRangedHitChance(const Actor& attacker, const Actor& defender,
                          const Weapon& wpn);

private:
  void printMeleeMessages(const MeleeAttackData& data);

  void printRangedInitiateMessages(const RangedAttackData& data);
  void printProjectileAtActorMessages(const RangedAttackData& data,
                                      const ProjectileHitType_t hitType);

  void projectileFire(const Actor& attacker, const coord& target,
                      const Weapon& weapon);

  void shotgun(const Actor& attacker, const coord& target, const Weapon& weapon);

  bool isCellOnLine(vector<coord> line, int x, int y);

  Engine* eng;
};

#endif


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

class AttackData {
public:
  Actor* attacker;
  Actor* currentDefender;
  AbilityRollResult_t attackResult;
  int dmgRolls, dmgSides, dmgPlus;
  int dmgRoll, dmg;
  bool isIntrinsicAttack;

protected:
  AttackData(Actor& attacker_, const Item& itemAttackedWith_, Engine* engine);
  Engine* const eng;
};

class MeleeAttackData: public AttackData {
public:
  MeleeAttackData(Actor& attacker_, const Weapon& wpn_,
                  Actor& defender_, Engine* engine);
  bool isDefenderDodging;
  bool isBackstab;
  bool isWeakAttack;
};

class RangedAttackData: public AttackData {
public:
  RangedAttackData(Actor& attacker_, const Weapon& wpn_, const Pos& aimPos_,
                   const Pos& curPos_, Engine* engine,
                   ActorSizes_t intendedAimLevel_ = actorSize_none);
  int           hitChanceTot;
  ActorSizes_t  intendedAimLevel;
  ActorSizes_t  currentDefenderSize;
  string        verbPlayerAttacks;
  string        verbOtherAttacks;
};

class MissileAttackData: public AttackData {
public:
  MissileAttackData(Actor& attacker_, const Item& item_, const Pos& aimPos_,
                    const Pos& curPos_, Engine* engine,
                    ActorSizes_t intendedAimLevel_ = actorSize_none);
  int           hitChanceTot;
  ActorSizes_t  intendedAimLevel;
  ActorSizes_t  currentDefenderSize;
};

struct Projectile {
  Projectile() : pos(Pos(-1, -1)), isObstructed(false), isVisibleToPlayer(true),
    actorHit(NULL), obstructedInElement(-1), isDoneRendering(false),
    glyph(-1), tile(tile_empty), clr(clrWhite), attackData(NULL) {}

  ~Projectile() {
    if(attackData != NULL) {
      delete attackData;
    }
  }

  void setAttackData(RangedAttackData* attackData_) {
    if(attackData != NULL) {
      delete attackData;
    }
    attackData = attackData_;
  }

  void setTile(const Tile_t tileToRender, const SDL_Color clrToRender) {
    tile = tileToRender;
    clr = clrToRender;
  }

  void setGlyph(const char GLYPH_TO_RENDER, const SDL_Color clrToRender) {
    glyph = GLYPH_TO_RENDER;
    clr = clrToRender;
  }

  Pos pos;
  bool isObstructed;
  bool isVisibleToPlayer;
  Actor* actorHit;
  int obstructedInElement;
  bool isDoneRendering;
  char glyph;
  Tile_t tile;
  SDL_Color clr;
  RangedAttackData* attackData;
};

class Attack {
public:
  Attack(Engine* engine) : eng(engine) {}

  bool ranged(Actor& attacker, Weapon& wpn, const Pos& aimPos);

  void melee(Actor& attacker, const Weapon& wpn, Actor& defender);

  void getRangedHitChance(const Actor& attacker, const Actor& defender,
                          const Weapon& wpn);

private:
  void printMeleeMessages(const MeleeAttackData& data, const Weapon& wpn);

  void printRangedInitiateMessages(const RangedAttackData& data) const;
  void printProjectileAtActorMessages(const RangedAttackData& data,
                                      const bool IS_HIT) const;

  void projectileFire(Actor& attacker, Weapon& wpn, const Pos& aimPos);

  void shotgun(Actor& attacker, const Weapon& wpn, const Pos& aimPos);

  bool isCellOnLine(vector<Pos> line, int x, int y);

  Engine* eng;
};

#endif


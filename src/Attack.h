#ifndef ATTACK_H
#define ATTACK_H

#include <vector>
#include <math.h>
#include <iostream>

#include "SDL/SDL.h"

#include "AbilityValues.h"
#include "ItemData.h"
#include "ActorData.h"

class Engine;

class Actor;
class Weapon;

enum ProjectileHitType_t {
    projectileHitType_miss,
    projectileHitType_strayHit,
    projectileHitType_cleanHit
};

struct AttackData {
    AttackData() : aimLevel(actorSize_none) {}
    bool				isSwiftAttack; //free turn
    bool				isMightyAttack; //full damage
    bool				isBackStab;
    string              weaponName_a;
    string              verbPlayerAttacksMissile;
    string              verbOtherAttacksMissile;
    Actor*              attacker;
    Actor*              currentDefender;
    int                 attackerX, attackerY;
    Abilities_t         abilityUsed;
    bool                isIntrinsic;
    bool                isMelee;
    int                 wpnBaseSkill, attackSkill, totalSkill;
    AbilityRollResult_t attackResult;
    int                 dmgRolls, dmgSides, dmgPlus;
    int                 dmgRoll, dmg;
    string              dmgDescript;
    bool                isPlayerAttacking;
    bool                isDefenderDodging;
    ActorSizes_t        aimLevel;
    ActorSizes_t        currentDefenderSize;
    bool                isTargetEthereal;
};

class Attack {
public:
    Attack(Engine* engine) :
        shotgunSpreadAngleHalf(asin( 0.5 / sqrt(12.5))),
        eng(engine) {}

    bool ranged(int attackX, int attackY, Weapon* weapon);
    void melee(int defenderX, int defenderY, Weapon* weapon);

private:
    double shotgunSpreadAngleHalf;

    void getAttackData(AttackData &data, const coord target, const coord currentPos, Weapon* const weapon, const bool IS_MELEE);

    void printMeleeMessages(AttackData data, Weapon* weapon);

    void printRangedInitiateMessages(AttackData data);
    void printProjectileAtActorMessages(AttackData data, ProjectileHitType_t hitType);

    void projectileFire(const coord origin, coord target, Weapon* const weapon, const unsigned int NR_OF_PROJECTILES);

    void shotgun(const coord origin, const coord target, Weapon* const weapon);

    bool isCellOnLine(vector<coord> line, int x, int y);

    Engine* eng;
};

#endif


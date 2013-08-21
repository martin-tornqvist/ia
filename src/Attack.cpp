#include "Attack.h"

#include <cassert>

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureTrap.h"
#include "PlayerBonuses.h"

using namespace std;

AttackData::AttackData(Actor& attacker_, const Item& itemAttackedWith_,
                       Engine* engine) :
  attacker(&attacker_), currentDefender(NULL), attackResult(failSmall),
  dmgRolls(0), dmgSides(0), dmgPlus(0), dmgRoll(0), dmg(0),
  isIntrinsicAttack(false), eng(engine) {
  isIntrinsicAttack = itemAttackedWith_.getDef().isIntrinsic;
}

MeleeAttackData::MeleeAttackData(Actor& attacker_, const Weapon& wpn_,
                                 Actor& defender_, Engine* engine) :
  AttackData(attacker_, wpn_, engine), isDefenderDodging(false), isBackstab(false),
  isWeakAttack(false) {
  currentDefender = &defender_;

  const Pos& defenderPos = currentDefender->pos;

  bool isDefenderAware = true;
  if(attacker == eng->player) {
    isDefenderAware =
      dynamic_cast<Monster*>(currentDefender)->playerAwarenessCounter > 0;
  } else {
    isDefenderAware =
      eng->player->checkIfSeeActor(*attacker, NULL) ||
      eng->playerBonHandler->isBonPicked(playerBon_vigilant);
  }

  isDefenderDodging = false;
  if(currentDefender->getDef()->canDodge) {
    const int DEFENDER_DODGE_SKILL =
      currentDefender->getDef()->abilityVals.getVal(
        ability_dodgeAttack, true, *currentDefender);

    const int DODGE_MOD_AT_FEATURE =
      eng->map->featuresStatic[defenderPos.x][defenderPos.y]->getDodgeModifier();

    const int DODGE_CHANCE_TOT = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

    if(DODGE_CHANCE_TOT > 0) {
      if(isDefenderAware) {
        isDefenderDodging = eng->abilityRoll->roll(DODGE_CHANCE_TOT) >= successSmall;
      }
    }
  }

  if(isDefenderDodging == false) {
    //--------------------------------------- DETERMINE ATTACK RESULT
    isBackstab = false;

    const int ATTACKER_SKILL      = attacker->getDef()->abilityVals.getVal(
                                      ability_accuracyMelee, true, *attacker);
    const int WPN_HIT_CHANCE_MOD  = wpn_.getDef().meleeHitChanceMod;

    int hitChanceTot              = ATTACKER_SKILL + WPN_HIT_CHANCE_MOD;

    bool isAttackerAware = true;
    if(attacker == eng->player) {
      isAttackerAware = eng->player->checkIfSeeActor(*currentDefender, NULL);
    } else {
      isAttackerAware = dynamic_cast<Monster*>(attacker)->playerAwarenessCounter > 0;
    }

    bool isDefenderHeldByWeb = false;
    const FeatureStatic* const f =
      eng->map->featuresStatic[defenderPos.x][defenderPos.y];
    if(f->getId() == feature_trap) {
      const Trap* const t = dynamic_cast<const Trap*>(f);
      if(t->getTrapType() == trap_spiderWeb) {
        const TrapSpiderWeb* const web =
          dynamic_cast<const TrapSpiderWeb*>(t->getSpecificTrap());
        if(web->isHolding()) {
          isDefenderHeldByWeb = true;
        }
      }
    }
    if(isAttackerAware) {
      StatusHandler* const defenderStatusHandler =
        currentDefender->getStatusHandler();
      if(
        (isDefenderAware == false ||
         isDefenderHeldByWeb ||
         defenderStatusHandler->hasEffect(statusParalyzed)  ||
         defenderStatusHandler->hasEffect(statusNailed)     ||
         defenderStatusHandler->hasEffect(statusFainted))) {
        hitChanceTot += 50;
      }
      if(
        defenderStatusHandler->allowSee() == false        ||
        defenderStatusHandler->hasEffect(statusConfused)  ||
        defenderStatusHandler->hasEffect(statusSlowed)    ||
        defenderStatusHandler->hasEffect(statusBurning)) {
        hitChanceTot += 20;
      }
    }

    attackResult = eng->abilityRoll->roll(hitChanceTot);

    //--------------------------------------- DETERMINE DAMAGE
    dmgRolls  = wpn_.getDef().meleeDmg.first;
    dmgSides  = wpn_.getDef().meleeDmg.second;
    dmgPlus   = wpn_.meleeDmgPlus;

    isWeakAttack = false;
    if(attacker->getStatusHandler()->hasEffect(statusWeak)) {
      //Weak attack (min damage)
      dmgRoll = dmgRolls;
      dmg = dmgRoll + dmgPlus;
      isWeakAttack = true;
    } else if(isAttackerAware && isDefenderAware == false) {
      //Backstab (Above max damage)
      dmgRoll = dmgRolls * dmgSides;
      dmg = ((dmgRoll + dmgPlus) * 3) / 2;
      isBackstab = true;
    } else if(attackResult == successCritical) {
      //Critical hit (max damage)
      dmgRoll = dmgRolls * dmgSides;
      dmg = max(0, dmgRoll + dmgPlus);
    } else if(attackResult >= successSmall) {
      //Normal hit
      dmgRoll = eng->dice(dmgRolls, dmgSides);
      dmg = max(0, dmgRoll + dmgPlus);
    }
  }
}

RangedAttackData::RangedAttackData(Actor& attacker_, const Weapon& wpn_,
                                   const Pos& aimPos_, const Pos& curPos_,
                                   Engine* engine, ActorSizes_t intendedAimLevel_) :
  AttackData(attacker_, wpn_, engine), hitChanceTot(0),
  intendedAimLevel(actorSize_none), currentDefenderSize(actorSize_none),
  verbPlayerAttacks(""), verbOtherAttacks("")  {

  verbPlayerAttacks = wpn_.getDef().rangedAttackMessages.player;
  verbOtherAttacks  = wpn_.getDef().rangedAttackMessages.other;

  Actor* const actorAimedAt = eng->mapTests->getActorAtPos(aimPos_);

  //If aim level parameter not given, determine it now
  if(intendedAimLevel_ == actorSize_none) {
    if(actorAimedAt != NULL) {
      intendedAimLevel = actorAimedAt->getDef()->actorSize;
    } else {
      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeShootBlockerFeaturesArray(blockers);
      intendedAimLevel = blockers[curPos_.x][curPos_.y] ?
                         actorSize_humanoid : actorSize_floor;
    }
  } else {
    intendedAimLevel = intendedAimLevel_;
  }

  currentDefender = eng->mapTests->getActorAtPos(curPos_);

  if(currentDefender != NULL) {
    tracer << "RangedAttackData: Defender found" << endl;
    const int ATTACKER_SKILL      = attacker->getDef()->abilityVals.getVal(
                                      ability_accuracyRanged, true, *attacker);
    const int WPN_HIT_MOD         = wpn_.getDef().rangedHitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(currentDefender->pos);
    const int DIST_TO_TARGET      = eng->basicUtils->chebyshevDistance(
                                      attPos.x, attPos.y, defPos.x, defPos.y);
    const int DIST_HIT_MOD        = 15 - (DIST_TO_TARGET * 5);
    const ActorSpeed_t defSpeed   = currentDefender->getDef()->speed;
    const int SPEED_HIT_MOD =
      defSpeed == actorSpeed_sluggish ?  20 :
      defSpeed == actorSpeed_slow     ?  10 :
      defSpeed == actorSpeed_normal   ?   0 :
      defSpeed == actorSpeed_fast     ? -10 : -30;
    currentDefenderSize           = currentDefender->getDef()->actorSize;
    const int SIZE_HIT_MOD = currentDefenderSize == actorSize_floor ? -10 : 0;
    hitChanceTot = max(5,
                       ATTACKER_SKILL +
                       WPN_HIT_MOD    +
                       DIST_HIT_MOD   +
                       SPEED_HIT_MOD  +
                       SIZE_HIT_MOD);

    attackResult = eng->abilityRoll->roll(hitChanceTot);

    if(attackResult >= successSmall) {
      tracer << "RangedAttackData: Attack roll succeeded" << endl;
      dmgRolls  = wpn_.getDef().rangedDmg.rolls;
      dmgSides  = wpn_.getDef().rangedDmg.sides;
      dmgPlus   = wpn_.getDef().rangedDmg.plus;
      dmgRoll   = eng->dice(dmgRolls, dmgSides);
      dmg       = dmgRoll + dmgPlus;
    }
  }
}

MissileAttackData::MissileAttackData(Actor& attacker_, const Item& item_, const Pos& aimPos_,
                                     const Pos& curPos_, Engine* engine,
                                     ActorSizes_t intendedAimLevel_) :
  AttackData(attacker_, item_, engine), hitChanceTot(0),
  intendedAimLevel(actorSize_none), currentDefenderSize(actorSize_none) {

  Actor* const actorAimedAt = eng->mapTests->getActorAtPos(aimPos_);

  //If aim level parameter not given, determine it now
  if(intendedAimLevel_ == actorSize_none) {
    if(actorAimedAt != NULL) {
      intendedAimLevel = actorAimedAt->getDef()->actorSize;
    } else {
      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeShootBlockerFeaturesArray(blockers);
      intendedAimLevel = blockers[curPos_.x][curPos_.y] ?
                         actorSize_humanoid : actorSize_floor;
    }
  } else {
    intendedAimLevel = intendedAimLevel_;
  }

  currentDefender = eng->mapTests->getActorAtPos(curPos_);

  if(currentDefender != NULL) {
    tracer << "MissileAttackData: Defender found" << endl;
    const int ATTACKER_SKILL      = attacker->getDef()->abilityVals.getVal(
                                      ability_accuracyRanged, true, *attacker);
    const int WPN_HIT_MOD         = item_.getDef().missileHitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(currentDefender->pos);
    const int DIST_TO_TARGET      = eng->basicUtils->chebyshevDistance(
                                      attPos.x, attPos.y, defPos.x, defPos.y);
    const int DIST_HIT_MOD        = 15 - (DIST_TO_TARGET * 5);
    const ActorSpeed_t defSpeed   = currentDefender->getDef()->speed;
    const int SPEED_HIT_MOD =
      defSpeed == actorSpeed_sluggish ?  20 :
      defSpeed == actorSpeed_slow     ?  10 :
      defSpeed == actorSpeed_normal   ?   0 :
      defSpeed == actorSpeed_fast     ? -15 : -35;
    currentDefenderSize           = currentDefender->getDef()->actorSize;
    const int SIZE_HIT_MOD = currentDefenderSize == actorSize_floor ? -15 : 0;
    hitChanceTot = max(5,
                       ATTACKER_SKILL +
                       WPN_HIT_MOD    +
                       DIST_HIT_MOD   +
                       SPEED_HIT_MOD  +
                       SIZE_HIT_MOD);

    attackResult = eng->abilityRoll->roll(hitChanceTot);

    if(attackResult >= successSmall) {
      tracer << "MissileAttackData: Attack roll succeeded" << endl;
      dmgRolls  = item_.getDef().missileDmg.rolls;
      dmgSides  = item_.getDef().missileDmg.sides;
      dmgPlus   = item_.getDef().missileDmg.plus;
      dmgRoll   = eng->dice(dmgRolls, dmgSides);
      dmg       = dmgRoll + dmgPlus;
    }
  }
}


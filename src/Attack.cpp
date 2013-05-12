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

AttackData::AttackData(Actor& attacker_, const Weapon& wpn_, Engine* engine) :
  attacker(&attacker_), eng(engine) {
  isIntrinsicAttack = wpn_.getDef().isIntrinsic;
}

MeleeAttackData::MeleeAttackData(Actor& attacker_, const Weapon& wpn_,
                                 Actor& defender_, Engine* engine) :
  AttackData(attacker_, wpn_, engine) {
  currentDefender = &defender_;

  const coord& defenderPos = currentDefender->pos;

  bool isDefenderAware = true;
  if(attacker == eng->player) {
    isDefenderAware =
      dynamic_cast<Monster*>(currentDefender)->playerAwarenessCounter > 0;
  } else {
    isDefenderAware =
      eng->player->checkIfSeeActor(*attacker, NULL) &&
      eng->playerBonusHandler->isBonusPicked(playerBonus_vigilant) == false;
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
        const TrapSpiderWeb* const web = dynamic_cast<const TrapSpiderWeb*>(t->getSpecificTrap());
        if(web->isHolding()) {
          isDefenderHeldByWeb = true;
        }
      }
    }
    if(isAttackerAware) {
      StatusEffectsHandler* const defenderStatusHandler =
        currentDefender->getStatusEffectsHandler();
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
    if(attacker->getStatusEffectsHandler()->hasEffect(statusWeak)) {
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
                                   const coord& aimPos_, const coord& curPos_,
                                   Engine* engine,
                                   ActorSizes_t intendedAimLevel_) :
  AttackData(attacker_, wpn_, engine) {

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
    const int WPN_HIT_CHANCE_MOD  = wpn_.getDef().rangedHitChanceMod;
    const int TOT_CHANCE_TO_HIT   = ATTACKER_SKILL + WPN_HIT_CHANCE_MOD;
    attackResult = eng->abilityRoll->roll(TOT_CHANCE_TO_HIT);

    if(attackResult >= successSmall) {
      tracer << "RangedAttackData: Attack roll succeeded" << endl;
      dmgRolls  = wpn_.getDef().rangedDmg.rolls;
      dmgSides  = wpn_.getDef().rangedDmg.sides;
      dmgPlus   = wpn_.getDef().rangedDmg.plus;
    }
  }
}


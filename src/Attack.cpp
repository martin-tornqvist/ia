#include "Attack.h"

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureTrap.h"
#include "PlayerBonuses.h"
#include "MapParsing.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Utils.h"

using namespace std;

AttackData::AttackData(Actor& attacker_, const Item& itemAttackedWith_,
                       Engine& engine) :
  attacker(&attacker_), curDefender(NULL), attackResult(failSmall),
  dmgRolls(0), dmgSides(0), dmgPlus(0), dmgRoll(0), dmg(0),
  isIntrinsicAttack(false), isEtherealDefenderMissed(false), eng(engine) {
  isIntrinsicAttack = itemAttackedWith_.getData().isIntrinsic;
}

MeleeAttackData::MeleeAttackData(Actor& attacker_, const Weapon& wpn_,
                                 Actor& defender_, Engine& engine) :
  AttackData(attacker_, wpn_, engine), isDefenderDodging(false),
  isBackstab(false), isWeakAttack(false) {

  curDefender = &defender_;

  const Pos& defPos = curDefender->pos;

  bool isDefenderAware = true;
  if(attacker == eng.player) {
    isDefenderAware =
      dynamic_cast<Monster*>(curDefender)->awareOfPlayerCounter_ > 0;
  } else {
    isDefenderAware =
      eng.player->isSeeingActor(*attacker, NULL) ||
      eng.playerBonHandler->hasTrait(traitVigilant);
  }

  isDefenderDodging = false;
  if(isDefenderAware) {
    const int DEFENDER_DODGE_SKILL =
      curDefender->getData().abilityVals.getVal(
        ability_dodgeAttack, true, *curDefender);

    const int DODGE_MOD_AT_FEATURE =
      eng.map->cells[defPos.x][defPos.y].featureStatic->getDodgeModifier();

    const int DODGE_CHANCE_TOT = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

    if(DODGE_CHANCE_TOT > 0) {
      isDefenderDodging =
        eng.abilityRoll->roll(DODGE_CHANCE_TOT) >= successSmall;
    }
  }

  if(isDefenderDodging == false) {
    //--------------------------------------- DETERMINE ATTACK RESULT
    isBackstab = false;

    const int ATTACKER_SKILL      = attacker->getData().abilityVals.getVal(
                                      ability_accuracyMelee, true, *attacker);
    const int WPN_HIT_CHANCE_MOD  = wpn_.getData().meleeHitChanceMod;

    int hitChanceTot              = ATTACKER_SKILL + WPN_HIT_CHANCE_MOD;

    bool isAttackerAware = true;
    if(attacker == eng.player) {
      isAttackerAware = eng.player->isSeeingActor(*curDefender, NULL);
    } else {
      Monster* const monster = dynamic_cast<Monster*>(attacker);
      isAttackerAware = monster->awareOfPlayerCounter_ > 0;
    }

    PropHandler& defPropHlr = curDefender->getPropHandler();
    vector<PropId> defProps;
    defPropHlr.getAllActivePropIds(defProps);

    if(isAttackerAware) {
      bool isBigBon   = false;
      bool isSmallBon = false;

      const FeatureStatic* const f =
        eng.map->cells[defPos.x][defPos.y].featureStatic;
      if(f->getId() == feature_trap) {
        const Trap* const t = dynamic_cast<const Trap*>(f);
        if(t->getTrapType() == trap_spiderWeb) {
          const TrapSpiderWeb* const web =
            dynamic_cast<const TrapSpiderWeb*>(t->getSpecificTrap());
          if(web->isHolding()) {
            isBigBon = true;
          }
        }
      }

      for(PropId propId : defProps) {
        if(
          isDefenderAware == false ||
          propId == propParalysed  ||
          propId == propNailed     ||
          propId == propFainted) {
          isBigBon = true;
          break;
        }
        if(
          propId == propBlind     ||
          propId == propConfused  ||
          propId == propSlowed    ||
          propId == propBurning) {
          isSmallBon = true;
        }
      }
      if(isBigBon == false && isSmallBon == false) {
        if(defPropHlr.allowSee() == false) {
          isSmallBon = true;
        }
      }

      hitChanceTot += isBigBon ? 50 : isSmallBon ? 20 : 0;
    }

    attackResult = eng.abilityRoll->roll(hitChanceTot);

    //Ethereal target missed?
    if(find(defProps.begin(), defProps.end(), propEthereal) != defProps.end()) {
      isEtherealDefenderMissed = Rnd::fraction(2, 3);
    }

    //--------------------------------------- DETERMINE DAMAGE
    dmgRolls  = wpn_.getData().meleeDmg.first;
    dmgSides  = wpn_.getData().meleeDmg.second;
    dmgPlus   = wpn_.meleeDmgPlus;

    vector<PropId> attProps;
    attacker->getPropHandler().getAllActivePropIds(attProps);

    isWeakAttack = false;
    if(find(attProps.begin(), attProps.end(), propWeakened) != attProps.end()) {
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
      dmgRoll = Rnd::dice(dmgRolls, dmgSides);
      dmg = max(0, dmgRoll + dmgPlus);
    }
  }
}

RangedAttackData::RangedAttackData(
  Actor& attacker_, const Weapon& wpn_, const Pos& aimPos_,
  const Pos& curPos_, Engine& engine, ActorSize intendedAimLevel_) :
  AttackData(attacker_, wpn_, engine), hitChanceTot(0),
  intendedAimLevel(actorSize_none), curDefenderSize(actorSize_none),
  verbPlayerAttacks(""), verbOtherAttacks("")  {

  verbPlayerAttacks = wpn_.getData().rangedAttackMessages.player;
  verbOtherAttacks  = wpn_.getData().rangedAttackMessages.other;

  Actor* const actorAimedAt = Utils::getActorAtPos(aimPos_, eng);

  //If aim level parameter not given, determine it now
  if(intendedAimLevel_ == actorSize_none) {
    if(actorAimedAt != NULL) {
      intendedAimLevel = actorAimedAt->getData().actorSize;
    } else {
      bool blockers[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksProjectiles(eng), blockers);
      intendedAimLevel = blockers[curPos_.x][curPos_.y] ?
                         actorSize_humanoid : actorSize_floor;
    }
  } else {
    intendedAimLevel = intendedAimLevel_;
  }

  curDefender = Utils::getActorAtPos(curPos_, eng);

  if(curDefender != NULL) {
    trace << "RangedAttackData: Defender found" << endl;
    const int ATTACKER_SKILL    = attacker->getData().abilityVals.getVal(
                                    ability_accuracyRanged, true, *attacker);
    const int WPN_MOD           = wpn_.getData().rangedHitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(curDefender->pos);
    const int DIST_TO_TGT       = Utils::chebyshevDist(
                                    attPos.x, attPos.y, defPos.x, defPos.y);
    const int DIST_MOD          = 15 - (DIST_TO_TGT * 5);
    const ActorSpeed defSpeed = curDefender->getData().speed;
    const int SPEED_MOD =
      defSpeed == ActorSpeed::sluggish ?  20 :
      defSpeed == ActorSpeed::slow     ?  10 :
      defSpeed == ActorSpeed::normal   ?   0 :
      defSpeed == ActorSpeed::fast     ? -10 : -30;
    curDefenderSize             = curDefender->getData().actorSize;
    const int SIZE_MOD          = curDefenderSize == actorSize_floor ? -10 : 0;

    int unawareDefMod = 0;
    const bool IS_ROGUE = eng.playerBonHandler->getBg() == bgRogue;
    if(attacker == eng.player && curDefender != eng.player && IS_ROGUE) {
      if(dynamic_cast<Monster*>(curDefender)->awareOfPlayerCounter_ <= 0) {
        unawareDefMod = 25;
      }
    }

    hitChanceTot = max(5,
                       ATTACKER_SKILL +
                       WPN_MOD    +
                       DIST_MOD   +
                       SPEED_MOD  +
                       SIZE_MOD   +
                       unawareDefMod);

    constrInRange(5, hitChanceTot, 99);

    attackResult = eng.abilityRoll->roll(hitChanceTot);

    if(attackResult >= successSmall) {
      trace << "RangedAttackData: Attack roll succeeded" << endl;

      vector<PropId> props;
      curDefender->getPropHandler().getAllActivePropIds(props);

      if(find(props.begin(), props.end(), propEthereal) != props.end()) {
        isEtherealDefenderMissed = Rnd::fraction(2, 3);
      }

      bool playerAimX3 = false;
      if(attacker == eng.player) {
        const Prop* const prop =
          attacker->getPropHandler().getProp(propAiming, PropSrc::applied);
        if(prop != NULL) {
          playerAimX3 = dynamic_cast<const PropAiming*>(prop)->isMaxRangedDmg();
        }
      }

      dmgRolls  = wpn_.getData().rangedDmg.rolls;
      dmgSides  = wpn_.getData().rangedDmg.sides;
      dmgPlus   = wpn_.getData().rangedDmg.plus;

      dmgRoll   = playerAimX3 ? dmgRolls * dmgSides :
                  Rnd::dice(dmgRolls, dmgSides);
      dmg       = dmgRoll + dmgPlus;
    }
  }
}

MissileAttackData::MissileAttackData(Actor& attacker_, const Item& item_,
                                     const Pos& aimPos_, const Pos& curPos_,
                                     Engine& engine,
                                     ActorSize intendedAimLevel_) :
  AttackData(attacker_, item_, engine), hitChanceTot(0),
  intendedAimLevel(actorSize_none), curDefenderSize(actorSize_none) {

  Actor* const actorAimedAt = Utils::getActorAtPos(aimPos_, eng);

  //If aim level parameter not given, determine it now
  if(intendedAimLevel_ == actorSize_none) {
    if(actorAimedAt != NULL) {
      intendedAimLevel = actorAimedAt->getData().actorSize;
    } else {
      bool blockers[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksProjectiles(eng), blockers);
      intendedAimLevel = blockers[curPos_.x][curPos_.y] ?
                         actorSize_humanoid : actorSize_floor;
    }
  } else {
    intendedAimLevel = intendedAimLevel_;
  }

  curDefender = Utils::getActorAtPos(curPos_, eng);

  if(curDefender != NULL) {
    trace << "MissileAttackData: Defender found" << endl;
    const int ATTACKER_SKILL    = attacker->getData().abilityVals.getVal(
                                    ability_accuracyRanged, true, *attacker);
    const int WPN_MOD           = item_.getData().missileHitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(curDefender->pos);
    const int DIST_TO_TGT       = Utils::chebyshevDist(
                                    attPos.x, attPos.y, defPos.x, defPos.y);
    const int DIST_MOD          = 15 - (DIST_TO_TGT * 5);
    const ActorSpeed defSpeed = curDefender->getData().speed;
    const int SPEED_MOD =
      defSpeed == ActorSpeed::sluggish ?  20 :
      defSpeed == ActorSpeed::slow     ?  10 :
      defSpeed == ActorSpeed::normal   ?   0 :
      defSpeed == ActorSpeed::fast     ? -15 : -35;
    curDefenderSize             = curDefender->getData().actorSize;
    const int SIZE_MOD          = curDefenderSize == actorSize_floor ? -15 : 0;

    int unawareDefMod = 0;
    const bool IS_ROGUE = eng.playerBonHandler->getBg() == bgRogue;
    if(attacker == eng.player && curDefender != eng.player && IS_ROGUE) {
      if(dynamic_cast<Monster*>(curDefender)->awareOfPlayerCounter_ <= 0) {
        unawareDefMod = 25;
      }
    }

    hitChanceTot = max(5,
                       ATTACKER_SKILL +
                       WPN_MOD    +
                       DIST_MOD   +
                       SPEED_MOD  +
                       SIZE_MOD   +
                       unawareDefMod);

    attackResult = eng.abilityRoll->roll(hitChanceTot);

    if(attackResult >= successSmall) {
      trace << "MissileAttackData: Attack roll succeeded" << endl;

      bool playerAimX3 = false;
      if(attacker == eng.player) {
        const Prop* const prop =
          attacker->getPropHandler().getProp(propAiming, PropSrc::applied);
        if(prop != NULL) {
          playerAimX3 = dynamic_cast<const PropAiming*>(prop)->isMaxRangedDmg();
        }
      }

      dmgRolls  = item_.getData().missileDmg.rolls;
      dmgSides  = item_.getData().missileDmg.sides;
      dmgPlus   = item_.getData().missileDmg.plus;

      dmgRoll   = playerAimX3 ? dmgRolls * dmgSides :
                  Rnd::dice(dmgRolls, dmgSides);
      dmg       = dmgRoll + dmgPlus;
    }
  }
}


#include "Attack.h"

#include <cassert>

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Log.h"
#include "FeatureTrap.h"
#include "PlayerBonuses.h"

using namespace std;

void Attack::getAttackData(AttackData& data, const coord& target, const coord& currentPos, Weapon* const weapon, const bool IS_MELEE) {
  data.isMelee = IS_MELEE;
  data.attacker = eng->gameTime->getCurrentActor();
  data.currentDefender = eng->mapTests->getActorAtPos(currentPos);
  assert(data.currentDefender != NULL || IS_MELEE == false);

  if(data.currentDefender != NULL) {
    data.currentDefenderSize = data.currentDefender->getDef()->actorSize;
  } else {
    data.currentDefenderSize = actorSize_none;
  }

  Actor* const aimedActor = eng->mapTests->getActorAtPos(target);

  //Get intended aim level
  if(aimedActor != NULL) {
    //aimedActorSize = aimedActor->getInstanceDefinition()->actorSize;
    data.aimLevel = aimedActor->getDef()->actorSize;
  } else {
    bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeShootBlockerFeaturesArray(blockers);
    data.aimLevel = blockers[target.x][target.y] ? actorSize_humanoid : actorSize_floor;
  }

  assert(data.aimLevel != actorSize_none);

  data.attackerX = data.attacker->pos.x;
  data.attackerY = data.attacker->pos.y;

  if(data.isMelee == true) {
    data.abilityUsed = weapon->getDef().meleeAbilityUsed;
    data.wpnBaseSkill = weapon->getDef().meleeBaseAttackSkill;
  } else {
    data.abilityUsed = weapon->getDef().rangedAbilityUsed;
    data.wpnBaseSkill = weapon->getDef().rangedBaseAttackSkill;
  }

  data.isIntrinsic = weapon->getDef().isIntrinsic;
  data.attackSkill = data.attacker->getDef()->abilityVals.getVal(data.abilityUsed, true, *(data.attacker));
  data.totalSkill = data.wpnBaseSkill + data.attackSkill;
  data.attackResult = eng->abilityRoll->roll(data.totalSkill);
  data.isPlayerAttacking = data.attacker == eng->player;

  //Dodge melee attack?
  data.isDefenderDodging = false;
  if(data.currentDefender != NULL && IS_MELEE == true) {
    if(data.currentDefender->getDef()->canDodge == true) {
      const int DEFENDER_DODGE_SKILL =
        data.currentDefender->getDef()->abilityVals.getVal(ability_dodgeAttack, true, *(data.currentDefender));

      const coord defenderPos = data.currentDefender->pos;
      const int DODGE_MOD_AT_FEATURE = eng->map->featuresStatic[defenderPos.x][defenderPos.y]->getDodgeModifier();

      const int TOTAL_DODGE = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

      if(TOTAL_DODGE > 0) {
        bool isDefenderAware = true;

        if(data.attacker == eng->player) {
          isDefenderAware = dynamic_cast<Monster*>(data.currentDefender)->playerAwarenessCounter > 0;
        } else {
          isDefenderAware = eng->player->checkIfSeeActor(*data.attacker, NULL);
        }

        if(isDefenderAware) {
          data.isDefenderDodging = eng->abilityRoll->roll(TOTAL_DODGE) >= successSmall;
        }
      }
    }
  }

  //Get weapon damage properties
  if(IS_MELEE == true) {
    data.dmgRolls = weapon->getDef().meleeDmg.first;
    data.dmgSides = weapon->getDef().meleeDmg.second;
    data.dmgPlus = weapon->meleeDmgPlus;
  } else {
    data.dmgRolls = weapon->getDef().rangedDmg.rolls;
    data.dmgSides = weapon->getDef().rangedDmg.sides;
    data.dmgPlus = weapon->getDef().rangedDmg.plus;
  }

  if(data.attackResult == successCritical) {
    data.dmgRoll = data.dmgRolls * data.dmgSides;
    data.dmg = max(0, data.dmgRoll + data.dmgPlus);
  } else {
    data.dmgRoll = eng->dice(data.dmgRolls, data.dmgSides);
    data.dmg = max(0, data.dmgRoll + data.dmgPlus);
  }

  data.isWeakAttack = false;
  data.isBackStab = false;

  if(IS_MELEE) {
    //Check if defender is in a bad situation (stuck, blind, fainted, confused...)
    //If so, give hit chance bonus to attacker if the attacker is aware
    bool isDefenderAware = true;
    if(data.attacker == eng->player) {
      isDefenderAware = dynamic_cast<Monster*>(data.currentDefender)->playerAwarenessCounter > 0;
    } else {
      isDefenderAware = eng->player->checkIfSeeActor(*data.attacker, NULL) &&
                        eng->playerBonusHandler->isBonusPicked(playerBonus_vigilant) == false;
    }
    bool isAttackerAware = true;
    if(data.attacker == eng->player) {
      isAttackerAware = eng->player->checkIfSeeActor(*data.currentDefender, NULL);
    } else {
      isAttackerAware = dynamic_cast<Monster*>(data.attacker)->playerAwarenessCounter > 0;
    }

    bool isDefenderHeldByWeb = false;
    const FeatureStatic* const f = eng->map->featuresStatic[target.x][target.y];
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
      StatusEffectsHandler* const status = data.currentDefender->getStatusEffectsHandler();
      if(
        (isDefenderAware == false ||
         isDefenderHeldByWeb ||
         status->hasEffect(statusParalyzed) ||
         status->hasEffect(statusNailed) ||
         status->hasEffect(statusFainted))) {
        data.totalSkill += 50;
      }
      if(
        status->allowSee() == false ||
        status->hasEffect(statusConfused) ||
        status->hasEffect(statusSlowed) ||
        status->hasEffect(statusBurning)) {
        data.totalSkill += 20;
      }
    }

    //Rolling attack result again after "situation" modifiers
    data.attackResult = eng->abilityRoll->roll(data.totalSkill);

    //Weak attack (due to status effect "weak")?
    if(data.attacker->getStatusEffectsHandler()->hasEffect(statusWeak)) {
      data.dmgRoll = data.dmgRolls;
      data.dmg = data.dmgRoll + data.dmgPlus;
      data.isWeakAttack = true;
    }

    //Backstab damage bonus?
    if(isAttackerAware && data.isWeakAttack == false) {
      if(isDefenderAware == false) {
        data.dmgRoll = data.dmgRolls * data.dmgSides;
        data.dmg = ((data.dmgRoll + data.dmgPlus) * 3) / 2;
        data.isBackStab = true;
      }
    }
  }

  data.weaponName_a = eng->itemData->getItemRef(weapon, itemRef_a, true);
  data.verbPlayerAttacksMissile = weapon->getDef().rangedAttackMessages.player;
  data.verbOtherAttacksMissile = weapon->getDef().rangedAttackMessages.other;

  if(data.currentDefender != NULL) {
    data.isTargetEthereal = data.currentDefender->getStatusEffectsHandler()->isEthereal();
  } else {
    data.isTargetEthereal = false;
  }

  if(data.isTargetEthereal == true) {
    data.attackResult = failSmall;
  }
}

void Attack::printRangedInitiateMessages(AttackData data) {
  if(data.isPlayerAttacking == true)
    eng->log->addMessage("I " + data.verbPlayerAttacksMissile + ".");
  else {
    if(eng->map->playerVision[data.attackerX][data.attackerY] == true) {
      const string attackerName = data.attacker->getNameThe();
      const string attackVerb = data.verbOtherAttacksMissile;
      eng->log->addMessage(attackerName + " " + attackVerb + ".", clrWhite, messageInterrupt_force);
    }
  }

  eng->renderer->drawMapAndInterface();
}

void Attack::printProjectileAtActorMessages(AttackData data, ProjectileHitType_t hitType) {
  //Only print messages if player can see the cell
  const int defX = data.currentDefender->pos.x;
  const int defY = data.currentDefender->pos.y;
  if(eng->map->playerVision[defX][defY]) {
    if(data.isTargetEthereal == true) {
      if(data.currentDefender == eng->player) {
        //Perhaps no text is needed here?
      } else {
        eng->log->addMessage("Projectile hits nothing but void.");
      }
    } else {
      //Punctuation or exclamation marks depending on attack strength
      data.dmgPunctuation = ".";
      const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
      if(MAX_DMG_ROLL >= 4) {
        data.dmgPunctuation = data.dmgRoll > MAX_DMG_ROLL * 5 / 6 ? "!!!" : data.dmgRoll > MAX_DMG_ROLL / 2 ? "!" : data.dmgPunctuation;
      }

      if(hitType == projectileHitType_cleanHit || hitType == projectileHitType_strayHit) {
        if(data.currentDefender == eng->player) {
          eng->log->addMessage("I am hit" + data.dmgPunctuation, clrMessageBad, messageInterrupt_force);

//          if(data.attackResult == successCritical) {
//            eng->log->addMessage("It was a great hit!", clrMessageBad, messageInterrupt_force);
//          }
        } else {
          string otherName = "It";

          if(eng->map->playerVision[defX][defY] == true)
            otherName = data.currentDefender->getNameThe();

          eng->log->addMessage(otherName + " is hit" + data.dmgPunctuation, clrMessageGood);

//          if(data.attackResult == successCritical) {
//            eng->log->addMessage("It was a great hit!", clrMessageGood);
//          }
        }
      }
    }
  }
}

void Attack::printMeleeMessages(AttackData data, Weapon* weapon) {
  //Punctuation or exclamation marks depending on attack strength
  data.dmgPunctuation = ".";
  const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
  if(MAX_DMG_ROLL >= 4) {
    data.dmgPunctuation = data.dmgRoll > MAX_DMG_ROLL * 5 / 6 ? "!!!" : data.dmgRoll > MAX_DMG_ROLL / 2 ? "!" : data.dmgPunctuation;
  }

  string otherName = "";

  if(data.isTargetEthereal == true) {
    if(data.isPlayerAttacking == true) {
      eng->log->addMessage("I hit nothing but void" + data.dmgPunctuation);
    } else {
      if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
        otherName = data.attacker->getNameThe();
      } else {
        otherName = "its";
      }

      eng->log->addMessage("I am unaffected by " + otherName + " attack" + data.dmgPunctuation, clrWhite, messageInterrupt_force);
    }
  } else {
    //----- ATTACK FUMBLE -----
//    if(data.attackResult == failCritical) {
//      if(data.isPlayerAttacking) {
//        eng->log->addMessage("I fumble!");
//      } else {
//        if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
//          otherName = data.attacker->getNameThe();
//        } else {
//          otherName = "It";
//        }
//        eng->log->addMessage(otherName + " fumbles.", clrWhite, messageInterrupt_force);
//      }
//    }

    //----- ATTACK MISS -------
    if(/*data.attackResult > failCritical &&*/ data.attackResult <= failSmall) {
      if(data.isPlayerAttacking) {
        if(data.attackResult == failSmall) {
          eng->log->addMessage("I barely miss!");
        } else if(data.attackResult == failNormal) {
          eng->log->addMessage("I miss.");
        } else if(data.attackResult == failBig) {
          eng->log->addMessage("I miss completely.");
        }
      } else {
        if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }
        if(data.attackResult == failSmall) {
          eng->log->addMessage(otherName + " barely misses me!", clrWhite, messageInterrupt_force);
        } else if(data.attackResult == failNormal) {
          eng->log->addMessage(otherName + " misses me.", clrWhite, messageInterrupt_force);
        } else if(data.attackResult == failBig) {
          eng->log->addMessage(otherName + " misses me completely.", clrWhite, messageInterrupt_force);
        }
      }
    }

    //----- ATTACK HIT -------- //----- ATTACK CRITICAL ---
    if(data.attackResult >= successSmall) {
      if(data.isDefenderDodging) {
        if(data.isPlayerAttacking) {
          if(eng->player->checkIfSeeActor(*data.currentDefender, NULL)) {
            otherName = data.currentDefender->getNameThe();
          } else {
            otherName = "It ";
          }
          eng->log->addMessage(otherName + " dodges my attack.");
        } else {
          if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
            otherName = data.attacker->getNameThe();
          } else {
            otherName = "It";
          }
          eng->log->addMessage("I dodge an attack from " + otherName + ".", clrMessageGood);
        }
      } else {
        if(data.isPlayerAttacking) {
          const string wpnVerb = weapon->getDef().meleeAttackMessages.player;

          if(eng->player->checkIfSeeActor(*data.currentDefender, NULL)) {
            otherName = data.currentDefender->getNameThe();
          } else {
            otherName = "it";
          }

          if(data.isIntrinsic) {
            const string ATTACK_MOD_TEXT = data.isWeakAttack ? " feebly" : "";
            eng->log->addMessage("I " + wpnVerb + " " + otherName + ATTACK_MOD_TEXT + data.dmgPunctuation, clrMessageGood);
          } else {
            const string ATTACK_MOD_TEXT = data.isWeakAttack ? "feebly " : (data.isBackStab ? "covertly " : "");
            const SDL_Color clr = data.isBackStab ? clrBlueLight : clrMessageGood;
            eng->log->addMessage("I " + wpnVerb + " " + otherName + " " + ATTACK_MOD_TEXT + "with " + data.weaponName_a + data.dmgPunctuation, clr);
          }
        } else {
          const string wpnVerb = weapon->getDef().meleeAttackMessages.other;

          if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
            otherName = data.attacker->getNameThe();
          } else {
            otherName = "It";
          }

          eng->log->addMessage(otherName + " " + wpnVerb + data.dmgPunctuation, clrMessageBad, messageInterrupt_force);
        }
      }
    }
  }
}


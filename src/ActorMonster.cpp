#include "ActorMonster.h"

#include <iostream>

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "GameTime.h"
#include "Attack.h"
#include "Reload.h"
#include "Inventory.h"
#include "FeatureTrap.h"

#include "AI_handleClosedBlockingDoor.h"
#include "AI_look_becomePlayerAware.h"
#include "AI_makeRoomForFriend.h"
#include "AI_stepPath.h"
#include "AI_moveTowardsTargetSimple.h"
#include "AI_stepToLairIfHasLosToLair.h"
#include "AI_setPathToPlayerIfAware.h"
#include "AI_setPathToLairIfNoLosToLair.h"
#include "AI_setPathToLeaderIfNoLosToLeader.h"
#include "AI_moveToRandomAdjacentCell.h"
#include "AI_castRandomSpell.h"
#include "AI_handleInventory.h"

void Monster::act() {
  waiting_ = !waiting_;

  if(waiting_) {
    if(playerAwarenessCounter <= 0) {
      eng->gameTime->letNextAct();
      return;
    }
  }

  getSpotedEnemies();

  target = eng->mapTests->getClosestActor(pos, spotedEnemies);

  if(spellCoolDownCurrent != 0) {
    spellCoolDownCurrent--;
  }

  if(playerAwarenessCounter > 0) {
    isRoamingAllowed = true;
    if(leader != NULL) {
      if(leader->deadState == actorDeadState_alive) {
        if(leader != eng->player) {
          dynamic_cast<Monster*>(leader)->playerAwarenessCounter = leader->getDef()->nrTurnsAwarePlayer;
        }
      }
    }
  }

  const bool HAS_SNEAK_SKILL = def_->abilityValues.getAbilityValue(ability_stealth, true, *this) > 0;
  isStealth = eng->player->checkIfSeeActor(*this, NULL) == false && HAS_SNEAK_SKILL;

  const AiBehavior& ai = def_->aiBehavior;

  //------------------------------ SPECIAL MONSTER ACTIONS (ZOMBIES RISING, WORMS MULTIPLYING...)
  // TODO temporary restriction, allow this later(?)
  if(leader != eng->player) {
    if(actorSpecificAct()) {
      return;
    }
  }

  //------------------------------ COMMON ACTIONS (MOVING, ATTACKING, CASTING SPELLS...)
  // Looking counts as an action if monster not aware before, and became aware from looking.
  // (This is to give the monsters some reaction time, and not instantly attack)
  if(ai.looks) {
    if(leader != eng->player) {
      if(AI_look_becomePlayerAware::action(this, eng)) {
        return;
      }
    }
  }

  if(ai.makesRoomForFriend) {
    if(leader != eng->player) {
      if(AI_makeRoomForFriend::action(this, eng)) {
        return;
      }
    }
  }

  if(target != NULL) {
    const int CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING = 65;
    if(eng->dice(1, 100) < CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING) {
      if(AI_castRandomSpellIfAware::action(this, eng)) {
        return;
      }
    }
  }

  if(ai.attemptsAttack) {
    if(target != NULL) {
      if(attemptAttack(target->pos)) {
        return;
      }
    }
  }

  if(target != NULL) {
    if(AI_castRandomSpellIfAware::action(this, eng)) {
      return;
    }
  }

  if(eng->dice(1, 100) < def_->erraticMovement) {
    if(AI_moveToRandomAdjacentCell::action(this, eng)) {
      return;
    }
  }

  if(ai.movesTowardTargetWhenVision) {
    if(AI_moveTowardsTargetSimple::action(this, eng)) {
      return;
    }
  }

  vector<coord> path;

  if(ai.pathsToTargetWhenAware) {
    if(leader != eng->player) {
      AI_setPathToPlayerIfAware::learn(this, &path, eng);
    }
  }

  if(leader != eng->player) {
    if(AI_handleClosedBlockingDoor::action(this, &path, eng)) {
      return;
    }
  }

  if(AI_stepPath::action(this, &path)) {
    return;
  }

  if(ai.movesTowardLeader) {
    AI_setPathToLeaderIfNoLosToleader::learn(this, &path, eng);
    if(AI_stepPath::action(this, &path)) {
      return;
    }
  }

  if(ai.movesTowardLair) {
    if(leader != eng->player) {
      if(AI_stepToLairIfHasLosToLair::action(this, lairCell_, eng)) {
        return;
      }
      AI_setPathToLairIfNoLosToLair::learn(this, &path, lairCell_, eng);
      if(AI_stepPath::action(this, &path)) {
        return;
      }
    }
  }

  if(AI_moveToRandomAdjacentCell::action(this, eng)) {
    return;
  }

  eng->gameTime->letNextAct();
}

void Monster::monsterHit() {
  playerAwarenessCounter = def_->nrTurnsAwarePlayer;
}

void Monster::moveToCell(const coord& targetCell) {
  coord dest = getStatusEffectsHandler()->changeMoveCoord(pos, targetCell);

  //Trap affects leaving?
  if(dest != pos) {
    Feature* f = eng->map->featuresStatic[pos.x][pos.y];
    if(f->getId() == feature_trap) {
      dest = dynamic_cast<Trap*>(f)->actorAttemptLeave(this, pos, dest);
      if(dest == pos) {
        eng->gameTime->letNextAct();
        return;
      }
    }
  }

  // Movement direction is stored for AI purposes
  lastDirectionTraveled = dest - pos;

  pos = dest;

  // Bump features in target cell (i.e. to trigger traps)
  vector<FeatureMob*> featureMobs = eng->gameTime->getFeatureMobsAtPos(pos);
  for(unsigned int i = 0; i < featureMobs.size(); i++) {
    featureMobs.at(i)->bump(this);
  }
  eng->map->featuresStatic[pos.x][pos.y]->bump(this);

  eng->gameTime->letNextAct();
}

void Monster::hearSound(const Sound& sound) {
  if(deadState == actorDeadState_alive) {
    if(sound.getIsAlertingMonsters()) {
      becomeAware();
    }
  }
}

void Monster::becomeAware() {
  if(deadState == actorDeadState_alive) {
    const int PLAYER_AWARENESS_BEFORE = playerAwarenessCounter;
    playerAwarenessCounter = def_->nrTurnsAwarePlayer;
    if(PLAYER_AWARENESS_BEFORE <= 0) {
      const bool IS_SEEN_BY_PLAYER = eng->player->checkIfSeeActor(*this, NULL);
      const string msg = IS_SEEN_BY_PLAYER ? getAggroPhraseMonsterSeen() : getAggroPhraseMonsterHidden();
      eng->soundEmitter->emitSound(Sound(msg, false, pos, false, true));
    }

  }
}

bool Monster::attemptAttack(const coord& attackPos) {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0 || leader == eng->player) {

      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(pos, blockers);

      if(checkIfSeeActor(*eng->player, blockers)) {
        AttackOpport opport = getAttackOpport(attackPos);
        BestAttack const attack = getBestAttack(opport);

        if(attack.weapon != NULL) {
          if(attack.melee) {
            if(attack.weapon->getDef().isMeleeWeapon) {
              eng->attack->melee(attackPos.x, attackPos.y, attack.weapon);
              return true;
            }
          } else {
            if(attack.weapon->getDef().isRangedWeapon) {
              if(opport.timeToReload) {
                eng->reload->reloadWeapon(this);
                return true;
              } else {
                const int NR_TURNS_DISABLED_RANGED = def_->rangedCooldownTurns;
                statusEffectsHandler_->attemptAddEffect(new StatusDisabledAttackRanged(NR_TURNS_DISABLED_RANGED));
                eng->attack->ranged(attackPos.x, attackPos.y, attack.weapon);
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

AttackOpport Monster::getAttackOpport(const coord& attackPos) {
  AttackOpport opport;
  if(statusEffectsHandler_->allowAttack(false)) {
    opport.melee = eng->mapTests->isCellsNeighbours(pos, attackPos, false);

    Weapon* weapon = NULL;
    const unsigned nrOfIntrinsics = inventory_->getIntrinsicsSize();
    if(opport.melee) {
      if(statusEffectsHandler_->allowAttackMelee(false)) {

        //Melee weapon in wielded slot?
        weapon = dynamic_cast<Weapon*>(inventory_->getItemInSlot(slot_wielded));
        if(weapon != NULL) {
          if(weapon->getDef().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }

        //Intrinsic melee attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
          weapon = dynamic_cast<Weapon*>(inventory_->getIntrinsicInElement(i));
          if(weapon->getDef().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }
      }
    } else {
      if(statusEffectsHandler_->allowAttackRanged(false) && statusEffectsHandler_->hasEffect(statusBurning) == false) {
        //Ranged weapon in wielded slot?
        weapon = dynamic_cast<Weapon*>(inventory_->getItemInSlot(slot_wielded));

        if(weapon != NULL) {
          if(weapon->getDef().isRangedWeapon == true) {
            opport.weapons.push_back(weapon);

            //Check if reload time instead
            if(weapon->ammoLoaded == 0 && weapon->getDef().rangedHasInfiniteAmmo == false) {
              if(inventory_->hasAmmoForFirearmInInventory()) {
                opport.timeToReload = true;
              }
            }
          }
        }

        //Intrinsic ranged attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
          weapon = dynamic_cast<Weapon*>(inventory_->getIntrinsicInElement(i));
          if(weapon->getDef().isRangedWeapon) {
            opport.weapons.push_back(weapon);
          }
        }
      }
    }
  }

  return opport;
}

// TODO Instead of using "strongest" weapon, use random
BestAttack Monster::getBestAttack(const AttackOpport& attackOpport) {
  BestAttack attack;
  attack.melee = attackOpport.melee;

  Weapon* newWeapon = NULL;

  const unsigned int nrOfWeapons = attackOpport.weapons.size();

  //If any possible attacks found
  if(nrOfWeapons > 0) {
    attack.weapon = attackOpport.weapons.at(0);

    const ItemDefinition* def = &(attack.weapon->getDef());

    //If there are more than one possible weapon, find strongest.
    if(nrOfWeapons > 1) {
      for(unsigned int i = 1; i < nrOfWeapons; i++) {

        //Found new weapon in element i.
        newWeapon = attackOpport.weapons.at(i);
        const ItemDefinition* newDef = &(newWeapon->getDef());

        //Compare definitions.
        //If weapon i is stronger -
        if(eng->itemData->isWeaponStronger(*def, *newDef, attack.melee) == true) {
          // - use new weapon instead.
          attack.weapon = newWeapon;
          def = newDef;
        }
      }
    }
  }

  return attack;
}



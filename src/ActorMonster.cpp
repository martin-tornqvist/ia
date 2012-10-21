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
#include "AI_listen_becomePlayerAware.h"
#include "AI_listen_respondWithAggroPhrase.h"
#include "AI_makeRoomForFriend.h"
#include "AI_stepPath.h"
#include "AI_moveTowardsTargetSimple.h"
#include "AI_stepToLairIfHasLosToLair.h"
#include "AI_setPathToPlayerIfAware.h"
#include "AI_setPathToLairIfNoLosToLair.h"
#include "AI_setPathToLeaderIfNoLosToLeader.h"
#include "AI_moveToRandomAdjacentCell.h"
#include "AI_castRandomSpell.h"

void Monster::act() {
//  tracer << "Monster::act(), " << m_instanceDefinition.name_a << ", x:" << pos.x << " y:" << pos.y << ", " << this << endl;

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
          dynamic_cast<Monster*>(leader)->playerAwarenessCounter = leader->getInstanceDefinition()->nrTurnsAwarePlayer;
        }
      }
    }
  }

  const ActorDefinition& def = m_instanceDefinition;

  const bool HAS_SNEAK_SKILL = def.abilityValues.getAbilityValue(ability_stealth, true) > 0;
  isStealth = eng->player->checkIfSeeActor(*this, NULL) == false && HAS_SNEAK_SKILL;

  const AiBehavior& ai = def.aiBehavior;

  //------------------------------ INFORMATION GATHERING (LOOKING, LISTENING...)
  if(ai.looks) {
    if(leader != eng->player) {
      AI_look_becomePlayerAware::learn(this, eng);
    }
  }
  if(ai.listens) {
    if(leader != eng->player) {
      AI_listen_becomePlayerAware::learn(this, soundsHeard);
    }
  }
  if(ai.respondsWithPhrase) {
    if(leader != eng->player) {
      AI_listen_respondWithAggroPhrase::learn(this, soundsHeard, eng);
    }
  }

  //------------------------------ SPECIAL MONSTER ACTIONS (ZOMBIES RISING, WORMS MULTIPLYING...)
  // TODO temporary restriction, allow this later(?)
  if(leader != eng->player) {
    if(actorSpecificAct()) {
//      tracer << "   Monster: Ran actorSpecificAct()" << endl;
      return;
    }
  }

  //------------------------------ COMMON ACTIONS (MOVING, ATTACKING, CASTING SPELLS...)
  if(ai.makesRoomForFriend) {
    if(leader != eng->player) {
      if(AI_makeRoomForFriend::action(this, eng)) {
//				tracer << "   Monster: Ran AI_makeRoomForFriend()" << endl;
        return;
      }
    }
  }

  if(eng->dice(1, 100) < def.erraticMovement) {
    if(AI_moveToRandomAdjacentCell::action(this, eng)) {
//			tracer << "   Monster: Ran AI_moveToRandomAdjacentCell() (first)" << endl;
      return;
    }
  }

  if(target != NULL) {
    const int CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING = 65;
    if(eng->dice(1, 100) < CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING) {
      if(AI_castRandomSpellIfAware::action(this, eng)) {
//				tracer << "   Monster: Ran AI_castRandomSpellIfAware() (first)" << endl;
        return;
      }
    }
  }

  if(ai.attemptsAttack) {
    if(target != NULL) {
      if(attemptAttack(target->pos)) {
//				tracer << "   Monster: Ran attemptAttack()" << endl;
        return;
      }
    }
  }

  if(target != NULL) {
    if(AI_castRandomSpellIfAware::action(this, eng)) {
//			tracer << "   Monster: Ran AI_castRandomSpellIfAware() (second)" << endl;
      return;
    }
  }

  if(ai.movesTowardTargetWhenVision) {
    if(AI_moveTowardsTargetSimple::action(this, eng)) {
//			tracer << "   Monster: Ran moveTowardsTargetSimple()" << endl;
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
//			tracer << "   Monster: Ran AI_handleClosedBlockingDoor()" << endl;
      return;
    }
  }

  if(AI_stepPath::action(this, &path)) {
//		tracer << "   Monster: Ran AI_stepPath() (after AI_setPathToPlayerIfAware())" << endl;
    return;
  }

  if(ai.movesTowardLeader) {
    AI_setPathToLeaderIfNoLosToleader::learn(this, &path, eng);
    if(AI_stepPath::action(this, &path)) {
//			tracer << "   Monster: Ran AI_stepPath() (after AI_setPathToLeaderIfNoLosToLeader())" << endl;
      return;
    }
  }

  if(ai.movesTowardLair) {
    if(leader != eng->player) {
      if(AI_stepToLairIfHasLosToLair::action(this, lairCell, eng)) {
//				tracer << "   Monster: Ran AI_stepToLairIfHasLosToLair()" << endl;
        return;
      }
      AI_setPathToLairIfNoLosToLair::learn(this, &path, lairCell, eng);
      if(AI_stepPath::action(this, &path)) {
//				tracer << "   Monster: Ran AI_stepPath() (after setPathToLairIfNoLosToLair())" << endl;
        return;
      }
    }
  }

  if(AI_moveToRandomAdjacentCell::action(this, eng)) {
//		tracer << "   Monster: Ran AI_moveToRandomAdjacentCell() (second)" << endl;
    return;
  }

//	tracer << "   Monster: No action taken" << endl;

  eng->gameTime->letNextAct();
}

void Monster::monsterHit() {
  playerAwarenessCounter = m_instanceDefinition.nrTurnsAwarePlayer;
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

void Monster::registerHeardSound(const Sound& sound) {
  if(deadState == actorDeadState_alive) {
    soundsHeard.push_back(sound);
  }
}

bool Monster::attemptAttack(const coord& attackPos) {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0 || leader == eng->player) {

      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(blockers);

      if(checkIfSeeActor(*eng->player, blockers)) {
        AttackOpport opport = getAttackOpport(attackPos);
        BestAttack const attack = getBestAttack(opport);

        if(attack.weapon != NULL) {
          if(attack.melee) {
            if(attack.weapon->getInstanceDefinition().isMeleeWeapon) {
              const int NR_TURNS_DISABLED_MELEE = m_instanceDefinition.nrTurnsAttackDisablesMelee;
              m_statusEffectsHandler->attemptAddEffect(new StatusDisabledAttackMelee(NR_TURNS_DISABLED_MELEE));
              eng->attack->melee(attackPos.x, attackPos.y, attack.weapon);
              return true;
            }
          } else {
            if(attack.weapon->getInstanceDefinition().isRangedWeapon) {
              if(opport.timeToReload) {
                eng->reload->reloadWeapon(this);
                return true;
              } else {
                const int NR_TURNS_DISABLED_RANGED = m_instanceDefinition.nrTurnsAttackDisablesRanged;
                m_statusEffectsHandler->attemptAddEffect(new StatusDisabledAttackRanged(NR_TURNS_DISABLED_RANGED));
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
  if(m_statusEffectsHandler->allowAttack(false)) {
    opport.melee = eng->mapTests->isCellsNeighbours(pos, attackPos, false);

    Weapon* weapon = NULL;
    const unsigned nrOfIntrinsics = m_inventory->getIntrinsicsSize();
    if(opport.melee) {
      if(m_statusEffectsHandler->allowAttackMelee(false)) {

        //Melee weapon in wielded slot?
        weapon = dynamic_cast<Weapon*>(m_inventory->getItemInSlot(slot_wielded));
        if(weapon != NULL) {
          if(weapon->getInstanceDefinition().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }

        //Intrinsic melee attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
          weapon = dynamic_cast<Weapon*>(m_inventory->getIntrinsicInElement(i));
          if(weapon->getInstanceDefinition().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }
      }
    } else {
      if(m_statusEffectsHandler->allowAttackRanged(false) && m_statusEffectsHandler->hasEffect(statusBurning) == false) {
        //Ranged weapon in wielded slot?
        weapon = dynamic_cast<Weapon*>(m_inventory->getItemInSlot(slot_wielded));

        if(weapon != NULL) {
          if(weapon->getInstanceDefinition().isRangedWeapon == true) {
            opport.weapons.push_back(weapon);

            //Check if reload time instead
            if(weapon->ammoLoaded == 0 && weapon->getInstanceDefinition().rangedHasInfiniteAmmo == false) {
              if(m_inventory->hasAmmoForFirearmInInventory()) {
                opport.timeToReload = true;
              }
            }
          }
        }

        //Intrinsic ranged attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
          weapon = dynamic_cast<Weapon*>(m_inventory->getIntrinsicInElement(i));
          if(weapon->getInstanceDefinition().isRangedWeapon) {
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

    const ItemDefinition* def = &(attack.weapon->getInstanceDefinition());

    //If there are more than one possible weapon, find strongest.
    if(nrOfWeapons > 1) {
      for(unsigned int i = 1; i < nrOfWeapons; i++) {

        //Found new weapon in element i.
        newWeapon = attackOpport.weapons.at(i);
        const ItemDefinition* newDef = &(newWeapon->getInstanceDefinition());

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



#include "ActorMonster.h"

#include <assert.h>

#include "Engine.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "GameTime.h"
#include "Attack.h"
#include "Reload.h"
#include "Inventory.h"
#include "FeatureTrap.h"

#include "AI_setSpecialBlockedCells.h"
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

Monster::Monster(Engine& engine) :
  Actor(engine), playerAwarenessCounter(0), messageMonsterInViewPrinted(false),
  lastDirTraveled(dirCenter), spellCoolDownCurrent(0),
  isRoamingAllowed(true), isStealth(false), leader(NULL), target(NULL),
  waiting_(false), shockCausedCurrent(0.0) {}

Monster::~Monster() {
  for(unsigned int i = 0; i < spellsKnown.size(); i++) {
    delete spellsKnown.at(i);
  }
}

void Monster::onActorTurn() {
  assert(eng.basicUtils->isPosInsideMap(pos));

  waiting_ = !waiting_;

  if(waiting_) {
    if(playerAwarenessCounter <= 0) {
      eng.gameTime->endTurnOfCurrentActor();
      return;
    }
  }

  vector<Actor*> spotedEnemies;
  getSpotedEnemies(spotedEnemies);
  target = eng.basicUtils->getClosestActor(pos, spotedEnemies);

  if(spellCoolDownCurrent != 0) {
    spellCoolDownCurrent--;
  }

  if(playerAwarenessCounter > 0) {
    isRoamingAllowed = true;
    if(leader == NULL) {
      if(deadState == actorDeadState_alive) {
        if(eng.dice.percentile() < 7) {
          speakPhrase();
        }
      }
    } else {
      if(leader->deadState == actorDeadState_alive) {
        if(leader != eng.player) {
          dynamic_cast<Monster*>(leader)->playerAwarenessCounter =
            leader->getData()->nrTurnsAwarePlayer;
        }
      }
    }
  }

  const bool HAS_SNEAK_SKILL = data_->abilityVals.getVal(
                                 ability_stealth, true, *this) > 0;
  isStealth = eng.player->checkIfSeeActor(*this, NULL) == false &&
              HAS_SNEAK_SKILL;

  //Array used for AI purposes, e.g. to prevent tactically bad positions,
  //or prevent certain monsters from walking on a certain type of cells, etc.
  //This is checked in all AI movement functions. Cells set to true are
  //totally forbidden for the monster to move into.
  bool aiSpecialBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  AI_setSpecialBlockedCells::learn(*this, aiSpecialBlockers, eng);

  //------------------------------ SPECIAL MONSTER ACTIONS
  //                               (ZOMBIES RISING, WORMS MULTIPLYING...)
  if(leader != eng.player/*TODO temporary restriction, allow this later(?)*/) {
    if(monsterSpecificOnActorTurn()) {
      return;
    }
  }

  //------------------------------ COMMON ACTIONS
  //                               (MOVING, ATTACKING, CASTING SPELLS...)
  //Looking counts as an action if monster not aware before, and became aware
  //from looking. (This is to give the monsters some reaction time, and not
  //instantly attack)
  if(data_->ai[aiLooks]) {
    if(leader != eng.player) {
      if(AI_look_becomePlayerAware::action(*this, eng)) {
        return;
      }
    }
  }

  if(data_->ai[aiMakesRoomForFriend]) {
    if(leader != eng.player) {
      if(AI_makeRoomForFriend::action(*this, eng)) {
        return;
      }
    }
  }

  if(target != NULL) {
    const int CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING = 65;
    if(eng.dice.percentile() < CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING) {
      if(AI_castRandomSpellIfAware::action(this, eng)) {
        return;
      }
    }
  }

  if(data_->ai[aiAttacks]) {
    if(target != NULL) {
      if(tryAttack(*target)) {
        return;
      }
    }
  }

  if(target != NULL) {
    if(AI_castRandomSpellIfAware::action(this, eng)) {
      return;
    }
  }

  if(eng.dice.percentile() < data_->erraticMovement) {
    if(AI_moveToRandomAdjacentCell::action(*this, eng)) {
      return;
    }
  }

  if(data_->ai[aiMovesTowardTargetWhenVision]) {
    if(AI_moveTowardsTargetSimple::action(*this, eng)) {
      return;
    }
  }

  vector<Pos> path;

  if(data_->ai[aiPathsToTargetWhenAware]) {
    if(leader != eng.player) {
      AI_setPathToPlayerIfAware::learn(*this, path, eng);
    }
  }

  if(leader != eng.player) {
    if(AI_handleClosedBlockingDoor::action(this, path, eng)) {
      return;
    }
  }

  if(AI_stepPath::action(*this, path)) {
    return;
  }

  if(data_->ai[aiMovesTowardLeader]) {
    AI_setPathToLeaderIfNoLosToleader::learn(*this, path, eng);
    if(AI_stepPath::action(*this, path)) {
      return;
    }
  }

  if(data_->ai[aiMovesTowardLair]) {
    if(leader != eng.player) {
      if(AI_stepToLairIfHasLosToLair::action(*this, lairCell_, eng)) {
        return;
      }
      AI_setPathToLairIfNoLosToLair::learn(*this, path, lairCell_, eng);
      if(AI_stepPath::action(*this, path)) {
        return;
      }
    }
  }

  if(AI_moveToRandomAdjacentCell::action(*this, eng)) {
    return;
  }

  eng.gameTime->endTurnOfCurrentActor();
}

void Monster::onMonsterHit(int& dmg) {
  playerAwarenessCounter = data_->nrTurnsAwarePlayer;

  if(data_->monsterShockLevel != monsterShockLevel_none) {
    dmg = (dmg * (100 + eng.player->getMth())) / 100;
  }
}

void Monster::moveDir(Dir_t dir) {
  assert(dir != endOfDirs);
  assert(eng.basicUtils->isPosInsideMap(pos));

  getPropHandler()->changeMoveDir(pos, dir);

  //Trap affects leaving?
  if(dir != dirCenter) {
    Feature* f = eng.map->cells[pos.x][pos.y].featureStatic;
    if(f->getId() == feature_trap) {
      dir = dynamic_cast<Trap*>(f)->actorTryLeave(*this, dir);
      if(dir == dirCenter) {
        traceVerbose << "Monster: Move prevented by trap" << endl;
        eng.gameTime->endTurnOfCurrentActor();
        return;
      }
    }
  }

  // Movement direction is stored for AI purposes
  lastDirTraveled = dir;

  const Pos targetCell(pos + DirConverter().getOffset(dir));

  assert(eng.basicUtils->isPosInsideMap(targetCell));

  pos = targetCell;

  // Bump features in target cell (i.e. to trigger traps)
  vector<FeatureMob*> featureMobs = eng.gameTime->getFeatureMobsAtPos(pos);
  for(unsigned int i = 0; i < featureMobs.size(); i++) {
    featureMobs.at(i)->bump(*this);
  }
  eng.map->cells[pos.x][pos.y].featureStatic->bump(*this);

  eng.gameTime->endTurnOfCurrentActor();
}

void Monster::hearSound(const Sound& snd) {
  if(deadState == actorDeadState_alive) {
    if(snd.getIsAlertingMonsters()) {
      becomeAware();
    }
  }
}

void Monster::speakPhrase() {
  const bool IS_SEEN_BY_PLAYER = eng.player->checkIfSeeActor(*this, NULL);
  const string msg = IS_SEEN_BY_PLAYER ?
                     getAggroPhraseMonsterSeen() :
                     getAggroPhraseMonsterHidden();
  const Sfx_t sfx = IS_SEEN_BY_PLAYER ?
                    getAggroSfxMonsterSeen() :
                    getAggroSfxMonsterHidden();
  eng.soundEmitter->emitSound(Sound(msg, sfx, false, pos, false, true));
}

void Monster::becomeAware() {
  if(deadState == actorDeadState_alive) {
    const int PLAYER_AWARENESS_BEFORE = playerAwarenessCounter;
    playerAwarenessCounter = data_->nrTurnsAwarePlayer;
    if(PLAYER_AWARENESS_BEFORE <= 0 && eng.dice.coinToss()) {
      speakPhrase();
    }
  }
}

bool Monster::tryAttack(Actor& defender) {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0 || leader == eng.player) {

      bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
      MapParser::parse(CellPredBlocksVision(eng), blockers);

      if(checkIfSeeActor(*eng.player, blockers)) {
        AttackOpport opport = getAttackOpport(defender);
        const BestAttack attack = getBestAttack(opport);

        if(attack.weapon != NULL) {
          if(attack.isMelee) {
            if(attack.weapon->getData().isMeleeWeapon) {
              eng.attack->melee(*this, *attack.weapon, defender);
              return true;
            }
          } else {
            if(attack.weapon->getData().isRangedWeapon) {
              if(opport.isTimeToReload) {
                eng.reload->reloadWieldedWpn(*this);
                return true;
              } else {
                //Check if friend is in the way
                //(with a small chance to ignore this)
                bool isBlockedByFriend = false;
                if(eng.dice.fraction(4, 5)) {
                  vector<Pos> line;
                  eng.lineCalc->calcNewLine(
                    pos, defender.pos, true, 9999, false, line);
                  for(unsigned int i = 0; i < line.size(); i++) {
                    const Pos& curPos = line.at(i);
                    if(curPos != pos && curPos != defender.pos) {
                      Actor* const actorHere =
                        eng.basicUtils->getActorAtPos(curPos);
                      if(actorHere != NULL) {
                        isBlockedByFriend = true;
                        break;
                      }
                    }
                  }
                }

                if(isBlockedByFriend == false) {
                  const int NR_TURNS_DISABLED_RANGED =
                    data_->rangedCooldownTurns;
                  PropDisabledRanged* status =
                    new PropDisabledRanged(
                    eng, propTurnsSpecified, NR_TURNS_DISABLED_RANGED);
                  propHandler_->tryApplyProp(status);
                  eng.attack->ranged(*this, *attack.weapon, defender.pos);
                  return true;
                } else {
                  return false;
                }
              }
            }
          }
        }
      }
    }
  }
  return false;
}

AttackOpport Monster::getAttackOpport(Actor& defender) {
  AttackOpport opport;
  if(propHandler_->allowAttack(false)) {
    opport.isMelee =
      eng.basicUtils->isPosAdj(pos, defender.pos, false);

    Weapon* weapon = NULL;
    const unsigned nrOfIntrinsics = inventory_->getIntrinsicsSize();
    if(opport.isMelee) {
      if(propHandler_->allowAttackMelee(false)) {

        //Melee weapon in wielded slot?
        weapon =
          dynamic_cast<Weapon*>(inventory_->getItemInSlot(slot_wielded));
        if(weapon != NULL) {
          if(weapon->getData().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }

        //Intrinsic melee attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
          weapon = dynamic_cast<Weapon*>(inventory_->getIntrinsicInElement(i));
          if(weapon->getData().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }
      }
    } else {
      if(
        propHandler_->allowAttackRanged(false) &&
        propHandler_->hasProp(propBurning) == false) {
        //Ranged weapon in wielded slot?
        weapon =
          dynamic_cast<Weapon*>(inventory_->getItemInSlot(slot_wielded));

        if(weapon != NULL) {
          if(weapon->getData().isRangedWeapon == true) {
            opport.weapons.push_back(weapon);

            //Check if reload time instead
            if(
              weapon->nrAmmoLoaded == 0 &&
              weapon->getData().rangedHasInfiniteAmmo == false) {
              if(inventory_->hasAmmoForFirearmInInventory()) {
                opport.isTimeToReload = true;
              }
            }
          }
        }

        //Intrinsic ranged attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; i++) {
          weapon = dynamic_cast<Weapon*>(inventory_->getIntrinsicInElement(i));
          if(weapon->getData().isRangedWeapon) {
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
  attack.isMelee = attackOpport.isMelee;

  Weapon* newWeapon = NULL;

  const unsigned int nrOfWeapons = attackOpport.weapons.size();

  //If any possible attacks found
  if(nrOfWeapons > 0) {
    attack.weapon = attackOpport.weapons.at(0);

    const ItemData* data = &(attack.weapon->getData());

    //If there are more than one possible weapon, find strongest.
    if(nrOfWeapons > 1) {
      for(unsigned int i = 1; i < nrOfWeapons; i++) {

        //Found new weapon in element i.
        newWeapon = attackOpport.weapons.at(i);
        const ItemData* newData = &(newWeapon->getData());

        //Compare definitions.
        //If weapon i is stronger -
        if(eng.itemDataHandler->isWeaponStronger(*data, *newData, attack.isMelee)) {
          // - use new weapon instead.
          attack.weapon = newWeapon;
          data = newData;
        }
      }
    }
  }

  return attack;
}



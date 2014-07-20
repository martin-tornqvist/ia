#include "ActorMonster.h"

#include <vector>
#include <assert.h>

#include "Init.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ActorPlayer.h"
#include "GameTime.h"
#include "Attack.h"
#include "Reload.h"
#include "Inventory.h"
#include "FeatureTrap.h"
#include "FeatureMob.h"
#include "Properties.h"
#include "Renderer.h"
#include "Sound.h"
#include "Utils.h"
#include "Map.h"
#include "Log.h"
#include "MapParsing.h"
#include "Ai.h"
#include "LineCalc.h"

using namespace std;

Monster::Monster() :
  Actor(),
  awareOfPlayerCounter_(0),
  playerAwareOfMeCounter_(0),
  messageMonsterInViewPrinted(false),
  lastDirTravelled_(Dir::center),
  spellCoolDownCur(0),
  isRoamingAllowed_(true),
  isStealth(false),
  leader(nullptr),
  target(nullptr),
  waiting_(false),
  shockCausedCur_(0.0),
  hasGivenXpForSpotting_(false) {}

Monster::~Monster() {
  for(Spell* const spell : spellsKnown) {delete spell;}
}

void Monster::onActorTurn() {
  assert(Utils::isPosInsideMap(pos));

  waiting_ = !waiting_;

  if(waiting_) {
    if(awareOfPlayerCounter_ <= 0) {
      GameTime::actorDidAct();
      return;
    }
  }

  vector<Actor*> spottedEnemies;
  getSpottedEnemies(spottedEnemies);
  target = Utils::getRandomClosestActor(pos, spottedEnemies);

  if(spellCoolDownCur != 0) {
    spellCoolDownCur--;
  }

  if(awareOfPlayerCounter_ > 0) {
    isRoamingAllowed_ = true;
    if(!leader) {
      if(deadState == ActorDeadState::alive) {
        if(Rnd::oneIn(14)) {
          speakPhrase();
        }
      }
    } else {
      if(leader->deadState == ActorDeadState::alive) {
        if(leader != Map::player) {
          static_cast<Monster*>(leader)->awareOfPlayerCounter_ =
            leader->getData().nrTurnsAwarePlayer;
        }
      }
    }
  }

  const bool HAS_SNEAK_SKILL = data_->abilityVals.getVal(
                                 AbilityId::stealth, true, *this) > 0;
  isStealth = !Map::player->isSeeingActor(*this, nullptr) && HAS_SNEAK_SKILL;

  //Array used for AI purposes, e.g. to prevent tactically bad positions,
  //or prevent certain monsters from walking on a certain type of cells, etc.
  //This is checked in all AI movement functions. Cells set to true are
  //totally forbidden for the monster to move into.
  bool aiSpecialBlockers[MAP_W][MAP_H];
  Ai::Info::setSpecialBlockedCells(*this, aiSpecialBlockers);

  //------------------------------ SPECIAL MONSTER ACTIONS
  //                               (ZOMBIES RISING, WORMS MULTIPLYING...)
  if(leader != Map::player/*TODO temporary restriction, allow this later(?)*/) {
    if(onActorTurn_()) {return;}
  }

  //------------------------------ COMMON ACTIONS
  //                               (MOVING, ATTACKING, CASTING SPELLS...)
  //Looking counts as an action if monster not aware before, and became aware
  //from looking. (This is to give the monsters some reaction time, and not
  //instantly attack)
  if(data_->ai[int(AiId::looks)]) {
    if(leader != Map::player) {
      if(Ai::Info::lookBecomePlayerAware(*this)) {return;}
    }
  }

  if(data_->ai[int(AiId::makesRoomForFriend)]) {
    if(leader != Map::player) {
      if(Ai::Action::makeRoomForFriend(*this)) {return;}
    }
  }

  if(target) {
    const int CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING = 65;
    if(Rnd::percentile() < CHANCE_TO_ATTEMPT_SPELL_BEFORE_ATTACKING) {
      if(Ai::Action::castRandomSpellIfAware(this)) {return;}
    }
  }

  if(data_->ai[int(AiId::attacks)]) {
    if(target) {
      if(tryAttack(*target)) {
        return;
      }
    }
  }

  if(target) {
    if(Ai::Action::castRandomSpellIfAware(this)) {
      return;
    }
  }

  if(Rnd::percentile() < data_->erraticMovement) {
    if(Ai::Action::moveToRandomAdjacentCell(*this)) {
      return;
    }
  }

  if(data_->ai[int(AiId::movesTowardTargetWhenVision)]) {
    if(Ai::Action::moveTowardsTargetSimple(*this)) {return;}
  }

  vector<Pos> path;

  if(data_->ai[int(AiId::pathsToTargetWhenAware)]) {
    if(leader != Map::player) {
      Ai::Info::setPathToPlayerIfAware(*this, path);
    }
  }

  if(leader != Map::player) {
    if(Ai::Action::handleClosedBlockingDoor(*this, path)) {
      return;
    }
  }

  if(Ai::Action::stepPath(*this, path)) {return;}

  if(data_->ai[int(AiId::movesTowardLeader)]) {
    Ai::Info::setPathToLeaderIfNoLosToleader(*this, path);
    if(Ai::Action::stepPath(*this, path)) {return;}
  }

  if(data_->ai[int(AiId::movesTowardLair)]) {
    if(leader != Map::player) {
      if(Ai::Action::stepToLairIfLos(*this, lairCell_)) {
        return;
      } else {
        Ai::Info::setPathToLairIfNoLos(*this, path, lairCell_);
        if(Ai::Action::stepPath(*this, path)) {return;}
      }
    }
  }

  if(Ai::Action::moveToRandomAdjacentCell(*this)) {return;}

  GameTime::actorDidAct();
}

void Monster::hit_(int& dmg, const bool ALLOW_WOUNDS) {
  (void)ALLOW_WOUNDS;
  (void)dmg;

  awareOfPlayerCounter_ = data_->nrTurnsAwarePlayer;
}

void Monster::moveDir(Dir dir) {
  assert(dir != Dir::END);
  assert(Utils::isPosInsideMap(pos));

  getPropHandler().changeMoveDir(pos, dir);

  //Trap affects leaving?
  if(dir != Dir::center) {
    auto* f = Map::cells[pos.x][pos.y].featureStatic;
    if(f->getId() == FeatureId::trap) {
      dir = static_cast<Trap*>(f)->actorTryLeave(*this, dir);
      if(dir == Dir::center) {
        TRACE_VERBOSE << "Monster: Move prevented by trap" << endl;
        GameTime::actorDidAct();
        return;
      }
    }
  }

  // Movement direction is stored for AI purposes
  lastDirTravelled_ = dir;

  const Pos targetCell(pos + DirUtils::getOffset(dir));

  assert(Utils::isPosInsideMap(targetCell));

  if(dir != Dir::center) {
    pos = targetCell;

    //Bump features in target cell (i.e. to trigger traps)
    vector<FeatureMob*> featureMobs;
    GameTime::getFeatureMobsAtPos(pos, featureMobs);
    for(auto* m : featureMobs) {m->bump(*this);}
    Map::cells[pos.x][pos.y].featureStatic->bump(*this);
  }

  GameTime::actorDidAct();
}

void Monster::hearSound(const Snd& snd) {
  if(deadState == ActorDeadState::alive) {
    if(snd.isAlertingMonsters()) {
      becomeAware(false);
    }
  }
}

void Monster::speakPhrase() {
  const bool IS_SEEN_BY_PLAYER = Map::player->isSeeingActor(*this, nullptr);
  const string msg = IS_SEEN_BY_PLAYER ?
                     getAggroPhraseMonsterSeen() :
                     getAggroPhraseMonsterHidden();
  const SfxId sfx = IS_SEEN_BY_PLAYER ?
                    getAggroSfxMonsterSeen() :
                    getAggroSfxMonsterHidden();

  Snd snd(msg, sfx, IgnoreMsgIfOriginSeen::no, pos, this,
          SndVol::low, AlertsMonsters::yes);
  SndEmit::emitSnd(snd);
}

void Monster::becomeAware(const bool IS_FROM_SEEING) {
  if(deadState == ActorDeadState::alive) {
    const int AWARENESS_CNT_BEFORE = awareOfPlayerCounter_;
    awareOfPlayerCounter_ = data_->nrTurnsAwarePlayer;
    if(AWARENESS_CNT_BEFORE <= 0) {
      if(IS_FROM_SEEING && Map::player->isSeeingActor(*this, nullptr)) {
        Map::player->updateFov();
        Renderer::drawMapAndInterface(true);
        Log::addMsg(getNameThe() + " sees me!");
      }
      if(Rnd::coinToss()) {speakPhrase();}
    }
  }
}

void Monster::playerBecomeAwareOfMe(const int DURATION_FACTOR) {
  const int LOWER         = 4 * DURATION_FACTOR;
  const int UPPER         = 6 * DURATION_FACTOR;
  const int ROLL          = Rnd::range(LOWER, UPPER);
  playerAwareOfMeCounter_ = max(playerAwareOfMeCounter_, ROLL);
}

bool Monster::tryAttack(Actor& defender) {
  if(
    deadState != ActorDeadState::alive ||
    (awareOfPlayerCounter_ <= 0 && leader != Map::player)) {
    return false;
  }

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blocked);

  if(!isSeeingActor(*Map::player, blocked)) {return false;}

  AttackOpport opport     = getAttackOpport(defender);
  const BestAttack attack = getBestAttack(opport);

  if(!attack.weapon) {return false;}

  if(attack.isMelee) {
    if(attack.weapon->getData().isMeleeWeapon) {
      Attack::melee(*this, *attack.weapon, defender);
      return true;
    }
    return false;
  }

  if(attack.weapon->getData().isRangedWeapon) {
    if(opport.isTimeToReload) {
      Reload::reloadWieldedWpn(*this);
      return true;
    }

    //Check if friend is in the way (with a small chance to ignore this)
    bool isBlockedByFriend = false;
    if(Rnd::fraction(4, 5)) {
      vector<Pos> line;
      LineCalc::calcNewLine(pos, defender.pos, true, 9999, false, line);
      for(Pos& linePos : line) {
        if(linePos != pos && linePos != defender.pos) {
          Actor* const actorHere = Utils::getActorAtPos(linePos);
          if(actorHere) {
            isBlockedByFriend = true;
            break;
          }
        }
      }
    }

    if(isBlockedByFriend) {return false;}

    const int NR_TURNS_NO_RANGED = data_->rangedCooldownTurns;
    PropDisabledRanged* status =
      new PropDisabledRanged(propTurnsSpecific, NR_TURNS_NO_RANGED);
    propHandler_->tryApplyProp(status);
    Attack::ranged(*this, *attack.weapon, defender.pos);
    return true;
  }

  return false;
}

AttackOpport Monster::getAttackOpport(Actor& defender) {
  AttackOpport opport;
  if(propHandler_->allowAttack(false)) {
    opport.isMelee =
      Utils::isPosAdj(pos, defender.pos, false);

    Weapon* weapon = nullptr;
    const unsigned nrOfIntrinsics = inv_->getIntrinsicsSize();
    if(opport.isMelee) {
      if(propHandler_->allowAttackMelee(false)) {

        //Melee weapon in wielded slot?
        weapon =
          static_cast<Weapon*>(inv_->getItemInSlot(SlotId::wielded));
        if(weapon) {
          if(weapon->getData().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }

        //Intrinsic melee attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; ++i) {
          weapon = static_cast<Weapon*>(inv_->getIntrinsicInElement(i));
          if(weapon->getData().isMeleeWeapon) {
            opport.weapons.push_back(weapon);
          }
        }
      }
    } else {
      if(propHandler_->allowAttackRanged(false)) {
        //Ranged weapon in wielded slot?
        weapon =
          static_cast<Weapon*>(inv_->getItemInSlot(SlotId::wielded));

        if(weapon) {
          if(weapon->getData().isRangedWeapon) {
            opport.weapons.push_back(weapon);

            //Check if reload time instead
            if(
              weapon->nrAmmoLoaded == 0 &&
              !weapon->getData().rangedHasInfiniteAmmo) {
              if(inv_->hasAmmoForFirearmInInventory()) {
                opport.isTimeToReload = true;
              }
            }
          }
        }

        //Intrinsic ranged attacks?
        for(unsigned int i = 0; i < nrOfIntrinsics; ++i) {
          weapon = static_cast<Weapon*>(inv_->getIntrinsicInElement(i));
          if(weapon->getData().isRangedWeapon) {
            opport.weapons.push_back(weapon);
          }
        }
      }
    }
  }

  return opport;
}

//TODO Instead of using "strongest" weapon, use random
BestAttack Monster::getBestAttack(const AttackOpport& attackOpport) {
  BestAttack attack;
  attack.isMelee = attackOpport.isMelee;

  Weapon* newWeapon = nullptr;

  const unsigned int nrOfWeapons = attackOpport.weapons.size();

  //If any possible attacks found
  if(nrOfWeapons > 0) {
    attack.weapon = attackOpport.weapons.at(0);

    const ItemDataT* data = &(attack.weapon->getData());

    //If there are more than one possible weapon, find strongest.
    if(nrOfWeapons > 1) {
      for(unsigned int i = 1; i < nrOfWeapons; ++i) {

        //Found new weapon in element i.
        newWeapon = attackOpport.weapons.at(i);
        const ItemDataT* newData = &(newWeapon->getData());

        //Compare definitions.
        //If weapon i is stronger -
        if(ItemData::isWeaponStronger(*data, *newData, attack.isMelee)) {
          // - use new weapon instead.
          attack.weapon = newWeapon;
          data = newData;
        }
      }
    }
  }

  return attack;
}



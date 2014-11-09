#include "ActorMonster.h"

#include <vector>
#include <assert.h>

#include "Init.h"
#include "Item.h"
#include "ActorPlayer.h"
#include "GameTime.h"
#include "Attack.h"
#include "Reload.h"
#include "Inventory.h"
#include "FeatureTrap.h"
#include "FeatureMob.h"
#include "Properties.h"
#include "Render.h"
#include "Sound.h"
#include "Utils.h"
#include "Map.h"
#include "Log.h"
#include "MapParsing.h"
#include "Ai.h"
#include "LineCalc.h"

using namespace std;

Mon::Mon() :
  Actor(),
  awareCounter_(0),
  playerAwareOfMeCounter_(0),
  isMsgMonInViewPrinted(false),
  lastDirTravelled_(Dir::center),
  spellCoolDownCur(0),
  isRoamingAllowed_(true),
  isStealth(false),
  leader(nullptr),
  target(nullptr),
  waiting_(false),
  shockCausedCur_(0.0),
  hasGivenXpForSpotting_(false),
  nrTurnsUntilUnsummoned_(-1) {}

Mon::~Mon()
{
  for(Spell* const spell : spellsKnown) {delete spell;}
}

void Mon::onActorTurn()
{
  //Test that monster is inside map
  assert(Utils::isPosInsideMap(pos));

  //Test that monster's leader does not have a leader (never allowed)
  if(leader && !isActorMyLeader(*Map::player) && static_cast<Mon*>(leader)->leader)
  {
    TRACE << "Two (or more) steps of leader is never allowed" << endl;
    assert(false);
  }

  if(awareCounter_ <= 0 && !isActorMyLeader(*Map::player))
  {
    waiting_ = !waiting_;

    if(waiting_)
    {
      GameTime::actorDidAct();
      return;
    }
  }
  else
  {
    waiting_ = false;
  }

  vector<Actor*> seenFoes;
  getSeenFoes(seenFoes);
  target = Utils::getRandomClosestActor(pos, seenFoes);

  if(spellCoolDownCur != 0) {spellCoolDownCur--;}

  if(awareCounter_ > 0)
  {
    isRoamingAllowed_ = true;
    if(leader)
    {
      if(leader->isAlive() && leader != Map::player)
      {
        static_cast<Mon*>(leader)->awareCounter_ = leader->getData().nrTurnsAwarePlayer;
      }
    }
    else //Monster does not have a leader
    {
      if(isAlive() && Rnd::oneIn(14)) {speakPhrase();}
    }
  }

  const bool HAS_SNEAK_SKILL =
    data_->abilityVals.getVal(AbilityId::stealth, true, *this) > 0;
  isStealth = !Map::player->isSeeingActor(*this, nullptr) && HAS_SNEAK_SKILL;

  //Array used for AI purposes, e.g. to prevent tactically bad positions,
  //or prevent certain monsters from walking on a certain type of cells, etc.
  //This is checked in all AI movement functions. Cells set to true are
  //totally forbidden for the monster to move into.
  bool aiSpecialBlockers[MAP_W][MAP_H];
  Ai::Info::setSpecialBlockedCells(*this, aiSpecialBlockers);

  //------------------------------ SPECIAL MONSTER ACTIONS
  //                               (ZOMBIES RISING, WORMS MULTIPLYING...)
  if(leader != Map::player/*TODO temporary restriction, allow this later(?)*/)
  {
    if(onActorTurn_()) {return;}
  }

  //------------------------------ COMMON ACTIONS
  //                               (MOVING, ATTACKING, CASTING SPELLS...)
  //Looking is as an action if monster not aware before, and became aware from looking.
  //(This is to give the monsters some reaction time, and not instantly attack)
  if(data_->ai[int(AiId::looks)] && leader != Map::player)
  {
    if(Ai::Info::lookBecomePlayerAware(*this)) {return;}
  }

  if(data_->ai[int(AiId::makesRoomForFriend)] && leader != Map::player)
  {
    if(Ai::Action::makeRoomForFriend(*this)) {return;}
  }

  if(target && Rnd::oneIn(6))
  {
    if(Ai::Action::castRandomSpellIfAware(*this)) {return;}
  }

  if(data_->ai[int(AiId::attacks)] && target)
  {
    if(tryAttack(*target)) {return;}
  }

  if(target)
  {
    if(Ai::Action::castRandomSpellIfAware(*this)) {return;}
  }

  int erraticMovePct = data_->erraticMovePct;
  if(isActorMyLeader(*Map::player))
  {
    //Move less erratically if allied to player
    erraticMovePct /= 2;
  }

  if(Rnd::percentile() < erraticMovePct)
  {
    if(Ai::Action::moveToRandomAdjCell(*this)) {return;}
  }

  if(data_->ai[int(AiId::movesToTgtWhenLos)])
  {
    if(Ai::Action::moveToTgtSimple(*this)) {return;}
  }

  vector<Pos> path;

  if(data_->ai[int(AiId::pathsToTgtWhenAware)] && leader != Map::player)
  {
    Ai::Info::setPathToPlayerIfAware(*this, path);
  }

  if(leader != Map::player)
  {
    if(Ai::Action::handleClosedBlockingDoor(*this, path)) {return;}
  }

  if(Ai::Action::stepPath(*this, path)) {return;}

  if(data_->ai[int(AiId::movesToLeader)])
  {
    Ai::Info::setPathToLeaderIfNoLosToleader(*this, path);
    if(Ai::Action::stepPath(*this, path)) {return;}
  }

  if(data_->ai[int(AiId::movesToLair)] && leader != Map::player)
  {
    if(Ai::Action::stepToLairIfLos(*this, lairCell_))
    {
      return;
    }
    else
    {
      Ai::Info::setPathToLairIfNoLos(*this, path, lairCell_);
      if(Ai::Action::stepPath(*this, path)) {return;}
    }
  }

  if(Ai::Action::moveToRandomAdjCell(*this)) {return;}

  GameTime::actorDidAct();
}

void Mon::onStdTurn()
{
  if(nrTurnsUntilUnsummoned_ > 0)
  {
    --nrTurnsUntilUnsummoned_;
    if(nrTurnsUntilUnsummoned_ <= 0)
    {
      if(Map::player->isSeeingActor(*this, nullptr))
      {
        Log::addMsg(getNameThe() + " suddenly disappears!");
//        Render::drawBlastAtCells({pos}, clrMagenta);
      }
      state = ActorState::destroyed;
      return;
    }
  }
  onStdTurn_();
}

void Mon::hit_(int& dmg)
{
  (void)dmg;
  awareCounter_ = data_->nrTurnsAwarePlayer;
}

void Mon::moveDir(Dir dir)
{
  assert(dir != Dir::END);
  assert(Utils::isPosInsideMap(pos, false));

  getPropHandler().changeMoveDir(pos, dir);

  //Trap affects leaving?
  if(dir != Dir::center)
  {
    auto* f = Map::cells[pos.x][pos.y].rigid;
    if(f->getId() == FeatureId::trap)
    {
      dir = static_cast<Trap*>(f)->actorTryLeave(*this, dir);
      if(dir == Dir::center)
      {
        TRACE_VERBOSE << "Monster move prevented by trap" << endl;
        GameTime::actorDidAct();
        return;
      }
    }
  }

  // Movement direction is stored for AI purposes
  lastDirTravelled_ = dir;

  const Pos targetCell(pos + DirUtils::getOffset(dir));

  if(dir != Dir::center && Utils::isPosInsideMap(targetCell, false))
  {
    pos = targetCell;

    //Bump features in target cell (i.e. to trigger traps)
    vector<Mob*> mobs;
    GameTime::getMobsAtPos(pos, mobs);
    for(auto* m : mobs) {m->bump(*this);}
    Map::cells[pos.x][pos.y].rigid->bump(*this);
  }

  GameTime::actorDidAct();
}

void Mon::hearSound(const Snd& snd)
{
  if(isAlive())
  {
    if(snd.isAlertingMon())
    {
      becomeAware(false);
    }
  }
}

void Mon::speakPhrase()
{
  const bool IS_SEEN_BY_PLAYER = Map::player->isSeeingActor(*this, nullptr);
  const string msg = IS_SEEN_BY_PLAYER ?
                     getAggroPhraseMonSeen() :
                     getAggroPhraseMonHidden();
  const SfxId sfx = IS_SEEN_BY_PLAYER ?
                    getAggroSfxMonSeen() :
                    getAggroSfxMonHidden();

  Snd snd(msg, sfx, IgnoreMsgIfOriginSeen::no, pos, this,
          SndVol::low, AlertsMon::yes);
  SndEmit::emitSnd(snd);
}

void Mon::becomeAware(const bool IS_FROM_SEEING)
{
  if(isAlive())
  {
    const int AWARENESS_CNT_BEFORE = awareCounter_;
    awareCounter_ = data_->nrTurnsAwarePlayer;
    if(AWARENESS_CNT_BEFORE <= 0)
    {
      if(IS_FROM_SEEING && Map::player->isSeeingActor(*this, nullptr))
      {
        Map::player->updateFov();
        Render::drawMapAndInterface(true);
        Log::addMsg(getNameThe() + " sees me!");
      }
      if(Rnd::coinToss()) {speakPhrase();}
    }
  }
}

void Mon::playerBecomeAwareOfMe(const int DURATION_FACTOR)
{
  const int LOWER         = 4 * DURATION_FACTOR;
  const int UPPER         = 6 * DURATION_FACTOR;
  const int ROLL          = Rnd::range(LOWER, UPPER);
  playerAwareOfMeCounter_ = max(playerAwareOfMeCounter_, ROLL);
}

bool Mon::tryAttack(Actor& defender)
{
  if(state != ActorState::alive || (awareCounter_ <= 0 && leader != Map::player))
  {
    return false;
  }

  AttackOpport opport     = getAttackOpport(defender);
  const BestAttack attack = getBestAttack(opport);

  if(!attack.weapon) {return false;}

  if(attack.isMelee)
  {
    if(attack.weapon->getData().melee.isMeleeWpn)
    {
      Attack::melee(*this, *attack.weapon, defender);
      return true;
    }
    return false;
  }

  if(attack.weapon->getData().ranged.isRangedWpn)
  {
    if(opport.isTimeToReload)
    {
      Reload::reloadWieldedWpn(*this);
      return true;
    }

    //Check if friend is in the way (with a small chance to ignore this)
    bool isBlockedByFriend = false;
    if(Rnd::fraction(4, 5))
    {
      vector<Pos> line;
      LineCalc::calcNewLine(pos, defender.pos, true, 9999, false, line);
      for(Pos& linePos : line)
      {
        if(linePos != pos && linePos != defender.pos)
        {
          Actor* const actorHere = Utils::getActorAtPos(linePos);
          if(actorHere)
          {
            isBlockedByFriend = true;
            break;
          }
        }
      }
    }

    if(isBlockedByFriend) {return false;}

    const int NR_TURNS_NO_RANGED = data_->rangedCooldownTurns;
    PropDisabledRanged* status =
      new PropDisabledRanged(PropTurns::specific, NR_TURNS_NO_RANGED);
    propHandler_->tryApplyProp(status);
    Attack::ranged(*this, *attack.weapon, defender.pos);
    return true;
  }

  return false;
}

AttackOpport Mon::getAttackOpport(Actor& defender)
{
  AttackOpport opport;
  if(propHandler_->allowAttack(false))
  {
    opport.isMelee = Utils::isPosAdj(pos, defender.pos, false);

    Wpn* weapon = nullptr;
    const size_t nrIntrinsics = inv_->getIntrinsicsSize();
    if(opport.isMelee)
    {
      if(propHandler_->allowAttackMelee(false))
      {

        //Melee weapon in wielded slot?
        weapon = static_cast<Wpn*>(inv_->getItemInSlot(SlotId::wielded));
        if(weapon)
        {
          if(weapon->getData().melee.isMeleeWpn)
          {
            opport.weapons.push_back(weapon);
          }
        }

        //Intrinsic melee attacks?
        for(size_t i = 0; i < nrIntrinsics; ++i)
        {
          weapon = static_cast<Wpn*>(inv_->getIntrinsicInElement(i));
          if(weapon->getData().melee.isMeleeWpn) {opport.weapons.push_back(weapon);}
        }
      }
    }
    else
    {
      if(propHandler_->allowAttackRanged(false))
      {
        //Ranged weapon in wielded slot?
        weapon =
          static_cast<Wpn*>(inv_->getItemInSlot(SlotId::wielded));

        if(weapon)
        {
          if(weapon->getData().ranged.isRangedWpn)
          {
            opport.weapons.push_back(weapon);

            //Check if reload time instead
            if(
              weapon->nrAmmoLoaded == 0 &&
              !weapon->getData().ranged.hasInfiniteAmmo)
            {
              if(inv_->hasAmmoForFirearmInInventory())
              {
                opport.isTimeToReload = true;
              }
            }
          }
        }

        //Intrinsic ranged attacks?
        for(size_t i = 0; i < nrIntrinsics; ++i)
        {
          weapon = static_cast<Wpn*>(inv_->getIntrinsicInElement(i));
          if(weapon->getData().ranged.isRangedWpn) {opport.weapons.push_back(weapon);}
        }
      }
    }
  }

  return opport;
}

//TODO Instead of using "strongest" weapon, use random
BestAttack Mon::getBestAttack(const AttackOpport& attackOpport)
{
  BestAttack attack;
  attack.isMelee = attackOpport.isMelee;

  Wpn* newWpn = nullptr;

  const size_t nrWpns = attackOpport.weapons.size();

  //If any possible attacks found
  if(nrWpns > 0)
  {
    attack.weapon = attackOpport.weapons[0];

    const ItemDataT* data = &(attack.weapon->getData());

    //If there are more than one possible weapon, find strongest.
    if(nrWpns > 1)
    {
      for(size_t i = 1; i < nrWpns; ++i)
      {
        //Found new weapon in element i.
        newWpn = attackOpport.weapons[i];
        const ItemDataT* newData = &(newWpn->getData());

        //Compare definitions.
        //If weapon i is stronger -
        if(ItemData::isWpnStronger(*data, *newData, attack.isMelee))
        {
          // - use new weapon instead.
          attack.weapon = newWpn;
          data = newData;
        }
      }
    }
  }
  return attack;
}

bool Mon::isLeaderOf(const Actor& actor) const
{
  if(actor.isPlayer())
  {
    return false;
  }
  else
  {
    return static_cast<const Mon*>(&actor)->leader == this;
  }
}

bool Mon::isActorMyLeader(const Actor& actor) const
{
  return leader == &actor;
}

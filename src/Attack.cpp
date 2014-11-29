#include "Attack.h"

#include "Init.h"

#include "Item.h"
#include "GameTime.h"
#include "ActorMon.h"
#include "Map.h"
#include "FeatureTrap.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"
#include "PlayerBon.h"
#include "MapParsing.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Utils.h"
#include "Log.h"
#include "LineCalc.h"
#include "Render.h"
#include "SdlWrapper.h"
#include "Knockback.h"

using namespace std;

AttData::AttData(Actor& attacker_, const Item& itemAttackedWith_) :
  attacker(&attacker_),
  defender(nullptr),
  attackResult(failSmall),
  nrDmgRolls(0),
  nrDmgSides(0),
  dmgPlus(0),
  dmgRoll(0),
  dmg(0),
  isIntrinsicAttack(false),
  isEtherealDefenderMissed(false)
{
  isIntrinsicAttack = itemAttackedWith_.getData().isIntrinsic;
}

MeleeAttData::MeleeAttData(Actor& attacker_, const Wpn& wpn_, Actor& defender_) :
  AttData(attacker_, wpn_),
  isDefenderDodging(false),
  isBackstab(false),
  isWeakAttack(false)
{

  defender = &defender_;

  const Pos& defPos = defender->pos;

  bool isDefenderAware = true;
  if (attacker == Map::player)
  {
    isDefenderAware = static_cast<Mon*>(defender)->awareCounter_ > 0;
  }
  else
  {
    isDefenderAware = Map::player->isSeeingActor(*attacker, nullptr) ||
                      PlayerBon::traitsPicked[int(Trait::vigilant)];
  }

  if (isDefenderAware)
  {
    const int DEFENDER_DODGE_SKILL =
      defender->getData().abilityVals.getVal(AbilityId::dodgeAttack, true, *defender);

    const int DODGE_MOD_AT_FEATURE =
      Map::cells[defPos.x][defPos.y].rigid->getDodgeModifier();

    const int DODGE_CHANCE_TOT = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

    if (DODGE_CHANCE_TOT > 0)
    {
      isDefenderDodging = AbilityRoll::roll(DODGE_CHANCE_TOT) >= successSmall;
    }
  }

  if (!isDefenderDodging)
  {
    //--------------------------------------- DETERMINE ATTACK RESULT
    const int ATTACKER_SKILL      = attacker->getData().abilityVals.getVal(
                                      AbilityId::melee, true, *attacker);
    const int WPN_HIT_CHANCE_MOD  = wpn_.getData().melee.hitChanceMod;

    int hitChanceTot              = ATTACKER_SKILL + WPN_HIT_CHANCE_MOD;

    bool isAttackerAware = true;
    if (attacker == Map::player)
    {
      isAttackerAware = Map::player->isSeeingActor(*defender, nullptr);
    }
    else
    {
      Mon* const mon = static_cast<Mon*>(attacker);
      isAttackerAware = mon->awareCounter_ > 0;
    }

    PropHandler& defPropHlr = defender->getPropHandler();
    bool defProps[endOfPropIds];
    defPropHlr.getActivePropIds(defProps);

    //If attacker is aware of the defender, check
    if (isAttackerAware)
    {
      bool isBigAttBon    = false;
      bool isSmallAttBon  = false;

      if (!isDefenderAware)
      {
        //Give big attack bonus if defender is unaware of the attacker.
        isBigAttBon = true;
      }

      if (!isBigAttBon)
      {
        //Give big attack bonus if defender is stuck in trap (web).
        const auto* const f = Map::cells[defPos.x][defPos.y].rigid;
        if (f->getId() == FeatureId::trap)
        {
          const auto* const t = static_cast<const Trap*>(f);
          if (t->getTrapType() == TrapId::web)
          {
            const auto* const web = static_cast<const TrapWeb*>(t->getSpecificTrap());
            if (web->isHolding())
            {
              isBigAttBon = true;
            }
          }
        }
      }

      if (!isBigAttBon)
      {
        //Check if attacker gets a bonus due to a defender property.

        if (defProps[propParalyzed] ||
            defProps[propNailed]    ||
            defProps[propFainted])
        {
          //Give big attack bonus if defender is completely unable to fight.
          isBigAttBon = true;
        }
        else if (defProps[propConfused] ||
                 defProps[propSlowed]   ||
                 defProps[propBurning])
        {
          //Give small attack bonus if defender has problems fighting.
          isSmallAttBon = true;
        }
      }

      //Give small attack bonus if defender cannot see.
      if (!isBigAttBon && !isSmallAttBon)
      {
        if (!defPropHlr.allowSee()) {isSmallAttBon = true;}
      }

      //Apply the hit chance bonus (if any)
      hitChanceTot += isBigAttBon ? 50 : (isSmallAttBon ? 20 : 0);
    }

    attackResult = AbilityRoll::roll(hitChanceTot);

    //Ethereal target missed?
    if (defProps[propEthereal])
    {
      isEtherealDefenderMissed = Rnd::fraction(2, 3);
    }

    //--------------------------------------- DETERMINE DAMAGE
    nrDmgRolls  = wpn_.getData().melee.dmg.first;
    nrDmgSides  = wpn_.getData().melee.dmg.second;
    dmgPlus     = wpn_.meleeDmgPlus_;

    bool attProps[endOfPropIds];
    attacker->getPropHandler().getActivePropIds(attProps);

    if (attProps[propWeakened])
    {
      //Weak attack (min damage)
      dmgRoll       = nrDmgRolls;
      dmg           = dmgRoll + dmgPlus;
      isWeakAttack  = true;
    }
    else
    {
      if (attackResult == successCritical)
      {
        //Critical hit (max damage)
        dmgRoll = nrDmgRolls * nrDmgSides;
        dmg     = max(0, dmgRoll + dmgPlus);
      }
      else
      {
        //Normal hit
        dmgRoll = Rnd::dice(nrDmgRolls, nrDmgSides);
        dmg     = max(0, dmgRoll + dmgPlus);
      }

      if (isAttackerAware && !isDefenderAware)
      {
        //Backstab (extra damage)

        int dmgPct = 150;

        //Double damage percent if attacking with a dagger.
        if (wpn_.getData().id == ItemId::dagger) {dmgPct *= 2;}

        //+50% if player and has the "Vicious" trait.
        if (attacker == Map::player)
        {
          if (PlayerBon::traitsPicked[int(Trait::vicious)])
          {
            dmgPct += 50;
          }
        }

        dmg = ((dmgRoll + dmgPlus) * dmgPct) / 100;

        isBackstab = true;
      }
    }
  }
}

RangedAttData::RangedAttData(Actor& attacker_, const Wpn& wpn_, const Pos& aimPos_,
                             const Pos& curPos_, ActorSize intendedAimLvl_) :
  AttData(attacker_, wpn_),
  hitChanceTot(0),
  intendedAimLvl(actorSize_none),
  defenderSize(actorSize_none),
  verbPlayerAttacks(wpn_.getData().ranged.attMsgs.player),
  verbOtherAttacks(wpn_.getData().ranged.attMsgs.other)
{

  Actor* const actorAimedAt = Utils::getActorAtPos(aimPos_);

  //If aim level parameter not given, determine it now
  if (intendedAimLvl_ == actorSize_none)
  {
    if (actorAimedAt)
    {
      intendedAimLvl = actorAimedAt->getData().actorSize;
    }
    else
    {
      bool blocked[MAP_W][MAP_H];
      MapParse::parse(CellCheck::BlocksProjectiles(), blocked);
      intendedAimLvl = blocked[curPos_.x][curPos_.y] ?
                       actorSize_humanoid : actorSize_floor;
    }
  }
  else
  {
    intendedAimLvl = intendedAimLvl_;
  }

  defender = Utils::getActorAtPos(curPos_);

  if (defender)
  {
    TRACE << "Defender found" << endl;
    const int ATTACKER_SKILL    = attacker->getData().abilityVals.getVal(
                                    AbilityId::ranged, true, *attacker);
    const int WPN_MOD           = wpn_.getData().ranged.hitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(defender->pos);
    const int DIST_TO_TGT       = Utils::kingDist(
                                    attPos.x, attPos.y, defPos.x, defPos.y);
    const int DIST_MOD          = 15 - (DIST_TO_TGT * 5);
    const ActorSpeed defSpeed   = defender->getData().speed;
    const int SPEED_MOD =
      defSpeed == ActorSpeed::sluggish ?  20 :
      defSpeed == ActorSpeed::slow     ?  10 :
      defSpeed == ActorSpeed::normal   ?   0 :
      defSpeed == ActorSpeed::fast     ? -10 : -30;
    defenderSize                = defender->getData().actorSize;
    const int SIZE_MOD          = defenderSize == actorSize_floor ? -10 : 0;

    int unawareDefMod = 0;
    const bool IS_ROGUE = PlayerBon::getBg() == Bg::rogue;
    if (attacker == Map::player && defender != Map::player && IS_ROGUE)
    {
      if (static_cast<Mon*>(defender)->awareCounter_ <= 0)
      {
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

    attackResult = AbilityRoll::roll(hitChanceTot);

    if (attackResult >= successSmall)
    {
      TRACE << "Attack roll succeeded" << endl;

      bool props[endOfPropIds];
      defender->getPropHandler().getActivePropIds(props);

      if (props[propEthereal])
      {
        isEtherealDefenderMissed = Rnd::fraction(2, 3);
      }

      bool playerAimX3 = false;
      if (attacker == Map::player)
      {
        const Prop* const prop =
          attacker->getPropHandler().getProp(propAiming, PropSrc::applied);
        if (prop)
        {
          playerAimX3 = static_cast<const PropAiming*>(prop)->isMaxRangedDmg();
        }
      }

      nrDmgRolls  = wpn_.getData().ranged.dmg.rolls;
      nrDmgSides  = wpn_.getData().ranged.dmg.sides;
      dmgPlus     = wpn_.getData().ranged.dmg.plus;

      dmgRoll     = playerAimX3 ? (nrDmgRolls * nrDmgSides) :
                    Rnd::dice(nrDmgRolls, nrDmgSides);
      dmg         = dmgRoll + dmgPlus;
    }
  }
}

ThrowAttData::ThrowAttData(Actor& attacker_, const Item& item_, const Pos& aimPos_,
                           const Pos& curPos_, ActorSize intendedAimLvl_) :
  AttData(attacker_, item_),
  hitChanceTot(0),
  intendedAimLvl(actorSize_none),
  defenderSize(actorSize_none)
{

  Actor* const actorAimedAt = Utils::getActorAtPos(aimPos_);

  //If aim level parameter not given, determine it now
  if (intendedAimLvl_ == actorSize_none)
  {
    if (actorAimedAt)
    {
      intendedAimLvl = actorAimedAt->getData().actorSize;
    }
    else
    {
      bool blocked[MAP_W][MAP_H];
      MapParse::parse(CellCheck::BlocksProjectiles(), blocked);
      intendedAimLvl = blocked[curPos_.x][curPos_.y] ?
                       actorSize_humanoid : actorSize_floor;
    }
  }
  else
  {
    intendedAimLvl = intendedAimLvl_;
  }

  defender = Utils::getActorAtPos(curPos_);

  if (defender)
  {
    TRACE << "Defender found" << endl;
    const int ATTACKER_SKILL    = attacker->getData().abilityVals.getVal(
                                    AbilityId::ranged, true, *attacker);
    const int WPN_MOD           = item_.getData().ranged.throwHitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(defender->pos);
    const int DIST_TO_TGT       = Utils::kingDist(
                                    attPos.x, attPos.y, defPos.x, defPos.y);
    const int DIST_MOD          = 15 - (DIST_TO_TGT * 5);
    const ActorSpeed defSpeed   = defender->getData().speed;
    const int SPEED_MOD =
      defSpeed == ActorSpeed::sluggish ?  20 :
      defSpeed == ActorSpeed::slow     ?  10 :
      defSpeed == ActorSpeed::normal   ?   0 :
      defSpeed == ActorSpeed::fast     ? -15 : -35;
    defenderSize                = defender->getData().actorSize;
    const int SIZE_MOD          = defenderSize == actorSize_floor ? -15 : 0;

    int unawareDefMod = 0;
    const bool IS_ROGUE = PlayerBon::getBg() == Bg::rogue;
    if (attacker == Map::player && defender != Map::player && IS_ROGUE)
    {
      if (static_cast<Mon*>(defender)->awareCounter_ <= 0)
      {
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

    attackResult = AbilityRoll::roll(hitChanceTot);

    if (attackResult >= successSmall)
    {
      TRACE << "Attack roll succeeded" << endl;

      bool props[endOfPropIds];
      defender->getPropHandler().getActivePropIds(props);

      if (props[propEthereal])
      {
        isEtherealDefenderMissed = Rnd::fraction(2, 3);
      }

      bool playerAimX3 = false;
      if (attacker == Map::player)
      {
        const Prop* const prop =
          attacker->getPropHandler().getProp(propAiming, PropSrc::applied);
        if (prop)
        {
          playerAimX3 = static_cast<const PropAiming*>(prop)->isMaxRangedDmg();
        }
      }

      nrDmgRolls  = item_.getData().ranged.throwDmg.rolls;
      nrDmgSides  = item_.getData().ranged.throwDmg.sides;
      dmgPlus     = item_.getData().ranged.throwDmg.plus;

      dmgRoll     = playerAimX3 ? (nrDmgRolls * nrDmgSides) :
                    Rnd::dice(nrDmgRolls, nrDmgSides);
      dmg         = dmgRoll + dmgPlus;
    }
  }
}

namespace Attack
{

namespace
{

void printMeleeMsgAndPlaySfx(const MeleeAttData& data, const Wpn& wpn)
{
  //No melee messages if player is not involved (reduce spam)
  if (data.attacker != Map::player && data.defender != Map::player) {return;}

  string otherName = "";

  if (data.isDefenderDodging)
  {
    //----- DEFENDER DODGES --------
    if (data.attacker == Map::player)
    {
      if (Map::player->isSeeingActor(*data.defender, nullptr))
      {
        otherName = data.defender->getNameThe();
      }
      else
      {
        otherName = "It ";
      }
      Log::addMsg(otherName + " dodges my attack.");
    }
    else
    {
      if (Map::player->isSeeingActor(*data.attacker, nullptr))
      {
        otherName = data.attacker->getNameThe();
      }
      else
      {
        otherName = "It";
      }
      Log::addMsg("I dodge an attack from " + otherName + ".", clrMsgGood);
    }
  }
  else if (data.attackResult <= failSmall)
  {
    //----- BAD AIMING --------
    if (data.attacker == Map::player)
    {
      if (data.attackResult == failSmall)
      {
        Log::addMsg("I barely miss!");
      }
      else if (data.attackResult == failNormal)
      {
        Log::addMsg("I miss.");
      }
      else if (data.attackResult == failBig)
      {
        Log::addMsg("I miss completely.");
      }
      Audio::play(wpn.getData().melee.missSfx);
    }
    else
    {
      if (Map::player->isSeeingActor(*data.attacker, nullptr))
      {
        otherName = data.attacker->getNameThe();
      }
      else
      {
        otherName = "It";
      }
      if (data.attackResult == failSmall)
      {
        Log::addMsg(otherName + " barely misses me!", clrWhite, true);
      }
      else if (data.attackResult == failNormal)
      {
        Log::addMsg(otherName + " misses me.", clrWhite, true);
      }
      else if (data.attackResult == failBig)
      {
        Log::addMsg(otherName + " misses me completely.", clrWhite, true);
      }
    }
  }
  else
  {
    //----- AIM IS CORRECT -------
    if (data.isEtherealDefenderMissed)
    {
      //----- ATTACK MISSED DUE TO ETHEREAL TARGET --------
      if (data.attacker == Map::player)
      {
        if (Map::player->isSeeingActor(*data.defender, nullptr))
        {
          otherName = data.defender->getNameThe();
        }
        else
        {
          otherName = "It ";
        }
        Log::addMsg(
          "My attack passes right through " + otherName + "!");
      }
      else
      {
        if (Map::player->isSeeingActor(*data.attacker, nullptr))
        {
          otherName = data.attacker->getNameThe();
        }
        else
        {
          otherName = "It";
        }
        Log::addMsg(
          "The attack of " + otherName + " passes right through me!",
          clrMsgGood);
      }
    }
    else
    {
      //----- ATTACK CONNECTS WITH DEFENDER --------
      //Determine the relative "size" of the hit
      MeleeHitSize hitSize = MeleeHitSize::small;
      const int MAX_DMG_ROLL = data.nrDmgRolls * data.nrDmgSides;
      if (MAX_DMG_ROLL >= 4)
      {
        if (data.dmgRoll > (MAX_DMG_ROLL * 5) / 6)
        {
          hitSize = MeleeHitSize::hard;
        }
        else if (data.dmgRoll >  MAX_DMG_ROLL / 2)
        {
          hitSize = MeleeHitSize::medium;
        }
      }

      //Punctuation depends on attack strength
      string dmgPunct = ".";
      switch (hitSize)
      {
        case MeleeHitSize::small:                     break;
        case MeleeHitSize::medium:  dmgPunct = "!";   break;
        case MeleeHitSize::hard:    dmgPunct = "!!!"; break;
      }

      if (data.attacker == Map::player)
      {
        const string wpnVerb = wpn.getData().melee.attMsgs.player;

        if (Map::player->isSeeingActor(*data.defender, nullptr))
        {
          otherName = data.defender->getNameThe();
        }
        else
        {
          otherName = "it";
        }

        if (data.isIntrinsicAttack)
        {
          const string ATTACK_MOD_STR = data.isWeakAttack ? " feebly" : "";
          Log::addMsg(
            "I " + wpnVerb + " " + otherName + ATTACK_MOD_STR + dmgPunct,
            clrMsgGood);
        }
        else
        {
          const string ATTACK_MOD_STR =
            data.isWeakAttack  ? "feebly "    :
            data.isBackstab    ? "covertly "  : "";
          const Clr     clr       = data.isBackstab ? clrBlueLgt : clrMsgGood;
          const string  wpnNameA  = wpn.getName(ItemRefType::a);
          Log::addMsg(
            "I " + wpnVerb + " " + otherName + " " + ATTACK_MOD_STR +
            "with " + wpnNameA + dmgPunct, clr);
        }
      }
      else
      {
        const string wpnVerb = wpn.getData().melee.attMsgs.other;

        if (Map::player->isSeeingActor(*data.attacker, nullptr))
        {
          otherName = data.attacker->getNameThe();
        }
        else
        {
          otherName = "It";
        }

        Log::addMsg(otherName + " " + wpnVerb + dmgPunct, clrMsgBad, true);
      }

      SfxId hitSfx = SfxId::END;
      switch (hitSize)
      {
        case MeleeHitSize::small:   {hitSfx = wpn.getData().melee.hitSmallSfx;}   break;
        case MeleeHitSize::medium:  {hitSfx = wpn.getData().melee.hitMediumSfx;}  break;
        case MeleeHitSize::hard:    {hitSfx = wpn.getData().melee.hitHardSfx;}    break;
      }
      Audio::play(hitSfx);
    }
  }
}

void printRangedInitiateMsgs(const RangedAttData& data)
{
  if (data.attacker == Map::player)
  {
    Log::addMsg("I " + data.verbPlayerAttacks + ".");
  }
  else
  {
    const Pos& p = data.attacker->pos;
    if (Map::cells[p.x][p.y].isSeenByPlayer)
    {
      const string attackerName = data.attacker->getNameThe();
      const string attackVerb = data.verbOtherAttacks;
      Log::addMsg(attackerName + " " + attackVerb + ".", clrWhite, true);
    }
  }
}

void printProjAtActorMsgs(const RangedAttData& data, const bool IS_HIT)
{
  //Only print messages if player can see the cell
  const int defX = data.defender->pos.x;
  const int defY = data.defender->pos.y;
  if (Map::cells[defX][defY].isSeenByPlayer)
  {

    //Punctuation or exclamation marks depending on attack strength
    if (IS_HIT)
    {
      string dmgPunctuation = ".";
      const int MAX_DMG_ROLL = data.nrDmgRolls * data.nrDmgSides;
      if (MAX_DMG_ROLL >= 4)
      {
        dmgPunctuation =
          data.dmgRoll > MAX_DMG_ROLL * 5 / 6 ? "!!!" :
          data.dmgRoll > MAX_DMG_ROLL / 2     ? "!"   : dmgPunctuation;
      }

      if (data.defender == Map::player)
      {
        Log::addMsg("I am hit" + dmgPunctuation, clrMsgBad, true);
      }
      else
      {
        string otherName = "It";

        if (Map::cells[defX][defY].isSeenByPlayer)
        {
          otherName = data.defender->getNameThe();
        }

        Log::addMsg(otherName + " is hit" + dmgPunctuation, clrMsgGood);
      }
    }
  }
}

void projectileFire(Actor& attacker, Wpn& wpn, const Pos& aimPos)
{
  const bool IS_ATTACKER_PLAYER = &attacker == Map::player;

  vector<Projectile*> projectiles;

  const bool IS_MACHINE_GUN = wpn.getData().ranged.isMachineGun;

  const int NR_PROJECTILES = IS_MACHINE_GUN ? NR_MG_PROJECTILES : 1;

  for (int i = 0; i < NR_PROJECTILES; ++i)
  {
    Projectile* const p = new Projectile;
    p->setAttData(new RangedAttData(attacker, wpn, aimPos, attacker.pos));
    projectiles.push_back(p);
  }

  const ActorSize aimLvl =
    projectiles[0]->attackData->intendedAimLvl;

  const int DELAY = Config::getDelayProjectileDraw() / (IS_MACHINE_GUN ? 2 : 1);

  printRangedInitiateMsgs(*projectiles[0]->attackData);

  const bool stopAtTarget = aimLvl == actorSize_floor;
  const int chebTrvlLim = 30;

  //Get projectile path
  const Pos origin = attacker.pos;
  vector<Pos> path;
  LineCalc::calcNewLine(origin, aimPos, stopAtTarget, chebTrvlLim, false, path);

  const Clr projectileClr = wpn.getData().ranged.missileClr;
  char projectileGlyph    = wpn.getData().ranged.missileGlyph;

  if (projectileGlyph == '/')
  {
    const int i = path.size() > 2 ? 2 : 1;
    if (path[i].y == origin.y) {projectileGlyph = '-';}
    if (path[i].x == origin.x) {projectileGlyph = '|';}
    if ((path[i].x > origin.x && path[i].y < origin.y) ||
        (path[i].x < origin.x && path[i].y > origin.y))
    {
      projectileGlyph = '/';
    }
    if ((path[i].x > origin.x && path[i].y > origin.y) ||
        (path[i].x < origin.x && path[i].y < origin.y))
    {
      projectileGlyph = '\\';
    }
  }
  TileId projectileTile = wpn.getData().ranged.missileTile;
  if (projectileTile == TileId::projectileStdFrontSlash)
  {
    if (projectileGlyph == '-')  {projectileTile = TileId::projectileStdDash;}
    if (projectileGlyph == '|')  {projectileTile = TileId::projectileStdVerticalBar;}
    if (projectileGlyph == '\\') {projectileTile = TileId::projectileStdBackSlash;}
  }

  const bool LEAVE_TRAIL = wpn.getData().ranged.missileLeavesTrail;

  const int SIZE_OF_PATH_PLUS_ONE =
    path.size() + (NR_PROJECTILES - 1) * NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES;

  for (int i = 1; i < SIZE_OF_PATH_PLUS_ONE; ++i)
  {

    for (int pCnt = 0; pCnt < NR_PROJECTILES; ++pCnt)
    {
      //Current projectile's place in the path is the current global place (i) minus a
      //certain number of elements
      int pathElement = i - (pCnt * NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES);

      //Emit sound
      if (pathElement == 1)
      {
        string sndMsg   = wpn.getData().ranged.sndMsg;
        const SfxId sfx = wpn.getData().ranged.attSfx;
        if (!sndMsg.empty())
        {
          if (IS_ATTACKER_PLAYER) sndMsg = "";
          const SndVol vol = wpn.getData().ranged.sndVol;
          SndEmit::emitSnd({sndMsg, sfx, IgnoreMsgIfOriginSeen::yes, attacker.pos,
                            &attacker, vol, AlertsMon::yes
                           });
        }
      }

      Projectile* const curProj = projectiles[pCnt];

      //All the following collision checks etc are only made if the projectiles
      //current path element corresponds to an element in the real path vector
      if (pathElement >= 1 && pathElement < int(path.size()) && !curProj->isObstructed)
      {
        curProj->pos = path[pathElement];

        curProj->isVisibleToPlayer =
          Map::cells[curProj->pos.x][curProj->pos.y].isSeenByPlayer;

        //Get attack data again for every cell traveled through
        curProj->setAttData(
          new RangedAttData(attacker, wpn, aimPos, curProj->pos , aimLvl));

        const Pos drawPos(curProj->pos);

        //HIT ACTOR?
        if (curProj->attackData->defender &&
            !curProj->isObstructed        &&
            !curProj->attackData->isEtherealDefenderMissed)
        {
          const bool IS_ACTOR_AIMED_FOR = curProj->pos == aimPos;

          if (curProj->attackData->defenderSize >= actorSize_humanoid ||
              IS_ACTOR_AIMED_FOR)
          {

            if (curProj->attackData->attackResult >= successSmall)
            {
              //RENDER ACTOR HIT
              if (curProj->isVisibleToPlayer)
              {
                if (Config::isTilesMode())
                {
                  curProj->setTile(TileId::blast1, clrRedLgt);
                  Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
                  SdlWrapper::sleep(DELAY / 2);
                  curProj->setTile(TileId::blast2, clrRedLgt);
                  Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
                  SdlWrapper::sleep(DELAY / 2);
                }
                else //Not tile mode
                {
                  curProj->setGlyph('*', clrRedLgt);
                  Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
                  SdlWrapper::sleep(DELAY);
                }

                //MESSAGES FOR ACTOR HIT
                printProjAtActorMsgs(*curProj->attackData, true);
                //Need to draw again here to show log message
                Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
              }

              curProj->isDoneRendering      = true;
              curProj->isObstructed         = true;
              curProj->actorHit             = curProj->attackData->defender;
              curProj->obstructedInElement  = pathElement;

              const ActorDied died = curProj->actorHit->hit(curProj->attackData->dmg,
                                     wpn.getData().ranged.dmgType);

              if (died == ActorDied::no)
              {
                //Hit properties
                PropHandler& defenderPropHandler = curProj->actorHit->getPropHandler();
                defenderPropHandler.tryApplyPropFromWpn(wpn, false);

                //Knock-back?
                if (wpn.getData().ranged.isKnockback)
                {
                  const AttData* const curData = curProj->attackData;
                  if (curData->attackResult >= successSmall)
                  {
                    const bool IS_SPIKE_GUN = wpn.getData().id == ItemId::spikeGun;
                    KnockBack::tryKnockBack(
                      *(curData->defender), curData->attacker->pos, IS_SPIKE_GUN);
                  }
                }
              }
            }
          }
        }

        //PROJECTILE HIT FEATURE?
        vector<Mob*> mobs;
        GameTime::getMobsAtPos(curProj->pos, mobs);
        Feature* featureBlockingShot = nullptr;
        for (auto* mob : mobs)
        {
          if (!mob->isProjectilePassable()) {featureBlockingShot = mob;}
        }
        Rigid* rigid =
          Map::cells[curProj->pos.x][curProj->pos.y].rigid;
        if (!rigid->isProjectilePassable())
        {
          featureBlockingShot = rigid;
        }

        if (featureBlockingShot && !curProj->isObstructed)
        {
          curProj->obstructedInElement = pathElement - 1;
          curProj->isObstructed = true;

          if (wpn.getData().ranged.makesRicochetSnd)
          {
            Snd snd("I hear a ricochet.", SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes, curProj->pos, nullptr,
                    SndVol::low, AlertsMon::yes);
            SndEmit::emitSnd(snd);
          }

          //RENDER FEATURE HIT
          if (curProj->isVisibleToPlayer)
          {
            if (Config::isTilesMode())
            {
              curProj->setTile(TileId::blast1, clrYellow);
              Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
              curProj->setTile(TileId::blast2, clrYellow);
              Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
            }
            else
            {
              curProj->setGlyph('*', clrYellow);
              Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY);
            }
          }
        }

        //PROJECTILE HIT THE GROUND?
        if (curProj->pos == aimPos && aimLvl == actorSize_floor &&
            !curProj->isObstructed)
        {
          curProj->isObstructed = true;
          curProj->obstructedInElement = pathElement;

          if (wpn.getData().ranged.makesRicochetSnd)
          {
            Snd snd("I hear a ricochet.", SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes, curProj->pos, nullptr,
                    SndVol::low, AlertsMon::yes);
            SndEmit::emitSnd(snd);
          }

          //RENDER GROUND HITS
          if (curProj->isVisibleToPlayer)
          {
            if (Config::isTilesMode())
            {
              curProj->setTile(TileId::blast1, clrYellow);
              Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
              curProj->setTile(TileId::blast2, clrYellow);
              Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
            }
            else
            {
              curProj->setGlyph('*', clrYellow);
              Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY);
            }
          }
        }

        //RENDER FLYING PROJECTILES
        if (!curProj->isObstructed && curProj->isVisibleToPlayer)
        {
          if (Config::isTilesMode())
          {
            curProj->setTile(projectileTile, projectileClr);
            Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
          }
          else
          {
            curProj->setGlyph(projectileGlyph, projectileClr);
            Render::drawProjectiles(projectiles, !LEAVE_TRAIL);
          }
        }
      }
    } //End projectile loop

    //If any projectile can be seen and not obstructed, delay
    for (Projectile* projectile : projectiles)
    {
      const Pos& pos = projectile->pos;
      if (Map::cells[pos.x][pos.y].isSeenByPlayer &&
          !projectile->isObstructed)
      {
        SdlWrapper::sleep(DELAY);
        break;
      }
    }

    //Check if all projectiles obstructed
    bool isAllObstructed = true;
    for (Projectile* projectile : projectiles)
    {
      if (!projectile->isObstructed) {isAllObstructed = false;}
    }
    if (isAllObstructed) {break;}

  } //End path-loop

  //So far, only projectile 0 can have special obstruction events***
  //Must be changed if something like an assault-incinerator is added
  const Projectile* const firstProjectile = projectiles[0];
  if (!firstProjectile->isObstructed)
  {
    wpn.onProjectileBlocked(aimPos, firstProjectile->actorHit);
  }
  else
  {
    const int element = firstProjectile->obstructedInElement;
    const Pos& pos = path[element];
    wpn.onProjectileBlocked(pos, firstProjectile->actorHit);
  }
  //Cleanup
  for (Projectile* projectile : projectiles) {delete projectile;}

  Render::drawMapAndInterface();
}

void shotgun(Actor& attacker, const Wpn& wpn, const Pos& aimPos)
{
  RangedAttData* data = new RangedAttData(attacker, wpn, aimPos, attacker.pos);

  printRangedInitiateMsgs(*data);

  const ActorSize intendedAimLvl = data->intendedAimLvl;

  bool featureBlockers[MAP_W][MAP_H];
  MapParse::parse(CellCheck::BlocksProjectiles(), featureBlockers);

  Actor* actorArray[MAP_W][MAP_H];
  Utils::mkActorArray(actorArray);

  const Pos origin = attacker.pos;
  vector<Pos> path;
  LineCalc::calcNewLine(origin, aimPos, false, 9999, false, path);

  int nrActorsHit = 0;

  int nrMonKilledInElem = -1;

  //Emit sound
  const bool IS_ATTACKER_PLAYER = &attacker == Map::player;
  string sndMsg = wpn.getData().ranged.sndMsg;
  if (!sndMsg.empty())
  {
    if (IS_ATTACKER_PLAYER) {sndMsg = "";}
    const SndVol vol  = wpn.getData().ranged.sndVol;
    const SfxId sfx   = wpn.getData().ranged.attSfx;
    Snd snd(sndMsg, sfx, IgnoreMsgIfOriginSeen::yes, attacker.pos, &attacker,
            vol, AlertsMon::yes);
    SndEmit::emitSnd(snd);
  }

  for (size_t i = 1; i < path.size(); ++i)
  {
    //If traveled more than two steps after a killed monster, stop projectile.
    if (nrMonKilledInElem != -1 && int(i) > nrMonKilledInElem + 1) {break;}

    const Pos curPos(path[i]);

    if (actorArray[curPos.x][curPos.y])
    {
      //Only attempt hit if aiming at a level that would hit the actor
      const ActorSize sizeOfActor =
        actorArray[curPos.x][curPos.y]->getData().actorSize;
      if (sizeOfActor >= actorSize_humanoid || curPos == aimPos)
      {
        //Actor hit?
        delete data;
        data = new RangedAttData(attacker, wpn, aimPos, curPos, intendedAimLvl);

        const bool IS_WITHIN_RANGE_LMT =
          Utils::kingDist(origin, curPos) <= wpn.effectiveRangeLmt;

        if (IS_WITHIN_RANGE_LMT                 &&
            data->attackResult >= successSmall  &&
            !data->isEtherealDefenderMissed)
        {
          if (Map::cells[curPos.x][curPos.y].isSeenByPlayer)
          {
            Render::drawMapAndInterface(false);
            Render::coverCellInMap(curPos);
            if (Config::isTilesMode())
            {
              Render::drawTile(TileId::blast2, Panel::map, curPos, clrRedLgt);
            }
            else
            {
              Render::drawGlyph('*', Panel::map, curPos, clrRedLgt);
            }
            Render::updateScreen();
            SdlWrapper::sleep(Config::getDelayShotgun());
          }

          //Messages
          printProjAtActorMsgs(*data, true);

          //Damage
          data->defender->hit(data->dmg, wpn.getData().ranged.dmgType);

          nrActorsHit++;

          Render::drawMapAndInterface();

          //Special shotgun behavior:
          //If current defender was killed, and player aimed at humanoid level,
          //or at floor level but beyond the current position, the shot will
          //continue one cell.
          const bool IS_TGT_KILLED = !data->defender->isAlive();
          if (IS_TGT_KILLED && nrMonKilledInElem == -1)
          {
            nrMonKilledInElem = i;
          }
          if (nrActorsHit >= 2 || !IS_TGT_KILLED ||
              (intendedAimLvl == actorSize_floor && curPos == aimPos))
          {
            break;
          }
        }
      }
    }

    //Wall hit?
    if (featureBlockers[curPos.x][curPos.y])
    {

      //TODO Check hit material (soft and wood should not cause ricochet)

      Snd snd("I hear a ricochet.", SfxId::ricochet, IgnoreMsgIfOriginSeen::yes,
              curPos, nullptr, SndVol::low, AlertsMon::yes);
      SndEmit::emitSnd(snd);

      Cell& cell = Map::cells[curPos.x][curPos.y];

      if (cell.isSeenByPlayer)
      {
        Render::drawMapAndInterface(false);
        Render::coverCellInMap(curPos);
        if (Config::isTilesMode())
        {
          Render::drawTile(TileId::blast2, Panel::map, curPos, clrYellow);
        }
        else
        {
          Render::drawGlyph('*', Panel::map, curPos, clrYellow);
        }
        Render::updateScreen();
        SdlWrapper::sleep(Config::getDelayShotgun());
        Render::drawMapAndInterface();
      }

      cell.rigid->hit(DmgType::physical, DmgMethod::shotgun, nullptr);

      break;
    }

    //Floor hit?
    if (intendedAimLvl == actorSize_floor && curPos == aimPos)
    {
      Snd snd("I hear a ricochet.", SfxId::ricochet, IgnoreMsgIfOriginSeen::yes,
              curPos, nullptr, SndVol::low, AlertsMon::yes);
      SndEmit::emitSnd(snd);

      if (Map::cells[curPos.x][curPos.y].isSeenByPlayer)
      {
        Render::drawMapAndInterface(false);
        Render::coverCellInMap(curPos);
        if (Config::isTilesMode())
        {
          Render::drawTile(TileId::blast2, Panel::map, curPos, clrYellow);
        }
        else
        {
          Render::drawGlyph('*', Panel::map, curPos, clrYellow);
        }
        Render::updateScreen();
        SdlWrapper::sleep(Config::getDelayShotgun());
        Render::drawMapAndInterface();
      }
      break;
    }
  }
}

} //namespace

void melee(Actor& attacker, const Wpn& wpn, Actor& defender)
{
  MeleeAttData data(attacker, wpn, defender);

  printMeleeMsgAndPlaySfx(data, wpn);

  if (!data.isEtherealDefenderMissed)
  {
    if (data.attackResult >= successSmall && !data.isDefenderDodging)
    {
      const ActorDied died =
        data.defender->hit(data.dmg, wpn.getData().melee.dmgType);

      if (died == ActorDied::no)
      {
        data.defender->getPropHandler().tryApplyPropFromWpn(wpn, true);
      }
      if (data.attackResult >= successNormal)
      {
        if (data.defender->getData().canBleed)
        {
          Map::mkBlood(data.defender->pos);
        }
      }
      if (died == ActorDied::no)
      {
        if (wpn.getData().melee.isKnockback)
        {
          if (data.attackResult > successSmall)
          {
            KnockBack::tryKnockBack(*(data.defender), data.attacker->pos, false);
          }
        }
      }
      const ItemDataT& itemData = wpn.getData();
      if (itemData.itemWeight > itemWeight_light && !itemData.isIntrinsic)
      {
        Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes,
                data.defender->pos, nullptr, SndVol::low, AlertsMon::yes);
        SndEmit::emitSnd(snd);
      }
    }
  }

  if (data.defender == Map::player)
  {
    if (data.attackResult >= failSmall)
    {
      static_cast<Mon*>(data.attacker)->isStealth_ = false;
    }
  }
  else
  {
    Mon* const mon = static_cast<Mon*>(data.defender);
    mon->awareCounter_ = mon->getData().nrTurnsAware;
  }
  GameTime::actorDidAct();
}

bool ranged(Actor& attacker, Wpn& wpn, const Pos& aimPos)
{
  bool didAttack = false;

  const bool HAS_INF_AMMO = wpn.getData().ranged.hasInfiniteAmmo;

  if (wpn.getData().ranged.isShotgun)
  {
    if (wpn.nrAmmoLoaded != 0 || HAS_INF_AMMO)
    {

      shotgun(attacker, wpn, aimPos);

      didAttack = true;
      if (!HAS_INF_AMMO) {wpn.nrAmmoLoaded -= 1;}
    }
  }
  else
  {
    int nrOfProjectiles = 1;

    if (wpn.getData().ranged.isMachineGun) {nrOfProjectiles = NR_MG_PROJECTILES;}

    if (wpn.nrAmmoLoaded >= nrOfProjectiles || HAS_INF_AMMO)
    {
      projectileFire(attacker, wpn, aimPos);

      if (Map::player->isAlive())
      {

        didAttack = true;

        if (!HAS_INF_AMMO) {wpn.nrAmmoLoaded -= nrOfProjectiles;}
      }
      else
      {
        return true;
      }
    }
  }

  Render::drawMapAndInterface();

  if (didAttack) {GameTime::actorDidAct();}

  return didAttack;
}

} //Attack

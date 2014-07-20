#include "Attack.h"

#include "Init.h"

#include "Item.h"
#include "ItemWeapon.h"
#include "GameTime.h"
#include "ActorMonster.h"
#include "Map.h"
#include "FeatureTrap.h"
#include "FeatureStatic.h"
#include "FeatureMob.h"
#include "PlayerBon.h"
#include "MapParsing.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "Utils.h"
#include "Log.h"
#include "LineCalc.h"
#include "Renderer.h"
#include "SdlWrapper.h"
#include "KnockBack.h"

using namespace std;

AttData::AttData(Actor& attacker_, const Item& itemAttackedWith_) :
  attacker(&attacker_),
  curDefender(nullptr),
  attackResult(failSmall),
  dmgRolls(0),
  dmgSides(0),
  dmgPlus(0),
  dmgRoll(0),
  dmg(0),
  isIntrinsicAttack(false),
  isEtherealDefenderMissed(false) {
  isIntrinsicAttack = itemAttackedWith_.getData().isIntrinsic;
}

MeleeAttData::MeleeAttData(Actor& attacker_, const Weapon& wpn_,
                           Actor& defender_) :
  AttData(attacker_, wpn_), isDefenderDodging(false),
  isBackstab(false), isWeakAttack(false) {

  curDefender = &defender_;

  const Pos& defPos = curDefender->pos;

  bool isDefenderAware = true;
  if(attacker == Map::player) {
    isDefenderAware =
      static_cast<Monster*>(curDefender)->awareOfPlayerCounter_ > 0;
  } else {
    isDefenderAware =
      Map::player->isSeeingActor(*attacker, nullptr) ||
      PlayerBon::hasTrait(Trait::vigilant);
  }

  isDefenderDodging = false;
  if(isDefenderAware) {
    const int DEFENDER_DODGE_SKILL =
      curDefender->getData().abilityVals.getVal(
        AbilityId::dodgeAttack, true, *curDefender);

    const int DODGE_MOD_AT_FEATURE =
      Map::cells[defPos.x][defPos.y].featureStatic->getDodgeModifier();

    const int DODGE_CHANCE_TOT = DEFENDER_DODGE_SKILL + DODGE_MOD_AT_FEATURE;

    if(DODGE_CHANCE_TOT > 0) {
      isDefenderDodging =
        AbilityRoll::roll(DODGE_CHANCE_TOT) >= successSmall;
    }
  }

  if(!isDefenderDodging) {
    //--------------------------------------- DETERMINE ATTACK RESULT
    isBackstab = false;

    const int ATTACKER_SKILL      = attacker->getData().abilityVals.getVal(
                                      AbilityId::melee, true, *attacker);
    const int WPN_HIT_CHANCE_MOD  = wpn_.getData().meleeHitChanceMod;

    int hitChanceTot              = ATTACKER_SKILL + WPN_HIT_CHANCE_MOD;

    bool isAttackerAware = true;
    if(attacker == Map::player) {
      isAttackerAware = Map::player->isSeeingActor(*curDefender, nullptr);
    } else {
      Monster* const monster = static_cast<Monster*>(attacker);
      isAttackerAware = monster->awareOfPlayerCounter_ > 0;
    }

    PropHandler& defPropHlr = curDefender->getPropHandler();
    vector<PropId> defProps;
    defPropHlr.getAllActivePropIds(defProps);

    if(isAttackerAware) {
      bool isBigBon   = false;
      bool isSmallBon = false;

      const auto* const f = Map::cells[defPos.x][defPos.y].featureStatic;
      if(f->getId() == FeatureId::trap) {
        const auto* const t = static_cast<const Trap*>(f);
        if(t->getTrapType() == TrapId::spiderWeb) {
          const auto* const web =
            static_cast<const TrapSpiderWeb*>(t->getSpecificTrap());
          if(web->isHolding()) {
            isBigBon = true;
          }
        }
      }

      for(PropId propId : defProps) {
        if(
          !isDefenderAware        ||
          propId == propParalyzed ||
          propId == propNailed    ||
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
      if(!isBigBon && !isSmallBon && !defPropHlr.allowSee()) {
        isSmallBon = true;
      }

      hitChanceTot += isBigBon ? 50 : isSmallBon ? 20 : 0;
    }

    attackResult = AbilityRoll::roll(hitChanceTot);

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
    } else if(isAttackerAware && !isDefenderAware) {
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

RangedAttData::RangedAttData(
  Actor& attacker_, const Weapon& wpn_, const Pos& aimPos_,
  const Pos& curPos_, ActorSize intendedAimLvl_) :
  AttData(attacker_, wpn_), hitChanceTot(0),
  intendedAimLvl(actorSize_none), curDefenderSize(actorSize_none),
  verbPlayerAttacks(""), verbOtherAttacks("")  {

  verbPlayerAttacks = wpn_.getData().rangedAttMsgs.player;
  verbOtherAttacks  = wpn_.getData().rangedAttMsgs.other;

  Actor* const actorAimedAt = Utils::getActorAtPos(aimPos_);

  //If aim level parameter not given, determine it now
  if(intendedAimLvl_ == actorSize_none) {
    if(actorAimedAt) {
      intendedAimLvl = actorAimedAt->getData().actorSize;
    } else {
      bool blocked[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksProjectiles(), blocked);
      intendedAimLvl = blocked[curPos_.x][curPos_.y] ?
                       actorSize_humanoid : actorSize_floor;
    }
  } else {
    intendedAimLvl = intendedAimLvl_;
  }

  curDefender = Utils::getActorAtPos(curPos_);

  if(curDefender) {
    TRACE << "RangedAttData: Defender found" << endl;
    const int ATTACKER_SKILL    = attacker->getData().abilityVals.getVal(
                                    AbilityId::ranged, true, *attacker);
    const int WPN_MOD           = wpn_.getData().rangedHitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(curDefender->pos);
    const int DIST_TO_TGT       = Utils::kingDist(
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
    const bool IS_ROGUE = PlayerBon::getBg() == Bg::rogue;
    if(attacker == Map::player && curDefender != Map::player && IS_ROGUE) {
      if(static_cast<Monster*>(curDefender)->awareOfPlayerCounter_ <= 0) {
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

    if(attackResult >= successSmall) {
      TRACE << "RangedAttData: Attack roll succeeded" << endl;

      vector<PropId> props;
      curDefender->getPropHandler().getAllActivePropIds(props);

      if(find(begin(props), end(props), propEthereal) != end(props)) {
        isEtherealDefenderMissed = Rnd::fraction(2, 3);
      }

      bool playerAimX3 = false;
      if(attacker == Map::player) {
        const Prop* const prop =
          attacker->getPropHandler().getProp(propAiming, PropSrc::applied);
        if(prop) {
          playerAimX3 = static_cast<const PropAiming*>(prop)->isMaxRangedDmg();
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

MissileAttData::MissileAttData(Actor& attacker_, const Item& item_,
                               const Pos& aimPos_, const Pos& curPos_,
                               ActorSize intendedAimLvl_) :
  AttData(attacker_, item_), hitChanceTot(0),
  intendedAimLvl(actorSize_none), curDefenderSize(actorSize_none) {

  Actor* const actorAimedAt = Utils::getActorAtPos(aimPos_);

  //If aim level parameter not given, determine it now
  if(intendedAimLvl_ == actorSize_none) {
    if(actorAimedAt) {
      intendedAimLvl = actorAimedAt->getData().actorSize;
    } else {
      bool blocked[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksProjectiles(), blocked);
      intendedAimLvl = blocked[curPos_.x][curPos_.y] ?
                       actorSize_humanoid : actorSize_floor;
    }
  } else {
    intendedAimLvl = intendedAimLvl_;
  }

  curDefender = Utils::getActorAtPos(curPos_);

  if(curDefender) {
    TRACE << "MissileAttData: Defender found" << endl;
    const int ATTACKER_SKILL    = attacker->getData().abilityVals.getVal(
                                    AbilityId::ranged, true, *attacker);
    const int WPN_MOD           = item_.getData().missileHitChanceMod;
    const Pos& attPos(attacker->pos);
    const Pos& defPos(curDefender->pos);
    const int DIST_TO_TGT       = Utils::kingDist(
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
    const bool IS_ROGUE = PlayerBon::getBg() == Bg::rogue;
    if(attacker == Map::player && curDefender != Map::player && IS_ROGUE) {
      if(static_cast<Monster*>(curDefender)->awareOfPlayerCounter_ <= 0) {
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

    if(attackResult >= successSmall) {
      TRACE << "MissileAttData: Attack roll succeeded" << endl;

      bool playerAimX3 = false;
      if(attacker == Map::player) {
        const Prop* const prop =
          attacker->getPropHandler().getProp(propAiming, PropSrc::applied);
        if(prop) {
          playerAimX3 = static_cast<const PropAiming*>(prop)->isMaxRangedDmg();
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

namespace Attack {

namespace {

void printMeleeMsgAndPlaySfx(const MeleeAttData& data, const Weapon& wpn) {
  string otherName = "";

  if(data.isDefenderDodging) {
    //----- DEFENDER DODGES --------
    if(data.attacker == Map::player) {
      if(Map::player->isSeeingActor(*data.curDefender, nullptr)) {
        otherName = data.curDefender->getNameThe();
      } else {
        otherName = "It ";
      }
      Log::addMsg(otherName + " dodges my attack.");
    } else {
      if(Map::player->isSeeingActor(*data.attacker, nullptr)) {
        otherName = data.attacker->getNameThe();
      } else {
        otherName = "It";
      }
      Log::addMsg("I dodge an attack from " + otherName + ".", clrMsgGood);
    }
  } else if(data.attackResult <= failSmall) {
    //----- BAD AIMING --------
    if(data.attacker == Map::player) {
      if(data.attackResult == failSmall) {
        Log::addMsg("I barely miss!");
      } else if(data.attackResult == failNormal) {
        Log::addMsg("I miss.");
      } else if(data.attackResult == failBig) {
        Log::addMsg("I miss completely.");
      }
      Audio::play(wpn.getData().meleeMissSfx);
    } else {
      if(Map::player->isSeeingActor(*data.attacker, nullptr)) {
        otherName = data.attacker->getNameThe();
      } else {
        otherName = "It";
      }
      if(data.attackResult == failSmall) {
        Log::addMsg(otherName + " barely misses me!", clrWhite, true);
      } else if(data.attackResult == failNormal) {
        Log::addMsg(otherName + " misses me.", clrWhite, true);
      } else if(data.attackResult == failBig) {
        Log::addMsg(otherName + " misses me completely.", clrWhite, true);
      }
    }
  } else {
    //----- AIM IS CORRECT -------
    if(data.isEtherealDefenderMissed) {
      //----- ATTACK MISSED DUE TO ETHEREAL TARGET --------
      if(data.attacker == Map::player) {
        if(Map::player->isSeeingActor(*data.curDefender, nullptr)) {
          otherName = data.curDefender->getNameThe();
        } else {
          otherName = "It ";
        }
        Log::addMsg(
          "My attack passes right through " + otherName + "!");
      } else {
        if(Map::player->isSeeingActor(*data.attacker, nullptr)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }
        Log::addMsg(
          "The attack of " + otherName + " passes right through me!",
          clrMsgGood);
      }
    } else {
      //----- ATTACK CONNECTS WITH DEFENDER --------
      //Determine the relative "size" of the hit
      MeleeHitSize hitSize = MeleeHitSize::small;
      const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
      if(MAX_DMG_ROLL >= 4) {
        if(data.dmgRoll > (MAX_DMG_ROLL * 5) / 6) {
          hitSize = MeleeHitSize::hard;
        } else if(data.dmgRoll >  MAX_DMG_ROLL / 2) {
          hitSize = MeleeHitSize::medium;
        }
      }

      //Punctuation depends on attack strength
      string dmgPunct = ".";
      switch(hitSize) {
        case MeleeHitSize::small:                     break;
        case MeleeHitSize::medium:  dmgPunct = "!";   break;
        case MeleeHitSize::hard:    dmgPunct = "!!!"; break;
      }

      if(data.attacker == Map::player) {
        const string wpnVerb = wpn.getData().meleeAttMsgs.player;

        if(Map::player->isSeeingActor(*data.curDefender, nullptr)) {
          otherName = data.curDefender->getNameThe();
        } else {
          otherName = "it";
        }

        if(data.isIntrinsicAttack) {
          const string ATTACK_MOD_STR = data.isWeakAttack ? " feebly" : "";
          Log::addMsg(
            "I " + wpnVerb + " " + otherName + ATTACK_MOD_STR + dmgPunct,
            clrMsgGood);
        } else {
          const string ATTACK_MOD_STR =
            data.isWeakAttack  ? "feebly "    :
            data.isBackstab    ? "covertly "  : "";
          const Clr clr =
            data.isBackstab ? clrBlueLgt : clrMsgGood;
          const string wpnName_a =
            ItemData::getItemRef(wpn, ItemRefType::a, true);
          Log::addMsg(
            "I " + wpnVerb + " " + otherName + " " + ATTACK_MOD_STR +
            "with " + wpnName_a + dmgPunct,
            clr);
        }
      } else {
        const string wpnVerb = wpn.getData().meleeAttMsgs.other;

        if(Map::player->isSeeingActor(*data.attacker, nullptr)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }

        Log::addMsg(otherName + " " + wpnVerb + dmgPunct,
                    clrMsgBad, true);
      }

      SfxId hitSfx = SfxId::END;
      switch(hitSize) {
        case MeleeHitSize::small: {
          hitSfx = wpn.getData().meleeHitSmallSfx;
        } break;
        case MeleeHitSize::medium: {
          hitSfx = wpn.getData().meleeHitMediumSfx;
        } break;
        case MeleeHitSize::hard: {
          hitSfx = wpn.getData().meleeHitHardSfx;
        } break;
      }
      Audio::play(hitSfx);
    }
  }
}

void printRangedInitiateMsgs(const RangedAttData& data) {
  if(data.attacker == Map::player) {
    Log::addMsg("I " + data.verbPlayerAttacks + ".");
  } else {
    const Pos& p = data.attacker->pos;
    if(Map::cells[p.x][p.y].isSeenByPlayer) {
      const string attackerName = data.attacker->getNameThe();
      const string attackVerb = data.verbOtherAttacks;
      Log::addMsg(attackerName + " " + attackVerb + ".", clrWhite, true);
    }
  }
}

void printProjAtActorMsgs(const RangedAttData& data, const bool IS_HIT) {
  //Only print messages if player can see the cell
  const int defX = data.curDefender->pos.x;
  const int defY = data.curDefender->pos.y;
  if(Map::cells[defX][defY].isSeenByPlayer) {

    //Punctuation or exclamation marks depending on attack strength
    if(IS_HIT) {
      string dmgPunctuation = ".";
      const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
      if(MAX_DMG_ROLL >= 4) {
        dmgPunctuation =
          data.dmgRoll > MAX_DMG_ROLL * 5 / 6 ? "!!!" :
          data.dmgRoll > MAX_DMG_ROLL / 2 ? "!" :
          dmgPunctuation;
      }

      if(data.curDefender == Map::player) {
        Log::addMsg("I am hit" + dmgPunctuation, clrMsgBad, true);
      } else {
        string otherName = "It";

        if(Map::cells[defX][defY].isSeenByPlayer)
          otherName = data.curDefender->getNameThe();

        Log::addMsg(otherName + " is hit" + dmgPunctuation, clrMsgGood);
      }
    }
  }
}

void projectileFire(Actor& attacker, Weapon& wpn, const Pos& aimPos) {
  const bool IS_ATTACKER_PLAYER = &attacker == Map::player;

  vector<Projectile*> projectiles;

  const bool IS_MACHINE_GUN = wpn.getData().isMachineGun;

  const int NR_PROJECTILES = IS_MACHINE_GUN ?
                             NR_MG_PROJECTILES : 1;

  for(int i = 0; i < NR_PROJECTILES; ++i) {
    Projectile* const p = new Projectile;
    p->setAttData(new RangedAttData(
                    attacker, wpn, aimPos, attacker.pos));
    projectiles.push_back(p);
  }

  const ActorSize aimLvl =
    projectiles.at(0)->attackData->intendedAimLvl;

  const int DELAY = Config::getDelayProjectileDraw() / (IS_MACHINE_GUN ? 2 : 1);

  printRangedInitiateMsgs(*projectiles.at(0)->attackData);

  const bool stopAtTarget = aimLvl == actorSize_floor;
  const int chebTrvlLim = 30;

  //Get projectile path
  const Pos origin = attacker.pos;
  vector<Pos> projectilePath;
  LineCalc::calcNewLine(origin, aimPos, stopAtTarget, chebTrvlLim, false,
                        projectilePath);

  const Clr projectileColor = wpn.getData().rangedMissileClr;
  char projectileGlyph = wpn.getData().rangedMissileGlyph;
  if(projectileGlyph == '/') {
    const int i = projectilePath.size() > 2 ? 2 : 1;
    if(projectilePath.at(i).y == origin.y)
      projectileGlyph = '-';
    if(projectilePath.at(i).x == origin.x)
      projectileGlyph = '|';
    if(
      (projectilePath.at(i).x > origin.x &&
       projectilePath.at(i).y < origin.y) ||
      (projectilePath.at(i).x < origin.x &&
       projectilePath.at(i).y > origin.y))
      projectileGlyph = '/';
    if(
      (projectilePath.at(i).x > origin.x &&
       projectilePath.at(i).y > origin.y) ||
      (projectilePath.at(i).x < origin.x &&
       projectilePath.at(i).y < origin.y))
      projectileGlyph = '\\';
  }
  TileId projectileTile = wpn.getData().rangedMissileTile;
  if(projectileTile == TileId::projectileStandardFrontSlash) {
    if(projectileGlyph == '-') {
      projectileTile = TileId::projectileStandardDash;
    }
    if(projectileGlyph == '|') {
      projectileTile = TileId::projectileStandardVerticalBar;
    }
    if(projectileGlyph == '\\') {
      projectileTile = TileId::projectileStandardBackSlash;
    }
  }

  const bool LEAVE_TRAIL = wpn.getData().rangedMissileLeavesTrail;

  const int SIZE_OF_PATH_PLUS_ONE =
    projectilePath.size() + (NR_PROJECTILES - 1) *
    NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES;

  for(int i = 1; i < SIZE_OF_PATH_PLUS_ONE; ++i) {

    for(int p = 0; p < NR_PROJECTILES; p++) {

      //Current projectile's place in the path is the current global place (i)
      //minus a certain number of elements
      int projectilePathElement =
        i - (p * NR_CELL_JUMPS_BETWEEN_MG_PROJECTILES);

      //Emit sound
      if(projectilePathElement == 1) {
        string sndMsg = wpn.getData().rangedSndMsg;
        const SfxId sfx = wpn.getData().rangedAttackSfx;
        if(!sndMsg.empty()) {
          if(IS_ATTACKER_PLAYER) sndMsg = "";
          const SndVol vol = wpn.getData().rangedSndVol;
          Snd snd(sndMsg, sfx, IgnoreMsgIfOriginSeen::yes, attacker.pos,
                  &attacker, vol, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
        }
      }

      Projectile* const curProj = projectiles.at(p);

      //All the following collision checks etc are only made if the projectiles
      //current path element corresponds to an element in the real path vector
      if(
        projectilePathElement >= 1 &&
        projectilePathElement < int(projectilePath.size()) &&
        !curProj->isObstructed) {

        curProj->pos = projectilePath.at(projectilePathElement);

        curProj->isVisibleToPlayer =
          Map::cells[curProj->pos.x][curProj->pos.y].isSeenByPlayer;

        //Get attack data again for every cell traveled through
        curProj->setAttData(
          new RangedAttData(
            attacker, wpn, aimPos, curProj->pos , aimLvl));

        const Pos drawPos(curProj->pos);

        //HIT ACTOR?
        if(
          curProj->attackData->curDefender &&
          !curProj->isObstructed &&
          !curProj->attackData->isEtherealDefenderMissed) {

          const bool IS_ACTOR_AIMED_FOR = curProj->pos == aimPos;

          if(
            curProj->attackData->curDefenderSize >= actorSize_humanoid ||
            IS_ACTOR_AIMED_FOR) {

            if(curProj->attackData->attackResult >= successSmall) {
              //RENDER ACTOR HIT
              if(curProj->isVisibleToPlayer) {
                if(Config::isTilesMode()) {
                  curProj->setTile(TileId::blast1, clrRedLgt);
                  Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
                  SdlWrapper::sleep(DELAY / 2);
                  curProj->setTile(TileId::blast2, clrRedLgt);
                  Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
                  SdlWrapper::sleep(DELAY / 2);
                } else {
                  curProj->setGlyph('*', clrRedLgt);
                  Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
                  SdlWrapper::sleep(DELAY);
                }

                //MESSAGES FOR ACTOR HIT
                printProjAtActorMsgs(*curProj->attackData, true);
                //Need to draw again here to show log message
                Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
              }

              curProj->isDoneRendering = true;
              curProj->isObstructed = true;
              curProj->actorHit = curProj->attackData->curDefender;
              curProj->obstructedInElement = projectilePathElement;

              const bool DIED = curProj->attackData->curDefender->hit(
                                  curProj->attackData->dmg,
                                  wpn.getData().rangedDmgType, true);
              if(!DIED) {
                //Hit properties
                PropHandler& defenderPropHandler =
                  curProj->attackData->curDefender->getPropHandler();
                defenderPropHandler.tryApplyPropFromWpn(wpn, false);

                //Knock-back?
                if(wpn.getData().rangedCausesKnockBack) {
                  const AttData* const curData = curProj->attackData;
                  if(curData->attackResult >= successSmall) {
                    const bool IS_SPIKE_GUN =
                      wpn.getData().id == ItemId::spikeGun;
                    KnockBack::tryKnockBack(
                      *(curData->curDefender), curData->attacker->pos,
                      IS_SPIKE_GUN);
                  }
                }
              }
            }
          }
        }

        //PROJECTILE HIT FEATURE?
        vector<FeatureMob*> featureMobs;
        GameTime::getFeatureMobsAtPos(curProj->pos, featureMobs);
        Feature* featureBlockingShot = nullptr;
        for(auto* mob : featureMobs) {
          if(!mob->isProjectilePassable()) {featureBlockingShot = mob;}
        }
        FeatureStatic* featureStatic =
          Map::cells[curProj->pos.x][curProj->pos.y].featureStatic;
        if(!featureStatic->isProjectilePassable()) {
          featureBlockingShot = featureStatic;
        }

        if(featureBlockingShot && !curProj->isObstructed) {
          curProj->obstructedInElement = projectilePathElement - 1;
          curProj->isObstructed = true;

          if(wpn.getData().rangedMakesRicochetSound) {
            Snd snd("I hear a ricochet.", SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes, curProj->pos, nullptr,
                    SndVol::low, AlertsMonsters::yes);
            SndEmit::emitSnd(snd);
          }

          //RENDER FEATURE HIT
          if(curProj->isVisibleToPlayer) {
            if(Config::isTilesMode()) {
              curProj->setTile(TileId::blast1, clrYellow);
              Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
              curProj->setTile(TileId::blast2, clrYellow);
              Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
            } else {
              curProj->setGlyph('*', clrYellow);
              Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY);
            }
          }
        }

        //PROJECTILE HIT THE GROUND?
        if(
          curProj->pos == aimPos && aimLvl == actorSize_floor &&
          !curProj->isObstructed) {
          curProj->isObstructed = true;
          curProj->obstructedInElement = projectilePathElement;

          if(wpn.getData().rangedMakesRicochetSound) {
            Snd snd("I hear a ricochet.", SfxId::ricochet,
                    IgnoreMsgIfOriginSeen::yes, curProj->pos, nullptr,
                    SndVol::low, AlertsMonsters::yes);
            SndEmit::emitSnd(snd);
          }

          //RENDER GROUND HITS
          if(curProj->isVisibleToPlayer) {
            if(Config::isTilesMode()) {
              curProj->setTile(TileId::blast1, clrYellow);
              Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
              curProj->setTile(TileId::blast2, clrYellow);
              Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY / 2);
            } else {
              curProj->setGlyph('*', clrYellow);
              Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
              SdlWrapper::sleep(DELAY);
            }
          }
        }

        //RENDER FLYING PROJECTILES
        if(!curProj->isObstructed && curProj->isVisibleToPlayer) {
          if(Config::isTilesMode()) {
            curProj->setTile(projectileTile, projectileColor);
            Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
          } else {
            curProj->setGlyph(projectileGlyph, projectileColor);
            Renderer::drawProjectiles(projectiles, !LEAVE_TRAIL);
          }
        }
      }
    } //End projectile loop

    //If any projectile can be seen and not obstructed, delay
    for(Projectile* projectile : projectiles) {
      const Pos& pos = projectile->pos;
      if(
        Map::cells[pos.x][pos.y].isSeenByPlayer &&
        !projectile->isObstructed) {
        SdlWrapper::sleep(DELAY);
        break;
      }
    }

    //Check if all projectiles obstructed
    bool isAllObstructed = true;
    for(Projectile* projectile : projectiles) {
      if(!projectile->isObstructed) {isAllObstructed = false;}
    }
    if(isAllObstructed) {
      break;
    }

  } //End path-loop

  //So far, only projectile 0 can have special obstruction events***
  //Must be changed if something like an assault-incinerator is added
  const Projectile* const firstProjectile = projectiles.at(0);
  if(!firstProjectile->isObstructed) {
    wpn.weaponSpecific_projectileObstructed(aimPos, firstProjectile->actorHit);
  } else {
    const int element = firstProjectile->obstructedInElement;
    const Pos& pos = projectilePath.at(element);
    wpn.weaponSpecific_projectileObstructed(pos, firstProjectile->actorHit);
  }
  //Cleanup
  for(Projectile* projectile : projectiles) {delete projectile;}

  Renderer::drawMapAndInterface();
}

void shotgun(Actor& attacker, const Weapon& wpn, const Pos& aimPos) {
  RangedAttData* data = new RangedAttData(
    attacker, wpn, aimPos, attacker.pos);

  printRangedInitiateMsgs(*data);

  const ActorSize intendedAimLvl = data->intendedAimLvl;

  bool featureBlockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksProjectiles(), featureBlockers);

  Actor* actorArray[MAP_W][MAP_H];
  Utils::mkActorArray(actorArray);

  const Pos origin = attacker.pos;
  vector<Pos> path;
  LineCalc::calcNewLine(origin, aimPos, false, 9999, false, path);

  int nrActorsHit = 0;

  int nrMonKilledInElem = -1;

  //Emit sound
  const bool IS_ATTACKER_PLAYER = &attacker == Map::player;
  string sndMsg = wpn.getData().rangedSndMsg;
  if(!sndMsg.empty()) {
    if(IS_ATTACKER_PLAYER) {sndMsg = "";}
    const SndVol vol = wpn.getData().rangedSndVol;
    const SfxId sfx = wpn.getData().rangedAttackSfx;
    Snd snd(sndMsg, sfx, IgnoreMsgIfOriginSeen::yes, attacker.pos, &attacker,
            vol, AlertsMonsters::yes);
    SndEmit::emitSnd(snd);
  }

  for(size_t i = 1; i < path.size(); ++i) {
    //If traveled more than two steps after a killed monster, stop projectile.
    if(nrMonKilledInElem != -1 && int(i) > nrMonKilledInElem + 1) {break;}

    const Pos curPos(path.at(i));

    if(actorArray[curPos.x][curPos.y]) {

      //Only attempt hit if aiming at a level that would hit the actor
      const ActorSize sizeOfActor =
        actorArray[curPos.x][curPos.y]->getData().actorSize;
      if(sizeOfActor >= actorSize_humanoid || curPos == aimPos) {

        //Actor hit?
        delete data;
        data = new RangedAttData(attacker, wpn, aimPos, curPos, intendedAimLvl);
        const bool IS_WITHIN_RANGE_LMT =
          Utils::kingDist(origin, curPos) <= wpn.effectiveRangeLimit;
        if(
           IS_WITHIN_RANGE_LMT &&
           data->attackResult >= successSmall &&
           !data->isEtherealDefenderMissed) {
          if(Map::cells[curPos.x][curPos.y].isSeenByPlayer) {
            Renderer::drawMapAndInterface(false);
            Renderer::coverCellInMap(curPos);
            if(Config::isTilesMode()) {
              Renderer::drawTile(TileId::blast2, Panel::map, curPos, clrRedLgt);
            } else {
              Renderer::drawGlyph('*', Panel::map, curPos, clrRedLgt);
            }
            Renderer::updateScreen();
            SdlWrapper::sleep(Config::getDelayShotgun());
          }

          //Messages
          printProjAtActorMsgs(*data, true);

          //Damage
          data->curDefender->hit(data->dmg, wpn.getData().rangedDmgType, true);

          nrActorsHit++;

          Renderer::drawMapAndInterface();

          //Special shotgun behavior:
          //If current defender was killed, and player aimed at humanoid level,
          //or at floor level but beyond the current position, the shot will
          //continue one cell.
          const bool IS_TARGET_KILLED =
            data->curDefender->deadState != ActorDeadState::alive;
          if(IS_TARGET_KILLED && nrMonKilledInElem == -1) {
            nrMonKilledInElem = i;
          }
          if(
            nrActorsHit >= 2 || !IS_TARGET_KILLED ||
            (intendedAimLvl == actorSize_floor && curPos == aimPos)) {
            break;
          }
        }
      }
    }

    //Wall hit?
    if(featureBlockers[curPos.x][curPos.y]) {

      //TODO Check hit material (soft and wood should not cause ricochet)

      Snd snd("I hear a ricochet.", SfxId::ricochet, IgnoreMsgIfOriginSeen::yes,
              curPos, nullptr, SndVol::low, AlertsMonsters::yes);
      SndEmit::emitSnd(snd);

      Cell& cell = Map::cells[curPos.x][curPos.y];

      if(cell.isSeenByPlayer) {
        Renderer::drawMapAndInterface(false);
        Renderer::coverCellInMap(curPos);
        if(Config::isTilesMode()) {
          Renderer::drawTile(TileId::blast2, Panel::map, curPos, clrYellow);
        } else {
          Renderer::drawGlyph('*', Panel::map, curPos, clrYellow);
        }
        Renderer::updateScreen();
        SdlWrapper::sleep(Config::getDelayShotgun());
        Renderer::drawMapAndInterface();
      }

      cell.featureStatic->hit(DmgType::physical, DmgMethod::shotgun, nullptr);

      break;
    }

    //Floor hit?
    if(intendedAimLvl == actorSize_floor && curPos == aimPos) {
      Snd snd("I hear a ricochet.", SfxId::ricochet, IgnoreMsgIfOriginSeen::yes,
              curPos, nullptr, SndVol::low, AlertsMonsters::yes);
      SndEmit::emitSnd(snd);

      if(Map::cells[curPos.x][curPos.y].isSeenByPlayer) {
        Renderer::drawMapAndInterface(false);
        Renderer::coverCellInMap(curPos);
        if(Config::isTilesMode()) {
          Renderer::drawTile(TileId::blast2, Panel::map, curPos, clrYellow);
        } else {
          Renderer::drawGlyph('*', Panel::map, curPos, clrYellow);
        }
        Renderer::updateScreen();
        SdlWrapper::sleep(Config::getDelayShotgun());
        Renderer::drawMapAndInterface();
      }
      break;
    }
  }
}

} //namespace

void melee(Actor& attacker, const Weapon& wpn, Actor& defender) {
  MeleeAttData data(attacker, wpn, defender);

  printMeleeMsgAndPlaySfx(data, wpn);

  if(!data.isEtherealDefenderMissed) {
    if(data.attackResult >= successSmall && !data.isDefenderDodging) {
      const bool IS_DEFENDER_KILLED =
        data.curDefender->hit(data.dmg, wpn.getData().meleeDmgType, true);

      if(!IS_DEFENDER_KILLED) {
        data.curDefender->getPropHandler().tryApplyPropFromWpn(wpn, true);
      }
      if(data.attackResult >= successNormal) {
        if(data.curDefender->getData().canBleed) {
          Map::mkBlood(data.curDefender->pos);
        }
      }
      if(!IS_DEFENDER_KILLED) {
        if(wpn.getData().meleeCausesKnockBack) {
          if(data.attackResult > successSmall) {
            KnockBack::tryKnockBack(
              *(data.curDefender), data.attacker->pos, false);
          }
        }
      }
      const ItemDataT& itemData = wpn.getData();
      if(itemData.itemWeight > itemWeight_light && !itemData.isIntrinsic) {
        Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes,
                data.curDefender->pos, nullptr, SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd);
      }
    }
  }

  if(data.curDefender == Map::player) {
    if(data.attackResult >= failSmall) {
      static_cast<Monster*>(data.attacker)->isStealth = false;
    }
  } else {
    Monster* const monster = static_cast<Monster*>(data.curDefender);
    monster->awareOfPlayerCounter_ = monster->getData().nrTurnsAwarePlayer;
  }
  GameTime::actorDidAct();
}

bool ranged(Actor& attacker, Weapon& wpn, const Pos& aimPos) {
  bool didAttack = false;

  const bool WPN_HAS_INF_AMMO = wpn.getData().rangedHasInfiniteAmmo;

  if(wpn.getData().isShotgun) {
    if(wpn.nrAmmoLoaded != 0 || WPN_HAS_INF_AMMO) {

      shotgun(attacker, wpn, aimPos);

      didAttack = true;
      if(!WPN_HAS_INF_AMMO) {wpn.nrAmmoLoaded -= 1;}
    }
  } else {
    int nrOfProjectiles = 1;

    if(wpn.getData().isMachineGun) {nrOfProjectiles = NR_MG_PROJECTILES;}

    if(wpn.nrAmmoLoaded >= nrOfProjectiles || WPN_HAS_INF_AMMO) {
      projectileFire(attacker, wpn, aimPos);

      if(Map::player->deadState == ActorDeadState::alive) {

        didAttack = true;

        if(!WPN_HAS_INF_AMMO) {wpn.nrAmmoLoaded -= nrOfProjectiles;}
      } else {
        return true;
      }
    }
  }

  Renderer::drawMapAndInterface();

  if(didAttack) {GameTime::actorDidAct();}

  return didAttack;
}

} //Attack

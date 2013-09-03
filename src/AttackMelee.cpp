#include "Attack.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Postmortem.h"
#include "Blood.h"
#include "Knockback.h"
#include "Log.h"

using namespace std;

void Attack::melee(Actor& attacker, const Weapon& wpn, Actor& defender) {
  MeleeAttackData data(attacker, wpn, defender, eng);

  printMeleeMessages(data, wpn);
  eng->renderer->drawMapAndInterface();

  if(data.attackResult >= successSmall && data.isDefenderDodging == false) {
    const bool IS_DEFENDER_KILLED =
      data.currentDefender->hit(data.dmg, wpn.getData().meleeDmgType);

    if(IS_DEFENDER_KILLED == false) {
      data.currentDefender->getPropHandler()->tryApplyPropFromWpn(wpn, true);
    }
    if(data.attackResult >= successNormal) {
      if(data.currentDefender->getData()->canBleed == true) {
        eng->gore->makeBlood(data.currentDefender->pos);
      }
    }
    if(IS_DEFENDER_KILLED == false) {
      if(wpn.getData().meleeCausesKnockBack) {
        if(data.attackResult > successSmall) {
          eng->knockBack->tryKnockBack(
            data.currentDefender, data.attacker->pos, false);
        }
      }
    }
    const ItemData& itemData = wpn.getData();
    if(itemData.itemWeight > itemWeight_light && itemData.isIntrinsic == false) {
      eng->soundEmitter->emitSound(Sound("", true, data.currentDefender->pos,
                                         false, true));
    }
  }

  if(data.currentDefender == eng->player) {
    if(data.attackResult >= failSmall) {
      Monster* const monster = dynamic_cast<Monster*>(data.attacker);
      monster->isStealth = false;
    }
  } else {
    Monster* const monster = dynamic_cast<Monster*>(data.currentDefender);
    monster->playerAwarenessCounter = monster->getData()->nrTurnsAwarePlayer;
  }
  eng->gameTime->endTurnOfCurrentActor();
//  const bool IS_SWIFT_ATTACK = data.attacker == eng->player && data.currentDefender->deadState != actorDeadState_alive && has swift assailant;
//  if(IS_SWIFT_ATTACK == false) {
//    eng->gameTime->endTurnOfCurrentActor();
//  }
}

void Attack::printMeleeMessages(const MeleeAttackData& data, const Weapon& wpn) {
  string otherName = "";

//  if(data.isTargetEthereal == true) {
//    if(data.isPlayerAttacking == true) {
//      eng->log->addMessage("I hit nothing but void.");
//    } else {
//      if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
//        otherName = data.attacker->getNameThe();
//      } else {
//        otherName = "its";
//      }
//
//      eng->log->addMessage("I am unaffected by " + otherName + " attack.", clrWhite, messageInterrupt_force);
//    }
//  } else {
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
    if(data.attacker == eng->player) {
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
      if(data.attacker == eng->player) {
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
      //Punctuation or exclamation marks depending on attack strength
      string dmgPunctuation = ".";
      const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
      if(MAX_DMG_ROLL >= 4) {
        dmgPunctuation =
          data.dmgRoll > MAX_DMG_ROLL * 5 / 6 ? "!!!" :
          data.dmgRoll > MAX_DMG_ROLL / 2 ? "!" :
          dmgPunctuation;
      }

      if(data.attacker == eng->player) {
        const string wpnVerb = wpn.getData().meleeAttackMessages.player;

        if(eng->player->checkIfSeeActor(*data.currentDefender, NULL)) {
          otherName = data.currentDefender->getNameThe();
        } else {
          otherName = "it";
        }

        if(data.isIntrinsicAttack) {
          const string ATTACK_MOD_TEXT = data.isWeakAttack ? " feebly" : "";
          eng->log->addMessage(
            "I " + wpnVerb + " " + otherName + ATTACK_MOD_TEXT + dmgPunctuation,
            clrMessageGood);
        } else {
          const string ATTACK_MOD_TEXT =
            data.isWeakAttack  ? "feebly "    :
            data.isBackstab    ? "covertly "  : "";
          const SDL_Color clr = data.isBackstab ? clrBlueLgt : clrMessageGood;
          const string wpnName_a = eng->itemDataHandler->getItemRef(wpn, itemRef_a, true);
          eng->log->addMessage(
            "I " + wpnVerb + " " + otherName + " " + ATTACK_MOD_TEXT + "with " +
            wpnName_a + dmgPunctuation,
            clr);
        }
      } else {
        const string wpnVerb = wpn.getData().meleeAttackMessages.other;

        if(eng->player->checkIfSeeActor(*data.attacker, NULL)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }

        eng->log->addMessage(otherName + " " + wpnVerb + dmgPunctuation,
                             clrMessageBad, messageInterrupt_force);
      }
    }
  }
//  }
}



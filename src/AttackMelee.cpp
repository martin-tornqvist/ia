#include "Attack.h"

#include "Engine.h"

#include "GameTime.h"
#include "ItemWeapon.h"
#include "Renderer.h"
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
  eng.renderer->drawMapAndInterface();

  if(data.isEtherealDefenderMissed == false) {
    if(data.attackResult >= successSmall && data.isDefenderDodging == false) {
      const bool IS_DEFENDER_KILLED =
        data.currentDefender->hit(data.dmg, wpn.getData().meleeDmgType, true);

      if(IS_DEFENDER_KILLED == false) {
        data.currentDefender->getPropHandler().tryApplyPropFromWpn(wpn, true);
      }
      if(data.attackResult >= successNormal) {
        if(data.currentDefender->getData().canBleed == true) {
          eng.gore->makeBlood(data.currentDefender->pos);
        }
      }
      if(IS_DEFENDER_KILLED == false) {
        if(wpn.getData().meleeCausesKnockBack) {
          if(data.attackResult > successSmall) {
            eng.knockBack->tryKnockBack(
              *(data.currentDefender), data.attacker->pos, false);
          }
        }
      }
      const ItemData& itemData = wpn.getData();
      if(
        itemData.itemWeight > itemWeight_light &&
        itemData.isIntrinsic == false) {
        Sound snd("", endOfSfx, true, data.currentDefender->pos, false, true);
        eng.soundEmitter->emitSound(snd);
      }
    }
  }

  if(data.currentDefender == eng.player) {
    if(data.attackResult >= failSmall) {
      dynamic_cast<Monster*>(data.attacker)->isStealth = false;
    }
  } else {
    Monster* const monster = dynamic_cast<Monster*>(data.currentDefender);
    monster->playerAwarenessCounter = monster->getData().nrTurnsAwarePlayer;
  }
  eng.gameTime->actorDidAct();
}

void Attack::printMeleeMessages(const MeleeAttackData& data,
                                const Weapon& wpn) {
  string otherName = "";


  if(data.isDefenderDodging) {
    //----- DEFENDER DODGES --------
    if(data.attacker == eng.player) {
      if(eng.player->checkIfSeeActor(*data.currentDefender, NULL)) {
        otherName = data.currentDefender->getNameThe();
      } else {
        otherName = "It ";
      }
      eng.log->addMsg(otherName + " dodges my attack.");
    } else {
      if(eng.player->checkIfSeeActor(*data.attacker, NULL)) {
        otherName = data.attacker->getNameThe();
      } else {
        otherName = "It";
      }
      eng.log->addMsg(
        "I dodge an attack from " + otherName + ".", clrMessageGood);
    }
  } else if(data.attackResult <= failSmall) {
    //----- BAD AIMING --------
    if(data.attacker == eng.player) {
      if(data.attackResult == failSmall) {
        eng.log->addMsg("I barely miss!");
      } else if(data.attackResult == failNormal) {
        eng.log->addMsg("I miss.");
      } else if(data.attackResult == failBig) {
        eng.log->addMsg("I miss completely.");
      }
    } else {
      if(eng.player->checkIfSeeActor(*data.attacker, NULL)) {
        otherName = data.attacker->getNameThe();
      } else {
        otherName = "It";
      }
      if(data.attackResult == failSmall) {
        eng.log->addMsg(otherName + " barely misses me!", clrWhite, true);
      } else if(data.attackResult == failNormal) {
        eng.log->addMsg(otherName + " misses me.", clrWhite, true);
      } else if(data.attackResult == failBig) {
        eng.log->addMsg(otherName + " misses me completely.", clrWhite, true);
      }
    }
  } else {
    //----- AIM IS CORRECT -------
    if(data.isEtherealDefenderMissed) {
      //----- ATTACK MISSED DUE TO ETHEREAL TARGET --------
      if(data.attacker == eng.player) {
        if(eng.player->checkIfSeeActor(*data.currentDefender, NULL)) {
          otherName = data.currentDefender->getNameThe();
        } else {
          otherName = "It ";
        }
        eng.log->addMsg(
          "My attack passes right throuth " + otherName + "!");
      } else {
        if(eng.player->checkIfSeeActor(*data.attacker, NULL)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }
        eng.log->addMsg(
          "The attack of " + otherName + " passes right through me!",
          clrMessageGood);
      }
    } else {
      //----- ATTACK CONNECTS WITH DEFENDER --------
      //Punctuation or exclamation marks depending on attack strength
      string dmgPunct = ".";
      const int MAX_DMG_ROLL = data.dmgRolls * data.dmgSides;
      if(MAX_DMG_ROLL >= 4) {
        dmgPunct =
          data.dmgRoll > MAX_DMG_ROLL * 5 / 6 ? "!!!" :
          data.dmgRoll > MAX_DMG_ROLL / 2 ? "!" :
          dmgPunct;
      }

      if(data.attacker == eng.player) {
        const string wpnVerb = wpn.getData().meleeAttackMessages.player;

        if(eng.player->checkIfSeeActor(*data.currentDefender, NULL)) {
          otherName = data.currentDefender->getNameThe();
        } else {
          otherName = "it";
        }

        if(data.isIntrinsicAttack) {
          const string ATTACK_MOD_STR = data.isWeakAttack ? " feebly" : "";
          eng.log->addMsg(
            "I " + wpnVerb + " " + otherName + ATTACK_MOD_STR + dmgPunct,
            clrMessageGood);
        } else {
          const string ATTACK_MOD_STR =
            data.isWeakAttack  ? "feebly "    :
            data.isBackstab    ? "covertly "  : "";
          const SDL_Color clr =
            data.isBackstab ? clrBlueLgt : clrMessageGood;
          const string wpnName_a =
            eng.itemDataHandler->getItemRef(wpn, itemRef_a, true);
          eng.log->addMsg(
            "I " + wpnVerb + " " + otherName + " " + ATTACK_MOD_STR +
            "with " + wpnName_a + dmgPunct,
            clr);
        }
      } else {
        const string wpnVerb = wpn.getData().meleeAttackMessages.other;

        if(eng.player->checkIfSeeActor(*data.attacker, NULL)) {
          otherName = data.attacker->getNameThe();
        } else {
          otherName = "It";
        }

        eng.log->addMsg(otherName + " " + wpnVerb + dmgPunct,
                         clrMessageBad, true);
      }
    }
  }
}



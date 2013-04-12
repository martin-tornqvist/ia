#include "Attack.h"

#include "Engine.h"
#include "ItemWeapon.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Postmortem.h"
#include "Blood.h"
#include "Knockback.h"

using namespace std;

void Attack::melee(const coord& defenderPos, Weapon* weapon) {
  //Get attack data
  AttackData data;
  getAttackData(data, defenderPos, defenderPos, weapon, true);

  //Print messages
  printMeleeMessages(data, weapon);
  eng->renderer->drawMapAndInterface();

  if(data.attackResult >= successSmall && data.isDefenderDodging == false) {
    //Do damage
    const bool DIED = data.currentDefender->hit(data.dmg, weapon->getDef().meleeDamageType);

    if(DIED == false) {
      //Apply weapon status effects
      data.currentDefender->getStatusEffectsHandler()->tryAddEffectsFromWeapon(weapon, true);
    }
    //Blood
    if(data.attackResult >= successNormal) {
      if(data.currentDefender->getDef()->canBleed == true) {
        eng->gore->makeBlood(defenderPos);
      }
    }

    //Knock-back?
    if(DIED == false) {
      if(weapon->getDef().meleeCausesKnockBack) {
        if(data.attackResult > successSmall) {
          eng->knockBack->tryKnockBack(data.currentDefender, data.attacker->pos, false);
        }
      }
    }

    //If weapon not light, make a sound
    const ItemDefinition& itemDef = weapon->getDef();
    if(itemDef.itemWeight > itemWeight_light && itemDef.isIntrinsic == false) {
      eng->soundEmitter->emitSound(Sound("", true, defenderPos, false, true));
    }
  }

  if(data.currentDefender == eng->player) {
    if(data.attackResult >= failSmall) {
      Monster* const monster = dynamic_cast<Monster*>(data.attacker);
      monster->isStealth = false;
    }
  } else {
    Monster* const monster = dynamic_cast<Monster*>(data.currentDefender);
    monster->playerAwarenessCounter = monster->getDef()->nrTurnsAwarePlayer;
  }

  //Let next act
  eng->gameTime->letNextAct();
//  const bool IS_SWIFT_ATTACK = data.attacker == eng->player && data.currentDefender->deadState != actorDeadState_alive && has swift assailant;
//  if(IS_SWIFT_ATTACK == false) {
//    eng->gameTime->letNextAct();
//  }
}


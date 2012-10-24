#include "StatusEffects.h"

#include "Engine.h"
#include "ItemArmor.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"
#include "Render.h"
#include "ItemWeapon.h"
#include "ActorMonster.h"
#include "Inventory.h"
#include "Map.h"

using namespace std;

void StatusBlessed::start() {
  owningActor->getStatusEffectsHandler()->endEffect(statusCursed);
}

void StatusCursed::start() {
  owningActor->getStatusEffectsHandler()->endEffect(statusBlessed);
}

void StatusDiseased::newTurn(Engine* engine) {
  int* const hp = &(owningActor->getInstanceDefinition()->HP);
  int* const hp_max = &(owningActor->getInstanceDefinition()->HP_max);

  if(*hp > ((*hp_max) * 3) / 4) {
    *hp = (*hp) - engine->dice(1, 2);
  }

  turnsLeft--;
}

bool StatusTerrified::allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
  if(ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
    if(owningActor == owningActor->eng->player) {
      owningActor->eng->log->addMessage("I am too terrified to engage in close combat!");
    }
  }
  return false;
}

bool StatusTerrified::allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;
  return true;
}

coord StatusNailed::changeMoveCoord(const coord& actorPos, const coord& movePos, Engine* engine) {
  (void)movePos;

  Actor* const player = owningActor->eng->player;

  if(owningActor == player) {
    engine->log->addMessage("I struggle to tear out the spike!", clrMessageBad);
    engine->postmortem->setCauseOfDeath("Attempting to tear out a spike from the flesh");
  } else {
    if(player->checkIfSeeActor(*owningActor, NULL)) {
      engine->log->addMessage(owningActor->getNameThe() +  " struggles in pain!", clrMessageGood);
    }
  }

  owningActor->hit(engine->dice(1, 2), damageType_physical);

  if(owningActor->deadState == actorDeadState_alive) {
    const int ACTOR_TOUGHNESS = owningActor->getInstanceDefinition()->abilityValues.getAbilityValue(ability_resistStatusBody, true);
    if(engine->abilityRoll->roll(ACTOR_TOUGHNESS + getSaveAbilityModifier()) >= successSmall) {
      nrOfSpikes--;
      if(nrOfSpikes > 0) {
        if(owningActor == player) {
          engine->log->addMessage("I rip out a spike from my flesh!");
        } else {
          if(engine->player->checkIfSeeActor(*owningActor, NULL)) {
            engine->log->addMessage(owningActor->getNameThe() + " tears out a spike!");
          }
        }
      }
    }
  }

  return actorPos;
}

bool StatusConfused::allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;

  if(owningActor != owningActor->eng->player) {
    return owningActor->eng->dice(1, 100) > 85;
  }
  return true;
}

bool StatusConfused::allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;

  if(owningActor != owningActor->eng->player) {
    return owningActor->eng->dice(1, 100) > 85;
  }
  return true;
}

void StatusBurning::start() {
  owningActor->addLight(owningActor->eng->map->light);
}

void StatusBurning::end() {

}

void StatusBurning::newTurn(Engine* engine) {
  owningActor->addLight(owningActor->eng->map->light);

  if(owningActor == engine->player) {
    engine->log->addMessage("AAAARGH IT BURNS!!!", clrRedLight);
  }
  bool DIED = owningActor->hit(engine->dice(1, 4), damageType_fire);
  if(DIED) {
    if(owningActor == engine->player) {
      engine->postmortem->setCauseOfDeath("Burning");
    }
  }

  turnsLeft--;
}

void StatusFlared::start() {
//	owningActor->setColor(clrRedLight);
}

void StatusFlared::end() {
//	owningActor->resetColor();
}

void StatusFlared::newTurn(Engine* engine) {
  bool DIED = owningActor->hit(engine->dice(1, 2), damageType_fire);
  if(DIED) {
    if(owningActor == engine->player) {
      engine->postmortem->setCauseOfDeath("Burning");
    }
  }

//	owningActor->setColor(clrRedLight);
  turnsLeft--;
}

coord StatusConfused::changeMoveCoord(const coord& actorPos, const coord& movePos, Engine* engine) {

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeMoveBlockerArray(owningActor, blockers);

  if(actorPos != movePos) {
    if(engine->dice(1, 100) < 50) {
      int triesLeft = 100;
      while(triesLeft != 0) {
        //-1 to 1 for x and y
        const coord delta(engine->dice(1, 3) - 2, engine->dice(1, 3) - 2);
        if(delta.x != 0 || delta.y != 0) {
          const coord c = actorPos + delta;
          if(blockers[c.x][c.y] == false) {
            return actorPos + delta;
          }
        }
        triesLeft--;
      }
    }
  }
  return movePos;
}

void StatusEffect::setTurnsFromRandomStandard(Engine* const engine) {
  const DiceParam diceParam = getRandomStandardNrTurns();
  turnsLeft = engine->dice(diceParam.rolls, diceParam.sides) + diceParam.plus;
}

//================================================================ STATUS EFFECTS HANDLER
StatusEffect* StatusEffectsHandler::makeEffectFromId(const StatusEffects_t id, const int TURNS_LEFT) {
  switch(id) {
  case statusNailed:
    return new StatusNailed(TURNS_LEFT);
    break;
  case statusBlind:
    return new StatusBlind(TURNS_LEFT);
    break;
  case statusBurning:
    return new StatusBurning(TURNS_LEFT);
    break;
  case statusFlared:
    return new StatusFlared(TURNS_LEFT);
    break;
  case statusParalyzed:
    return new StatusParalyzed(TURNS_LEFT);
    break;
  case statusTerrified:
    return new StatusTerrified(TURNS_LEFT);
    break;
  case statusConfused:
    return new StatusConfused(TURNS_LEFT);
    break;
  case statusWaiting:
    return new StatusWaiting(TURNS_LEFT);
    break;
  case statusSlowed:
    return new StatusSlowed(TURNS_LEFT);
    break;
  case statusDiseased:
    return new StatusDiseased(TURNS_LEFT);
    break;
  case statusFainted:
    return new StatusFainted(TURNS_LEFT);
    break;
  case statusPerfectReflexes:
    return new StatusPerfectReflexes(TURNS_LEFT);
    break;
  case statusPerfectAim:
    return new StatusPerfectAim(TURNS_LEFT);
    break;
  case statusPerfectStealth:
    return new StatusPerfectStealth(TURNS_LEFT);
    break;
  case statusPerfectFortitude:
    return new StatusPerfectFortitude(TURNS_LEFT);
    break;
  case statusPerfectToughness:
    return new StatusPerfectToughness(TURNS_LEFT);
    break;
  case statusStill:
    return new StatusStill(TURNS_LEFT);
    break;
  case statusDisabledAttack:
    return new StatusDisabledAttack(TURNS_LEFT);
    break;
  case statusDisabledMelee:
    return new StatusDisabledAttackMelee(TURNS_LEFT);
    break;
  case statusDisabledRanged:
    return new StatusDisabledAttackRanged(TURNS_LEFT);
    break;
  case statusBlessed:
    return new StatusBlessed(TURNS_LEFT);
    break;
  case statusCursed:
    return new StatusCursed(TURNS_LEFT);
    break;
  default: {} break;
  }
  return NULL;
}

bool StatusEffectsHandler::allowSee() {
  for(unsigned int i = 0; i < effects.size(); i++) {
    if(effects.at(i)->allowSee() == false) {
      return false;
    }
  }
  return true;
}

void StatusEffectsHandler::attemptAddEffect(StatusEffect* const effect, const bool FORCE_EFFECT, const bool NO_MESSAGES) {
  const bool OWNER_IS_PLAYER = owningActor == eng->player;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);
  const bool PLAYER_SEE_OWNER = eng->player->checkIfSeeActor(*owningActor, blockers);

  const Abilities_t saveAbility = effect->getSaveAbility();
  const int ACTOR_ABILITY = owningActor->getInstanceDefinition()->abilityValues.getAbilityValue(saveAbility, true);
  const int SAVE_ABILITY_MOD = effect->getSaveAbilityModifier();
  int statusProtectionFromArmor = 0;

  if(effect->getEffectId() == statusBurning) {
    if(OWNER_IS_PLAYER) {
      Item* const armor = owningActor->getInventory()->getItemInSlot(slot_armorBody);
      if(armor != NULL) {
        const bool ARMOR_PROTECTS_FROM_BURNING = armor->getInstanceDefinition().armorData.protectsAgainstStatusBurning;
        if(ARMOR_PROTECTS_FROM_BURNING) {
          statusProtectionFromArmor = max(0, dynamic_cast<Armor*>(armor)->getDurability());
        }
      }
    }
  }

  const int TOTAL_SAVE_ABILITY = ACTOR_ABILITY + SAVE_ABILITY_MOD + statusProtectionFromArmor;

  AbilityRollResult_t result = eng->abilityRoll->roll(TOTAL_SAVE_ABILITY);

  if(result <= failSmall || FORCE_EFFECT) {
    //Saving throw failed, apply effect.

    //If actor already has effect, use maximum nr of turns of old effect or new effect.
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effect->getEffectId() == effects.at(i)->getEffectId()) {
        const int TURNS_LEFT_OLD = effects.at(i)->turnsLeft;
        const int TURNS_LEFT_NEW = effect->turnsLeft;

        if(TURNS_LEFT_NEW >= TURNS_LEFT_OLD) {
          if(OWNER_IS_PLAYER) {
            if(effect->messageWhenMore() != "") {
              if(NO_MESSAGES == false) {
                eng->log->addMessage(effect->messageWhenMore());
              }
            }
          } else {
            if(PLAYER_SEE_OWNER) {
              if(NO_MESSAGES == false) {
                if(effect->messageWhenMore() != "") {
                  eng->log->addMessage(owningActor->getNameThe() + " " + effect->messageWhenMoreOther());
                }
              }
            }
          }
        }

        effects.at(i)->turnsLeft = max(TURNS_LEFT_OLD, TURNS_LEFT_NEW);

        delete effect;
        return;
      }
    }

    //This part reached means the applied effect is new.
    effect->setOwningActor(owningActor);
    effects.push_back(effect);
    effect->start();

    owningActor->updateColor();
    eng->player->FOVupdate();
    eng->renderer->drawMapAndInterface(true);

    if(OWNER_IS_PLAYER) {
      if(NO_MESSAGES == false) {
        if(effect->messageWhenStart() != "") {
          eng->log->addMessage(effect->messageWhenStart());
        }
      }
    } else {
      if(PLAYER_SEE_OWNER) {
        if(NO_MESSAGES == false) {
          if(effect->messageWhenStartOther() != "") {
            eng->log->addMessage(owningActor->getNameThe() + " " + effect->messageWhenStartOther());
          }
        }
      }
    }
    return;
  }

  //This part reached means saving throw succeeded.
  if(OWNER_IS_PLAYER) {
    if(NO_MESSAGES == false) {
      if(effect->messageWhenSaves() != "") {
        eng->log->addMessage(effect->messageWhenSaves());
      }
    }
  } else {
    if(PLAYER_SEE_OWNER) {
      if(NO_MESSAGES == false) {
        if(effect->messageWhenSavesOther() != "") {
          eng->log->addMessage(owningActor->getNameThe() + " " + effect->messageWhenSavesOther());
        }
      }
    }
  }
  delete effect;
}

void StatusEffectsHandler::attemptAddEffectsFromWeapon(Weapon* weapon, const bool IS_MELEE) {
  const ItemDefinition& wpnDef = weapon->getInstanceDefinition();
  StatusEffect* wpnEffect = IS_MELEE ? wpnDef.meleeStatusEffect : wpnDef.rangedStatusEffect;

  if(wpnEffect != NULL) {
    //Make a new copy of the weapon effect
    StatusEffect* cpy = wpnEffect->copy();

    //Attempt to add the effect to the actor.
    //If attempt fails (saving throw succeeds, immune, etc), the copy is destroyed.
    attemptAddEffect(cpy);
  }
}

void StatusEffectsHandler::newTurnAllEffects() {
  const bool OWNER_IS_PLAYER = owningActor == eng->player;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);
  const bool PLAYER_SEE_OWNER = OWNER_IS_PLAYER ? true : eng->player->checkIfSeeActor(*owningActor, blockers);

  for(unsigned int i = 0; i < effects.size();) {
    if(OWNER_IS_PLAYER == false) {
      dynamic_cast<Monster*>(owningActor)->playerAwarenessCounter = owningActor->getInstanceDefinition()->nrTurnsAwarePlayer;
    }

    StatusEffect* const curEffect = effects.at(i);
    if(curEffect->isFinnished()) {

      curEffect->end();
      effects.erase(effects.begin() + i);

      eng->player->FOVupdate();
      eng->renderer->drawMapAndInterface();

      if(OWNER_IS_PLAYER && curEffect->messageWhenEnd() != "") {
        eng->log->addMessage(curEffect->messageWhenEnd(), clrWhite);
      } else {
        if(PLAYER_SEE_OWNER && curEffect->messageWhenEndOther() != "") {
          eng->log->addMessage(owningActor->getNameThe() + " " + curEffect->messageWhenEndOther());
        }
      }

      delete curEffect;
    } else {
      curEffect->newTurn(eng);
      i++;
    }
  }
}

bool StatusEffectsHandler::allowAttackMelee(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
  for(unsigned int i = 0; i < effects.size(); i++) {
    if(effects.at(i)->allowAttackMelee(ALLOW_PRINT_MESSAGE_WHEN_FALSE) == false) {
      return false;
    }
  }
  return true;
}

bool StatusEffectsHandler::allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
  for(unsigned int i = 0; i < effects.size(); i++) {
    if(effects.at(i)->allowAttackRanged(ALLOW_PRINT_MESSAGE_WHEN_FALSE) == false) {
      return false;
    }
  }
  return true;
}


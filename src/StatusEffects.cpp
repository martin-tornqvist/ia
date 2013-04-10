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
#include "Explosion.h"
#include "FeatureFactory.h"
#include "PlayerBonuses.h"

using namespace std;

StatusEffect::~StatusEffect() {

}

void StatusEffect::setTurnsFromRandomStandard(Engine* const engine) {
  const DiceParam diceParam = getRandomStandardNrTurns();
  turnsLeft = engine->dice(diceParam.rolls, diceParam.sides) + diceParam.plus;
}

void StatusBlessed::start(Engine* const engine) {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  owningActor->getStatusEffectsHandler()->endEffect(statusCursed, visionBlockers, false);
}

void StatusCursed::start(Engine* const engine) {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  owningActor->getStatusEffectsHandler()->endEffect(statusBlessed, visionBlockers, false);
}

void StatusDiseased::start(Engine* const engine) {
  //player->getHpMax() will now return a decreased value,
  //cap current HP to the new, lower, maximum
  int& hp = engine->player->hp_;
  hp = min(engine->player->getHpMax(true), hp);
}

void StatusDiseased::newTurn(Engine* const engine) {
  (void)engine;
  turnsLeft--;
}

void StatusPoisoned::newTurn(Engine* const engine) {
  const int DMG_N_TURN = 4;
  const int TURN = engine->gameTime->getTurn();
  if(TURN == (TURN / DMG_N_TURN) * DMG_N_TURN) {

    if(owningActor == engine->player) {
      engine->log->addMessage("I am suffering from the poison!", clrMessageBad, messageInterrupt_force);
    } else {
      if(engine->player->checkIfSeeActor(*owningActor, NULL)) {
        engine->log->addMessage(owningActor->getNameThe() + " suffers from poisoning!");
      }
    }

    owningActor->hit(1, damageType_pure);
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

coord StatusNailed::changeMoveCoord(const coord& actorPos, const coord& movePos, Engine* const engine) {
  (void)movePos;

  Actor* const player = owningActor->eng->player;

  if(owningActor == player) {
    engine->log->addMessage("I struggle to tear out the spike!", clrMessageBad);
  } else {
    if(player->checkIfSeeActor(*owningActor, NULL)) {
      engine->log->addMessage(owningActor->getNameThe() +  " struggles in pain!", clrMessageGood);
    }
  }

  owningActor->hit(engine->dice(1, 3), damageType_physical);

  if(owningActor->deadState == actorDeadState_alive) {
    const int ACTOR_TOUGHNESS = owningActor->getDef()->abilityValues.getAbilityValue(ability_resistStatusBody, true, *(owningActor));
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
    return owningActor->eng->dice.coinToss();
  }
  return true;
}

bool StatusConfused::allowAttackRanged(const bool ALLOW_PRINT_MESSAGE_WHEN_FALSE) {
  (void)ALLOW_PRINT_MESSAGE_WHEN_FALSE;

  if(owningActor != owningActor->eng->player) {
    return owningActor->eng->dice.coinToss();
  }
  return true;
}

coord StatusConfused::changeMoveCoord(const coord& actorPos, const coord& movePos, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeMoveBlockerArray(owningActor, blockers);

  if(actorPos != movePos) {
    const int CHANCE_TO_MOVE_WRONG = 40;
    if(engine->dice.percentile() < CHANCE_TO_MOVE_WRONG) {
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

void StatusBurning::start(Engine* const engine) {
  owningActor->addLight(engine->map->light);
}

void StatusBurning::end(Engine* const engine) {
  (void)engine;
}

bool StatusBurning::isPlayerVisualUpdateNeededWhenStartOrEnd() {
  return true;
}

void StatusBurning::doDamage(Engine* const engine) {
  if(owningActor == engine->player) {
    engine->log->addMessage("AAAARGH IT BURNS!!!", clrRedLight);
//    owningActor->eng->renderer->drawMapAndInterface();
  }
  owningActor->hit(engine->dice(1, 2), damageType_fire);
}

void StatusBurning::newTurn(Engine* const engine) {
//  owningActor->addLight(owningActor->eng->map->light);
  doDamage(engine);
  turnsLeft--;
}

void StatusBlind::start(Engine* const engine) {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  owningActor->getStatusEffectsHandler()->endEffect(statusClairvoyant, visionBlockers, false);
}

void StatusBlind::end(Engine* const engine) {
  (void)engine;
}

bool StatusBlind::isPlayerVisualUpdateNeededWhenStartOrEnd() {
  return owningActor == owningActor->eng->player;
}

void StatusParalyzed::start(Engine* const engine) {
  Player* const player = engine->player;
  if(owningActor == player) {
    const coord& playerPos = player->pos;
    const int DYNAMITE_FUSE = engine->player->dynamiteFuseTurns;
    const int FLARE_FUSE = engine->player->flareFuseTurns;
    const int MOLOTOV_FUSE = engine->player->molotovFuseTurns;

    if(DYNAMITE_FUSE > 0) {
      player->dynamiteFuseTurns = -1;
      player->updateColor();
      engine->log->addMessage("The lit Dynamite stick falls from my hands!");
      if(engine->map->featuresStatic[playerPos.x][playerPos.y]->isBottomless() == false) {
        engine->featureFactory->spawnFeatureAt(feature_litDynamite, playerPos, new DynamiteSpawnData(DYNAMITE_FUSE));
      }
    }
    if(FLARE_FUSE > 0) {
      player->flareFuseTurns = -1;
      player->updateColor();
      engine->log->addMessage("The lit Flare falls from my hands.");
      if(engine->map->featuresStatic[playerPos.x][playerPos.y]->isBottomless() == false) {
        engine->featureFactory->spawnFeatureAt(feature_litFlare, playerPos, new DynamiteSpawnData(FLARE_FUSE));
      }
      engine->gameTime->updateLightMap();
      player->updateFov();
      engine->renderer->drawMapAndInterface();
    }
    if(MOLOTOV_FUSE > 0) {
      player->molotovFuseTurns = -1;
      player->updateColor();
      engine->log->addMessage("The lit Molotov Cocktail falls from my hands!");
      engine->explosionMaker->runExplosion(player->pos, false, new StatusBurning(engine));
    }
  }
}

void StatusFainted::start(Engine* const engine) {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  owningActor->getStatusEffectsHandler()->endEffect(statusClairvoyant, visionBlockers, false);
}

void StatusFainted::end(Engine* const engine) {
  (void)engine;
}

bool StatusFainted::isPlayerVisualUpdateNeededWhenStartOrEnd() {
  return owningActor == owningActor->eng->player;
}

void StatusClairvoyant::start(Engine* const engine) {
  bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers);
  owningActor->getStatusEffectsHandler()->endEffect(statusBlind, visionBlockers, false);
}

void StatusClairvoyant::end(Engine* const engine) {
  (void)engine;
}

bool StatusClairvoyant::isPlayerVisualUpdateNeededWhenStartOrEnd() {
  return owningActor == owningActor->eng->player;
}

void StatusClairvoyant::newTurn(Engine* const engine) {
  (void)engine;
  turnsLeft--;
}

void StatusFlared::start(Engine* const engine) {
  (void)engine;
}

void StatusFlared::end(Engine* const engine) {
  (void)engine;
}

void StatusFlared::newTurn(Engine* const engine) {
  owningActor->hit(1, damageType_fire);
  turnsLeft--;

  if(turnsLeft == 0) {
    bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
    engine->mapTests->makeVisionBlockerArray(engine->player->pos, visionBlockers, 99999);
    owningActor->getStatusEffectsHandler()->attemptAddEffect(new StatusBurning(engine));
    owningActor->getStatusEffectsHandler()->endEffect(statusFlared, visionBlockers, false);
  }
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
  case statusWeak:
    return new StatusWeak(TURNS_LEFT);
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
  case statusPoisoned:
    return new StatusPoisoned(TURNS_LEFT);
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
  case statusClairvoyant:
    return new StatusClairvoyant(TURNS_LEFT);
    break;
  default:
  {} break;
  }
  return NULL;
}

bool StatusEffectsHandler::allowSee() {
  if(hasEffect(statusClairvoyant)) {
    return true;
  }

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
  const int ACTOR_ABILITY = owningActor->getDef()->abilityValues.getAbilityValue(saveAbility, true, *(owningActor));
  const int SAVE_ABILITY_MOD = effect->getSaveAbilityModifier();
  int statusProtectionFromArmor = 0;

  if(OWNER_IS_PLAYER) {
    if(effect->getEffectId() == statusBurning) {
      Item* const armor = owningActor->getInventory()->getItemInSlot(slot_armorBody);
      if(armor != NULL) {
        const bool ARMOR_PROTECTS_FROM_BURNING = armor->getDef().armorData.protectsAgainstStatusBurning;
        if(ARMOR_PROTECTS_FROM_BURNING) {
          statusProtectionFromArmor = max(0, dynamic_cast<Armor*>(armor)->getDurability());
        }
      }
    } else if(effect->getEffectId() == statusConfused) {
      if(eng->playerBonusHandler->isBonusPicked(playerBonus_selfAware)) {
        eng->log->addMessage(effect->messageWhenSaves());
        delete effect;
        return;
      }
    }
  }

  const int TOTAL_SAVE_ABILITY = ACTOR_ABILITY + SAVE_ABILITY_MOD + statusProtectionFromArmor;

  AbilityRollResult_t result = eng->abilityRoll->roll(TOTAL_SAVE_ABILITY);

  if(result <= failSmall || FORCE_EFFECT) {
    //Saving throw failed, apply effect.

    //Actor already has effect?
    for(unsigned int i = 0; i < effects.size(); i++) {
      if(effect->getEffectId() == effects.at(i)->getEffectId()) {

        if(effect->canBeAppliedWhileSameEffectOngoing() == false) {
          delete effect;
          return;
        }

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
    effect->owningActor = owningActor;
    effects.push_back(effect);
    effect->start(eng);

    if(effect->isPlayerVisualUpdateNeededWhenStartOrEnd()) {
      effect->owningActor->updateColor();
      eng->player->updateFov();
      eng->renderer->drawMapAndInterface();
    }

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
  const ItemDefinition& wpnDef = weapon->getDef();
  StatusEffect* wpnEffect = IS_MELEE ? wpnDef.meleeStatusEffect : wpnDef.rangedStatusEffect;

  if(wpnEffect != NULL) {
    //Make a new copy of the weapon effect
    StatusEffect* cpy = wpnEffect->copy();

    //Attempt to add the effect to the actor.
    //If attempt fails (saving throw succeeds, immune, etc), the copy is destroyed.
    attemptAddEffect(cpy);
  }
}

void StatusEffectsHandler::runEffectEndAndRemoveFromList(const unsigned int index, const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS]) {
  const bool OWNER_IS_PLAYER = owningActor == eng->player;
  const bool PLAYER_SEE_OWNER = OWNER_IS_PLAYER ? true : eng->player->checkIfSeeActor(*owningActor, visionBlockingArray);

  StatusEffect* const effect = effects.at(index);

  effect->end(eng);

  const bool IS_VISUAL_UPDATE_NEEDED = effect->isPlayerVisualUpdateNeededWhenStartOrEnd();

  effects.erase(effects.begin() + index);

  if(IS_VISUAL_UPDATE_NEEDED) {
    effect->owningActor->updateColor();
    eng->player->updateFov();
    eng->renderer->drawMapAndInterface();
  }

  if(OWNER_IS_PLAYER && effect->messageWhenEnd() != "") {
    eng->log->addMessage(effect->messageWhenEnd(), clrWhite);
  } else {
    if(PLAYER_SEE_OWNER && effect->messageWhenEndOther() != "") {
      eng->log->addMessage(owningActor->getNameThe() + " " + effect->messageWhenEndOther());
    }
  }

  delete effect;
}

void StatusEffectsHandler::newTurnAllEffects(const bool visionBlockingArray[MAP_X_CELLS][MAP_Y_CELLS]) {
  for(unsigned int i = 0; i < effects.size();) {
    StatusEffect* const curEffect = effects.at(i);

    if(owningActor != eng->player) {
      if(curEffect->isMakingOwnerAwareOfPlayer()) {
        dynamic_cast<Monster*>(owningActor)->playerAwarenessCounter = owningActor->getDef()->nrTurnsAwarePlayer;
      }
    }

    if(curEffect->isFinnished()) {
      runEffectEndAndRemoveFromList(i, visionBlockingArray);
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

#include "FeatureTrap.h"

#include <cassert>

#include "Engine.h"

#include "FeatureFactory.h"
#include "FeatureData.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ItemDrop.h"
#include "Postmortem.h"
#include "Explosion.h"
#include "Popup.h"
#include "ActorMonster.h"

Trap::Trap(Feature_t id, coord pos, Engine* engine, TrapSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), mimicFeature_(spawnData->mimicFeature_), isHidden_(true) {

  assert(spawnData->trapType_ != endOfTraps);

  if(spawnData->trapType_ == trap_any) {
    Trap_t trapType = endOfTraps;
    while(true) {
      tracer << "Trap: Picking trap type" << endl;
      trapType = static_cast<Trap_t>(eng->dice.getInRange(0, endOfTraps - 1));
      tracer << "Trap: Picked trap type id: " << trapType << endl;

      if(trapType == trap_teleport) {
        tracer << "Trap: Picked trap is teleport trap" << endl;
        bool isNextToWalkBlocking = false;
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            const Feature* const f = eng->map->featuresStatic[pos.x + dx][pos.y + dy];
            if(f->isMoveTypePassable(moveType_walk) == false) {
              isNextToWalkBlocking = true;
              dy = 999;
              dx = 999;
            }
          }
        }
        if(isNextToWalkBlocking) {
          tracer << "Trap: Non-walkable adjacent cell found, picking other trap type" << endl;
          continue;
        } else {
          tracer << "Trap: All adjacent cells are walkable, ok to spawn teleport trap" << endl;
        }
      }
      break;
    }
    setSpecificTrapFromId(trapType);
  } else {
    setSpecificTrapFromId(spawnData->trapType_);
  }
  assert(specificTrap_ != NULL);
  assert(mimicFeature_ != NULL);
}

void Trap::setSpecificTrapFromId(const Trap_t id) {
  switch(id) {
  case trap_dart:
    specificTrap_ = new TrapDart(pos_, eng);
    break;
  case trap_spear:
    specificTrap_ = new TrapSpear(pos_, eng);
    break;
  case trap_gasConfusion:
    specificTrap_ = new TrapGasConfusion(pos_, eng);
    break;
  case trap_gasParalyze:
    specificTrap_ = new TrapGasParalyzation(pos_, eng);
    break;
  case trap_gasFear:
    specificTrap_ = new TrapGasFear(pos_, eng);
    break;
  case trap_blinding:
    specificTrap_ = new TrapBlindingFlash(pos_, eng);
    break;
  case trap_teleport:
    specificTrap_ = new TrapTeleport(pos_, eng);
    break;
  case trap_smoke:
    specificTrap_ = new TrapSmoke(pos_, eng);
    break;
  case trap_spiderWeb:
    specificTrap_ = new TrapSpiderWeb(pos_, eng);
    break;
  default: {
    specificTrap_ = NULL;
  }
  break;
  }
}

Trap_t Trap::getTrapType() const {
  return specificTrap_->trapType_;
}

void Trap::triggerOnPurpose(Actor* actorTriggering) {
  const AbilityRollResult_t DODGE_RESULT = failSmall;
  specificTrap_->trapSpecificTrigger(actorTriggering, DODGE_RESULT);
}

void Trap::bump(Actor* actorBumping) {
  const ActorDefinition* const d = actorBumping->getDef();

  if(d->moveType == moveType_walk) {
    const bool IS_PLAYER = actorBumping == actorBumping->eng->player;
    const bool ACTOR_CAN_SEE = actorBumping->getStatusEffectsHandler()->allowSee();
    const int DODGE_SKILL_VALUE = d->abilityValues.getAbilityValue(ability_dodgeTrap, true, *actorBumping);
    const int BASE_CHANCE_TO_AVOID = 40;

    const string trapName = specificTrap_->getTrapSpecificTitle();

    if(IS_PLAYER) {
      const int CHANCE_TO_AVOID = isHidden_ == true ? 10 : (BASE_CHANCE_TO_AVOID + DODGE_SKILL_VALUE);
      const AbilityRollResult_t result = actorBumping->eng->abilityRoll->roll(CHANCE_TO_AVOID);

      if(result >= successSmall) {
        if(isHidden_ == false) {
          if(ACTOR_CAN_SEE) {
            actorBumping->eng->log->addMessage("I avoid a " + trapName + ".", clrMessageGood);
          }
        }
      } else {
        const AbilityRollResult_t DODGE_RESULT = actorBumping->eng->abilityRoll->roll(DODGE_SKILL_VALUE);
        reveal(false);
        specificTrap_->trapSpecificTrigger(actorBumping, DODGE_RESULT);
      }

    } else {
      if(d->actorSize == actorSize_humanoid) {
        Monster* const monster = dynamic_cast<Monster*>(actorBumping);
        if(monster->playerAwarenessCounter > 0 && monster->isStealth == false) {

          const bool IS_ACTOR_SEEN_BY_PLAYER = actorBumping->eng->player->checkIfSeeActor(*actorBumping, NULL);

          const int CHANCE_TO_AVOID = BASE_CHANCE_TO_AVOID + d->canDodge ? DODGE_SKILL_VALUE : 0;
          const AbilityRollResult_t result = actorBumping->eng->abilityRoll->roll(CHANCE_TO_AVOID);

          if(result >= successSmall) {
            if(isHidden_ == false && IS_ACTOR_SEEN_BY_PLAYER) {
              const string actorName = actorBumping->getNameThe();
              actorBumping->eng->log->addMessage(actorName + " avoids a " + trapName + ".");
            }
          } else {
            const AbilityRollResult_t DODGE_RESULT =
              d->canDodge ? actorBumping->eng->abilityRoll->roll(DODGE_SKILL_VALUE) :  failNormal;
            if(IS_ACTOR_SEEN_BY_PLAYER) {
              reveal(false);
            }
            specificTrap_->trapSpecificTrigger(actorBumping, DODGE_RESULT);
          }
        }
      }
    }
  }
}

void Trap::reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES) {
  isHidden_ = false;
  clearGore();

  Item* item = eng->map->items[pos_.x][pos_.y];
  if(item != NULL) {
    eng->map->items[pos_.x][pos_.y] = NULL;
    eng->itemDrop->dropItemOnMap(pos_, &item);
  }

  if(eng->map->playerVision[pos_.x][pos_.y]) {
    eng->renderer->drawMapAndInterface();

    if(PRINT_MESSSAGE_WHEN_PLAYER_SEES) {
      const string name = specificTrap_->getTrapSpecificTitle();
      eng->log->addMessage("I spot a " + name + ".");
    }
  }
}

void Trap::playerTrySpotHidden() {
  if(isHidden_) {
    if(eng->mapTests->isCellsNeighbours(pos_, eng->player->pos, false)) {
      const Abilities_t abilityUsed = ability_searching;
      const int PLAYER_SKILL = eng->player->getDef()->abilityValues.getAbilityValue(abilityUsed, true, *(eng->player));

      if(eng->abilityRoll->roll(PLAYER_SKILL) >= successSmall) {
        reveal(true);
      }
    }
  }
}

string Trap::getDescription(const bool DEFINITE_ARTICLE) const {
  if(isHidden_) {
    return DEFINITE_ARTICLE == true ? mimicFeature_->name_the : mimicFeature_->name_a;
  } else {
    return "a " + specificTrap_->getTrapSpecificTitle();
  }
}

sf::Color Trap::getColor() const {
  return isHidden_ ? mimicFeature_->color : specificTrap_->getTrapSpecificColor();
}

char Trap::getGlyph() const {
  return isHidden_ ? mimicFeature_->glyph : specificTrap_->getTrapSpecificGlyph();
}

Tile_t Trap::getTile() const {
  return isHidden_ ? mimicFeature_->tile : specificTrap_->getTrapSpecificTile();
}

bool Trap::canHaveCorpse() const {
  return isHidden_;
}

bool Trap::canHaveBlood() const {
  return isHidden_;
}

bool Trap::canHaveGore() const {
  return isHidden_;
}

bool Trap::canHaveItem() const {
  return isHidden_;
}

coord Trap::actorAttemptLeave(Actor* const actor, const coord& pos, const coord& dest) {
  assert(specificTrap_ != NULL);
  return specificTrap_->specificTrapActorAttemptLeave(actor, pos, dest);
}

MaterialType_t Trap::getMaterialType() const {
  return isHidden_ ? mimicFeature_->materialType : def_->materialType;
}

//============================================================= TRAP LIST

void TrapDart::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng->log->addMessage("I dodge a dart!", clrMessageGood);
      } else {
        eng->log->addMessage("I feel a mechanism trigger and quickly leap aside!", clrMessageGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng->log->addMessage(actorName +  " dodges a dart!");
      }
    }
  } else {
    //Trap misses?
    const int CHANCE_TO_HIT = 75;
    if(eng->dice(1, 100) > CHANCE_TO_HIT) {
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng->log->addMessage("A dart barely misses me!", clrMessageGood);
        } else {
          eng->log->addMessage("A mechanism triggers, I hear something barely missing me!", clrMessageGood);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          eng->log->addMessage("A dart barely misses " + actorName + "!");
        }
      }
    } else {
      //Dodge failed and trap hits
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng->log->addMessage("I am hit by a dart!", clrMessageBad);
        } else {
          eng->log->addMessage("A mechanism triggers, I feel a needle piercing my skin!", clrMessageBad);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          eng->log->addMessage(actorName + " is hit by a dart!", clrMessageGood);
        }
      }

      const int DMG = eng->dice(1, 8);
      actor->hit(DMG, damageType_physical);
    }
  }
}

void TrapSpear::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng->log->addMessage("I dodge a spear!", clrMessageGood);
      } else {
        eng->log->addMessage("I feel a mechanism trigger and quickly leap aside!", clrMessageGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng->log->addMessage(actorName +  " dodges a spear!");
      }
    }
  } else {
    //Trap misses?
    const int CHANCE_TO_HIT = 75;
    if(eng->dice(1, 100) > CHANCE_TO_HIT) {
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng->log->addMessage("A spear barely misses me!", clrMessageGood);
        } else {
          eng->log->addMessage("A mechanism triggers, I hear a *swoosh*!", clrMessageGood);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          eng->log->addMessage("A spear barely misses " + actorName + "!");
        }
      }
    } else {
      //Dodge failed and trap hits
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng->log->addMessage("I am hit by a spear!", clrMessageBad);
        } else {
          eng->log->addMessage("A mechanism triggers, something sharp pierces my skin!", clrMessageBad);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          eng->log->addMessage(actorName + " is hit by a spear!", clrMessageGood);
        }
      }

      const int DMG = eng->dice(2, 6);
      actor->hit(DMG, damageType_physical);
    }
  }
}

void TrapGasConfusion::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  (void)dodgeResult;

  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng->log->addMessage("I am hit by a burst of gas!");
    } else {
      eng->log->addMessage("A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng->log->addMessage(actorName + " is hit by a burst of gas!");
    }
  }

  actor->eng->explosionMaker->runExplosion(pos_, false, new StatusConfused(eng), true, getTrapSpecificColor());
}

void TrapGasParalyzation::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  (void)dodgeResult;

  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng->log->addMessage("I am hit by a burst of gas!");
    } else {
      eng->log->addMessage("A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng->log->addMessage(actorName + " is hit by a burst of gas!");
    }
  }

  actor->eng->explosionMaker-> runExplosion(pos_, false, new StatusParalyzed(eng), true, getTrapSpecificColor());
}

void TrapGasFear::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  (void)dodgeResult;

  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng->log->addMessage("I am hit by a burst of gas!");
    } else {
      eng->log->addMessage("A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng->log->addMessage(actorName + " is hit by a burst of gas!");
    }
  }

  actor->eng->explosionMaker-> runExplosion(pos_, false, new StatusTerrified(eng), true, getTrapSpecificColor());
}

void TrapBlindingFlash::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng->log->addMessage("I cover my eyes just in time to avoid an intense flash!", clrMessageGood);
      } else {
        eng->log->addMessage("I feel a mechanism trigger!", clrMessageGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng->log->addMessage(actorName + " covers from a blinding flash!");
      }
    }
  } else {
    //Dodge failed
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng->log->addMessage("A sharp flash of light pierces my eyes!", clrWhite);
        actor->getStatusEffectsHandler()->attemptAddEffect(new StatusBlind(eng));
      } else {
        eng->log->addMessage("I feel a mechanism trigger!", clrWhite);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng->log->addMessage(actorName + " is hit by a flash of blinding light!");
        actor->getStatusEffectsHandler()->attemptAddEffect(new StatusBlind(eng));
      }
    }
  }
}

void TrapTeleport::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  (void)dodgeResult;

  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  if(IS_PLAYER) {
    eng->player->FOVupdate();
    if(CAN_SEE) {
      eng->popup->showMessage("A curious shape on the floor starts to glow!", true);
    } else {
      eng->popup->showMessage("I feel a peculiar energy around me!", true);
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng->log->addMessage("A curious shape on the floor starts go glow under " + actorName);
    }
  }

  actor->teleportToRandom();
}

void TrapSmoke::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  (void)dodgeResult;

  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng->log->addMessage("Suddenly the air is thick with smoke!");
    } else {
      eng->log->addMessage("A mechanism triggers, the air is thick with smoke!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng->log->addMessage("Suddenly the air around " + actorName + " is thick with smoke!");
    }
  }

  actor->eng->explosionMaker->runSmokeExplosion(pos_);
}

void TrapSpiderWeb::trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) {
  (void)dodgeResult;

  isHoldingActor = true;

  const bool IS_PLAYER = actor == eng->player;
  const bool CAN_SEE = actor->getStatusEffectsHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
  const string actorName = actor->getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng->log->addMessage("I find myself entangled in a huge spider web!");
    } else {
      eng->log->addMessage("I am entangled by a sticky mass of threads!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng->log->addMessage(actorName + " is entangled in a huge spider web!");
    }
  }
}

coord TrapSpiderWeb::specificTrapActorAttemptLeave(Actor* const actor, const coord& pos, const coord& dest) {
  if(isHoldingActor) {

    const bool IS_PLAYER = actor == eng->player;
    const bool PLAYER_CAN_SEE = eng->player->getStatusEffectsHandler()->allowSee();
    const bool PLAYER_CAN_SEE_ACTOR = eng->player->checkIfSeeActor(*actor, NULL);
    const string actorName = actor->getNameThe();

    const int ABILITY_VALUE = max(30, actor->getDef()->abilityValues.getAbilityValue(ability_resistStatusBody, true, *actor));

    const AbilityRollResult_t rollResult = eng->abilityRoll->roll(ABILITY_VALUE);
    if(rollResult >= successSmall) {

      isHoldingActor = false;

      if(IS_PLAYER) {
        eng->log->addMessage("I break free.");
      } else {
        if(PLAYER_CAN_SEE_ACTOR) {
          eng->log->addMessage(actorName + " breaks free from a spiderweb.");
        }
      }

      if(eng->dice(1, 100) <= 50) {

        if((IS_PLAYER && PLAYER_CAN_SEE) || (IS_PLAYER == false && PLAYER_CAN_SEE_ACTOR)) {
          eng->log->addMessage("The web is destroyed.");
        }

        eng->featureFactory->spawnFeatureAt(feature_trashedSpiderWeb, pos_);
      }
      return dest;
    } else {
      if(IS_PLAYER) {
        eng->log->addMessage("I struggle to break free.");
      } else {
        if(PLAYER_CAN_SEE_ACTOR) {
          eng->log->addMessage(actorName + " struggles to break free.");
        }
      }
      return pos;
    }
  }
  return dest;
}

#include "FeatureDoor.h"

#include "Engine.h"

#include "Actor.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "FeatureData.h"
#include "Map.h"
#include "Log.h"
#include "Postmortem.h"

//---------------------------------------------------INHERITED FUNCTIONS
Door::Door(Feature_t id, coord pos, Engine* engine, DoorSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), mimicFeature_(spawnData->mimicFeature_), nrOfSpikes_(0) {

  const int ROLL = eng->dice(1, 100);
  const DoorSpawnState_t doorState =
    ROLL < 5 ? doorSpawnState_secretAndStuck :
    ROLL < 30 ? doorSpawnState_secret :
    ROLL < 40? doorSpawnState_stuck :
    ROLL < 50 ? doorSpawnState_broken :
    ROLL < 65 ? doorSpawnState_open :
    doorSpawnState_closed;

  switch(static_cast<DoorSpawnState_t>(doorState)) {
  case doorSpawnState_broken: {
    isOpen_ = true;
    isBroken_ = true;
    isStuck_ = false;
    isSecret_ = false;
  }
    break;

  case doorSpawnState_open: {
    isOpen_ = true;
    isBroken_ = false;
    isStuck_ = false;
    isSecret_ = false;
  }
    break;

  case doorSpawnState_closed: {
    isOpen_ = false;
    isBroken_ = false;
    isStuck_ = false;
    isSecret_ = false;
  }
    break;

  case doorSpawnState_stuck: {
    isOpen_ = false;
    isBroken_ = false;
    isStuck_ = true;
    isSecret_ = false;
  }
    break;

  case doorSpawnState_secret: {
    isOpen_ = false;
    isBroken_ = false;
    isStuck_ = false;
    isSecret_ = true;
  }
    break;

  case doorSpawnState_secretAndStuck: {
    isOpen_ = false;
    isBroken_ = false;
    isStuck_ = true;
    isSecret_ = true;
  }
    break;

  }

  const int CHANCE_FOR_METAL = 20;
  material_ = eng->dice(1, 100) < CHANCE_FOR_METAL ? doorMaterial_metal : doorMaterial_wood;
}

bool Door::isMovePassable(Actor* const actorMoving) const {
  (void)actorMoving;
  return isOpen_;
}

bool Door::isMoveTypePassable(const MoveType_t moveType) const {
  switch(moveType) {
  case moveType_walk:
    return isOpen_;
    break;
  case moveType_ethereal:
    return true;
    break;
  case moveType_ooze:
    return true;
    break;
  case moveType_fly:
    return isOpen_;
    break;
  default:
    return isOpen_;
    break;
  }
  return false;
}

bool Door::isVisionPassable() const {
  return isOpen_;
}

bool Door::isShootPassable() const {
  return isOpen_;
}

bool Door::isSmokePassable() const {
  return isOpen_;
}

SDL_Color Door::getColor() const {
  if(isSecret_) {
    return mimicFeature_->color;
  }
  return clrBrownDark; /*material_ == doorMaterial_metal ? clrGray : clrBrownDark;*/
}

char Door::getGlyph() const {
  return isSecret_ ? mimicFeature_->glyph : (isOpen_ ? 39 : '+');
}

Tile_t Door::getTile() const {
  return isSecret_ ? mimicFeature_->tile : (isOpen_ ? (isBroken_ ? tile_doorBroken : tile_doorOpen) : tile_doorClosed);
}

MaterialType_t Door::getMaterialType() const {
  return isSecret_ ? mimicFeature_->materialType : def_->materialType;
}

void Door::bump(Actor* actorBumping) {
  if(isSecret_) {
    if(eng->map->playerVision[pos_.x][pos_.y]) {
      eng->log->addMessage("That way is blocked.");
    } else {
      eng->log->addMessage("I bump into something.");
    }
    return;
  }

  if(isOpen_ == false) {
    tryOpen(actorBumping);
  }
}

string Door::getDescription(const bool DEFINITE_ARTICLE) const {
  if(isOpen_ && isBroken_ == false) {
    return DEFINITE_ARTICLE ? "the open door" : "an open door";
  }
  if(isBroken_) {
    return DEFINITE_ARTICLE ? "the broken door" : "a broken door";
  }
  if(isSecret_) {
    return DEFINITE_ARTICLE ? mimicFeature_->name_the : mimicFeature_->name_a;
  }
  if(isOpen_ == false) {
    return DEFINITE_ARTICLE ? "the closed door" : "a closed door";
  }

  return "[WARNING] Door lacks description?";
}
//----------------------------------------------------------------------

void Door::reveal(const bool PRINT_MESSAGE) {
  isSecret_ = false;

  if(eng->map->playerVision[pos_.x][pos_.y]) {
    eng->renderer->drawMapAndInterface();
    if(PRINT_MESSAGE) {
      eng->log->addMessage("A secret is revealed.");
      eng->renderer->flip();
    }
  }
}

void Door::playerTrySpotHidden() {
  if(isSecret_) {
    if(eng->mapTests->isCellsNeighbours(coord(pos_.x, pos_.y), eng->player->pos, false)) {
      const Abilities_t abilityUsed = ability_searching;
      const int PLAYER_SKILL = eng->player->getInstanceDefinition()->abilityValues.getAbilityValue(abilityUsed, true);

      if(eng->abilityRoll->roll(PLAYER_SKILL) >= successSmall) {
	reveal(true);
      }
    }
  }
}

bool Door::trySpike(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND = actorTrying->getStatusEffectsHandler()->allowSee() == false;

  if(isSecret_ || isOpen_) {
    return false;
  }

  //Door is in correct state for spiking (known, closed)
  nrOfSpikes_++;
  isStuck_ = true;

  if(IS_PLAYER) {
    if(TRYER_IS_BLIND == false) {
      eng->log->addMessage("I jam the door with a spike.");
    } else {
      eng->log->addMessage("I jam a door with a spike.");
    }
    eng->soundEmitter->emitSound(Sound("", true, coord(pos_.x, pos_.y), false, IS_PLAYER));
  }
  eng->gameTime->letNextAct();
  return true;

}

void Door::tryBash(Actor* actorTrying) {
  tracer << "Door::tryBash()..." << endl;
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND = actorTrying->getStatusEffectsHandler()->allowSee() == false;
  const bool PLAYER_SEE_DOOR = eng->map->playerVision[pos_.x][pos_.y];
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(blockers);

  const bool PLAYER_SEE_TRYER = IS_PLAYER ? true : eng->player->checkIfSeeActor(*actorTrying, blockers);

  bool bashable = true;

  if(isOpen_ || (isSecret_ && IS_PLAYER)) {
    tracer << "Door: Not in bashable state (is open, or player trying and is secret)" << endl;
    bashable = false;
    if(IS_PLAYER) {
      if(TRYER_IS_BLIND == false) {
	eng->log->addMessage("I see nothing there to bash.");
      } else {
	eng->log->addMessage("I find nothing there to bash.");
      }
    }
  }

  if(bashable) {
    tracer << "Door: Is in bashable state" << endl;
    //Door is in correct state for bashing (known, closed)

    if(IS_PLAYER) {
      tracer << "Door: Basher is player" << endl;
      if(TRYER_IS_BLIND == false) {
	eng->log->addMessage("I smash into the door!");
      } else {
	eng->log->addMessage("I smash into a door!");
      }
      eng->soundEmitter->emitSound(Sound("", true, coord(pos_.x, pos_.y), false, IS_PLAYER));
    } else {
      if(PLAYER_SEE_TRYER) {
	eng->log->addMessage(actorTrying->getNameThe() + " bashes at a door!");
      }
      //(the sound emits from the actor instead of the door here, because the sound should
      //be heard even if the door is seen, and the parameter for muting messages from seen sounds
      //should be off)
      eng->soundEmitter->emitSound(Sound("I hear a loud *THUD* at a door.", true, actorTrying->pos, false, IS_PLAYER));
    }

    //Various things that can happen...
    const int SKILL_VALUE_SMASH = actorTrying == eng->player ? 60 - min(58, nrOfSpikes_ * 20) : 10 - min(9, nrOfSpikes_ * 3);
    const bool DOOR_SMASHED = eng->abilityRoll->roll(SKILL_VALUE_SMASH) >= successSmall;

    if(IS_PLAYER) {
      const int SKILL_VALUE_UNHURT = 75;
      const bool TRYER_SPRAINED = eng->abilityRoll->roll(SKILL_VALUE_UNHURT) <= failSmall;

      const int SKILL_VALUE_BALANCE = 75;
      const bool TRYER_OFF_BALANCE = eng->abilityRoll->roll(SKILL_VALUE_BALANCE) <= failSmall;

      if(TRYER_SPRAINED) {
	if(IS_PLAYER)
	  eng->log->addMessage("I sprain myself.", clrMessageBad);
	else if(PLAYER_SEE_TRYER)
	  eng->log->addMessage(actorTrying->getNameThe() + " is hurt.");
	const int SPRAIN_DMG = 1;
	const bool DIED = actorTrying->hit(SPRAIN_DMG, damageType_direct);
	if(DIED) {
	  if(actorTrying == eng->player) {
	    eng->postmortem->setCauseOfDeath("Killed by a door");
	  }
	}
      }

      if(TRYER_OFF_BALANCE) {
	if(IS_PLAYER)
	  eng->log->addMessage("I am off-balance.");
	else if(PLAYER_SEE_TRYER)
	  eng->log->addMessage(actorTrying->getNameThe() + " is off-balance.");

	actorTrying->getStatusEffectsHandler()->attemptAddEffect(new StatusParalyzed(2));
      }
    }

    if(DOOR_SMASHED) {
      tracer << "Door: Bash successful" << endl;
      isBroken_ = true;
      isStuck_ = false;
      isSecret_ = false;
      isOpen_ = true;
      if(IS_PLAYER) {
	if(TRYER_IS_BLIND == false) {
	  eng->log->addMessage("The door crashes open!");
	} else {
	  eng->log->addMessage("I feel the door crashing open!");
	}
	eng->soundEmitter->emitSound(Sound("", true, coord(pos_.x, pos_.y), false, IS_PLAYER));
      } else {
	if(PLAYER_SEE_TRYER) {
	  eng->log->addMessage(actorTrying->getNameThe() + " smashes into a door.");
	  eng->log->addMessage("The door crashes open!");
	} else if(PLAYER_SEE_DOOR) {
	  eng->log->addMessage("A door crashes open!");
	}
	eng->soundEmitter->emitSound(Sound("I hear a door crashing open!", true, coord(pos_.x, pos_.y), false, IS_PLAYER));
      }
    }
    
    tracer << "Door: Calling GameTime::letNextAct()" << endl;
    eng->gameTime->letNextAct();
  }
  tracer << "Door::tryBash() [DONE]" << endl;
}

void Door::tryClose(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND = actorTrying->getStatusEffectsHandler()->allowSee() == false;
  //const bool PLAYER_SEE_DOOR    = eng->map->playerVision[pos_.x][pos_.y];
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(blockers);

  const bool PLAYER_SEE_TRYER = IS_PLAYER ? true : eng->player->checkIfSeeActor(*actorTrying, blockers);

  bool closable = true;

  //Broken?
  if(isBroken_) {
    closable = false;
    if(IS_PLAYER) {
      if(IS_PLAYER)
	eng->log->addMessage("The door appears to be broken.");
    }
  }

  //Already closed?
  if(closable) {
    if(isOpen_ == false) {
      closable = false;
      if(IS_PLAYER) {
	if(TRYER_IS_BLIND == false)
	  eng->log->addMessage("I see nothing there to close.");
	else eng->log->addMessage("I find nothing there to close.");
      }
    }
  }

  //Blocked?
  if(closable) {
    eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
    eng->mapTests->addItemsToBlockerArray(blockers);
    const bool BLOCKED = blockers[pos_.x][pos_.y];
    if(BLOCKED) {
      closable = false;
      if(IS_PLAYER) {
	if(TRYER_IS_BLIND == false) {
	  eng->log->addMessage("The door is blocked.");
	}
	else {
	  eng->log->addMessage("Something is blocking the door.");
	}
      }
    }
  }

  if(closable) {

    //Door is in correct state for closing (open, working, not blocked)


    //need sounds***

    if(TRYER_IS_BLIND == false) {
      isOpen_ = false;
      if(IS_PLAYER)
	eng->log->addMessage("I close the door.");
      else if(PLAYER_SEE_TRYER)
	eng->log->addMessage(actorTrying->getNameThe() + " closes a door.");
    } else {
      if(eng->dice(1, 100) < 50) {
	isOpen_ = false;
	if(IS_PLAYER) {
	  eng->log->addMessage("I fumble with a door and succeed to close it.");
	} else {
	  if(PLAYER_SEE_TRYER)
	    eng->log->addMessage(actorTrying->getNameThe() + "fumbles about and succeeds to close a door.");
	}
      } else {
	if(IS_PLAYER) {
	  eng->log->addMessage("I fumble blindly with a door and fail to close it.");
	} else {
	  if(PLAYER_SEE_TRYER)
	    eng->log->addMessage(actorTrying->getNameThe() + " fumbles blindly and fails to close a door.");
	}
      }
    }
  }

  if(isOpen_ == false && closable) {
    eng->gameTime->letNextAct();
  }
}

void Door::tryOpen(Actor* actorTrying) {
  tracer << "Door::tryOpen()..." << endl;
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND = actorTrying->getStatusEffectsHandler()->allowSee() == false;
  const bool PLAYER_SEE_DOOR = eng->map->playerVision[pos_.x][pos_.y];
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(blockers);

  const bool PLAYER_SEE_TRYER = IS_PLAYER ? true : eng->player->checkIfSeeActor(*actorTrying, blockers);

  if(isStuck_) {
    tracer << "Door: Is stuck" << endl;
    //placeholder, need lockpicking***
    if(IS_PLAYER)
      eng->log->addMessage("The door seems to be stuck.");

  } else {
    tracer << "Door: Is not stuck" << endl;
    if(TRYER_IS_BLIND == false) {
      tracer << "Door: Tryer can see, opening" << endl;
      isOpen_ = true;
      if(IS_PLAYER) {
	eng->log->addMessage("I open the door.");
	eng->soundEmitter->emitSound(Sound("", true, coord(pos_.x, pos_.y), false, IS_PLAYER));
      } else {
	if(PLAYER_SEE_TRYER) {
	  eng->log->addMessage(actorTrying->getNameThe() + " opens a door.");
	} else if(PLAYER_SEE_DOOR) {
	  eng->log->addMessage("I see a door opening.");
	}
	eng->soundEmitter->emitSound(Sound("I hear a door open.", true, coord(pos_.x, pos_.y), false, IS_PLAYER));
      }
    } else {
      if(eng->dice(1, 100) < 50) {
	tracer << "Door: Tryer is blind, but open succeeded anyway" << endl;
	isOpen_ = true;
	if(IS_PLAYER) {
	  eng->log->addMessage("I fumble with a door and succeed to open it.");
	  eng->soundEmitter->emitSound(Sound("", true, pos_, false, IS_PLAYER));
	} else {
	  if(PLAYER_SEE_TRYER) {
	    eng->log->addMessage(actorTrying->getNameThe() + "fumbles about and succeeds to open a door.");
	  } else if(PLAYER_SEE_DOOR) {
	    eng->log->addMessage("I see a door open clumsily.");
	  }
	  eng->soundEmitter->emitSound(Sound("I hear something open a door clumsily.", true, pos_, false, IS_PLAYER));
	}
      } else {
	tracer << "Door: Tryer is blind, and open failed" << endl;
	if(IS_PLAYER) {
	  eng->log->addMessage("I fumble blindly with a door and fail to open it.");
	  eng->soundEmitter->emitSound(Sound("", true, pos_, false, IS_PLAYER));
	} else {
	  if(PLAYER_SEE_TRYER) {
	    eng->log->addMessage(actorTrying->getNameThe() + " fumbles blindly and fails to open a door.");
	  }
	  //(emitting the sound from the actor instead of the door here, beacause the sound should
	  //be heard even if the door is seen, and the parameter for muting messages from seen sounds
	  //should be off)
	  eng->soundEmitter->emitSound(Sound("I hear something attempting to open a door.", true, actorTrying->pos, false, IS_PLAYER));
	}
      }
    }
  }

  if(isOpen_) {
    tracer << "Door: Open was successful" << endl;
    if(isSecret_) {
      tracer << "Door: Was secret, now revealing" << endl;
      reveal(true);
    }
    tracer << "Door: Calling GameTime::letNextAct()" << endl;
    eng->gameTime->letNextAct();
  }
  tracer << "Door::tryOpen() [DONE]" << endl;
}


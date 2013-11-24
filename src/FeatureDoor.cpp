#include "FeatureDoor.h"

#include "Engine.h"

#include "Actor.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "FeatureData.h"
#include "Map.h"
#include "Log.h"
#include "Postmortem.h"
#include "PlayerBonuses.h"
#include "Renderer.h"

//---------------------------------------------------INHERITED FUNCTIONS
Door::Door(Feature_t id, Pos pos, Engine* engine, DoorSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), mimicFeature_(spawnData->mimicFeature_),
  nrSpikes_(0) {

  isOpenedAndClosedExternally_ = false;

  isClued_ = false;

  const int ROLL = eng->dice.percentile();
  const DoorSpawnState_t doorState =
    ROLL < 5 ? doorSpawnState_secretAndStuck :
    ROLL < 40 ? doorSpawnState_secret :
    ROLL < 50 ? doorSpawnState_stuck :
    ROLL < 60 ? doorSpawnState_broken :
    ROLL < 75 ? doorSpawnState_open :
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

  material_ = doorMaterial_wood;
}

bool Door::isMovePassable(Actor* const actorMoving) const {
  (void)actorMoving;
  return isOpen_;
}

bool Door::isBodyTypePassable(const ActorBodyType_t bodyType) const {
  switch(bodyType) {
    case actorBodyType_normal:    return isOpen_;   break;
    case actorBodyType_ethereal:  return true;      break;
    case actorBodyType_ooze:      return true;      break;
    case actorBodyType_flying:    return isOpen_;   break;
    case endOfActorBodyTypes:     return isOpen_;   break;
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
    if(isClued_) {
      return clrYellow;
    } else {
      return mimicFeature_->color;
    }
  }
  return material_ == doorMaterial_metal ? clrGray : clrBrownDark;
}

char Door::getGlyph() const {
  return isSecret_ ? mimicFeature_->glyph : (isOpen_ ? 39 : '+');
}

Tile_t Door::getTile() const {
  if(isSecret_) {
    return mimicFeature_->tile;
  } else {
    if(isOpen_) {
      return isBroken_ ? tile_doorBroken : tile_doorOpen;
    } else {
      return tile_doorClosed;
    }
  }
}

MaterialType_t Door::getMaterialType() const {
  return isSecret_ ? mimicFeature_->materialType : data_->materialType;
}

void Door::bump(Actor& actorBumping) {
  if(&actorBumping == eng->player) {
    if(isSecret_) {
      if(eng->map->playerVision[pos_.x][pos_.y]) {
        trace << "Door: Player bumped into secret door, with vision in cell" << endl;
        eng->log->addMsg("That way is blocked.");
      } else {
        trace << "Door: Player bumped into secret door, without vision in cell" << endl;
        eng->log->addMsg("I bump into something.");
      }
      return;
    }

    if(isOpen_ == false) {
      tryOpen(&actorBumping);
    }
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
    const string cluedStr = isClued_ ? " {strange}" : "";
    return (DEFINITE_ARTICLE ?
            mimicFeature_->name_the :
            mimicFeature_->name_a) +
           cluedStr;
  }
  if(isOpen_ == false) {
    return DEFINITE_ARTICLE ? "the closed door" : "a closed door";
  }

  return "[WARNING] Door lacks description?";
}
//----------------------------------------------------------------------

void Door::reveal(const bool ALLOW_MESSAGE) {
  if(isSecret_) {
    isSecret_ = false;
    if(eng->map->playerVision[pos_.x][pos_.y]) {
      eng->renderer->drawMapAndInterface();
      if(ALLOW_MESSAGE) {
        eng->log->addMsg("A secret is revealed.");
        eng->renderer->drawMapAndInterface();
      }
    }
  }
}

void Door::clue() {
  isClued_ = true;
  if(eng->dice.coinToss()) {
    eng->log->addMsg("Something seems odd about the wall here...");
  } else {
    eng->log->addMsg("I sense a draft here...");
  }
  eng->renderer->drawMapAndInterface();
}

void Door::playerTrySpotHidden() {
  if(isSecret_) {
    if(
      eng->mapTests->isCellsAdj(
        Pos(pos_.x, pos_.y), eng->player->pos, false)) {
      const int PLAYER_SKILL =
        eng->player->getData()->abilityVals.getVal(
          ability_searching, true, *(eng->player));
      if(eng->abilityRoll->roll(PLAYER_SKILL) >= successSmall) {
        reveal(true);
      }
    }
  }
}

void Door::playerTryClueHidden() {
  if(isSecret_ && isClued_ == false) {
    const int PLAYER_SKILL =
      eng->player->getData()->abilityVals.getVal(
        ability_searching, true, *(eng->player));
    const int BONUS = 10;
    if(eng->abilityRoll->roll(PLAYER_SKILL + BONUS) >= successSmall) {
      clue();
    }
  }
}

bool Door::trySpike(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND =
    actorTrying->getPropHandler()->allowSee() == false;

  if(isSecret_ || isOpen_) {
    return false;
  }

  //Door is in correct state for spiking (known, closed)
  nrSpikes_++;
  isStuck_ = true;

  if(IS_PLAYER) {
    if(TRYER_IS_BLIND == false) {
      eng->log->addMsg("I jam the door with a spike.");
    } else {
      eng->log->addMsg("I jam a door with a spike.");
    }
    eng->soundEmitter->emitSound(
      Sound("", endOfSfx, true, Pos(pos_.x, pos_.y), false, IS_PLAYER));
  }
  eng->gameTime->endTurnOfCurrentActor();
  return true;

}

void Door::tryBash(Actor* actorTrying) {
  trace << "Door::tryBash()..." << endl;
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND =
    actorTrying->getPropHandler()->allowSee() == false;
  const bool PLAYER_SEE_DOOR = eng->map->playerVision[pos_.x][pos_.y];
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true : eng->player->checkIfSeeActor(*actorTrying, blockers);

  bool bashable = true;

  if(
    isOpen_ ||
    (isSecret_ && IS_PLAYER) ||
    (material_ == doorMaterial_metal && IS_PLAYER == false)) {
    trace << "Door: Not in bashable state (is open, or player trying and is secret)" << endl;
    bashable = false;
    if(IS_PLAYER) {
      if(TRYER_IS_BLIND == false) {
        eng->log->addMsg("I see nothing there to bash.");
      } else {
        eng->log->addMsg("I find nothing there to bash.");
      }
    }
  }

  if(bashable) {
    trace << "Door: Is in bashable state" << endl;

    if(IS_PLAYER) {
      trace << "Door: Basher is player" << endl;
      if(TRYER_IS_BLIND) {
        eng->log->addMsg("I smash into a door!");
      } else {
        eng->log->addMsg("I smash into the door!");
      }
      Sound snd("", sfxDoorBang, true, pos_, false, IS_PLAYER);
      eng->soundEmitter->emitSound(snd);
    } else {
      if(PLAYER_SEE_TRYER) {
        eng->log->addMsg(actorTrying->getNameThe() + " bashes at a door!");
      }
      // (The sound emits from the actor instead of the door, because the sound should be heard even
      // if the door is seen, and the parameter for muting messages from seen sounds should be off)
      Sound snd("I hear a loud banging on a door.",
                sfxDoorBang, true, actorTrying->pos, false, IS_PLAYER);
      eng->soundEmitter->emitSound(snd);
    }

    //Various things that can happen...
    int skillValueBash = 0;
    bool isBasherWeak = actorTrying->getPropHandler()->hasProp(propWeakened);
    if(isBasherWeak == false) {
      if(actorTrying == eng->player) {
        const int BON = eng->playerBonHandler->isBonPicked(playerBon_tough) ? 20 : 0;
        skillValueBash = 60 + BON - min(58, nrSpikes_ * 20);
      } else {
        skillValueBash = 10 - min(9, nrSpikes_ * 3);
      }
    }
    const bool DOOR_SMASHED =
      (material_ == doorMaterial_metal || isBasherWeak) ?
      false :
      eng->abilityRoll->roll(skillValueBash) >= successSmall;

    if(IS_PLAYER) {
      const int SKILL_VALUE_UNHURT = 75;
      const bool TRYER_SPRAINED =
        eng->abilityRoll->roll(SKILL_VALUE_UNHURT) <= failSmall;

      const int SKILL_VALUE_BALANCE = 75;
      const bool TRYER_OFF_BALANCE =
        eng->abilityRoll->roll(SKILL_VALUE_BALANCE) <= failSmall;

      if(TRYER_SPRAINED) {
        if(IS_PLAYER) {
          eng->log->addMsg("I sprain myself.", clrMessageBad);
        } else {
          if(PLAYER_SEE_TRYER) {
            eng->log->addMsg(actorTrying->getNameThe() + " is hurt.");
          }
        }
        const int SPRAIN_DMG = eng->dice.range(1, 5);
        actorTrying->hit(SPRAIN_DMG, dmgType_pure, false);
      }

      if(TRYER_OFF_BALANCE) {
        if(IS_PLAYER) {
          eng->log->addMsg("I am off-balance.");
        } else if(PLAYER_SEE_TRYER) {
          eng->log->addMsg(actorTrying->getNameThe() + " is off-balance.");
        }

        actorTrying->getPropHandler()->tryApplyProp(
          new PropParalyzed(eng, propTurnsSpecified, 2));
      }

      if(IS_PLAYER && (material_ == doorMaterial_metal || isBasherWeak)) {
        eng->log->addMsg("It seems futile.");
      }
    }

    if(DOOR_SMASHED) {
      trace << "Door: Bash successful" << endl;
      isBroken_ = true;
      isStuck_ = false;
      isSecret_ = false;
      isOpen_ = true;
      if(IS_PLAYER) {
        Sound snd("", sfxDoorBreak, true, pos_, false, IS_PLAYER);
        eng->soundEmitter->emitSound(snd);
        if(TRYER_IS_BLIND == false) {
          eng->log->addMsg("The door crashes open!");
        } else {
          eng->log->addMsg("I feel the door crashing open!");
        }
      } else {
        if(PLAYER_SEE_TRYER) {
          eng->log->addMsg(
            actorTrying->getNameThe() + " smashes into a door.");
          eng->log->addMsg("The door crashes open!");
        } else if(PLAYER_SEE_DOOR) {
          eng->log->addMsg("A door crashes open!");
        }
        Sound snd("I hear a door crashing open!",
                  sfxDoorBreak, true, pos_, false, IS_PLAYER);
        eng->soundEmitter->emitSound(snd);
      }
    }

    eng->renderer->drawMapAndInterface();

    trace << "Door: Calling GameTime::endTurnOfCurrentActor()" << endl;
    eng->gameTime->endTurnOfCurrentActor();
  }
  trace << "Door::tryBash() [DONE]" << endl;
}

void Door::tryClose(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND =
    actorTrying->getPropHandler()->allowSee() == false;
  //const bool PLAYER_SEE_DOOR    = eng->map->playerVision[pos_.x][pos_.y];
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true :
    eng->player->checkIfSeeActor(*actorTrying, blockers);

  bool closable = true;

  if(isOpenedAndClosedExternally_) {
    if(IS_PLAYER) {
      eng->log->addMsg("This door refuses to be closed, perhaps it is handled elsewhere?");
      eng->renderer->drawMapAndInterface();
    }
    return;
  }

  //Broken?
  if(isBroken_) {
    closable = false;
    if(IS_PLAYER) {
      if(IS_PLAYER)
        eng->log->addMsg("The door appears to be broken.");
    }
  }

  //Already closed?
  if(closable) {
    if(isOpen_ == false) {
      closable = false;
      if(IS_PLAYER) {
        if(TRYER_IS_BLIND == false)
          eng->log->addMsg("I see nothing there to close.");
        else eng->log->addMsg("I find nothing there to close.");
      }
    }
  }

  //Blocked?
  if(closable) {
    eng->mapTests->makeMoveBlockerArrayForBodyType(
      actorBodyType_normal, blockers);
    eng->mapTests->addItemsToBlockerArray(blockers);
    const bool BLOCKED = blockers[pos_.x][pos_.y];
    if(BLOCKED) {
      closable = false;
      if(IS_PLAYER) {
        if(TRYER_IS_BLIND == false) {
          eng->log->addMsg("The door is blocked.");
        } else {
          eng->log->addMsg("Something is blocking the door.");
        }
      }
    }
  }

  if(closable) {
    //Door is in correct state for closing (open, working, not blocked)

    if(TRYER_IS_BLIND == false) {
      isOpen_ = false;
      if(IS_PLAYER) {
        Sound snd("", sfxDoorClose, true, pos_, false, IS_PLAYER);
        eng->soundEmitter->emitSound(snd);
        eng->log->addMsg("I close the door.");
      } else {
        Sound snd("I hear a door closing.",
                  sfxDoorClose, true, pos_, false, IS_PLAYER);
        eng->soundEmitter->emitSound(snd);
        if(PLAYER_SEE_TRYER) {
          eng->log->addMsg(actorTrying->getNameThe() + " closes a door.");
        }
      }
    } else {
      if(eng->dice.percentile() < 50) {
        isOpen_ = false;
        if(IS_PLAYER) {
          Sound snd("", sfxDoorClose, true, pos_, false, IS_PLAYER);
          eng->soundEmitter->emitSound(snd);
          eng->log->addMsg("I fumble with a door and succeed to close it.");
        } else {
          Sound snd("I hear a door closing.",
                    sfxDoorClose, true, pos_, false, IS_PLAYER);
          eng->soundEmitter->emitSound(snd);
          if(PLAYER_SEE_TRYER) {
            eng->log->addMsg(actorTrying->getNameThe() +
                             "fumbles about and succeeds to close a door.");
          }
        }
      } else {
        if(IS_PLAYER) {
          eng->log->addMsg(
            "I fumble blindly with a door and fail to close it.");
        } else {
          if(PLAYER_SEE_TRYER) {
            eng->log->addMsg(actorTrying->getNameThe() +
                             " fumbles blindly and fails to close a door.");
          }
        }
      }
    }
  }

  if(isOpen_ == false && closable) {
    eng->gameTime->endTurnOfCurrentActor();
  }
}

void Door::tryOpen(Actor* actorTrying) {
  trace << "Door::tryOpen()" << endl;
  const bool IS_PLAYER = actorTrying == eng->player;
  const bool TRYER_IS_BLIND = actorTrying->getPropHandler()->allowSee() == false;
  const bool PLAYER_SEE_DOOR = eng->map->playerVision[pos_.x][pos_.y];
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(eng->player->pos, blockers);

  const bool PLAYER_SEE_TRYER = IS_PLAYER ? true : eng->player->checkIfSeeActor(*actorTrying, blockers);

  if(isOpenedAndClosedExternally_) {
    if(IS_PLAYER) {
      eng->log->addMsg("I see no way to open this door, perhaps it is opened elsewhere?");
      eng->renderer->drawMapAndInterface();
    }
    return;
  }

  if(isStuck_) {
    trace << "Door: Is stuck" << endl;

    if(IS_PLAYER) {
      eng->log->addMsg("The door seems to be stuck.");
    }

  } else {
    trace << "Door: Is not stuck" << endl;
    if(TRYER_IS_BLIND == false) {
      trace << "Door: Tryer can see, opening" << endl;
      isOpen_ = true;
      if(IS_PLAYER) {
        Sound snd("", sfxDoorOpen, true, pos_, false, IS_PLAYER);
        eng->soundEmitter->emitSound(snd);
        eng->log->addMsg("I open the door.");
      } else {
        Sound snd("I hear a door open.",
                  sfxDoorOpen, true, pos_, false, IS_PLAYER);
        eng->soundEmitter->emitSound(snd);
        if(PLAYER_SEE_TRYER) {
          eng->log->addMsg(actorTrying->getNameThe() + " opens a door.");
        } else if(PLAYER_SEE_DOOR) {
          eng->log->addMsg("I see a door opening.");
        }
      }
    } else {
      if(eng->dice.percentile() < 50) {
        trace << "Door: Tryer is blind, but open succeeded anyway" << endl;
        isOpen_ = true;
        if(IS_PLAYER) {
          Sound snd("", sfxDoorOpen, true, pos_, false, IS_PLAYER);
          eng->soundEmitter->emitSound(snd);
          eng->log->addMsg("I fumble with a door and succeed to open it.");
        } else {
          Sound snd("I hear something open a door clumsily.",
                    sfxDoorOpen, true, pos_, false, IS_PLAYER);
          eng->soundEmitter->emitSound(snd);
          if(PLAYER_SEE_TRYER) {
            eng->log->addMsg(actorTrying->getNameThe() +
                             "fumbles about and succeeds to open a door.");
          } else if(PLAYER_SEE_DOOR) {
            eng->log->addMsg("I see a door open clumsily.");
          }
        }
      } else {
        trace << "Door: Tryer is blind, and open failed" << endl;
        if(IS_PLAYER) {
          Sound snd("", endOfSfx, true, pos_, false, IS_PLAYER);
          eng->soundEmitter->emitSound(snd);
          eng->log->addMsg("I fumble blindly with a door and fail to open it.");
        } else {
          //(emitting the sound from the actor instead of the door here, beacause the sound should
          //be heard even if the door is seen, and the parameter for muting messages from seen sounds
          //should be off)
          Sound snd("I hear something attempting to open a door.",
                    endOfSfx, true, actorTrying->pos, false, IS_PLAYER);
          eng->soundEmitter->emitSound(snd);
          if(PLAYER_SEE_TRYER) {
            eng->log->addMsg(actorTrying->getNameThe() +
                             " fumbles blindly and fails to open a door.");
          }
        }
      }
    }
  }

  if(isOpen_) {
    trace << "Door: Open was successful" << endl;
    if(isSecret_) {
      trace << "Door: Was secret, now revealing" << endl;
      reveal(true);
    }
    trace << "Door: Calling GameTime::endTurnOfCurrentActor()" << endl;
    eng->gameTime->endTurnOfCurrentActor();
  }
}

bool Door::openFeature() {
  isOpen_ = true;
  return true;
}





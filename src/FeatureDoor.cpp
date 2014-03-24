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
#include "MapParsing.h"
#include "Utils.h"

//---------------------------------------------------INHERITED FUNCTIONS
Door::Door(FeatureId id, Pos pos, Engine& engine, DoorSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), mimicFeature_(spawnData->mimicFeature_),
  nrSpikes_(0) {

  isOpenedAndClosedExternally_ = false;

  const int ROLL = Rnd::percentile();
  const DoorSpawnState doorState =
    ROLL < 5 ? doorSpawnState_secretAndStuck :
    ROLL < 40 ? doorSpawnState_secret :
    ROLL < 50 ? doorSpawnState_stuck :
    ROLL < 60 ? doorSpawnState_broken :
    ROLL < 75 ? doorSpawnState_open :
    doorSpawnState_closed;

  switch(DoorSpawnState(doorState)) {
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

bool Door::canMoveCmn() const {
  return isOpen_;
}

bool Door::canMove(const vector<PropId>& actorsProps) const {
  if(isOpen_) return true;

  for(PropId propId : actorsProps) {
    if(propId == propEthereal || propId == propOoze) {
      return true;
    }
  }

  return isOpen_;
}

bool Door::isVisionPassable() const {
  return isOpen_;
}

bool Door::isProjectilePassable() const {
  return isOpen_;
}

bool Door::isSmokePassable() const {
  return isOpen_;
}

SDL_Color Door::getClr() const {
  if(isSecret_) {
    return mimicFeature_->color;
  }
  return material_ == doorMaterial_metal ? clrGray : clrBrownDrk;
}

char Door::getGlyph() const {
  return isSecret_ ? mimicFeature_->glyph : (isOpen_ ? 39 : '+');
}

TileId Door::getTile() const {
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

MaterialType Door::getMaterialType() const {
  return isSecret_ ? mimicFeature_->materialType : data_->materialType;
}

void Door::bump(Actor& actorBumping) {
  if(&actorBumping == eng.player) {
    if(isSecret_) {
      if(eng.map->cells[pos_.x][pos_.y].isSeenByPlayer) {
        trace << "Door: Player bumped into secret door, ";
        trace << "with vision in cell" << endl;
        eng.log->addMsg("That way is blocked.");
      } else {
        trace << "Door: Player bumped into secret door, ";
        trace << "without vision in cell" << endl;
        eng.log->addMsg("I bump into something.");
      }
      return;
    }

    if(isOpen_ == false) {
      tryOpen(&actorBumping);
    }
  }
}

string Door::getDescr(const bool DEFINITE_ARTICLE) const {
  if(isOpen_ && isBroken_ == false) {
    return DEFINITE_ARTICLE ? "the open door" : "an open door";
  }
  if(isBroken_) {
    return DEFINITE_ARTICLE ? "the broken door" : "a broken door";
  }
  if(isSecret_) {
    return (DEFINITE_ARTICLE ?
            mimicFeature_->name_the :
            mimicFeature_->name_a);
  }
  if(isOpen_ == false) {
    return DEFINITE_ARTICLE ? "the door" : "a door";
  }


  assert(false && "Failed to get door description");
  return "";
}
//----------------------------------------------------------------------

void Door::reveal(const bool ALLOW_MESSAGE) {
  if(isSecret_) {
    isSecret_ = false;
    if(eng.map->cells[pos_.x][pos_.y].isSeenByPlayer) {
      Renderer::drawMapAndInterface();
      if(ALLOW_MESSAGE) {
        eng.log->addMsg("A secret is revealed.");
        Renderer::drawMapAndInterface();
      }
    }
  }
}

void Door::playerTrySpotHidden() {
  if(isSecret_) {
    const int PLAYER_SKILL = eng.player->getData().abilityVals.getVal(
                               ability_searching, true, *(eng.player));
    if(eng.abilityRoll->roll(PLAYER_SKILL) >= successSmall) {
      reveal(true);
    }
  }
}

bool Door::trySpike(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == eng.player;
  const bool TRYER_IS_BLIND =
    actorTrying->getPropHandler().allowSee() == false;

  if(isSecret_ || isOpen_) {
    return false;
  }

  //Door is in correct state for spiking (known, closed)
  nrSpikes_++;
  isStuck_ = true;

  if(IS_PLAYER) {
    if(TRYER_IS_BLIND == false) {
      eng.log->addMsg("I jam the door with a spike.");
    } else {
      eng.log->addMsg("I jam a door with a spike.");
    }
  }
  eng.gameTime->actorDidAct();
  return true;
}

void Door::bash_(Actor& actorTrying) {
  trace << "Door::bash()..." << endl;

  if(isOpen_ == false) {

    const bool IS_PLAYER = &actorTrying == eng.player;

    int skillValueBash = 0;

    vector<PropId> props;
    actorTrying.getPropHandler().getAllActivePropIds(props);
    const bool IS_BASHER_WEAK =
      find(props.begin(), props.end(), propWeakened) != props.end();

    if(IS_BASHER_WEAK == false) {
      if(IS_PLAYER) {
        const int BON =
          eng.playerBonHandler->hasTrait(traitTough) ? 20 : 0;
        skillValueBash = 40 + BON - min(58, nrSpikes_ * 20);
      } else {
        skillValueBash = 10 - min(9, nrSpikes_ * 3);
      }
    }
    const bool IS_DOOR_SMASHED =
      (material_ == doorMaterial_metal || IS_BASHER_WEAK) ? false :
      Rnd::percentile() < skillValueBash;

    if(
      IS_PLAYER && isSecret_ == false &&
      (material_ == doorMaterial_metal || IS_BASHER_WEAK)) {
      eng.log->addMsg("It seems futile.");
    }

    if(IS_DOOR_SMASHED) {
      trace << "Door: Bash successful" << endl;
      isBroken_ = true;
      isStuck_ = false;
      const bool IS_SECRET_BEFORE = isSecret_;
      isSecret_ = false;
      isOpen_ = true;
      if(IS_PLAYER) {
        Snd snd("", SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_,
                &actorTrying, SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd, eng);
        if(actorTrying.getPropHandler().allowSee() == false) {
          eng.log->addMsg("I feel a door crashing open!");
        } else {
          if(IS_SECRET_BEFORE) {
            eng.log->addMsg("A door crashes open!");
          } else {
            eng.log->addMsg("The door crashes open!");
          }
        }
      } else {
        if(eng.player->isSeeingActor(actorTrying, NULL)) {
          eng.log->addMsg("The door crashes open!");
        } else if(eng.map->cells[pos_.x][pos_.y].isSeenByPlayer) {
          eng.log->addMsg("A door crashes open!");
        }
        Snd snd("I hear a door crashing open!",
                SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_, &actorTrying,
                SndVol::high, AlertsMonsters::no);
        SndEmit::emitSnd(snd, eng);
      }
    } else {
      if(IS_PLAYER) {
        const SfxId sfx = isSecret_ ? SfxId::endOfSfxId : SfxId::doorBang;
        Snd snd("", sfx, IgnoreMsgIfOriginSeen::yes, actorTrying.pos,
                &actorTrying, SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd, eng);
      } else {
        //Emitting the sound from the actor instead of the door, because the
        //sound message should be received even if the door is seen
        Snd snd("I hear a loud banging on a door.",
                SfxId::doorBang, IgnoreMsgIfOriginSeen::yes, actorTrying.pos,
                &actorTrying, SndVol::low, AlertsMonsters::no);
        SndEmit::emitSnd(snd, eng);
        if(eng.player->isSeeingActor(actorTrying, NULL)) {
          eng.log->addMsg(actorTrying.getNameThe() + " bashes at a door!");
        }
      }
    }
  }
  trace << "Door::bash() [DONE]" << endl;
}

void Door::tryClose(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == eng.player;
  const bool TRYER_IS_BLIND =
    actorTrying->getPropHandler().allowSee() == false;
  //const bool PLAYER_SEE_DOOR    = eng.map->playerVision[pos_.x][pos_.y];
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true :
    eng.player->isSeeingActor(*actorTrying, blockers);

  bool isClosable = true;

  if(isOpenedAndClosedExternally_) {
    if(IS_PLAYER) {
      eng.log->addMsg(
        "This door refuses to be closed, perhaps it is handled elsewhere?");
      Renderer::drawMapAndInterface();
    }
    return;
  }

  //Broken?
  if(isBroken_) {
    isClosable = false;
    if(IS_PLAYER) {
      if(IS_PLAYER)
        eng.log->addMsg("The door appears to be broken.");
    }
  }

  //Already closed?
  if(isClosable) {
    if(isOpen_ == false) {
      isClosable = false;
      if(IS_PLAYER) {
        if(TRYER_IS_BLIND == false)
          eng.log->addMsg("I see nothing there to close.");
        else eng.log->addMsg("I find nothing there to close.");
      }
    }
  }

  //Blocked?
  if(isClosable) {
    bool isblockedByActor = false;
    for(Actor * actor : eng.gameTime->actors_) {
      if(actor->pos == pos_) {
        isblockedByActor = true;
        break;
      }
    }
    if(isblockedByActor || eng.map->cells[pos_.x][pos_.y].item != NULL) {
      isClosable = false;
      if(IS_PLAYER) {
        if(TRYER_IS_BLIND == false) {
          eng.log->addMsg("The door is blocked.");
        } else {
          eng.log->addMsg("Something is blocking the door.");
        }
      }
    }
  }

  if(isClosable) {
    //Door is in correct state for closing (open, working, not blocked)

    if(TRYER_IS_BLIND == false) {
      isOpen_ = false;
      if(IS_PLAYER) {
        Snd snd("", SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_,
                actorTrying, SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd, eng);
        eng.log->addMsg("I close the door.");
      } else {
        Snd snd("I hear a door closing.",
                SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_, actorTrying,
                SndVol::low, AlertsMonsters::no);
        SndEmit::emitSnd(snd, eng);
        if(PLAYER_SEE_TRYER) {
          eng.log->addMsg(actorTrying->getNameThe() + " closes a door.");
        }
      }
    } else {
      if(Rnd::percentile() < 50) {
        isOpen_ = false;
        if(IS_PLAYER) {
          Snd snd("", SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd, eng);
          eng.log->addMsg("I fumble with a door and succeed to close it.");
        } else {
          Snd snd("I hear a door closing.",
                  SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_, actorTrying,
                  SndVol::low, AlertsMonsters::no);
          SndEmit::emitSnd(snd, eng);
          if(PLAYER_SEE_TRYER) {
            eng.log->addMsg(actorTrying->getNameThe() +
                            "fumbles about and succeeds to close a door.");
          }
        }
      } else {
        if(IS_PLAYER) {
          eng.log->addMsg(
            "I fumble blindly with a door and fail to close it.");
        } else {
          if(PLAYER_SEE_TRYER) {
            eng.log->addMsg(actorTrying->getNameThe() +
                            " fumbles blindly and fails to close a door.");
          }
        }
      }
    }
  }

  if(isOpen_ == false && isClosable) {
    eng.gameTime->actorDidAct();
  }
}

void Door::tryOpen(Actor* actorTrying) {
  trace << "Door::tryOpen()" << endl;
  const bool IS_PLAYER = actorTrying == eng.player;
  const bool TRYER_IS_BLIND =
    actorTrying->getPropHandler().allowSee() == false;
  const bool PLAYER_SEE_DOOR = eng.map->cells[pos_.x][pos_.y].isSeenByPlayer;
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true : eng.player->isSeeingActor(*actorTrying, blockers);

  if(isOpenedAndClosedExternally_) {
    if(IS_PLAYER) {
      eng.log->addMsg(
        "I see no way to open this door, perhaps it is opened elsewhere?");
      Renderer::drawMapAndInterface();
    }
    return;
  }

  if(isStuck_) {
    trace << "Door: Is stuck" << endl;

    if(IS_PLAYER) {
      eng.log->addMsg("The door seems to be stuck.");
    }

  } else {
    trace << "Door: Is not stuck" << endl;
    if(TRYER_IS_BLIND == false) {
      trace << "Door: Tryer can see, opening" << endl;
      isOpen_ = true;
      if(IS_PLAYER) {
        Snd snd("", SfxId::doorOpen, IgnoreMsgIfOriginSeen::yes, pos_, actorTrying,
                SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd, eng);
        eng.log->addMsg("I open the door.");
      } else {
        Snd snd("I hear a door open.", SfxId::doorOpen, IgnoreMsgIfOriginSeen::yes,
                pos_, actorTrying, SndVol::low, AlertsMonsters::no);
        SndEmit::emitSnd(snd, eng);
        if(PLAYER_SEE_TRYER) {
          eng.log->addMsg(actorTrying->getNameThe() + " opens a door.");
        } else if(PLAYER_SEE_DOOR) {
          eng.log->addMsg("I see a door opening.");
        }
      }
    } else {
      if(Rnd::percentile() < 50) {
        trace << "Door: Tryer is blind, but open succeeded anyway" << endl;
        isOpen_ = true;
        if(IS_PLAYER) {
          Snd snd("", SfxId::doorOpen, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd, eng);
          eng.log->addMsg("I fumble with a door and succeed to open it.");
        } else {
          Snd snd("I hear something open a door clumsily.", SfxId::doorOpen,
                  IgnoreMsgIfOriginSeen::yes, pos_, actorTrying, SndVol::low,
                  AlertsMonsters::no);
          SndEmit::emitSnd(snd, eng);
          if(PLAYER_SEE_TRYER) {
            eng.log->addMsg(actorTrying->getNameThe() +
                            "fumbles about and succeeds to open a door.");
          } else if(PLAYER_SEE_DOOR) {
            eng.log->addMsg("I see a door open clumsily.");
          }
        }
      } else {
        trace << "Door: Tryer is blind, and open failed" << endl;
        if(IS_PLAYER) {
          Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd, eng);
          eng.log->addMsg("I fumble blindly with a door and fail to open it.");
        } else {
          //Emitting the sound from the actor instead of the door, because the
          //sound message should be received even if the door is seen
          Snd snd("I hear something attempting to open a door.", SfxId::endOfSfxId,
                  IgnoreMsgIfOriginSeen::yes, actorTrying->pos, actorTrying,
                  SndVol::low, AlertsMonsters::no);
          SndEmit::emitSnd(snd, eng);
          if(PLAYER_SEE_TRYER) {
            eng.log->addMsg(actorTrying->getNameThe() +
                            " fumbles blindly and fails to open a door.");
          }
        }
        eng.gameTime->actorDidAct();
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
    eng.gameTime->actorDidAct();
  }
}

bool Door::open() {
  isOpen_   = true;
  isSecret_ = false;
  isStuck_  = false;
  return true;
}

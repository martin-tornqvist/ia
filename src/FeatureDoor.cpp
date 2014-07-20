#include "FeatureDoor.h"

#include "Init.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "FeatureData.h"
#include "Map.h"
#include "Log.h"
#include "Postmortem.h"
#include "PlayerBon.h"
#include "Renderer.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

//---------------------------------------------------INHERITED FUNCTIONS
Door::Door(const Pos& pos, const FeatureDataT& mimicFeature) :
  FeatureStatic(pos), mimicFeature_(&mimicFeature), nrSpikes_(0) {

  isHandledExternally_ = false;

  const int ROLL = Rnd::percentile();
  const DoorSpawnState doorState =
    ROLL < 5 ? DoorSpawnState::secretAndStuck :
    ROLL < 40 ? DoorSpawnState::secret :
    ROLL < 50 ? DoorSpawnState::stuck :
    ROLL < 75 ? DoorSpawnState::open :
    DoorSpawnState::closed;

  switch(DoorSpawnState(doorState)) {
    case DoorSpawnState::open: {
      isOpen_   = true;
      isStuck_  = false;
      isSecret_ = false;
    } break;

    case DoorSpawnState::closed: {
      isOpen_   = false;
      isStuck_  = false;
      isSecret_ = false;
    } break;

    case DoorSpawnState::stuck: {
      isOpen_   = false;
      isStuck_  = true;
      isSecret_ = false;
    } break;

    case DoorSpawnState::secret: {
      isOpen_   = false;
      isStuck_  = false;
      isSecret_ = true;
    } break;

    case DoorSpawnState::secretAndStuck: {
      isOpen_   = false;
      isStuck_  = true;
      isSecret_ = true;
    } break;

  }

  type_ = DoorType::wood;

  onHit[int(DmgType::physical)][int(DmgMethod::forced)] = [&](Actor * const actor) {
    (void)actor;
    Map::put(new RubbleLow(pos_));
  };

  onHit[int(DmgType::physical)][int(DmgMethod::shotgun)] = [&](Actor * const actor) {
    (void)actor;
    if(!isOpen_) {
      switch(type_) {
        case DoorType::wood: {
          if(Rnd::fraction(7, 10)) {
            if(Map::canPlayerSeePos(pos_)) {
              const string a = isSecret_ ? "A" : "The";
              Log::addMsg(a + " door is blown to splinters!");
            }
            Map::put(new RubbleLow(pos_));
          }
        } break;

        case DoorType::metal: {} break;
      }
    }
  };

  onHit[int(DmgType::physical)][int(DmgMethod::explosion)] = [&](Actor * const actor) {
    (void)actor;
    //TODO
  };

  onHit[int(DmgType::physical)][int(DmgMethod::bluntHeavy)] = [&](Actor * const actor) {
    assert(actor);
    switch(type_) {
      case DoorType::wood: {
        Fraction destrChance(6, 10);
        if(actor == Map::player) {
          if(PlayerBon::hasTrait(Trait::tough))   {destrChance.numerator += 2;}
          if(PlayerBon::hasTrait(Trait::rugged))  {destrChance.numerator += 2;}

          if(Rnd::fraction(destrChance)) {

          }
        } else {
          if(Rnd::fraction(destrChance)) {

          }
        }
      } break;

      case DoorType::metal: {} break;
    }
  };

  onHit[int(DmgType::physical)][int(DmgMethod::kick)] = [&](Actor * const actor) {
    assert(actor);

    const bool IS_PLAYER    = actor == Map::player;
    const bool IS_CELL_SEEN = Map::canPlayerSeePos(pos_);

    vector<PropId> props;
    if(actor) {actor->getPropHandler().getAllActivePropIds(props);}

    const bool IS_WEAK = find(begin(props), end(props), propWeakened) != end(props);

    switch(type_) {
      case DoorType::wood: {
        if(IS_PLAYER) {
          Fraction destrChance(4 - nrSpikes_, 10);
          destrChance.numerator = max(1, destrChance.numerator);

          if(PlayerBon::hasTrait(Trait::tough))   {destrChance.numerator += 2;}
          if(PlayerBon::hasTrait(Trait::rugged))  {destrChance.numerator += 2;}

          if(IS_WEAK) {destrChance.numerator = 0;}

          if(destrChance.numerator > 0) {
            if(Rnd::fraction(destrChance)) {
              Snd snd("", SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_,
                      actor, SndVol::low, AlertsMonsters::yes);
              SndEmit::emitSnd(snd);
              if(IS_CELL_SEEN) {
                if(isSecret_) {
                  Log::addMsg("A door crashes open!");
                } else {
                  Log::addMsg("The door crashes open!");
                }
              } else {
                Log::addMsg("I feel a door crashing open!");
              }
              Map::put(new RubbleLow(pos_));
            }
          } else {
            if(IS_CELL_SEEN && !isSecret_) {Log::addMsg("It seems futile.");}
          }
        } else { //Not player
          Fraction destrChance(10 - (nrSpikes_ * 3), 100);
          destrChance.numerator = max(1, destrChance.numerator);

          if(IS_WEAK) {destrChance.numerator = 0;}

          if(Rnd::fraction(destrChance)) {
            Snd snd("I hear a door crashing open!",
                    SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_, actor,
                    SndVol::high, AlertsMonsters::no);
            SndEmit::emitSnd(snd);
            if(Map::player->isSeeingActor(*actor, nullptr)) {
              Log::addMsg("The door crashes open!");
            } else if(IS_CELL_SEEN) {
              Log::addMsg("A door crashes open!");
            }
            Map::put(new RubbleLow(pos_));
          }
        }

      } break;

      case DoorType::metal: {
        if(IS_PLAYER && IS_CELL_SEEN && !isSecret_) {
          Log::addMsg("It seems futile.");
        }
      } break;
    }
  };

  //-------------------------------------- OLD

//    if(method == DmgMethod::kick) {
//      int skillValueBash = 0;
//
//      if(!IS_BASHER_WEAK) {
//        if(IS_PLAYER) {
//          const int BON   = PlayerBon::hasTrait(Trait::tough) ? 20 : 0;
//          skillValueBash  = 40 + BON - min(58, nrSpikes_ * 20);
//        } else {
//          skillValueBash  = 10 - min(9, nrSpikes_ * 3);
//        }
//      }
//      const bool IS_DOOR_SMASHED =
//        (type_ == DoorType::metal || IS_BASHER_WEAK) ? false :
//        Rnd::percentile() < skillValueBash;
//
//      if(IS_PLAYER && !isSecret_ && (type_ == DoorType::metal || IS_BASHER_WEAK)) {
//        Log::addMsg("It seems futile.");
//      }
//    }
//
//    if(IS_DOOR_SMASHED) {
//      TRACE << "Door: Bash successful" << endl;
//      const bool IS_SECRET_BEFORE = isSecret_;
//      isStuck_  = false;
//      isSecret_ = false;
//      isOpen_   = true;
//      if(IS_PLAYER) {
//        Snd snd("", SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_,
//                &actor, SndVol::low, AlertsMonsters::yes);
//        SndEmit::emitSnd(snd);
//        if(!actor.getPropHandler().allowSee()) {
//          Log::addMsg("I feel a door crashing open!");
//        } else {
//          if(IS_SECRET_BEFORE) {
//            Log::addMsg("A door crashes open!");
//          } else {
//            Log::addMsg("The door crashes open!");
//          }
//        }
//      } else {
//        Snd snd("I hear a door crashing open!",
//                SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_, &actor,
//                SndVol::high, AlertsMonsters::no);
//        SndEmit::emitSnd(snd);
//        if(Map::player->isSeeingActor(actor, nullptr)) {
//          Log::addMsg("The door crashes open!");
//        } else if(Map::cells[pos_.x][pos_.y].isSeenByPlayer) {
//          Log::addMsg("A door crashes open!");
//        }
//      }
//    } else {
//      if(IS_PLAYER) {
//        const SfxId sfx = isSecret_ ? SfxId::END : SfxId::doorBang;
//        Snd snd("", sfx, IgnoreMsgIfOriginSeen::yes, actor.pos,
//                &actor, SndVol::low, AlertsMonsters::yes);
//        SndEmit::emitSnd(snd);
//      } else {
//        //Emitting the sound from the actor instead of the door, because the
//        //sound message should be received even if the door is seen
//        Snd snd("I hear a loud banging on a door.",
//                SfxId::doorBang, IgnoreMsgIfOriginSeen::yes, actor.pos,
//                &actor, SndVol::low, AlertsMonsters::no);
//        SndEmit::emitSnd(snd);
//        if(Map::player->isSeeingActor(actor, nullptr)) {
//          Log::addMsg(actor.getNameThe() + " bashes at a door!");
//        }
//      }
//    }
//  }
}

bool Door::canMoveCmn() const {return isOpen_;}

bool Door::canMove(const vector<PropId>& actorsProps) const {
  if(isOpen_) {return true;}

  for(PropId propId : actorsProps) {
    if(propId == propEthereal || propId == propOoze) {return true;}
  }

  return isOpen_;
}

bool Door::isVisionPassable()     const {return isOpen_;}
bool Door::isProjectilePassable() const {return isOpen_;}
bool Door::isSmokePassable()      const {return isOpen_;}

Clr Door::getClr() const {
  if(isSecret_) {
    return mimicFeature_->clr;
  } else {
    switch(type_) {
      case DoorType::wood:  return clrBrownDrk; break;
      case DoorType::metal: return clrGray;     break;
    }
  }
  assert(false && "Failed to get door color");
  return clrGray;
}

char Door::getGlyph() const {
  return isSecret_ ? mimicFeature_->glyph : (isOpen_ ? 39 : '+');
}

TileId Door::getTile() const {
  return isSecret_ ? mimicFeature_->tile :
         (isOpen_ ? TileId::doorOpen : TileId::doorClosed);
}

Matl Door::getMatl() const {
  return isSecret_ ? mimicFeature_->matlType : Matl::hard;
}

void Door::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {
    if(isSecret_) {
      if(Map::cells[pos_.x][pos_.y].isSeenByPlayer) {
        TRACE << "Door: Player bumped into secret door, ";
        TRACE << "with vision in cell" << endl;
        Log::addMsg("That way is blocked.");
      } else {
        TRACE << "Door: Player bumped into secret door, ";
        TRACE << "without vision in cell" << endl;
        Log::addMsg("I bump into something.");
      }
      return;
    }

    if(!isOpen_) {tryOpen(&actorBumping);}
  }
}

string Door::getDescr(const bool DEFINITE_ARTICLE) const {
  if(isSecret_) {
    return (DEFINITE_ARTICLE ? mimicFeature_->nameThe : mimicFeature_->nameA);
  }
  if(isOpen_) {
    return DEFINITE_ARTICLE ? "the open door" : "an open door";
  } else {
    return DEFINITE_ARTICLE ? "the door" : "a door";
  }
}
//----------------------------------------------------------------------

void Door::reveal(const bool ALLOW_MESSAGE) {
  if(isSecret_) {
    isSecret_ = false;
    if(Map::cells[pos_.x][pos_.y].isSeenByPlayer) {
      Renderer::drawMapAndInterface();
      if(ALLOW_MESSAGE) {
        Log::addMsg("A secret is revealed.");
        Renderer::drawMapAndInterface();
      }
    }
  }
}

void Door::playerTrySpotHidden() {
  if(isSecret_) {
    const int PLAYER_SKILL = Map::player->getData().abilityVals.getVal(
                               AbilityId::searching, true, *(Map::player));
    if(AbilityRoll::roll(PLAYER_SKILL) >= successSmall) {reveal(true);}
  }
}

bool Door::trySpike(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == Map::player;
  const bool TRYER_IS_BLIND = !actorTrying->getPropHandler().allowSee();

  if(isSecret_ || isOpen_) {return false;}

  //Door is in correct state for spiking (known, closed)
  nrSpikes_++;
  isStuck_ = true;

  if(IS_PLAYER) {
    if(!TRYER_IS_BLIND) {
      Log::addMsg("I jam the door with a spike.");
    } else {
      Log::addMsg("I jam a door with a spike.");
    }
  }
  GameTime::actorDidAct();
  return true;
}

void Door::tryClose(Actor* actorTrying) {
  const bool IS_PLAYER = actorTrying == Map::player;
  const bool TRYER_IS_BLIND = !actorTrying->getPropHandler().allowSee();
  //const bool PLAYER_SEE_DOOR    = Map::playerVision[pos_.x][pos_.y];
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blocked);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true :
    Map::player->isSeeingActor(*actorTrying, blocked);

  bool isClosable = true;

  if(isHandledExternally_) {
    if(IS_PLAYER) {
      Log::addMsg(
        "This door refuses to be closed, perhaps it is handled elsewhere?");
      Renderer::drawMapAndInterface();
    }
    return;
  }

  //Already closed?
  if(isClosable) {
    if(!isOpen_) {
      isClosable = false;
      if(IS_PLAYER) {
        if(!TRYER_IS_BLIND)
          Log::addMsg("I see nothing there to close.");
        else Log::addMsg("I find nothing there to close.");
      }
    }
  }

  //Blocked?
  if(isClosable) {
    bool isblockedByActor = false;
    for(Actor* actor : GameTime::actors_) {
      if(actor->pos == pos_) {
        isblockedByActor = true;
        break;
      }
    }
    if(isblockedByActor || Map::cells[pos_.x][pos_.y].item) {
      isClosable = false;
      if(IS_PLAYER) {
        if(!TRYER_IS_BLIND) {
          Log::addMsg("The door is blocked.");
        } else {
          Log::addMsg("Something is blocking the door.");
        }
      }
    }
  }

  if(isClosable) {
    //Door is in correct state for closing (open, working, not blocked)

    if(!TRYER_IS_BLIND) {
      isOpen_ = false;
      if(IS_PLAYER) {
        Snd snd("", SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_,
                actorTrying, SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd);
        Log::addMsg("I close the door.");
      } else {
        Snd snd("I hear a door closing.",
                SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_, actorTrying,
                SndVol::low, AlertsMonsters::no);
        SndEmit::emitSnd(snd);
        if(PLAYER_SEE_TRYER) {
          Log::addMsg(actorTrying->getNameThe() + " closes a door.");
        }
      }
    } else {
      if(Rnd::percentile() < 50) {
        isOpen_ = false;
        if(IS_PLAYER) {
          Snd snd("", SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
          Log::addMsg("I fumble with a door and succeed to close it.");
        } else {
          Snd snd("I hear a door closing.",
                  SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_, actorTrying,
                  SndVol::low, AlertsMonsters::no);
          SndEmit::emitSnd(snd);
          if(PLAYER_SEE_TRYER) {
            Log::addMsg(actorTrying->getNameThe() +
                        "fumbles about and succeeds to close a door.");
          }
        }
      } else {
        if(IS_PLAYER) {
          Log::addMsg(
            "I fumble blindly with a door and fail to close it.");
        } else {
          if(PLAYER_SEE_TRYER) {
            Log::addMsg(actorTrying->getNameThe() +
                        " fumbles blindly and fails to close a door.");
          }
        }
      }
    }
  }

  if(!isOpen_ && isClosable) {GameTime::actorDidAct();}
}

void Door::tryOpen(Actor* actorTrying) {
  TRACE << "Door::tryOpen()" << endl;
  const bool IS_PLAYER        = actorTrying == Map::player;
  const bool TRYER_IS_BLIND   = !actorTrying->getPropHandler().allowSee();
  const bool PLAYER_SEE_DOOR  = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blocked);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true : Map::player->isSeeingActor(*actorTrying, blocked);

  if(isHandledExternally_) {
    if(IS_PLAYER) {
      Log::addMsg(
        "I see no way to open this door, perhaps it is opened elsewhere?");
      Renderer::drawMapAndInterface();
    }
    return;
  }

  if(isStuck_) {
    TRACE << "Door: Is stuck" << endl;

    if(IS_PLAYER) {
      Log::addMsg("The door seems to be stuck.");
    }

  } else {
    TRACE << "Door: Is not stuck" << endl;
    if(!TRYER_IS_BLIND) {
      TRACE << "Door: Tryer can see, opening" << endl;
      isOpen_ = true;
      if(IS_PLAYER) {
        Snd snd("", SfxId::doorOpen, IgnoreMsgIfOriginSeen::yes, pos_,
                actorTrying, SndVol::low, AlertsMonsters::yes);
        SndEmit::emitSnd(snd);
        Log::addMsg("I open the door.");
      } else {
        Snd snd("I hear a door open.", SfxId::doorOpen,
                IgnoreMsgIfOriginSeen::yes, pos_, actorTrying, SndVol::low,
                AlertsMonsters::no);
        SndEmit::emitSnd(snd);
        if(PLAYER_SEE_TRYER) {
          Log::addMsg(actorTrying->getNameThe() + " opens a door.");
        } else if(PLAYER_SEE_DOOR) {
          Log::addMsg("I see a door opening.");
        }
      }
    } else {
      if(Rnd::percentile() < 50) {
        TRACE << "Door: Tryer is blind, but open succeeded anyway" << endl;
        isOpen_ = true;
        if(IS_PLAYER) {
          Snd snd("", SfxId::doorOpen, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
          Log::addMsg("I fumble with a door and succeed to open it.");
        } else {
          Snd snd("I hear something open a door clumsily.", SfxId::doorOpen,
                  IgnoreMsgIfOriginSeen::yes, pos_, actorTrying, SndVol::low,
                  AlertsMonsters::no);
          SndEmit::emitSnd(snd);
          if(PLAYER_SEE_TRYER) {
            Log::addMsg(actorTrying->getNameThe() +
                        "fumbles about and succeeds to open a door.");
          } else if(PLAYER_SEE_DOOR) {
            Log::addMsg("I see a door open clumsily.");
          }
        }
      } else {
        TRACE << "Door: Tryer is blind, and open failed" << endl;
        if(IS_PLAYER) {
          Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMonsters::yes);
          SndEmit::emitSnd(snd);
          Log::addMsg("I fumble blindly with a door and fail to open it.");
        } else {
          //Emitting the sound from the actor instead of the door, because the
          //sound message should be received even if the door is seen
          Snd snd("I hear something attempting to open a door.", SfxId::END,
                  IgnoreMsgIfOriginSeen::yes, actorTrying->pos, actorTrying,
                  SndVol::low, AlertsMonsters::no);
          SndEmit::emitSnd(snd);
          if(PLAYER_SEE_TRYER) {
            Log::addMsg(actorTrying->getNameThe() +
                        " fumbles blindly and fails to open a door.");
          }
        }
        GameTime::actorDidAct();
      }
    }
  }

  if(isOpen_) {
    TRACE << "Door: Open was successful" << endl;
    if(isSecret_) {
      TRACE << "Door: Was secret, now revealing" << endl;
      reveal(true);
    }
    TRACE << "Door: Calling GameTime::endTurnOfCurActor()" << endl;
    GameTime::actorDidAct();
  }
}

bool Door::open() {
  isOpen_   = true;
  isSecret_ = false;
  isStuck_  = false;
  return true;
}

#include "FeatureDoor.h"

#include "Init.h"
#include "Actor.h"
#include "ActorPlayer.h"
#include "FeatureData.h"
#include "Map.h"
#include "Log.h"
#include "Postmortem.h"
#include "PlayerBon.h"
#include "Render.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

//---------------------------------------------------INHERITED FUNCTIONS
Door::Door(const Pos& pos, const Rigid* const mimicFeature, DoorSpawnState spawnState) :
  Rigid         (pos),
  mimicFeature_ (mimicFeature),
  nrSpikes_     (0)
{

  isHandledExternally_ = false;

  const int ROLL = Rnd::percentile();

  const DoorSpawnState doorState = spawnState == DoorSpawnState::any ?
                                   (ROLL < 5  ? DoorSpawnState::secretAndStuck  :
                                    ROLL < 40 ? DoorSpawnState::secret          :
                                    ROLL < 50 ? DoorSpawnState::stuck           :
                                    ROLL < 75 ? DoorSpawnState::open            :
                                    DoorSpawnState::closed)                     :
                                   spawnState;

  switch (DoorSpawnState(doorState))
  {
    case DoorSpawnState::open:
    {
      isOpen_   = true;
      isStuck_  = false;
      isSecret_ = false;
    } break;

    case DoorSpawnState::closed:
    {
      isOpen_   = false;
      isStuck_  = false;
      isSecret_ = false;
    } break;

    case DoorSpawnState::stuck:
    {
      isOpen_   = false;
      isStuck_  = true;
      isSecret_ = false;
    } break;

    case DoorSpawnState::secret:
    {
      isOpen_   = false;
      isStuck_  = false;
      isSecret_ = true;
    } break;

    case DoorSpawnState::secretAndStuck:
    {
      isOpen_   = false;
      isStuck_  = true;
      isSecret_ = true;
    } break;

    case DoorSpawnState::any:
    {
      assert(false && "Should not happen");
      isOpen_   = false;
      isStuck_  = false;
      isSecret_ = false;
    }
  }

  matl_ = Matl::wood;
}

Door::~Door()
{
  if (mimicFeature_) {delete mimicFeature_;}
}

void Door::onHit(const DmgType dmgType, const DmgMethod dmgMethod, Actor* const actor)
{
  if (dmgType == DmgType::physical)
  {
    if (dmgMethod == DmgMethod::forced)
    {
      //---------------------------------------------------------------------- FORCED
      (void)actor;
      Map::put(new RubbleLow(pos_));
    }

    //---------------------------------------------------------------------- SHOTGUN
    if (dmgMethod == DmgMethod::shotgun)
    {
      (void)actor;
      if (!isOpen_)
      {
        switch (matl_)
        {
          case Matl::wood:
          {
            if (Rnd::fraction(7, 10))
            {
              if (Map::isPosSeenByPlayer(pos_))
              {
                const string a = isSecret_ ? "A" : "The";
                Log::addMsg(a + " door is blown to splinters!");
              }
              Map::put(new RubbleLow(pos_));
            }
          } break;

          case Matl::empty:
          case Matl::cloth:
          case Matl::fluid:
          case Matl::plant:
          case Matl::stone:
          case Matl::metal: {} break;
        }
      }
    }

    //---------------------------------------------------------------------- EXPLOSION
    if (dmgMethod == DmgMethod::explosion)
    {
      (void)actor;
      //TODO
    }

    //---------------------------------------------------------------------- HEAVY BLUNT
    if (dmgMethod == DmgMethod::bluntHeavy)
    {
      assert(actor);
      switch (matl_)
      {
        case Matl::wood:
        {
          Fraction destrChance(6, 10);
          if (actor == Map::player)
          {
            if (PlayerBon::traitsPicked[int(Trait::tough)])
            {
              destrChance.numerator += 2;
            }
            if (PlayerBon::traitsPicked[int(Trait::rugged)])
            {
              destrChance.numerator += 2;
            }
            if (PlayerBon::traitsPicked[int(Trait::unbreakable)])
            {
              destrChance.numerator += 2;
            }

            if (Rnd::fraction(destrChance))
            {

            }
          }
          else
          {
            if (Rnd::fraction(destrChance))
            {

            }
          }
        } break;

        case Matl::empty:
        case Matl::cloth:
        case Matl::fluid:
        case Matl::plant:
        case Matl::stone:
        case Matl::metal: {} break;
      }
    }

    //---------------------------------------------------------------------- KICK
    if (dmgMethod == DmgMethod::kick)
    {
      assert(actor);

      const bool IS_PLAYER    = actor == Map::player;
      const bool IS_CELL_SEEN = Map::isPosSeenByPlayer(pos_);

      bool props[endOfPropIds];
      if (actor) {actor->getPropHandler().getPropIds(props);}

      const bool IS_WEAK = props[propWeakened];

      switch (matl_)
      {
        case Matl::wood:
        {
          if (IS_PLAYER)
          {
            Fraction destrChance(4 - nrSpikes_, 10);
            destrChance.numerator = max(1, destrChance.numerator);

            if (PlayerBon::traitsPicked[int(Trait::tough)])
            {
              destrChance.numerator += 2;
            }
            if (PlayerBon::traitsPicked[int(Trait::rugged)])
            {
              destrChance.numerator += 2;
            }
            if (PlayerBon::traitsPicked[int(Trait::unbreakable)])
            {
              destrChance.numerator += 2;
            }

            if (IS_WEAK) {destrChance.numerator = 0;}

            if (destrChance.numerator > 0)
            {
              if (Rnd::fraction(destrChance))
              {
                Snd snd("", SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_,
                        actor, SndVol::low, AlertsMon::yes);
                SndEmit::emitSnd(snd);
                if (IS_CELL_SEEN)
                {
                  if (isSecret_)
                  {
                    Log::addMsg("A door crashes open!");
                  }
                  else
                  {
                    Log::addMsg("The door crashes open!");
                  }
                }
                else
                {
                  Log::addMsg("I feel a door crashing open!");
                }
                Map::put(new RubbleLow(pos_));
              }
              else //Not broken
              {
                const SfxId sfx = isSecret_ ? SfxId::END : SfxId::doorBang;
                SndEmit::emitSnd({"", sfx, IgnoreMsgIfOriginSeen::no, pos_, actor,
                                  SndVol::low, AlertsMon::yes
                                 });
              }
            }
            else //No chance of success
            {
              if (IS_CELL_SEEN && !isSecret_)
              {
                SndEmit::emitSnd({"", SfxId::doorBang, IgnoreMsgIfOriginSeen::no, pos_,
                                  actor, SndVol::low, AlertsMon::yes
                                 });
                Log::addMsg("It seems futile.", clrMsgNote, false, true);
              }
            }
          }
          else //Not player
          {
            Fraction destrChance(10 - (nrSpikes_ * 3), 100);
            destrChance.numerator = max(1, destrChance.numerator);

            if (IS_WEAK) {destrChance.numerator = 0;}

            if (Rnd::fraction(destrChance))
            {
              Snd snd("I hear a door crashing open!",
                      SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_, actor,
                      SndVol::high, AlertsMon::no);
              SndEmit::emitSnd(snd);
              if (Map::player->isSeeingActor(*actor, nullptr))
              {
                Log::addMsg("The door crashes open!");
              }
              else if (IS_CELL_SEEN)
              {
                Log::addMsg("A door crashes open!");
              }
              Map::put(new RubbleLow(pos_));
            }
            else //Not broken
            {
              Snd snd("I hear a loud banging on a door.",
                      SfxId::doorBang, IgnoreMsgIfOriginSeen::no, pos_,
                      actor, SndVol::low, AlertsMon::no);
              SndEmit::emitSnd(snd);
            }
          }

        } break;

        case Matl::metal:
        {
          if (IS_PLAYER && IS_CELL_SEEN && !isSecret_)
          {
            Log::addMsg("It seems futile.", clrMsgNote, false, true);
          }
        } break;

        case Matl::empty:
        case Matl::cloth:
        case Matl::fluid:
        case Matl::plant:
        case Matl::stone: {} break;
      }
    }
  }

  //---------------------------------------------------------------------- FIRE
  if (dmgMethod == DmgMethod::elemental)
  {
    if (dmgType == DmgType::fire)
    {
      (void)actor;
      if (matl_ == Matl::wood)
      {
        tryStartBurning(true);
      }
    }
  }

  //-------------------------------------- OLD

//    if(method == DmgMethod::kick) {
//      int skillValueBash = 0;
//
//      if(!IS_BASHER_WEAK) {
//        if(IS_PLAYER) {
//          const int BON   = PlayerBon::traitsPicked[int(Trait::tough)] ? 20 : 0;
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
//      TRACE << "Bash successful" << endl;
//      const bool IS_SECRET_BEFORE = isSecret_;
//      isStuck_  = false;
//      isSecret_ = false;
//      isOpen_   = true;
//      if(IS_PLAYER) {
//        Snd snd("", SfxId::doorBreak, IgnoreMsgIfOriginSeen::yes, pos_,
//                &actor, SndVol::low, AlertsMon::yes);
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
//                SndVol::high, AlertsMon::no);
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
//                &actor, SndVol::low, AlertsMon::yes);
//        SndEmit::emitSnd(snd);
//      } else {
//        //Emitting the sound from the actor instead of the door, because the
//        //sound message should be received even if the door is seen
//        Snd snd("I hear a loud banging on a door.",
//                SfxId::doorBang, IgnoreMsgIfOriginSeen::yes, actor.pos,
//                &actor, SndVol::low, AlertsMon::no);
//        SndEmit::emitSnd(snd);
//        if(Map::player->isSeeingActor(actor, nullptr)) {
//          Log::addMsg(actor.getNameThe() + " bashes at a door!");
//        }
//      }
//    }
//  }
}

WasDestroyed Door::onFinishedBurning()
{
  if (Map::isPosSeenByPlayer(pos_))
  {
    Log::addMsg("The door burns down.");
  }
  RubbleLow* const rubble = new RubbleLow(pos_);
  rubble->setHasBurned();
  Map::put(rubble);
  return WasDestroyed::yes;
}

bool Door::canMoveCmn() const {return isOpen_;}

bool Door::canMove(const bool actorPropIds[endOfPropIds]) const
{
  if (isOpen_)
  {
    return true;
  }

  if (actorPropIds[propEthereal] || actorPropIds[propOoze])
  {
    return true;
  }

  return isOpen_;
}

bool Door::isLosPassable()     const {return isOpen_;}
bool Door::isProjectilePassable() const {return isOpen_;}
bool Door::isSmokePassable()      const {return isOpen_;}

string Door::getName(const Article article) const
{
  if (isSecret_) {return mimicFeature_->getName(article);}

  string ret = "";

  if (getBurnState() == BurnState::burning)
  {
    ret = article == Article::a ? "a " : "the ";
    ret += "burning ";
  }
  else
  {
    ret = article == Article::a ? (isOpen_ ? "an " : "a ") : "the ";
  }

  ret += isOpen_ ? "open " : "closed ";
  ret += matl_ == Matl::wood ? "wooden " : "metal ";

  return ret + "door";
}

Clr Door::getClr_() const
{
  if (isSecret_)
  {
    return mimicFeature_->getClr();
  }
  else
  {
    switch (matl_)
    {
      case Matl::wood:      return clrBrownDrk; break;
      case Matl::metal:     return clrGray;     break;
      case Matl::empty:
      case Matl::cloth:
      case Matl::fluid:
      case Matl::plant:
      case Matl::stone:     return clrYellow;   break;
    }
  }
  assert(false && "Failed to get door color");
  return clrGray;
}

char Door::getGlyph() const
{
  return isSecret_ ? mimicFeature_->getGlyph() : (isOpen_ ? 39 : '+');
}

TileId Door::getTile() const
{
  return isSecret_ ? mimicFeature_->getTile() :
         (isOpen_ ? TileId::doorOpen : TileId::doorClosed);
}

Matl Door::getMatl() const
{
  return isSecret_ ? mimicFeature_->getMatl() : matl_;
}

void Door::bump(Actor& actorBumping)
{
  if (actorBumping.isPlayer())
  {
    if (isSecret_)
    {
      if (Map::cells[pos_.x][pos_.y].isSeenByPlayer)
      {
        TRACE << "Player bumped into secret door, with vision in cell" << endl;
        Log::addMsg("That way is blocked.");
      }
      else
      {
        TRACE << "Player bumped into secret door, without vision in cell" << endl;
        Log::addMsg("I bump into something.");
      }
      return;
    }

    if (!isOpen_) {tryOpen(&actorBumping);}
  }
}

void Door::reveal(const bool ALLOW_MESSAGE)
{
  if (isSecret_)
  {
    isSecret_ = false;
    if (Map::cells[pos_.x][pos_.y].isSeenByPlayer)
    {
      Render::drawMapAndInterface();
      if (ALLOW_MESSAGE)
      {
        Log::addMsg("A secret is revealed.");
        Render::drawMapAndInterface();
      }
    }
  }
}

void Door::playerTrySpotHidden()
{
  if (isSecret_)
  {
    const int PLAYER_SKILL = Map::player->getData().abilityVals.getVal(
                               AbilityId::searching, true, *(Map::player));
    if (AbilityRoll::roll(PLAYER_SKILL) >= successSmall) {reveal(true);}
  }
}

bool Door::trySpike(Actor* actorTrying)
{
  const bool IS_PLAYER = actorTrying == Map::player;
  const bool TRYER_IS_BLIND = !actorTrying->getPropHandler().allowSee();

  if (isSecret_ || isOpen_) {return false;}

  //Door is in correct state for spiking (known, closed)
  nrSpikes_++;
  isStuck_ = true;

  if (IS_PLAYER)
  {
    if (!TRYER_IS_BLIND)
    {
      Log::addMsg("I jam the door with a spike.");
    }
    else
    {
      Log::addMsg("I jam a door with a spike.");
    }
  }
  GameTime::tick();
  return true;
}

void Door::tryClose(Actor* actorTrying)
{
  const bool IS_PLAYER = actorTrying == Map::player;
  const bool TRYER_IS_BLIND = !actorTrying->getPropHandler().allowSee();
  //const bool PLAYER_SEE_DOOR    = Map::playerVision[pos_.x][pos_.y];
  bool blocked[MAP_W][MAP_H];
  MapParse::run(CellCheck::BlocksLos(), blocked);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true :
    Map::player->isSeeingActor(*actorTrying, blocked);

  bool isClosable = true;

  if (isHandledExternally_)
  {
    if (IS_PLAYER)
    {
      Log::addMsg(
        "This door refuses to be closed, perhaps it is handled elsewhere?");
      Render::drawMapAndInterface();
    }
    return;
  }

  //Already closed?
  if (isClosable)
  {
    if (!isOpen_)
    {
      isClosable = false;
      if (IS_PLAYER)
      {
        if (!TRYER_IS_BLIND)
          Log::addMsg("I see nothing there to close.");
        else Log::addMsg("I find nothing there to close.");
      }
    }
  }

  //Blocked?
  if (isClosable)
  {
    bool isblockedByActor = false;
    for (Actor* actor : GameTime::actors_)
    {
      if (actor->pos == pos_)
      {
        isblockedByActor = true;
        break;
      }
    }
    if (isblockedByActor || Map::cells[pos_.x][pos_.y].item)
    {
      isClosable = false;
      if (IS_PLAYER)
      {
        if (!TRYER_IS_BLIND)
        {
          Log::addMsg("The door is blocked.");
        }
        else
        {
          Log::addMsg("Something is blocking the door.");
        }
      }
    }
  }

  if (isClosable)
  {
    //Door is in correct state for closing (open, working, not blocked)

    if (!TRYER_IS_BLIND)
    {
      isOpen_ = false;
      if (IS_PLAYER)
      {
        Snd snd("", SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_,
                actorTrying, SndVol::low, AlertsMon::yes);
        SndEmit::emitSnd(snd);
        Log::addMsg("I close the door.");
      }
      else
      {
        Snd snd("I hear a door closing.",
                SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_, actorTrying,
                SndVol::low, AlertsMon::no);
        SndEmit::emitSnd(snd);
        if (PLAYER_SEE_TRYER)
        {
          Log::addMsg(actorTrying->getNameThe() + " closes a door.");
        }
      }
    }
    else
    {
      if (Rnd::percentile() < 50)
      {
        isOpen_ = false;
        if (IS_PLAYER)
        {
          Snd snd("", SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMon::yes);
          SndEmit::emitSnd(snd);
          Log::addMsg("I fumble with a door and succeed to close it.");
        }
        else
        {
          Snd snd("I hear a door closing.",
                  SfxId::doorClose, IgnoreMsgIfOriginSeen::yes, pos_, actorTrying,
                  SndVol::low, AlertsMon::no);
          SndEmit::emitSnd(snd);
          if (PLAYER_SEE_TRYER)
          {
            Log::addMsg(actorTrying->getNameThe() +
                        "fumbles about and succeeds to close a door.");
          }
        }
      }
      else
      {
        if (IS_PLAYER)
        {
          Log::addMsg(
            "I fumble blindly with a door and fail to close it.");
        }
        else
        {
          if (PLAYER_SEE_TRYER)
          {
            Log::addMsg(actorTrying->getNameThe() +
                        " fumbles blindly and fails to close a door.");
          }
        }
      }
    }
  }

  if (!isOpen_ && isClosable) {GameTime::tick();}
}

void Door::tryOpen(Actor* actorTrying)
{
  TRACE_FUNC_BEGIN;
  const bool IS_PLAYER        = actorTrying == Map::player;
  const bool TRYER_IS_BLIND   = !actorTrying->getPropHandler().allowSee();
  const bool PLAYER_SEE_DOOR  = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  bool blocked[MAP_W][MAP_H];
  MapParse::run(CellCheck::BlocksLos(), blocked);

  const bool PLAYER_SEE_TRYER =
    IS_PLAYER ? true : Map::player->isSeeingActor(*actorTrying, blocked);

  if (isHandledExternally_)
  {
    if (IS_PLAYER)
    {
      Log::addMsg(
        "I see no way to open this door, perhaps it is opened elsewhere?");
      Render::drawMapAndInterface();
    }
    return;
  }

  if (isStuck_)
  {
    TRACE << "Is stuck" << endl;

    if (IS_PLAYER)
    {
      Log::addMsg("The door seems to be stuck.");
    }

  }
  else
  {
    TRACE << "Is not stuck" << endl;
    if (!TRYER_IS_BLIND)
    {
      TRACE << "Tryer can see, opening" << endl;
      isOpen_ = true;
      if (IS_PLAYER)
      {
        Snd snd("", SfxId::doorOpen, IgnoreMsgIfOriginSeen::yes, pos_,
                actorTrying, SndVol::low, AlertsMon::yes);
        SndEmit::emitSnd(snd);
        Log::addMsg("I open the door.");
      }
      else
      {
        Snd snd("I hear a door open.", SfxId::doorOpen,
                IgnoreMsgIfOriginSeen::yes, pos_, actorTrying, SndVol::low,
                AlertsMon::no);
        SndEmit::emitSnd(snd);
        if (PLAYER_SEE_TRYER)
        {
          Log::addMsg(actorTrying->getNameThe() + " opens a door.");
        }
        else if (PLAYER_SEE_DOOR)
        {
          Log::addMsg("I see a door opening.");
        }
      }
    }
    else
    {
      if (Rnd::percentile() < 50)
      {
        TRACE << "Tryer is blind, but open succeeded anyway" << endl;
        isOpen_ = true;
        if (IS_PLAYER)
        {
          Snd snd("", SfxId::doorOpen, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMon::yes);
          SndEmit::emitSnd(snd);
          Log::addMsg("I fumble with a door and succeed to open it.");
        }
        else
        {
          Snd snd("I hear something open a door clumsily.", SfxId::doorOpen,
                  IgnoreMsgIfOriginSeen::yes, pos_, actorTrying, SndVol::low,
                  AlertsMon::no);
          SndEmit::emitSnd(snd);
          if (PLAYER_SEE_TRYER)
          {
            Log::addMsg(actorTrying->getNameThe() +
                        "fumbles about and succeeds to open a door.");
          }
          else if (PLAYER_SEE_DOOR)
          {
            Log::addMsg("I see a door open clumsily.");
          }
        }
      }
      else
      {
        TRACE << "Tryer is blind, and open failed" << endl;
        if (IS_PLAYER)
        {
          Snd snd("", SfxId::END, IgnoreMsgIfOriginSeen::yes, pos_,
                  actorTrying, SndVol::low, AlertsMon::yes);
          SndEmit::emitSnd(snd);
          Log::addMsg("I fumble blindly with a door and fail to open it.");
        }
        else
        {
          //Emitting the sound from the actor instead of the door, because the
          //sound message should be received even if the door is seen
          Snd snd("I hear something attempting to open a door.", SfxId::END,
                  IgnoreMsgIfOriginSeen::yes, actorTrying->pos, actorTrying,
                  SndVol::low, AlertsMon::no);
          SndEmit::emitSnd(snd);
          if (PLAYER_SEE_TRYER)
          {
            Log::addMsg(actorTrying->getNameThe() +
                        " fumbles blindly and fails to open a door.");
          }
        }
        GameTime::tick();
      }
    }
  }

  if (isOpen_)
  {
    TRACE << "Open was successful" << endl;
    if (isSecret_)
    {
      TRACE << "Was secret, now revealing" << endl;
      reveal(true);
    }
    TRACE << "Calling GameTime::endTurnOfCurActor()" << endl;
    GameTime::tick();
  }
}

DidOpen Door::open(Actor* const actorOpening)
{
  (void)actorOpening;

  isOpen_   = true;
  isSecret_ = false;
  isStuck_  = false;
  return DidOpen::yes;
}

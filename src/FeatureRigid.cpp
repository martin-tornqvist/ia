#include "FeatureRigid.h"

#include <string>

#include "Init.h"
#include "Log.h"
#include "Render.h"
#include "Map.h"
#include "Utils.h"
#include "Popup.h"
#include "MapTravel.h"
#include "SaveHandling.h"
#include "ItemFactory.h"
#include "MapParsing.h"
#include "FeatureMob.h"
#include "ItemDrop.h"
#include "Explosion.h"
#include "ActorFactory.h"
#include "ActorMon.h"

using namespace std;

//--------------------------------------------------------------------- RIGID
Rigid::Rigid(Pos pos) :
  Feature(pos),
  goreTile_(TileId::empty),
  goreGlyph_(0),
  isBloody_(false),
  burnState_(BurnState::notBurned) {}

void Rigid::onNewTurn()
{
  if (burnState_ == BurnState::burning)
  {
    clearGore();

    auto scorchActor = [](Actor & actor)
    {
      if (&actor == Map::player)
      {
        Log::addMsg("I am scorched by flames.", clrMsgBad);
      }
      else
      {
        if (Map::player->isSeeingActor(actor, nullptr))
        {
          Log::addMsg(actor.getNameThe() + " is scorched by flames.", clrMsgGood);
        }
      }
      actor.hit(1, DmgType::fire);
    };

    //TODO Hit dead actors

    //Hit actor standing on feature
    auto* actor = Utils::getActorAtPos(pos_);
    if (actor)
    {
      //Occasionally try to set actor on fire, otherwise just do small fire damage
      if (Rnd::oneIn(4))
      {
        auto& propHandler = actor->getPropHandler();
        propHandler.tryApplyProp(new PropBurning(PropTurns::std));
      }
      else
      {
        scorchActor(*actor);
      }
    }

    //Finished burning?
    int finishBurningOneInN = 1;
    int hitAdjacentOneInN   = 1;

    switch (getMatl())
    {
      case Matl::fluid:
      case Matl::empty:
        finishBurningOneInN = 1;
        hitAdjacentOneInN   = 1;
        break;

      case Matl::stone:
        finishBurningOneInN = 12;
        hitAdjacentOneInN   = 12;
        break;

      case Matl::metal:
        finishBurningOneInN = 12;
        hitAdjacentOneInN   = 6;
        break;

      case Matl::plant:
        finishBurningOneInN = 20;
        hitAdjacentOneInN   = 6;
        break;

      case Matl::wood:
        finishBurningOneInN = 60;
        hitAdjacentOneInN   = 4;
        break;

      case Matl::cloth:
        finishBurningOneInN = 20;
        hitAdjacentOneInN   = 6;
        break;
    }

    if (Rnd::oneIn(finishBurningOneInN))
    {
      burnState_ = BurnState::hasBurned;
      if (onFinishedBurning() == IsDestroyed::yes)
      {
        return;
      }
    }

    //Hit adjacent features and actors?
    if (Rnd::oneIn(hitAdjacentOneInN))
    {
      const Pos p(DirUtils::getRndAdjPos(pos_, false));
      if (Utils::isPosInsideMap(p))
      {
        Map::cells[p.x][p.y].rigid->hit(DmgType::fire, DmgMethod::elemental);

        actor = Utils::getActorAtPos(p);
        if (actor) {scorchActor(*actor);}
      }
    }

    //Create smoke?
    if (Rnd::oneIn(20))
    {
      const Pos p(DirUtils::getRndAdjPos(pos_, true));
      if (Utils::isPosInsideMap(p))
      {
        if (!CellCheck::BlocksMoveCmn(false).check(Map::cells[p.x][p.y]))
        {
          GameTime::addMob(new Smoke(p, 10));
        }
      }
    }
  }

  //Run specialized new turn actions
  onNewTurn_();
}

void Rigid::tryStartBurning(const bool IS_MSG_ALLOWED)
{
  clearGore();

  if (burnState_ == BurnState::notBurned)
  {
    if (Map::isPosSeenByPlayer(pos_) && IS_MSG_ALLOWED)
    {
      string str = getName(Article::the) + " catches fire.";
      str[0] = toupper(str[0]);
      Log::addMsg(str);
    }
    burnState_ = BurnState::burning;
  }
}

void Rigid::examine()
{
  Log::addMsg("I find nothing specific there to examine.");
}

void Rigid::disarm()
{
  Log::addMsg(msgDisarmNoTrap);
  Render::drawMapAndInterface();
}

void Rigid::hit(const DmgType dmgType, const DmgMethod dmgMethod, Actor* actor)
{

  bool isFeatureHit = true;

  if (actor == Map::player && dmgMethod == DmgMethod::kick)
  {
    const bool IS_BLIND    = !Map::player->getPropHandler().allowSee();
    const bool IS_BLOCKING = !canMoveCmn() && getId() != FeatureId::stairs;

    if (IS_BLOCKING)
    {
      Log::addMsg("I kick " + (IS_BLIND ? "something" : getName(Article::a)) + "!");

      if (Rnd::oneIn(4))
      {
        Log::addMsg("I sprain myself.", clrMsgBad);
        actor->hit(Rnd::range(1, 2), DmgType::pure);
      }

      if (Rnd::oneIn(4))
      {
        Log::addMsg("I am off-balance.");

        actor->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::specific, 2));
      }

    }
    else
    {
      isFeatureHit = false;
      Log::addMsg("I kick the air!");
      Audio::play(SfxId::missMedium);
    }
  }

  if (isFeatureHit) {onHit(dmgType, dmgMethod, actor);}

  if (actor) {GameTime::actorDidAct();} //TODO This should probably be done elsewhere.
}

void Rigid::tryPutGore()
{
  if (getData().canHaveGore)
  {
    const int ROLL_GLYPH = Rnd::dice(1, 4);
    switch (ROLL_GLYPH)
    {
      case 1: goreGlyph_ = ',';  break;
      case 2: goreGlyph_ = '`';  break;
      case 3: goreGlyph_ = 39;   break;
      case 4: goreGlyph_ = ';';  break;
    }
  }

  const int ROLL_TILE = Rnd::dice(1, 8);
  switch (ROLL_TILE)
  {
    case 1: goreTile_ = TileId::gore1;  break;
    case 2: goreTile_ = TileId::gore2;  break;
    case 3: goreTile_ = TileId::gore3;  break;
    case 4: goreTile_ = TileId::gore4;  break;
    case 5: goreTile_ = TileId::gore5;  break;
    case 6: goreTile_ = TileId::gore6;  break;
    case 7: goreTile_ = TileId::gore7;  break;
    case 8: goreTile_ = TileId::gore8;  break;
  }
}

Clr Rigid::getClr() const
{
  if (burnState_ == BurnState::burning)
  {
    return clrOrange;
  }
  else
  {
    if (isBloody_)
    {
      return clrRedLgt;
    }
    else
    {
      return burnState_ == BurnState::notBurned ? getClr_() : clrGrayDrk;
    }
  }
}

Clr Rigid::getClrBg() const
{
  switch (burnState_)
  {
    case BurnState::notBurned:  return getClrBg_();
    case BurnState::burning:    return Clr {Uint8(Rnd::range(32, 255)), 0, 0, 0};
    case BurnState::hasBurned:  return getClrBg_();
  }
  assert(false && "Failed to set color");
  return clrYellow;
}

void Rigid::clearGore()
{
  goreTile_   = TileId::empty;
  goreGlyph_  = ' ';
  isBloody_   = false;
}

//--------------------------------------------------------------------- FLOOR
Floor::Floor(Pos pos) : Rigid(pos), type_(FloorType::cmn) {}

void Floor::onHit(const DmgType dmgType, const DmgMethod dmgMethod, Actor* const actor)
{
  if (dmgType == DmgType::fire && dmgMethod == DmgMethod::elemental)
  {
    (void)actor;
    if (Rnd::oneIn(3)) {tryStartBurning(false);}
  }
}

TileId Floor::getTile() const
{
  return getBurnState() ==
         BurnState::hasBurned ? TileId::scorchedGround : getData().tile;
}

string Floor::getName(const Article article) const
{
  string ret = article == Article::a ? "" : "the ";

  if (getBurnState() == BurnState::burning)
  {
    ret += "flames";
  }
  else
  {
    if (getBurnState() == BurnState::hasBurned) {ret += "scorched ";}

    switch (type_)
    {
      case FloorType::cmn:        ret += "stone floor";   break;
      case FloorType::cave:       ret += "cavern floor";  break;
      case FloorType::stonePath:  ret += "stone path";    break;
    }
  }

  return ret;
}

Clr Floor::getClr_() const
{
  return clrGray;
}

//--------------------------------------------------------------------- WALL
Wall::Wall(Pos pos) : Rigid(pos), type_(WallType::cmn), isMossy_(false) {}

void Wall::onHit(const DmgType dmgType, const DmgMethod dmgMethod, Actor* const actor)
{

  auto destrAdjDoors = [&]()
  {
    for (const Pos& d : DirUtils::cardinalList)
    {
      const Pos p(pos_ + d);
      if (Utils::isPosInsideMap(p))
      {
        if (Map::cells[p.x][p.y].rigid->getId() == FeatureId::door)
        {
          Map::put(new RubbleLow(p));
        }
      }
    }
  };

  auto mkLowRubbleAndRocks = [&]()
  {
    const Pos p(pos_);
    Map::put(new RubbleLow(p)); //Note: "this" is now deleted!
    if (Rnd::coinToss()) {ItemFactory::mkItemOnMap(ItemId::rock, p);}
  };

  if (dmgType == DmgType::physical)
  {

    if (dmgMethod == DmgMethod::forced)
    {
      (void)actor;
      destrAdjDoors();
      mkLowRubbleAndRocks();
    }

    if (dmgMethod == DmgMethod::explosion)
    {
      (void)actor;

      destrAdjDoors();

      if (Rnd::coinToss())
      {
        mkLowRubbleAndRocks();
      }
      else
      {
        Map::put(new RubbleHigh(pos_));
      }
    }

    if (dmgMethod == DmgMethod::bluntHeavy)
    {
      (void)actor;

      if (Rnd::fraction(1, 4))
      {

        destrAdjDoors();

        if (Rnd::coinToss())
        {
          mkLowRubbleAndRocks();
        }
        else
        {
          Map::put(new RubbleHigh(pos_));
        }
      }
    }
  }
}

bool Wall::isTileAnyWallFront(const TileId tile)
{
  return
    tile == TileId::wallFront      ||
    tile == TileId::wallFrontAlt1  ||
    tile == TileId::wallFrontAlt2  ||
    tile == TileId::caveWallFront  ||
    tile == TileId::egyptWallFront;
}

bool Wall::isTileAnyWallTop(const TileId tile)
{
  return
    tile == TileId::wallTop      ||
    tile == TileId::caveWallTop  ||
    tile == TileId::egyptWallTop ||
    tile == TileId::rubbleHigh;
}

string Wall::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  if (isMossy_) {ret += "moss-grown ";}

  switch (type_)
  {
    case WallType::cmn:
    case WallType::cmnAlt:
    case WallType::lengMonestary:
    case WallType::egypt:
      ret += "stone wall";
      break;

    case WallType::cave:
      ret += "cavern wall";
      break;

    case WallType::cliff:
      ret += "cliff";
      break;
  }
  return ret;
}

Clr Wall::getClr_() const
{
  if (isMossy_) {return clrGreenDrk;}
  switch (type_)
  {
    case WallType::cliff:
      return clrGrayDrk;

    case WallType::egypt:
    case WallType::cave:
      return clrBrownGray;

    case WallType::cmn:
    case WallType::cmnAlt:
      return clrGray;

    case WallType::lengMonestary:
      return clrRed;
  }
  assert(false && "Failed to set color");
  return clrYellow;
}

char Wall::getGlyph() const
{
  return Config::isAsciiWallFullSquare() ? 10 : '#';
}

TileId Wall::getFrontWallTile() const
{
  if (Config::isTilesWallFullSquare())
  {
    switch (type_)
    {
      case WallType::cmn:
      case WallType::cmnAlt:
        return TileId::wallTop;

      case WallType::cliff:
      case WallType::cave:
        return TileId::caveWallTop;

      case WallType::lengMonestary:
      case WallType::egypt:
        return TileId::egyptWallTop;
    }
  }
  else
  {
    switch (type_)
    {
      case WallType::cmn:
        return TileId::wallFront;

      case WallType::cmnAlt:
        return TileId::wallFrontAlt1;

      case WallType::cliff:
      case WallType::cave:
        return TileId::caveWallFront;

      case WallType::lengMonestary:
      case WallType::egypt:
        return TileId::egyptWallFront;
    }
  }
  assert(false && "Failed to set front wall tile");
  return TileId::empty;
}

TileId Wall::getTopWallTile() const
{
  switch (type_)
  {
    case WallType::cmn:
    case WallType::cmnAlt:
      return TileId::wallTop;

    case WallType::cliff:
    case WallType::cave:
      return TileId::caveWallTop;

    case WallType::lengMonestary:
    case WallType::egypt:
      return TileId::egyptWallTop;
  }
  assert(false && "Failed to set top wall tile");
  return TileId::empty;
}

void Wall::setRndCmnWall()
{
  const int RND = Rnd::range(1, 6);
  switch (RND)
  {
    case 1:   type_ = WallType::cmnAlt; break;
    default:  type_ = WallType::cmn;    break;
  }
}

void Wall::setRandomIsMossGrown()
{
  isMossy_ = Rnd::oneIn(40);
}

//--------------------------------------------------------------------- HIGH RUBBLE
RubbleHigh::RubbleHigh(Pos pos) : Rigid(pos) {}

void RubbleHigh::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                       Actor* const actor)
{
  auto mkLowRubbleAndRocks = [&]()
  {
    const Pos p(pos_);
    Map::put(new RubbleLow(p)); //Note: "this" is now deleted!
    if (Rnd::coinToss()) {ItemFactory::mkItemOnMap(ItemId::rock, p);}
  };

  if (dmgType == DmgType::physical)
  {

    if (dmgMethod == DmgMethod::forced)
    {
      (void)actor;
      mkLowRubbleAndRocks();
    }

    if (dmgMethod == DmgMethod::explosion)
    {
      (void)actor;
      mkLowRubbleAndRocks();
    }

    if (dmgMethod == DmgMethod::bluntHeavy)
    {
      (void)actor;
      if (Rnd::fraction(2, 4)) {mkLowRubbleAndRocks();}
    }
  }
}

string RubbleHigh::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "big pile of debris";
}

Clr RubbleHigh::getClr_() const
{
  return clrGray;
}

//--------------------------------------------------------------------- LOW RUBBLE
RubbleLow::RubbleLow(Pos pos) : Rigid(pos) {}

void RubbleLow::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                      Actor* const actor)
{
  if (dmgType == DmgType::fire && dmgMethod == DmgMethod::elemental)
  {
    (void)actor;
    tryStartBurning(false);
  }
}

string RubbleLow::getName(const Article article) const
{
  string ret = "";
  if (article == Article::the)               {ret += "the ";}
  if (getBurnState() == BurnState::burning)  {ret += "burning ";}
  return ret + "rubble";
}

Clr RubbleLow::getClr_() const
{
  return clrGray;
}

//--------------------------------------------------------------------- GRAVE
GraveStone::GraveStone(Pos pos) : Rigid(pos) {}

void GraveStone::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                       Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

void GraveStone::bump(Actor& actorBumping)
{
  if (actorBumping.isPlayer()) {Log::addMsg(inscr_);}
}

string GraveStone::getName(const Article article) const
{
  const string ret = article == Article::a ? "a " : "the ";
  return ret + "gravestone; " + inscr_;
}

Clr GraveStone::getClr_() const
{
  return clrWhite;
}

//--------------------------------------------------------------------- CHURCH BENCH
ChurchBench::ChurchBench(Pos pos) : Rigid(pos) {}

void ChurchBench::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                        Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

string ChurchBench::getName(const Article article) const
{
  const string ret = article == Article::a ? "a " : "the ";
  return ret + "church bench";
}

Clr ChurchBench::getClr_() const
{
  return clrBrown;
}

//--------------------------------------------------------------------- STATUE
Statue::Statue(Pos pos) :
  Rigid(pos), type_(Rnd::oneIn(8) ? StatueType::ghoul : StatueType::cmn) {}

void Statue::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                   Actor* const actor)
{
  if (dmgType == DmgType::physical && dmgMethod == DmgMethod::kick)
  {
    assert(actor);

    const AlertsMon alertsMon = actor == Map::player ?
                                AlertsMon::yes :
                                AlertsMon::no;
    if (Rnd::coinToss())
    {
      if (Map::cells[pos_.x][pos_.y].isSeenByPlayer) {Log::addMsg("It topples over.");}

      Snd snd("I hear a crash.", SfxId::END, IgnoreMsgIfOriginSeen::yes,
              pos_, actor, SndVol::low, alertsMon);

      SndEmit::emitSnd(snd);

      const Pos dstPos = pos_ + (pos_ - actor->pos);

      Map::put(new RubbleLow(pos_)); //Note: "this" is now deleted!

      Map::player->updateFov();
      Render::drawMapAndInterface();
      Map::updateVisualMemory();

      if (!CellCheck::BlocksMoveCmn(false).check(Map::cells[dstPos.x][dstPos.y]))
      {
        Actor* const actorBehind = Utils::getActorAtPos(dstPos);
        if (actorBehind)
        {
          if (actorBehind->isAlive())
          {
            bool props[endOfPropIds];
            actorBehind->getPropHandler().getAllActivePropIds(props);
            if (props[propEthereal])
            {
              if (actorBehind == Map::player)
              {
                Log::addMsg("It falls on me!");
              }
              else if (Map::player->isSeeingActor(*actorBehind, nullptr))
              {
                Log::addMsg("It falls on " + actorBehind->getNameA() + ".");
              }
              actorBehind->hit(Rnd::dice(3, 5), DmgType::physical);
            }
          }
        }
        Map::put(new RubbleLow(dstPos));
      }
    }
  }
}

string Statue::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  switch (type_)
  {
    case StatueType::cmn:   ret += "statue"; break;
    case StatueType::ghoul: ret += "statue of a ghoulish creature"; break;
  }

  return ret;
}

Clr Statue::getClr_() const
{
  return clrWhite;
}

//--------------------------------------------------------------------- PILLAR
Pillar::Pillar(Pos pos) : Rigid(pos) {}

void Pillar::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                   Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

string Pillar::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "pillar";
}

Clr Pillar::getClr_() const
{
  return clrWhite;
}

//--------------------------------------------------------------------- STAIRS
Stairs::Stairs(Pos pos) : Rigid(pos) {}

void Stairs::onHit(const DmgType dmgType, const DmgMethod dmgMethod, Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

void Stairs::onNewTurn_()
{
  assert(!Map::cells[pos_.x][pos_.y].item);
}

void Stairs::bump(Actor& actorBumping)
{
  if (actorBumping.isPlayer())
  {
    const vector<string> choices {"Descend", "Save and quit", "Cancel"};
    const string title  = "A staircase leading downwards";
    const int CHOICE    =  Popup::showMenuMsg("", true, choices, title);

    switch (CHOICE)
    {
      case 0:
        Map::player->pos = pos_;
        MapTravel::goToNxt();
        break;

      case 1:
        Map::player->pos = pos_;
        SaveHandling::save();
        Init::quitToMainMenu = true;
        break;

      default:
        Log::clearLog();
        Render::drawMapAndInterface();
        break;
    }
  }
}

string Stairs::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "downward staircase";
}

Clr Stairs::getClr_() const
{
  return clrYellow;
}

//--------------------------------------------------------------------- BRIDGE
TileId Bridge::getTile() const
{
  return dir_ == hor ? TileId::hangbridgeHor : TileId::hangbridgeVer;
}

void Bridge::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                   Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

char Bridge::getGlyph() const
{
  return dir_ == hor ? '|' : '=';
}

string Bridge::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "bridge";
}

Clr Bridge::getClr_() const
{
  return clrBrownDrk;
}

//--------------------------------------------------------------------- SHALLOW LIQUID
LiquidShallow::LiquidShallow(Pos pos) : Rigid(pos), type_(LiquidType::water) {}

void LiquidShallow::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                          Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

void LiquidShallow::bump(Actor& actorBumping)
{
  bool props[endOfPropIds];
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if (!props[propEthereal] && !props[propFlying])
  {
    actorBumping.getPropHandler().tryApplyProp(new PropWaiting(PropTurns::std));

    if (actorBumping.isPlayer())
    {
      Log::addMsg("*glop*");
    }
  }
}

string LiquidShallow::getName(const Article article) const
{
  string ret = "";
  if (article == Article::the) {ret += "the ";}

  ret += "a pool of ";

  switch (type_)
  {
    case LiquidType::water:   ret += "water"; break;
    case LiquidType::acid:    ret += "acid";  break;
    case LiquidType::blood:   ret += "blood"; break;
    case LiquidType::lava:    ret += "lava";  break;
    case LiquidType::mud:     ret += "mud";   break;
  }

  return ret;
}

Clr LiquidShallow::getClr_() const
{
  switch (type_)
  {
    case LiquidType::water:   return clrBlueLgt;  break;
    case LiquidType::acid:    return clrGreenLgt; break;
    case LiquidType::blood:   return clrRedLgt;   break;
    case LiquidType::lava:    return clrOrange;   break;
    case LiquidType::mud:     return clrBrown;    break;
  }
  assert(false && "Failed to set color");
  return clrYellow;
}

//--------------------------------------------------------------------- DEEP LIQUID
LiquidDeep::LiquidDeep(Pos pos) : Rigid(pos), type_(LiquidType::water) {}

void LiquidDeep::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                       Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

void LiquidDeep::bump(Actor& actorBumping)
{
  (void)actorBumping;
}

string LiquidDeep::getName(const Article article) const
{
  string ret = "";
  if (article == Article::the) {ret += "the ";}

  ret += "deep ";

  switch (type_)
  {
    case LiquidType::water:   ret += "water"; break;
    case LiquidType::acid:    ret += "acid";  break;
    case LiquidType::blood:   ret += "blood"; break;
    case LiquidType::lava:    ret += "lava";  break;
    case LiquidType::mud:     ret += "mud";   break;
  }

  return ret;
}

Clr LiquidDeep::getClr_() const
{
  switch (type_)
  {
    case LiquidType::water:   return clrBlue;     break;
    case LiquidType::acid:    return clrGreen;    break;
    case LiquidType::blood:   return clrRed;      break;
    case LiquidType::lava:    return clrOrange;   break;
    case LiquidType::mud:     return clrBrownDrk; break;
  }
  assert(false && "Failed to set color");
  return clrYellow;
}

//--------------------------------------------------------------------- CHASM
Chasm::Chasm(Pos pos) : Rigid(pos) {}

void Chasm::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                  Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}


string Chasm::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "chasm";
}

Clr Chasm::getClr_() const
{
  return clrBlack;
}

//--------------------------------------------------------------------- LEVER
Lever::Lever(Pos pos) : Rigid(pos), isPositionLeft_(true), doorLinkedTo_(nullptr)  {}

void Lever::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                  Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

string Lever::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "lever";
}

Clr Lever::getClr_() const
{
  return isPositionLeft_ ? clrGray : clrWhite;
}

TileId Lever::getTile() const
{
  return isPositionLeft_ ? TileId::leverLeft : TileId::leverRight;
}

void Lever::examine()
{
  pull();
}

void Lever::pull()
{
  TRACE_FUNC_BEGIN;
  isPositionLeft_ = !isPositionLeft_;

  //TODO Implement something like openByLever in the Door class
  //Others should not poke around in the doors internal variables

//  if(!doorLinkedTo_->isBroken_) {
//    TRACE << "Door linked to is not broken" << endl;
//    if(!doorLinkedTo_->isOpen_) {doorLinkedTo_->reveal(true);}
//    doorLinkedTo_->isOpen_  = !doorLinkedTo_->isOpen_;
//    doorLinkedTo_->isStuck_ = false;
//  }
  Map::player->updateFov();
  Render::drawMapAndInterface();
  TRACE_FUNC_END;
}

//--------------------------------------------------------------------- ALTAR
Altar::Altar(Pos pos) : Rigid(pos) {}

void Altar::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                  Actor* const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

string Altar::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "altar";
}

Clr Altar::getClr_() const
{
  return clrWhite;
}

//--------------------------------------------------------------------- CARPET
Carpet::Carpet(Pos pos) : Rigid(pos) {}

void Carpet::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                   Actor* const actor)
{
  if (dmgType == DmgType::fire && dmgMethod == DmgMethod::elemental)
  {
    (void)actor;
    tryStartBurning(false);
  }
}

IsDestroyed Carpet::onFinishedBurning()
{
  Floor* const floor = new Floor(pos_);
  floor->setHasBurned();
  Map::put(floor);
  return IsDestroyed::yes;
}

string Carpet::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "carpet";
}

Clr Carpet::getClr_() const
{
  return clrRed;
}

//--------------------------------------------------------------------- GRASS
Grass::Grass(Pos pos) : Rigid(pos), type_(GrassType::cmn)
{
  if (Rnd::oneIn(5)) {type_ = GrassType::withered;}
}

void Grass::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                  Actor* const actor)
{
  if (dmgType == DmgType::fire && dmgMethod == DmgMethod::elemental)
  {
    (void)actor;
    tryStartBurning(false);
  }
}


TileId Grass::getTile() const
{
  return getBurnState() ==
         BurnState::hasBurned ? TileId::scorchedGround : getData().tile;
}

string Grass::getName(const Article article) const
{
  string ret = "";
  if (article == Article::the) {ret += "the ";}

  switch (getBurnState())
  {
    case BurnState::notBurned:
      switch (type_)
      {
        case GrassType::cmn:
          return ret + "grass";

        case GrassType::withered:
          return ret + "withered grass";
      }
      break;

    case BurnState::burning:
      return ret + "burning grass";

    case BurnState::hasBurned:
      return ret + "scorched ground";
  }

  assert("Failed to set name" && false);
  return "";
}

Clr Grass::getClr_() const
{
  switch (type_)
  {
    case GrassType::cmn:      return clrGreen;    break;
    case GrassType::withered: return clrBrownDrk; break;
  }
  assert(false && "Failed to set color");
  return clrYellow;
}

//--------------------------------------------------------------------- BUSH
Bush::Bush(Pos pos) : Rigid(pos), type_(GrassType::cmn)
{
  if (Rnd::oneIn(5)) {type_ = GrassType::withered;}
}

void Bush::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                 Actor* const actor)
{
  if (dmgType == DmgType::fire && dmgMethod == DmgMethod::elemental)
  {
    (void)actor;
    tryStartBurning(false);
  }
}

IsDestroyed Bush::onFinishedBurning()
{
  Grass* const grass = new Grass(pos_);
  grass->setHasBurned();
  Map::put(grass);
  return IsDestroyed::yes;
}

string Bush::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  switch (getBurnState())
  {
    case BurnState::notBurned:
      switch (type_)
      {
        case GrassType::cmn:
          return ret + "shrub";

        case GrassType::withered:
          return ret + "withered shrub";
      }
      break;

    case BurnState::burning:
      return ret + "burning shrub";

    case BurnState::hasBurned: {/*Should not happen*/}
      break;
  }

  assert("Failed to set name" && false);
  return "";
}

Clr Bush::getClr_() const
{
  switch (type_)
  {
    case GrassType::cmn:      return clrGreen;    break;
    case GrassType::withered: return clrBrownDrk; break;
  }
  assert(false && "Failed to set color");
  return clrYellow;
}

//--------------------------------------------------------------------- TREE
Tree::Tree(Pos pos) : Rigid(pos) {}

void Tree::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                 Actor * const actor)
{
  if (dmgType == DmgType::fire && dmgMethod == DmgMethod::elemental)
  {
    (void)actor;
    if (Rnd::oneIn(3)) {tryStartBurning(false);}
  }
}


string Tree::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  switch (getBurnState())
  {
    case BurnState::notBurned:  {}                  break;
    case BurnState::burning:    ret += "burning ";  break;
    case BurnState::hasBurned:  ret += "scorched "; break;
  }

  return ret + "tree";
}

Clr Tree::getClr_() const
{
  return clrBrownDrk;
}

//--------------------------------------------------------------------- BRAZIER
string Brazier::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";
  return ret + "brazier";
}

void Brazier::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                    Actor * const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

Clr Brazier::getClr_() const
{
  return clrYellow;
}

//--------------------------------------------------------------------- ITEM CONTAINER
ItemContainer::ItemContainer() {items_.clear();}

ItemContainer::~ItemContainer()
{
  for (unsigned int i = 0; i < items_.size(); ++i)
  {
    delete items_[i];
  }
}

void ItemContainer::setRandomItemsForFeature(
  const FeatureId featureId,
  const int NR_ITEMS_TO_ATTEMPT)
{
  for (unsigned int i = 0; i < items_.size(); ++i)
  {
    delete items_[i];
  }
  items_.clear();

  if (NR_ITEMS_TO_ATTEMPT > 0)
  {
    while (items_.empty())
    {
      vector<ItemId> itemBucket;
      for (int i = 0; i < int(ItemId::END); ++i)
      {
        ItemDataT* const curData = ItemData::data[i];
        for (
          unsigned int ii = 0;
          ii < curData->featuresCanBeFoundIn.size();
          ii++)
        {
          auto featuresFoundIn = curData->featuresCanBeFoundIn[ii];
          if (featuresFoundIn.first == featureId)
          {
            if (Rnd::percentile() < featuresFoundIn.second)
            {
              if (Rnd::percentile() < curData->chanceToIncludeInSpawnList)
              {
                itemBucket.push_back(ItemId(i));
                break;
              }
            }
          }
        }
      }

      const int NR_CANDIDATES = int(itemBucket.size());
      if (NR_CANDIDATES > 0)
      {
        for (int i = 0; i < NR_ITEMS_TO_ATTEMPT; ++i)
        {
          const auto ELEMENT = Rnd::range(0, NR_CANDIDATES - 1);
          Item* item = ItemFactory::mk(itemBucket[ELEMENT]);
          ItemFactory::setItemRandomizedProperties(item);
          items_.push_back(item);
        }
      }
    }
  }
}

void ItemContainer::dropItems(const Pos & pos)
{
  for (auto* item : items_) {ItemDrop::dropItemOnMap(pos, *item);}
  items_.clear();
}

void ItemContainer::destroySingleFragile()
{
  //TODO Generalize this function (perhaps isFragile variable in item data)

  for (size_t i = 0; i < items_.size(); ++i)
  {
    Item* const item = items_[i];
    const ItemDataT& d = item->getData();
    if (d.isPotion || d.id == ItemId::molotov)
    {
      delete item;
      items_.erase(items_.begin() + i);
      Log::addMsg("I hear a muffled shatter.");
      break;
    }
  }
}

//--------------------------------------------------------------------- TOMB
Tomb::Tomb(const Pos & pos) :
  Rigid(pos), isContentKnown_(false), isTraitKnown_(false),
  pushLidOneInN_(Rnd::range(6, 14)), appearance_(TombAppearance::common),
  trait_(TombTrait::END)
{

  //Contained items
  const int NR_ITEMS_MIN = Rnd::oneIn(3) ? 0 : 1;
  const int NR_ITEMS_MAX =
    NR_ITEMS_MIN + (PlayerBon::traitsPicked[int(Trait::treasureHunter)] ? 1 : 0);

  itemContainer_.setRandomItemsForFeature(
    FeatureId::tomb, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

  //Appearance
  if (Rnd::oneIn(5))
  {
    const TombAppearance lastAppearance = TombAppearance::END;
    appearance_ = TombAppearance(Rnd::range(0, int(lastAppearance) - 1));
  }
  else
  {
    for (Item* item : itemContainer_.items_)
    {
      const ItemValue itemValue = item->getData().itemValue;
      if (itemValue == ItemValue::majorTreasure)
      {
        appearance_ = TombAppearance::marvelous;
        break;
      }
      else if (itemValue == ItemValue::minorTreasure)
      {
        appearance_ = TombAppearance::ornate;
      }
    }
  }

  if (!itemContainer_.items_.empty())
  {
    const int RND = Rnd::percentile();
    if (RND < 15)
    {
      trait_ = TombTrait::forebodingCarvedSigns;
    }
    else if (RND < 45)
    {
      trait_ = TombTrait::stench;
    }
    else if (RND < 75)
    {
      trait_ = TombTrait::auraOfUnrest;
    }
  }
}

void Tomb::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                 Actor * const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

string Tomb::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  switch (appearance_)
  {
    case TombAppearance::common:
    case TombAppearance::END: {}
      break;

    case TombAppearance::ornate:
      ret += "ornate ";
      break;

    case TombAppearance::marvelous:
      ret += "marvelous ";
      break;
  }
  return ret + "tomb";
}

Clr Tomb::getClr_() const
{
  switch (appearance_)
  {
    case TombAppearance::common:
      return clrGray;

    case TombAppearance::ornate:
      return clrWhite;

    case TombAppearance::marvelous:
      return clrYellow;

    case TombAppearance::END: {}
      break;
  }
  assert("Failed to set Tomb color" && false);
  return clrBlack;
}

void Tomb::bump(Actor & actorBumping)
{
  if (actorBumping.isPlayer())
  {
    if (itemContainer_.items_.empty() && isContentKnown_)
    {
      Log::addMsg("The tomb is empty.");
    }
    else
    {
      Log::addMsg("I attempt to push the lid.");

      bool props[endOfPropIds];
      Map::player->getPropHandler().getAllActivePropIds(props);

      if (props[propWeakened])
      {
        trySprainPlayer();
        Log::addMsg("It seems futile.");
      }
      else //Not weakened
      {
        const int BON = PlayerBon::traitsPicked[int(Trait::rugged)] ? 8 :
                        PlayerBon::traitsPicked[int(Trait::tough)]  ? 4 : 0;

        TRACE << "Base chance to push lid is: 1 in " << pushLidOneInN_ << endl;

        TRACE << "Bonus to roll: " << BON << endl;

        const int ROLL_TOT = Rnd::range(1, pushLidOneInN_) + BON;

        TRACE << "Roll + bonus = " << ROLL_TOT << endl;

        bool isSuccess = false;

        if (ROLL_TOT < pushLidOneInN_ - 9)
        {
          Log::addMsg("It does not yield at all.");
        }
        else if (ROLL_TOT < pushLidOneInN_ - 1)
        {
          Log::addMsg("It resists.");
        }
        else if (ROLL_TOT == pushLidOneInN_ - 1)
        {
          Log::addMsg("It moves a little!");
          pushLidOneInN_--;
        }
        else
        {
          isSuccess = true;
        }

        if (isSuccess)
        {
          open();
        }
        else
        {
          trySprainPlayer();
        }
      }
      GameTime::actorDidAct();
    }
  }
}

void Tomb::trySprainPlayer()
{
  const int SPRAIN_ONE_IN_N = PlayerBon::traitsPicked[int(Trait::rugged)] ? 6 :
                              PlayerBon::traitsPicked[int(Trait::tough)]  ? 5 : 4;
  if (Rnd::oneIn(SPRAIN_ONE_IN_N))
  {
    Log::addMsg("I sprain myself.", clrMsgBad);
    Map::player->hit(Rnd::range(1, 5), DmgType::pure);
  }
}

bool Tomb::open()
{
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  if (IS_SEEN)
  {
    Log::addMsg("The lid comes off!");
    Log::addMsg("The tomb opens.");
  }
  Snd snd("I hear heavy stone sliding.", SfxId::tombOpen,
          IgnoreMsgIfOriginSeen::yes, pos_, nullptr, SndVol::high,
          AlertsMon::yes);
  SndEmit::emitSnd(snd);

  triggerTrap(*Map::player);
  if (itemContainer_.items_.size() > 0)
  {
    if (IS_SEEN) Log::addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_);
  }
  else
  {
    if (IS_SEEN)Log::addMsg("There is nothing of value inside.");
  }
  if (IS_SEEN) {isContentKnown_ = isTraitKnown_ = true;}
  Render::drawMapAndInterface();
  return true;
}

void Tomb::examine()
{
  bool props[endOfPropIds];
  Map::player->getPropHandler().getAllActivePropIds(props);

  if (props[propConfused])
  {
    Log::addMsg("I start to search the tomb...");
    Log::addMsg("but I cannot grasp what for.");
    GameTime::actorDidAct();
  }
  else if (itemContainer_.items_.empty() && isContentKnown_)
  {
    Log::addMsg("The tomb is empty.");
  }
  else
  {
    if (!isTraitKnown_ && trait_ != TombTrait::END)
    {
      const int FIND_ONE_IN_N = PlayerBon::traitsPicked[int(Trait::perceptive)] ? 2 :
                                (PlayerBon::traitsPicked[int(Trait::observant)] ? 3 : 6);

      isTraitKnown_ = Rnd::oneIn(FIND_ONE_IN_N);
    }

    if (isTraitKnown_)
    {
      switch (trait_)
      {
        case TombTrait::auraOfUnrest:
          Log::addMsg("It has a certain aura of unrest about it.");
          break;

        case TombTrait::forebodingCarvedSigns:
          if (PlayerBon::getBg() == Bg::occultist)
          {
            Log::addMsg("There is a curse carved on the box.");
          }
          else
          {
            Log::addMsg("There are some ominous runes carved on the box.");
          }
          break;

        case TombTrait::stench:
          Log::addMsg("There is a pungent stench.");
          break;

        case TombTrait::END: {}
          break;
      }
    }
    else //Tomb trait not discovered
    {
      Log::addMsg("I find nothing significant.");
    }
    GameTime::actorDidAct();
  }
}

//void Tomb::kick(Actor& actorTrying) {
//  (void)actorTrying;

//  const Inventory& inv = Map::player->getInv();
//  bool hasSledgehammer = false;
//  Item* item = inv.getItemInSlot(SlotId::wielded);
//  if(item) {
//    hasSledgehammer = item->getData().id == ItemId::sledgeHammer;
//  }
//  if(!hasSledgehammer) {
//    item = inv.getItemInSlot(SlotId::wieldedAlt);
//    hasSledgehammer = item->getData().id == ItemId::sledgeHammer;
//  }
//  if(!hasSledgehammer) {
//    hasSledgehammer = inv.hasItemInGeneral(ItemId::sledgeHammer);
//  }
//  if(hasSledgehammer) {
//    possibleActions.push_back(tombAction_smashLidWithSledgehammer);
//  }

//  Log::addMsg("I strike at the lid with a Sledgehammer.");
//  const int BREAK_N_IN_10 = IS_WEAK ? 1 : 8;
//  if(Rnd::fraction(BREAK_N_IN_10, 10)) {
//    Log::addMsg("The lid cracks open!");
//    if(!IS_BLESSED && (IS_CURSED || Rnd::oneIn(3))) {
//      itemContainer_.destroySingleFragile();
//    }
//    open();
//  } else {
//    Log::addMsg("The lid resists.");
//  }

//  GameTime::actorDidAct();
//}

//void Tomb::disarm() {

//case tombAction_carveCurseWard: {
//    if(!IS_CURSED && (IS_BLESSED || Rnd::fraction(4, 5))) {
//      Log::addMsg("The curse is cleared.");
//    } else {
//      Log::addMsg("I make a mistake, the curse is doubled!");
//      PropCursed* const curse =
//        new PropCursed(PropTurns::std);
//      curse->turnsLeft_ *= 2;
//      propHlr.tryApplyProp(curse, true);
//    }
//    trait_ = endOfTombTraits;
//    break;
//}

void Tomb::triggerTrap(Actor & actor)
{
  (void)actor;

  vector<ActorId> actorBucket;

  switch (trait_)
  {
    case TombTrait::auraOfUnrest:
      for (int i = 0; i < int(ActorId::END); ++i)
      {
        const ActorDataT& d = ActorData::data[i];

        if (d.isGhost                           &&
            d.isAutoSpawnAllowed && !d.isUnique &&
            ((Map::dlvl + 5) >= d.spawnMinDLVL || Map::dlvl >= MIN_DLVL_HARDER_TRAPS))
        {
          actorBucket.push_back(ActorId(i));
        }
      }
      Log::addMsg("Something rises from the tomb!");
      break;

    case TombTrait::forebodingCarvedSigns:
      Map::player->getPropHandler().tryApplyProp(new PropCursed(PropTurns::std));
      break;

    case TombTrait::stench:
      if (Rnd::coinToss())
      {
        Log::addMsg("Fumes burst out from the tomb!");
        Prop* prop    = nullptr;
        Clr fumeClr   = clrMagenta;
        const int RND = Rnd::percentile();
        if (RND < 20)
        {
          prop    = new PropPoisoned(PropTurns::std);
          fumeClr = clrGreenLgt;
        }
        else if (RND < 40)
        {
          prop    = new PropDiseased(PropTurns::specific, 50);
          fumeClr = clrGreen;
        }
        else
        {
          prop = new PropParalyzed(PropTurns::std);
          prop->turnsLeft_ *= 2;
        }
        Explosion::runExplosionAt(pos_, ExplType::applyProp, ExplSrc::misc, 0,
                                  SfxId::END, prop, &fumeClr);
      }
      else
      {
        for (int i = 0; i < int(ActorId::END); ++i)
        {
          const ActorDataT& d = ActorData::data[i];
          if (d.intrProps[propOoze] &&
              d.isAutoSpawnAllowed  &&
              !d.isUnique)
          {
            actorBucket.push_back(ActorId(i));
          }
        }
        Log::addMsg("Something creeps up from the tomb!");
      }
      break;

    default: {} break;
  }

  if (!actorBucket.empty())
  {
    const size_t  ELEMENT         = Rnd::range(0, actorBucket.size() - 1);
    const ActorId actorIdToSpawn  = actorBucket[ELEMENT];
    Actor* const  mon             = ActorFactory::mk(actorIdToSpawn, pos_);
    static_cast<Mon*>(mon)->becomeAware(false);
  }
}

//--------------------------------------------------------------------- CHEST
Chest::Chest(const Pos & pos) :
  Rigid(pos),
  isContentKnown_(false),
  isLocked_(false),
  isTrapped_(false),
  isTrapStatusKnown_(false),
  matl_(ChestMatl(Rnd::range(0, int(ChestMatl::END) - 1)))
{

  const bool IS_TREASURE_HUNTER = PlayerBon::traitsPicked[int(Trait::treasureHunter)];
  const int NR_ITEMS_MIN        = Rnd::oneIn(10) ? 0 : 1;
  const int NR_ITEMS_MAX        = IS_TREASURE_HUNTER ? 3 : 2;
  itemContainer_.setRandomItemsForFeature(FeatureId::chest,
                                          Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));

  if (!itemContainer_.items_.empty())
  {
    isLocked_   = Rnd::fraction(6, 10);
    isTrapped_  = Rnd::fraction(6, 10);
  }
}

void Chest::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                  Actor * const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

void Chest::bump(Actor & actorBumping)
{
  if (actorBumping.isPlayer())
  {
    if (itemContainer_.items_.empty() && isContentKnown_)
    {
      Log::addMsg("The chest is empty.");
    }
    else
    {
      if (isLocked_) {Log::addMsg("The chest is locked.");}
      else {open();}
      GameTime::actorDidAct();
    }
  }
}

void Chest::trySprainPlayer()
{
  const int SPRAIN_ONE_IN_N = PlayerBon::traitsPicked[int(Trait::rugged)] ? 6 :
                              PlayerBon::traitsPicked[int(Trait::tough)]  ? 5 : 4;
  if (Rnd::oneIn(SPRAIN_ONE_IN_N))
  {
    Log::addMsg("I sprain myself.", clrMsgBad);
    Map::player->hit(Rnd::range(1, 5), DmgType::pure);
  }
}

bool Chest::open()
{
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;

  if (IS_SEEN) Log::addMsg("The chest opens.");
  triggerTrap(*Map::player);

  if (Map::player->isAlive())
  {
    if (itemContainer_.items_.empty())
    {
      if (IS_SEEN) Log::addMsg("There is nothing of value inside.");
    }
    else
    {
      if (IS_SEEN) Log::addMsg("There are some items inside.");
      itemContainer_.dropItems(pos_);
    }
  }
  if (IS_SEEN)
  {
    isContentKnown_     = true;
    isTrapStatusKnown_  = true;
  }
  isLocked_ = false;
  Render::drawMapAndInterface();
  return true;
}

//void Chest::hit_(const DmgType dmgType, const DmgMethod dmgMethod,
//                 Actor* const actor) {
//  (void)actor;
//
//  switch(dmgType) {
//    case DmgType::physical: {
//      switch(dmgMethod) {
//        case DmgMethod::kick: {
//          if(itemContainer_.items_.empty() && isContentKnown_) {
//            Log::addMsg("The chest is empty.");
//          } else {
//
//            Log::addMsg("I kick the lid.");
//
//            bool props[endOfPropIds];
//            Map::player->getPropHandler().getAllActivePropIds(props);
//
//            if(
//              find(begin(props), end(props), propWeakened) != end(props) ||
//              matl == ChestMatl::iron) {
//              trySprainPlayer();
//              Log::addMsg("It seems futile.");
//            } else {
//
//              const bool IS_CURSED
//                = find(begin(props), end(props), propCursed)  != end(props);
//              const bool IS_BLESSED
//                = find(begin(props), end(props), propBlessed) != end(props);
//
//              if(!IS_BLESSED && (IS_CURSED || Rnd::oneIn(3))) {
//                itemContainer_.destroySingleFragile();
//              }
//
//              const bool IS_TOUGH   = PlayerBon::traitsPicked[int(Trait::tough)];
//              const bool IS_RUGGED  = PlayerBon::traitsPicked[int(Trait::rugged)];
//
//              const int OPEN_ONE_IN_N = IS_RUGGED ? 2 : IS_TOUGH ? 3 : 5;
//
//              if(Rnd::oneIn(OPEN_ONE_IN_N)) {
//                Log::addMsg("I kick the lid open!");
//                open();
//              } else {
//                Log::addMsg("The lock resists.");
//                trySprainPlayer();
//              }
//            }
//            GameTime::actorDidAct();
//          }
//          break;
//
//        default: {  break;
//
//      } //dmgMethod
//
//      break;
//
//    default: {  break;
//
//  } //dmgType
//
//  //TODO Force lock with weapon
////      Inventory& inv    = Map::player->getInv();
////      Item* const item  = inv.getItemInSlot(SlotId::wielded);
////
////      if(!item) {
////        Log::addMsg(
////          "I attempt to punch the lock open, nearly breaking my hand.",
////          clrMsgBad);
////        Map::player->hit(1, DmgType::pure, false);
////      } else {
////        const int CHANCE_TO_DMG_WPN = IS_BLESSED ? 1 : (IS_CURSED ? 80 : 15);
////
////        if(Rnd::percentile() < CHANCE_TO_DMG_WPN) {
////          const string wpnName = ItemData::getItemRef(
////                                   *item, ItemRefType::plain, true);
////
////          Wpn* const wpn = static_cast<Wpn*>(item);
////
////          if(wpn->meleeDmgPlus == 0) {
////            Log::addMsg("My " + wpnName + " breaks!");
////            delete wpn;
////            inv.getSlot(SlotId::wielded)->item = nullptr;
////          } else {
////            Log::addMsg("My " + wpnName + " is damaged!");
////            wpn->meleeDmgPlus--;
////          }
////          return;
////        }
////
////        if(IS_WEAK) {
////          Log::addMsg("It seems futile.");
////        } else {
////          const int CHANCE_TO_OPEN = 40;
////          if(Rnd::percentile() < CHANCE_TO_OPEN) {
////            Log::addMsg("I force the lock open!");
////            open();
////          } else {
////            Log::addMsg("The lock resists.");
////          }
////        }
////      }
//}

void Chest::examine()
{
  bool props[endOfPropIds];
  Map::player->getPropHandler().getAllActivePropIds(props);

  if (props[propConfused])
  {
    Log::addMsg("I start to search the chest...");
    Log::addMsg("but I cannot grasp the purpose.");
    GameTime::actorDidAct();
  }
  else if (itemContainer_.items_.empty() && isContentKnown_)
  {
    Log::addMsg("The chest is empty.");
  }
  else
  {
    if (isLocked_)
    {
      Log::addMsg("The chest is locked.");
    }

    const int FIND_ONE_IN_N = PlayerBon::traitsPicked[int(Trait::perceptive)] ? 3 :
                              (PlayerBon::traitsPicked[int(Trait::observant)] ? 4 : 7);

    if (isTrapped_ && (isTrapStatusKnown_ || (Rnd::oneIn(FIND_ONE_IN_N))))
    {
      Log::addMsg("There appears to be a hidden trap mechanism!");
      isTrapStatusKnown_ = true;
    }
    else
    {
      Log::addMsg("I find nothing unusual about it.");
    }
    GameTime::actorDidAct();
  }
}

void Chest::disarm()
{
  //Try disarming trap
  if (isTrapped_ && isTrapStatusKnown_)
  {
    Log::addMsg("I attempt to disarm the trap.");

    const int TRIGGER_ONE_IN_N = 5;
    if (Rnd::oneIn(TRIGGER_ONE_IN_N))
    {
      Log::addMsg("I set off the trap!");
      triggerTrap(*Map::player);
    }
    else
    {

      const int DISARM_ONE_IN_N = 2;

      if (Rnd::oneIn(DISARM_ONE_IN_N))
      {
        Log::addMsg("I successfully disarm it!");
        isTrapped_ = false;
      }
      else
      {
        Log::addMsg("I failed to disarm it.");
      }
    }
    GameTime::actorDidAct();
    return;
  }


  //Try picking the lock //TODO Implement
//  if(isLocked_) {
//      GameTime::actorDidAct();
//  }

  //If no other action was taken, try examining the chest instead
  examine();
}

void Chest::triggerTrap(Actor & actor)
{
  (void)actor;

  isTrapStatusKnown_ = true;

  if (isTrapped_)
  {
    isTrapped_ = false;

    const int EXPLODE_ONE_IN_N = 7;

    if (Map::dlvl >= MIN_DLVL_HARDER_TRAPS && Rnd::oneIn(EXPLODE_ONE_IN_N))
    {
      Log::addMsg("The trap explodes!");
      Explosion::runExplosionAt(pos_, ExplType::expl, ExplSrc::misc, 0,
                                SfxId::explosion);
      if (Map::player->isAlive())
      {
        Map::put(new RubbleLow(pos_));
      }
    }
    else
    {
      Log::addMsg("Fumes burst out from the chest!");
      Prop* prop = nullptr;
      Clr fumeClr = clrMagenta;
      const int RND = Rnd::percentile();
      if (RND < 20)
      {
        prop = new PropPoisoned(PropTurns::std);
        fumeClr = clrGreenLgt;
      }
      else if (RND < 40)
      {
        prop = new PropDiseased(PropTurns::specific, 50);
        fumeClr = clrGreen;
      }
      else
      {
        prop = new PropParalyzed(PropTurns::std);
        prop->turnsLeft_ *= 2;
      }
      Explosion::runExplosionAt(pos_, ExplType::applyProp, ExplSrc::misc,
                                0, SfxId::END, prop, &fumeClr);
    }
  }
}

string Chest::getName(const Article article) const
{
  string ret = article == Article::a ?
               "the " : (matl_ == ChestMatl::wood ? "a " : "an ");

  ret += matl_ == ChestMatl::wood ? "wooden " : "iron ";

  return ret + "chest";
}

Clr Chest::getClr_() const
{
  return matl_ == ChestMatl::wood ? clrBrownDrk : clrGray;
}

//--------------------------------------------------------------------- FOUNTAIN
Fountain::Fountain(const Pos & pos) :
  Rigid(pos),
  isDried_(false),
  fountainEffects_(vector<FountainEffect>()),
  fountainMatl_(FountainMatl::stone)
{
  if (Rnd::oneIn(14))
  {
    fountainMatl_ = FountainMatl::gold;
  }

  switch (fountainMatl_)
  {
    case FountainMatl::stone:
      if (Rnd::fraction(5, 6))
      {
        fountainEffects_.push_back(FountainEffect::refreshing);
      }
      else
      {
        const int   NR_TYPES  = int(FountainEffect::END);
        const auto  effect    = FountainEffect(Rnd::range(0, NR_TYPES - 1));
        fountainEffects_.push_back(effect);
      }
      break;

    case FountainMatl::gold:
      vector<FountainEffect> effectBucket
      {
        FountainEffect::bless, FountainEffect::refreshing, FountainEffect::spirit,
        FountainEffect::vitality, FountainEffect::rFire, FountainEffect::rCold,
        FountainEffect::rElec, FountainEffect::rFear, FountainEffect::rConfusion
      };

      std::random_shuffle(begin(effectBucket), end(effectBucket));

      const int NR_EFFECTS = 3;

      for (int i = 0; i < NR_EFFECTS; ++i)
      {
        fountainEffects_.push_back(effectBucket[i]);
      }
      break;
  }
}

void Fountain::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                     Actor * const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

Clr Fountain::getClr_() const
{
  if (isDried_)
  {
    return clrGray;
  }
  else
  {
    switch (fountainMatl_)
    {
      case FountainMatl::stone: return clrBlueLgt;
      case FountainMatl::gold:  return clrYellow;
    }
  }
  assert("Failed to get fountain color" && false);
  return clrBlack;
}

string Fountain::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  switch (fountainMatl_)
  {
    case FountainMatl::stone: {}                break;
    case FountainMatl::gold:  ret += "golden "; break;
  }
  return ret + "fountain";
}

void Fountain::bump(Actor & actorBumping)
{
  if (actorBumping.isPlayer())
  {
    if (isDried_)
    {
      Log::addMsg("The fountain is dried out.");
    }
    else
    {
      PropHandler& propHlr = Map::player->getPropHandler();

      Log::addMsg("I drink from the fountain...");

      for (auto effect : fountainEffects_)
      {
        switch (effect)
        {
          case FountainEffect::tepid:
          {
            Log::addMsg("The water is tepid.");
          } break;

          case FountainEffect::refreshing:
          {
            Log::addMsg("It's very refreshing.");
            Map::player->restoreHp(1, false);
            Map::player->restoreSpi(1, false);
            Map::player->restoreShock(5, false);
          } break;

          case FountainEffect::bless:
          {
            propHlr.tryApplyProp(new PropBlessed(PropTurns::std));
          } break;

          case FountainEffect::curse:
          {
            propHlr.tryApplyProp(new PropCursed(PropTurns::std));
          } break;

          case FountainEffect::spirit:
          {
            Map::player->restoreSpi(2, true, true);
          } break;

          case FountainEffect::vitality:
          {
            Map::player->restoreHp(2, true, true);
          } break;

          case FountainEffect::disease:
          {
            propHlr.tryApplyProp(new PropDiseased(PropTurns::specific, 50));
          } break;

          case FountainEffect::poison:
          {
            propHlr.tryApplyProp(new PropPoisoned(PropTurns::std));
          } break;

          case FountainEffect::frenzy:
          {
            propHlr.tryApplyProp(new PropFrenzied(PropTurns::std));
          } break;

          case FountainEffect::paralyze:
          {
            propHlr.tryApplyProp(new PropParalyzed(PropTurns::std));
          } break;

          case FountainEffect::blind:
          {
            propHlr.tryApplyProp(new PropBlind(PropTurns::std));
          } break;

          case FountainEffect::faint:
          {
            propHlr.tryApplyProp(new PropFainted(PropTurns::specific, 10));
          } break;

          case FountainEffect::rFire:
          {
            Prop* const prop = new PropRFire(PropTurns::std);
            prop->turnsLeft_ *= 2;
            propHlr.tryApplyProp(prop);
          } break;

          case FountainEffect::rCold:
          {
            Prop* const prop = new PropRCold(PropTurns::std);
            prop->turnsLeft_ *= 2;
            propHlr.tryApplyProp(prop);
          } break;

          case FountainEffect::rElec:
          {
            Prop* const prop = new PropRElec(PropTurns::std);
            prop->turnsLeft_ *= 2;
            propHlr.tryApplyProp(prop);
          } break;

          case FountainEffect::rConfusion:
          {
            Prop* const prop = new PropRConfusion(PropTurns::std);
            prop->turnsLeft_ *= 2;
            propHlr.tryApplyProp(prop);
          } break;

          case FountainEffect::rFear:
          {
            Prop* const prop = new PropRFear(PropTurns::std);
            prop->turnsLeft_ *= 2;
            propHlr.tryApplyProp(prop);
          } break;

          case FountainEffect::END: {}
            break;
        }

        if (Rnd::oneIn(5))
        {
          Log::addMsg("The fountain dries out.");
          isDried_ = true;
        }
      }
      GameTime::actorDidAct();
    }
  }
}

//--------------------------------------------------------------------- CABINET
Cabinet::Cabinet(const Pos & pos) : Rigid(pos), isContentKnown_(false)
{
  const int IS_EMPTY_N_IN_10  = 5;
  const int NR_ITEMS_MIN      = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX =
    PlayerBon::traitsPicked[int(Trait::treasureHunter)] ? 2 : 1;

  itemContainer_.setRandomItemsForFeature(
    FeatureId::cabinet, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
}

void Cabinet::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                    Actor * const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

void Cabinet::bump(Actor & actorBumping)
{
  if (actorBumping.isPlayer())
  {
    if (itemContainer_.items_.empty() && isContentKnown_)
    {
      Log::addMsg("The cabinet is empty.");
    }
    else
    {
      open();
    }
  }
}

bool Cabinet::open()
{
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;

  if (IS_SEEN) Log::addMsg("The cabinet opens.");

  if (itemContainer_.items_.size() > 0)
  {
    if (IS_SEEN) Log::addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_);
  }
  else
  {
    if (IS_SEEN) Log::addMsg("There is nothing of value inside.");
  }
  if (IS_SEEN) isContentKnown_ = true;
  Render::drawMapAndInterface(true);
  return true;
}

string Cabinet::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  if (getBurnState() == BurnState::burning) {ret += "burning ";}

  return ret + "cabinet";
}

Clr Cabinet::getClr_() const
{
  return clrBrownDrk;
}

//--------------------------------------------------------------------- COCOON
Cocoon::Cocoon(const Pos & pos) : Rigid(pos), isContentKnown_(false)
{
  const bool IS_TREASURE_HUNTER = PlayerBon::traitsPicked[int(Trait::treasureHunter)];
  const int IS_EMPTY_N_IN_10    = 6;
  const int NR_ITEMS_MIN        = Rnd::fraction(IS_EMPTY_N_IN_10, 10) ? 0 : 1;
  const int NR_ITEMS_MAX        = NR_ITEMS_MIN + (IS_TREASURE_HUNTER ? 1 : 0);
  itemContainer_.setRandomItemsForFeature(
    FeatureId::cocoon, Rnd::range(NR_ITEMS_MIN, NR_ITEMS_MAX));
}

void Cocoon::onHit(const DmgType dmgType, const DmgMethod dmgMethod,
                   Actor * const actor)
{
  (void)dmgType; (void)dmgMethod; (void)actor;
}

void Cocoon::bump(Actor & actorBumping)
{
  if (actorBumping.isPlayer())
  {
    if (itemContainer_.items_.empty() && isContentKnown_)
    {
      Log::addMsg("The cocoon is empty.");
    }
    else
    {
      open();
    }
  }
}

void Cocoon::triggerTrap(Actor & actor)
{
  (void)actor;

  const int RND = Rnd::percentile();

  if (RND < 15)
  {
    Log::addMsg("There is a half-dissolved human body inside!");
    Map::player->incrShock(ShockValue::heavy, ShockSrc::misc);
  }
  else if (RND < 50)
  {
    TRACE << "Attempting to spawn spiders" << endl;
    vector<ActorId> spawnBucket;
    for (int i = 0; i < int(ActorId::END); ++i)
    {
      const ActorDataT& d = ActorData::data[i];

      if (d.isSpider                      &&
          d.actorSize == actorSize_floor  &&
          !d.isAutoSpawnAllowed           &&
          d.isUnique)
      {
        spawnBucket.push_back(d.id);
      }
    }

    const int NR_CANDIDATES = spawnBucket.size();
    if (NR_CANDIDATES > 0)
    {
      TRACE << "Spawn candidates found, attempting to place" << endl;
      Log::addMsg("There are spiders inside!");
      const int NR_SPIDERS = Rnd::range(2, 5);
      const int ELEMENT = Rnd::range(0, NR_CANDIDATES - 1);
      const ActorId actorIdToSummon = spawnBucket[ELEMENT];
      ActorFactory::summonMon(pos_, vector<ActorId>(NR_SPIDERS, actorIdToSummon), true);
    }
  }
}

bool Cocoon::open()
{
  const bool IS_SEEN = Map::cells[pos_.x][pos_.y].isSeenByPlayer;
  if (IS_SEEN) Log::addMsg("The cocoon opens.");
  triggerTrap(*Map::player);
  if (itemContainer_.items_.size() > 0)
  {
    if (IS_SEEN) Log::addMsg("There are some items inside.");
    itemContainer_.dropItems(pos_);
  }
  else
  {
    if (IS_SEEN) Log::addMsg("There is nothing of value inside.");
  }
  if (IS_SEEN) isContentKnown_ = true;
  Render::drawMapAndInterface(true);

  return true;
}

string Cocoon::getName(const Article article) const
{
  string ret = article == Article::a ? "a " : "the ";

  if (getBurnState() == BurnState::burning) {ret += "burning ";}

  return ret + "cocoon";
}

Clr Cocoon::getClr_() const
{
  return clrWhite;
}


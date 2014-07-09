#include "FeatureStatic.h"

#include <string>

#include "Init.h"
#include "Log.h"
#include "Renderer.h"
#include "Map.h"
#include "Utils.h"
#include "Popup.h"
#include "DungeonClimb.h"
#include "SaveHandling.h"
#include "FeatureStatic.h"
#include "ItemFactory.h"

using namespace std;

//------------------------------------------------------------------- STATIC FEATURE
void FeatureStatic::examine() {
  Log::addMsg("I find nothing specific there to examine or use.");
}

void FeatureStatic::disarm() {
  Log::addMsg(msgDisarmNoTrap);
  Renderer::drawMapAndInterface();
}

void FeatureStatic::bash(Actor& actorTrying) {
  if(&actorTrying == Map::player) {
    const bool IS_BLIND    = !Map::player->getPropHandler().allowSee();
    const bool IS_BLOCKING = !canMoveCmn() && getId() != FeatureId::stairs;
    if(IS_BLOCKING) {
      Log::addMsg("I smash into " + (IS_BLIND ? " something" : getDescr(false)) + "!");

      if(Rnd::oneIn(4)) {
        Log::addMsg("I sprain myself.", clrMsgBad);
        const int SPRAIN_DMG = Rnd::range(1, 5);
        actorTrying.hit(SPRAIN_DMG, DmgType::pure, false);
      }

      if(Rnd::oneIn(4)) {
        Log::addMsg("I am off-balance.");

        actorTrying.getPropHandler().tryApplyProp(
          new PropParalyzed(propTurnsSpecific, 2));
      }

    } else {
      Log::addMsg("I kick the air!");
      Audio::play(SfxId::missMedium);
    }
  }

  bash_(actorTrying);

  GameTime::actorDidAct();

  Map::player->updateFov();
  Renderer::drawMapAndInterface();
}

void FeatureStatic::bash_(Actor& actorTrying) {
  //Emitting the sound from the actor instead of the bashed object, because the
  //sound massage should be received even if the object is seen
  const AlertsMonsters alertsMonsters = &actorTrying == Map::player ?
                                        AlertsMonsters::yes :
                                        AlertsMonsters::no;
  Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, actorTrying.pos,
          &actorTrying, SndVol::low, alertsMonsters);
  SndEmit::emitSnd(snd);
}

void FeatureStatic::tryPutGore() {
  if(getData().canHaveGore) {
    const int ROLL_GLYPH = Rnd::dice(1, 4);
    switch(ROLL_GLYPH) {
      case 1: {goreGlyph_ = ',';} break;
      case 2: {goreGlyph_ = '`';} break;
      case 3: {goreGlyph_ = 39;}  break;
      case 4: {goreGlyph_ = ';';} break;
    }

    const int ROLL_TILE = Rnd::dice(1, 8);
    switch(ROLL_TILE) {
      case 1: {goreTile_ = TileId::gore1;} break;
      case 2: {goreTile_ = TileId::gore2;} break;
      case 3: {goreTile_ = TileId::gore3;} break;
      case 4: {goreTile_ = TileId::gore4;} break;
      case 5: {goreTile_ = TileId::gore5;} break;
      case 6: {goreTile_ = TileId::gore6;} break;
      case 7: {goreTile_ = TileId::gore7;} break;
      case 8: {goreTile_ = TileId::gore8;} break;
    }
  }
}

string FeatureStatic::getDescr(const bool DEFINITE_ARTICLE) const {
  if(goreGlyph_ == ' ') {
    return DEFINITE_ARTICLE ? getData().nameThe : getData().nameA;
  } else {
    return DEFINITE_ARTICLE ? "the blood and gore" : "blood and gore";
  }
}

void FeatureStatic::clearGore() {
  goreTile_   = TileId::empty;
  goreGlyph_  = ' ';
  hasBlood_   = false;
}

//------------------------------------------------------------------- WALL
void Wall::hit(const DmgType dmgType, const DmgMethod dmgMethod) {
  if(dmgType == DmgType::physical) {
    if(
      (dmgMethod == DmgMethod::explosion   && Rnd::fraction(3, 4)) ||
      (dmgMethod == DmgMethod::bluntHeavy  && Rnd::oneIn(4))) {
      destroy(dmgType);
    }
  }
}

void Wall::destroy(const DmgType dmgType) {
  (void)dmgType;

  //First, destroy any cardinally adjacent doors
  for(const Pos& d : DirUtils::cardinalList) {
    const Pos p(pos_ + d);
    if(Utils::isPosInsideMap(p)) {
      if(Map::cells[p.x][p.y].featureStatic->getId() == FeatureId::door) {
        Map::put(new RubbleLow(p));
      }
    }
  }

  if(Rnd::coinToss()) {
    Map::put(new RubbleHigh(pos_));
  } else {
    const Pos pos = pos_;

    Map::put(new RubbleLow(pos_)); //Note: "this" is now deleted!

    if(Rnd::coinToss()) {
      ItemFactory::mkItemOnMap(ItemId::rock, pos);
    }
  }
}

bool Wall::isTileAnyWallFront(const TileId tile) {
  return
    tile == TileId::wallFront      ||
    tile == TileId::wallFrontAlt1  ||
    tile == TileId::wallFrontAlt2  ||
    tile == TileId::caveWallFront  ||
    tile == TileId::egyptWallFront;
}

bool Wall::isTileAnyWallTop(const TileId tile) {
  return
    tile == TileId::wallTop      ||
    tile == TileId::caveWallTop  ||
    tile == TileId::egyptWallTop ||
    tile == TileId::rubbleHigh;
}

string Wall::getDescr(const bool DEFINITE_ARTICLE) const {
  const string modStr   = isMossy_ ? "moss-grown " : "";
  const string article  = (DEFINITE_ARTICLE ? "the " : "a ");

  switch(type_) {
    case WallType::cmn:
    case WallType::cmnAlt: {return article + modStr + "stone wall";}
    case WallType::cave:   {return article + modStr + "cavern wall";}
    case WallType::egypt:  {return article + modStr + "stone wall";}
  }
  assert(false && "Failed to get door description");
  return "";
}

SDL_Color Wall::getClr() const {
  if(isMossy_)                  {return clrGreenDrk;}
  if(type_ == WallType::cave)   {return clrBrownGray;}
  if(type_ == WallType::egypt)  {return clrBrownGray;}
  return getData().clr;
}

char Wall::getGlyph() const {
  return Config::isAsciiWallFullSquare() ? 10 : '#';
}

TileId Wall::getFrontWallTile() const {
  if(Config::isTilesWallFullSquare()) {
    switch(type_) {
      case WallType::cmn:     return TileId::wallTop;         break;
      case WallType::cmnAlt:  return TileId::wallTop;         break;
      case WallType::cave:    return TileId::caveWallTop;     break;
      case WallType::egypt:   return TileId::egyptWallTop;    break;
      default:                return TileId::wallTop;         break;
    }
  } else {
    switch(type_) {
      case WallType::cmn:     return TileId::wallFront;       break;
      case WallType::cmnAlt:  return TileId::wallFrontAlt1;   break;
      case WallType::cave:    return TileId::caveWallFront;   break;
      case WallType::egypt:   return TileId::egyptWallFront;  break;
      default:                return TileId::wallFront;       break;
    }
  }
}

TileId Wall::getTopWallTile() const {
  switch(type_) {
    case WallType::cmn:       return TileId::wallTop;         break;
    case WallType::cmnAlt:    return TileId::wallTop;         break;
    case WallType::cave:      return TileId::caveWallTop;     break;
    case WallType::egypt:     return TileId::egyptWallTop;    break;
    default:                  return TileId::wallTop;         break;
  }
}

void Wall::setRandomNormalWall() {
  const int RND = Rnd::range(1, 6);
  switch(RND) {
    case 1:   type_ = WallType::cmnAlt;     break;
    default:  type_ = WallType::cmn;   break;
  }
}

void Wall::setRandomIsMossGrown() {
  isMossy_ = Rnd::oneIn(40);
}

//------------------------------------------------------------------- HIGH RUBBLE
void RubbleHigh::hit(const DmgType type, const DmgMethod method) {
  if(type == DmgType::physical) {
    if(
      method == DmgMethod::explosion ||
      (method == DmgMethod::bluntHeavy  && Rnd::coinToss())) {

      const Pos pos = pos_;

      Map::put(new RubbleLow(pos_)); //Note: "this" is now deleted!

      if(Rnd::coinToss()) {
        ItemFactory::mkItemOnMap(ItemId::rock, pos);
      }
    }
  }
}

//------------------------------------------------------------------- GRAVE
string GraveStone::getDescr(const bool DEFINITE_ARTICLE) const {
  return (DEFINITE_ARTICLE ? getData().nameThe : getData().nameA) + "; " + inscr_;
}

void GraveStone::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {Log::addMsg(inscr_);}
}

//------------------------------------------------------------------- STAIRS
void Stairs::bump(Actor& actorBumping) {
  if(&actorBumping == Map::player) {

    const vector<string> choices {"Descend", "Save and quit", "Cancel"};
    const int CHOICE =
      Popup::showMenuMsg("", true, choices, "A staircase leading downwards");

    if(CHOICE == 0) {
      Map::player->pos = pos_;
      TRACE << "Calling DungeonClimb::tryUseDownStairs()" << endl;
      DungeonClimb::tryUseDownStairs();
    } else if(CHOICE == 1) {
      Map::player->pos = pos_;
      SaveHandling::save();
      Init::quitToMainMenu = true;
    } else {
      Log::clearLog();
      Renderer::drawMapAndInterface();
    }
  }
}

//------------------------------------------------------------------- BRIDGE
TileId Bridge::getTile() const {
  return dir_ == hor ? TileId::hangbridgeHor : TileId::hangbridgeVer;
}

char Bridge::getGlyph() const {
  return dir_ == hor ? '|' : '=';
}

//------------------------------------------------------------------- SHALLOW LIQUID
void LiquidShallow::bump(Actor& actorBumping) {
  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(
    find(begin(props), end(props), propEthereal)  == end(props) &&
    find(begin(props), end(props), propFlying)    == end(props)) {

    actorBumping.getPropHandler().tryApplyProp(
      new PropWaiting(propTurnsStd));

    if(&actorBumping == Map::player) Log::addMsg("*glop*");
  }
}

//------------------------------------------------------------------- DEEP LIQUID
void LiquidDeep::bump(Actor& actorBumping) {
  (void)actorBumping;
}

//------------------------------------------------------------------- LEVER
SDL_Color Lever::getClr() const {
  return isPositionLeft_ ? clrGray : clrWhite;
}

TileId Lever::getTile() const {
  return isPositionLeft_ ? TileId::leverLeft : TileId::leverRight;
}

void Lever::examine() {
  pull();
}

void Lever::pull() {
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
  Renderer::drawMapAndInterface();
  TRACE_FUNC_END;
}

#include "FeatureWall.h"

#include <string>
#include <assert.h>

#include "Map.h"
#include "Utils.h"

using namespace std;

Wall::Wall(FeatureId id, Pos pos) :
  FeatureStatic(id, pos), wallType(wall_common), isMossGrown(false) {
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
  const string modStr   = isMossGrown ? "moss-grown " : "";
  const string article  = (DEFINITE_ARTICLE ? "the " : "a ");

  switch(wallType) {
    case wall_common:
    case wall_alt1:   {return article + modStr + "stone wall";}
    case wall_cave:   {return article + modStr + "cavern wall";}
    case wall_egypt:  {return article + modStr + "stone wall";}
  }
  assert(false && "Failed to get door description");
  return "";
}

SDL_Color Wall::getClr() const {
  if(isMossGrown)             {return clrGreen;}
  if(wallType == wall_cave)   {return clrBrownGray;}
  if(wallType == wall_egypt)  {return clrBrownGray;}
  return data_->color;
}

char Wall::getGlyph() const {
  return Config::isAsciiWallFullSquare() ? 10 : '#';
}

TileId Wall::getFrontWallTile() const {
  if(Config::isTilesWallFullSquare()) {
    switch(wallType) {
      case wall_common:   return TileId::wallTop;        break;
      case wall_alt1:     return TileId::wallTop;        break;
      case wall_cave:     return TileId::caveWallTop;    break;
      case wall_egypt:    return TileId::egyptWallTop;   break;
      default:            return TileId::wallTop;        break;
    }
  } else {
    switch(wallType) {
      case wall_common:   return TileId::wallFront;        break;
      case wall_alt1:     return TileId::wallFrontAlt1;    break;
      case wall_cave:     return TileId::caveWallFront;    break;
      case wall_egypt:    return TileId::egyptWallFront;   break;
      default:            return TileId::wallFront;        break;
    }
  }
}

TileId Wall::getTopWallTile() const {
  switch(wallType) {
    case wall_common:   return TileId::wallTop;        break;
    case wall_alt1:     return TileId::wallTop;        break;
    case wall_cave:     return TileId::caveWallTop;    break;
    case wall_egypt:    return TileId::egyptWallTop;   break;
    default:            return TileId::wallTop;        break;
  }
}

void Wall::setRandomNormalWall() {
  const int RND = Rnd::range(1, 6);
  switch(RND) {
    case 1:   wallType = wall_alt1;     break;
    default:  wallType = wall_common;   break;
  }
}

void Wall::setRandomIsMossGrown() {
  isMossGrown = Rnd::oneIn(40);
}



#include "FeatureWall.h"

#include <assert.h>

#include "Engine.h"

#include "Map.h"

Wall::Wall(FeatureId id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine), wallType(wall_common), isMossGrown(false) {
}

bool Wall::isTileAnyWallFront(const Tile tile) {
  return
    tile == tile_wallFront      ||
    tile == tile_wallFrontAlt1  ||
    tile == tile_wallFrontAlt2  ||
    tile == tile_caveWallFront  ||
    tile == tile_egyptWallFront;
}

bool Wall::isTileAnyWallTop(const Tile tile) {
  return
    tile == tile_wallTop      ||
    tile == tile_caveWallTop  ||
    tile == tile_egyptWallTop ||
    tile == tile_rubbleHigh;
}

string Wall::getDescr(const bool DEFINITE_ARTICLE) const {
  const string modStr   = isMossGrown ? "moss-grown " : "";
  const string article  = (DEFINITE_ARTICLE ? "the " : "a ");

  switch(wallType) {
    case wall_common:
    case wall_alt1: {
      return article + modStr + "stone wall";
    }

    case wall_cave: {
      return article + modStr + "cavern wall";
    }

    case wall_egypt: {
      return article + modStr + "stone wall";
    }

  }
  assert(false && "Failed to get door description");
}

SDL_Color Wall::getColor() const {
  if(isMossGrown)             {return clrGreen;}
  if(wallType == wall_cave)   {return clrBrownGray;}
  if(wallType == wall_egypt)  {return clrBrownGray;}
  return data_->color;
}

char Wall::getGlyph() const {
  return eng.config->isAsciiWallSymbolFullSquare ? 10 : '#';
}

Tile Wall::getFrontWallTile() const {
  if(eng.config->isTilesWallSymbolFullSquare) {
    switch(wallType) {
      case wall_common:   return tile_wallTop;        break;
      case wall_alt1:     return tile_wallTop;        break;
      case wall_cave:     return tile_caveWallTop;    break;
      case wall_egypt:    return tile_egyptWallTop;   break;
      default:            return tile_wallTop;        break;
    }
  } else {
    switch(wallType) {
      case wall_common:   return tile_wallFront;        break;
      case wall_alt1:     return tile_wallFrontAlt1;    break;
      case wall_cave:     return tile_caveWallFront;    break;
      case wall_egypt:    return tile_egyptWallFront;   break;
      default:            return tile_wallFront;        break;
    }
  }
}

Tile Wall::getTopWallTile() const {
  switch(wallType) {
    case wall_common:   return tile_wallTop;        break;
    case wall_alt1:     return tile_wallTop;        break;
    case wall_cave:     return tile_caveWallTop;    break;
    case wall_egypt:    return tile_egyptWallTop;   break;
    default:            return tile_wallTop;        break;
  }
}

void Wall::setRandomNormalWall() {
  const int RND = eng.dice.range(1, 6);
  switch(RND) {
    case 1:   wallType = wall_alt1;     break;
    default:  wallType = wall_common;   break;
  }
}

void Wall::setRandomIsMossGrown() {
  isMossGrown = eng.dice.oneIn(40);
}



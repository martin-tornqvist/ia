#include "FeatureWall.h"

#include "Engine.h"

#include "Map.h"

Wall::Wall(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine), wallType(wall_common), isSlimy(false) {

}

string Wall::getDescription(const bool DEFINITE_ARTICLE) const {
  switch(wallType) {
  case wall_common:
  case wall_alt1:
  /*case wall_alt2:*/ {
    const string modStr = isSlimy ? "slimy " : "";
    return DEFINITE_ARTICLE ? "the " + modStr + "stone wall" : "a " + modStr + "stone wall";
  }
  break;
  case wall_cave: {
    const string modStr = isSlimy ? "slimy " : "";
    return DEFINITE_ARTICLE ? "the " + modStr + "cavern wall" : "a " + modStr + "cavern wall";

  }
  }
  return "[ERROR]";
}

sf::Color Wall::getColor() const {
  return isSlimy ? clrGreen : clrGray;
}

char Wall::getGlyph() const {
  if(eng->config->WALL_SYMBOL_FULL_SQUARE) {
    return 10;
  } else {
    return '#';
  }
}

Tile_t Wall::getFrontWallTile() const {
  switch(wallType) {
  case wall_common: return tile_wallFront; break;
  case wall_alt1: return tile_wallFrontAlt1; break;
//  case wall_alt2: return tile_wallFrontAlt2; break;
  case wall_cave: return tile_caveWallFront; break;
  default: return tile_wallFront; break;
  }
}

Tile_t Wall::getTopWallTile() const {
  switch(wallType) {
  case wall_common: return tile_wallTop; break;
  case wall_alt1: return tile_wallTop; break;
//  case wall_alt2: return tile_wallTop; break;
  case wall_cave: return tile_caveWallTop; break;
  default: return tile_wallTop; break;
  }
}

void Wall::setRandomNormalWall() {
  const int RND = eng->dice.getInRange(1, 4);
  switch(RND) {
  case 1: wallType = wall_alt1; break;
//  case 2: wallType = wall_alt2; break;
  default: wallType = wall_common; break;
  }
}

void Wall::setRandomIsSlimy() {
  const int RND = eng->dice.getInRange(1, 30);
  isSlimy = RND == 1;
}



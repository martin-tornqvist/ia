#ifndef FEATURE_WALL_H
#define FEATURE_WALL_H

#include "Feature.h"

enum Wall_t {
  wall_common,
  wall_alt1,
//  wall_alt2,
  wall_cave
};

class Wall: public FeatureStatic {
public:
  ~Wall() {
  }

  string getDescription(const bool DEFINITE_ARTICLE) const;
  sf::Color getColor() const;
  char getGlyph() const;

  Tile_t getFrontWallTile() const;
  Tile_t getTopWallTile() const;

  void setRandomNormalWall();
  void setRandomIsSlimy();

  Wall_t wallType;
  bool isSlimy;

  static bool isTileAnyWallFront(const Tile_t tile) {
    return tile == tile_wallFront || tile == tile_wallFrontAlt1 ||
           tile == tile_wallFrontAlt2 || tile == tile_caveWallFront;
  }

  static bool isTileAnyWallTop(const Tile_t tile) {
    return tile == tile_wallTop || tile == tile_caveWallTop || tile == tile_rubbleHigh;
  }

private:
  friend class FeatureFactory;
  Wall(Feature_t id, coord pos, Engine* engine);
};

#endif

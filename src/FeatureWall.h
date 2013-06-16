#ifndef FEATURE_WALL_H
#define FEATURE_WALL_H

#include "Feature.h"

enum Wall_t {
  wall_common,
  wall_alt1,
  wall_cave,
  wall_egypt
};

class Wall: public FeatureStatic {
public:
  ~Wall() {
  }

  string getDescription(const bool DEFINITE_ARTICLE) const;
  SDL_Color getColor() const;
  char getGlyph() const;

  Tile_t getFrontWallTile() const;
  Tile_t getTopWallTile() const;

  void setRandomNormalWall();
  void setRandomIsMossGrown();

  Wall_t wallType;
  bool isMossGrown;

  static bool isTileAnyWallFront(const Tile_t tile);

  static bool isTileAnyWallTop(const Tile_t tile);

private:
  friend class FeatureFactory;
  Wall(Feature_t id, coord pos, Engine* engine);
};

#endif

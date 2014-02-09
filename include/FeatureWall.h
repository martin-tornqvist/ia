#ifndef FEATURE_WALL_H
#define FEATURE_WALL_H

#include "Feature.h"

enum WallType {
  wall_common,
  wall_alt1,
  wall_cave,
  wall_egypt
};

class Wall: public FeatureStatic {
public:
  ~Wall() {
  }

  string getDescr(const bool DEFINITE_ARTICLE) const override;
  SDL_Color getClr() const;
  char getGlyph() const;

  TileId getFrontWallTile() const;
  TileId getTopWallTile() const;

  void setRandomNormalWall();
  void setRandomIsMossGrown();

  WallType wallType;
  bool isMossGrown;

  static bool isTileAnyWallFront(const TileId tile);

  static bool isTileAnyWallTop(const TileId tile);

private:
  friend class FeatureFactory;
  Wall(FeatureId id, Pos pos, Engine& engine);
};

#endif

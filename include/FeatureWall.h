#ifndef FEATURE_WALL_H
#define FEATURE_WALL_H

#include "Feature.h"

enum class WallType {common, alt1, cave, egypt};

class Wall: public FeatureStatic {
public:
  Wall(FeatureId id, Pos pos);

  ~Wall() {}

  std::string getDescr(const bool DEFINITE_ARTICLE) const override;
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
};

#endif

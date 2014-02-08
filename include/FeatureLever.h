#ifndef FEATURE_LEVER_H
#define FEATURE_LEVER_H

#include "Feature.h"

class Door;
class LeverSpawnData;

class FeatureLever: public FeatureStatic {
public:
  ~FeatureLever() {}
  SDL_Color getColor() const;
  TileId getTile() const;

  void examine() override;

protected:
  friend class FeatureFactory;
  friend class MapGenBsp;
  FeatureLever(FeatureId id, Pos pos, Engine& engine, LeverSpawnData* spawnData);

  void pull();

  bool isPositionLeft_;
  Door* const doorLinkedTo_;
};


#endif

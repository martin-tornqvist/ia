#ifndef FEATURE_LEVER_H
#define FEATURE_LEVER_H

#include "Feature.h"

class Door;
class LeverSpawnData;

class FeatureLever: public FeatureStatic {
public:
  FeatureLever(FeatureId id, Pos pos, LeverSpawnData* spawnData);
  ~FeatureLever() {}
  SDL_Color getClr() const;
  TileId getTile() const;

  void examine() override;

protected:
  void pull();

  bool isPositionLeft_;
  Door* const doorLinkedTo_;
};


#endif

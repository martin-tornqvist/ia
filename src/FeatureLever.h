#ifndef FEATURE_LEVER_H
#define FEATURE_LEVER_H

#include "Feature.h"

class Door;
class LeverSpawnData;

class FeatureLever: public FeatureStatic {
public:
  ~FeatureLever() {
  }
  sf::Color getColor() const;
  Tile_t getTile() const;

  void examine();

protected:
  friend class FeatureFactory;
  friend class MapBuildBSP;
  FeatureLever(Feature_t id, coord pos, Engine* engine, LeverSpawnData* spawnData);

  void pull();

  bool isPositionLeft_;
  Door* const doorLinkedTo_;
};


#endif

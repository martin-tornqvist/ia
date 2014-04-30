#ifndef FEATURE_PROX_EVENT_H
#define FEATURE_PROX_EVENT_H

#include "Feature.h"
#include "FeatureFactory.h"

class ProxEvent: public FeatureMob {
public:
  ~ProxEvent() {}

  void newTurn();

protected:
  ProxEvent(FeatureId id, Pos pos) :
    FeatureMob(id, pos) {}

  virtual void playerIsNear() = 0;
};

class ProxEventWallCrumble: public ProxEvent {
public:
  ProxEventWallCrumble(FeatureId id, Pos pos,
                       ProxEventWallCrumbleSpawnData* spawnData) :
    ProxEvent(id, pos), wallCells_(spawnData->wallCells_),
    innerCells_(spawnData->innerCells_) {}

private:
  void playerIsNear();

  std::vector<Pos> wallCells_;
  std::vector<Pos> innerCells_;
};

#endif

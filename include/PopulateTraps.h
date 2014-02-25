#ifndef POPULATE_TRAPS_H
#define POPULATE_TRAPS_H

#include "RoomTheme.h"
#include "FeatureTrap.h"

class Engine;

class PopulateTraps {
public:
  PopulateTraps(Engine& engine) : eng(engine) {}

  void populateRoomAndCorridorLevel() const;

private:
  void spawnTrapAt(const TrapId id, const Pos& pos) const;

  Engine& eng;
};

#endif

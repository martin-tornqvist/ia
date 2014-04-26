#ifndef POPULATE_TRAPS_H
#define POPULATE_TRAPS_H

#include "RoomTheme.h"
#include "FeatureTrap.h"

class PopulateTraps {
public:
  PopulateTraps() {}

  void populateRoomAndCorridorLevel() const;

private:
  void spawnTrapAt(const TrapId id, const Pos& pos) const;
};

#endif

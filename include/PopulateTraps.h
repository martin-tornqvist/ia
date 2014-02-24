#ifndef POPULATE_TRAPS_H
#define POPULATE_TRAPS_H

#include "RoomTheme.h"

class Engine;

class PopulateTraps {
public:
  PopulateTraps(Engine& engine) : eng(engine) {}

  void populateRoomAndCorridorLevel() const;

private:
  Engine& eng;
};

#endif

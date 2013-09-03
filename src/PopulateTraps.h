#ifndef POPULATE_TRAPS_H
#define POPULATE_TRAPS_H

#include "RoomTheme.h"

class Engine;

class PopulateTraps {
public:
  PopulateTraps(Engine* engine) : eng(engine) {
  }

  void populateRoomAndCorridorLevel(
    RoomTheme_t themeMap[MAP_X_CELLS][MAP_Y_CELLS],
    const vector<Room*>& rooms) const;

private:
  Engine* const eng;
};

#endif

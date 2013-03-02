#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "ActorData.h"

class Engine;


class PopulateMonsters {
public:
  PopulateMonsters(Engine* engine) : eng(engine) {}

  void spawnGroupOfRandomAt(const coord& origin) const;

  void attemptSpawnDueToTimePassed() const;

  void populateRoomAndCorridorLevel(RoomTheme_t themeMap[MAP_X_CELLS][MAP_Y_CELLS], const vector<Room*>& rooms) const;

private:
  bool spawnGroupOfRandomNativeToRoomThemeAt(const RoomTheme_t roomTheme, const vector<coord>& sortedFreeCellsVector,
      bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_ROAMING_ALLOWED) const;

  void spawnGroupAt(const ActorDevNames_t id, const vector<coord>& sortedFreeCellsVector,
                    bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_ROAMING_ALLOWED) const;

  void makeSortedFreeCellsVector(const coord& origin, const bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], vector<coord>& vectorToFill) const;

  int getRandomOutOfDepth() const;

  Engine* eng;
};


#endif

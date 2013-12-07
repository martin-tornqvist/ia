#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "ActorData.h"

class Engine;


class PopulateMonsters {
public:
  PopulateMonsters(Engine* engine) : eng(engine) {}

  void trySpawnDueToTimePassed() const;

  void populateRoomAndCorridorLevel(RoomTheme_t themeMap[MAP_X_CELLS][MAP_Y_CELLS], const vector<Room*>& rooms) const;

  void populateCaveLevel() const;

  void populateIntroLevel();

  void spawnGroupAt(const ActorId_t id, const vector<Pos>& sortedFreeCellsVector,
                    bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_ROAMING_ALLOWED) const;

  void makeSortedFreeCellsVector(const Pos& origin, const bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], vector<Pos>& vectorToSet) const;

private:
  bool spawnGroupOfRandomNativeToRoomThemeAt(const RoomTheme_t roomTheme, const vector<Pos>& sortedFreeCellsVector,
      bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_ROAMING_ALLOWED) const;

  int getRandomOutOfDepth() const;

  void spawnGroupOfRandomAt(const vector<Pos>& sortedFreeCellsVector, bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS],
                            const int NR_LVLS_OUT_OF_DEPTH, const bool IS_ROAMING_ALLOWED) const;

  void makeListOfMonstersEligibleForAutoSpawning(const int NR_LVLS_OUT_OF_DEPTH_ALLOWED, vector<ActorId_t>& listToSet) const;

  Engine* eng;
};


#endif

#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "ActorData.h"

class Engine;


class PopulateMonsters {
public:
  PopulateMonsters(Engine& engine) : eng(engine) {}

  void trySpawnDueToTimePassed() const;

  void populateRoomAndCorridorLevel() const;

  void populateCaveLevel() const;

  void populateIntroLevel();

  void spawnGroupAt(
    const ActorId id, const vector<Pos>& sortedFreeCellsVector,
    bool forbiddenCells[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) const;

  void makeSortedFreeCellsVector(
    const Pos& origin, const bool forbiddenCells[MAP_W][MAP_H],
    vector<Pos>& vectorRef) const;

private:
  bool spawnGroupOfRandomNativeToRoomThemeAt(
    const RoomThemeId roomTheme, const vector<Pos>& sortedFreeCellsVector,
    bool forbiddenCells[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) const;

  int getRandomOutOfDepth() const;

  void spawnGroupOfRandomAt(
    const vector<Pos>& sortedFreeCellsVector, bool forbiddenCells[MAP_W][MAP_H],
    const int NR_LVLS_OUT_OF_DEPTH, const bool IS_ROAMING_ALLOWED) const;

  void makeListOfMonstersEligibleForAutoSpawning(
    const int NR_LVLS_OUT_OF_DEPTH_ALLOWED, vector<ActorId>& listRef) const;

  Engine& eng;
};


#endif

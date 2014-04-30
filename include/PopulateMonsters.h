#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "ActorData.h"




class PopulateMonsters {
public:
  PopulateMonsters() {}

  void trySpawnDueToTimePassed() const;

  void populateRoomAndCorridorLevel() const;

  void populateCaveLevel() const;

  void populateIntroLevel();

  void spawnGroupAt(
    const ActorId id, const std::vector<Pos>& sortedFreeCellsVector,
    bool forbiddenCells[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) const;

  void makeSortedFreeCellsVector(
    const Pos& origin, const bool forbiddenCells[MAP_W][MAP_H],
    std::vector<Pos>& vectorRef) const;

private:
  bool spawnGroupOfRandomNativeToRoomThemeAt(
    const RoomThemeId roomTheme, const std::vector<Pos>& sortedFreeCellsVector,
    bool forbiddenCells[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) const;

  int getRandomOutOfDepth() const;

  void spawnGroupOfRandomAt(
    const std::vector<Pos>& sortedFreeCellsVector, bool forbiddenCells[MAP_W][MAP_H],
    const int NR_LVLS_OUT_OF_DEPTH, const bool IS_ROAMING_ALLOWED) const;

  void makeListOfMonstersEligibleForAutoSpawning(
    const int NR_LVLS_OUT_OF_DEPTH_ALLOWED, std::vector<ActorId>& listRef) const;


};


#endif

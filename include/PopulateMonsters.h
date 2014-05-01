#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "ActorData.h"

namespace PopulateMonsters {

void trySpawnDueToTimePassed();

void populateRoomAndCorridorLevel();

void populateCaveLevel();

void populateIntroLevel();

void spawnGroupAt(const ActorId id,
                  const std::vector<Pos>& sortedFreeCellsVector,
                  bool blockers[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED);

void makeSortedFreeCellsVector(const Pos& origin,
                               const bool blockers[MAP_W][MAP_H],
                               std::vector<Pos>& vectorRef);

} //PopulateMonsters

#endif

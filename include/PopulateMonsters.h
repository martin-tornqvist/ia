#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "ActorData.h"

namespace PopulateMon
{

void trySpawnDueToTimePassed();

void populateStdLvl();

void populateCaveLvl();

void populateIntroLvl();

void mkGroupAt(const ActorId id, const std::vector<Pos>& sortedFreeCellsVector,
               bool blocked[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED);

void mkSortedFreeCellsVector(const Pos& origin,
                             const bool blocked[MAP_W][MAP_H],
                             std::vector<Pos>& vectorRef);

} //PopulateMon

#endif

#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

#include <vector>

#include "CmnData.h"
#include "CmnTypes.h"

//TODO: This should be merged into MapParsing
//Also, most/all things in here really needs clearer purpose/naming

class Room;

enum class PlacementRule
{
  adjToWalls, awayFromWalls, either
};

namespace MapPatterns
{

void getCellsInRoom(const Room& room, std::vector<Pos>& adjToWalls,
                    std::vector<Pos>& awayFromWalls);

int getWalkBlockersInDir(const Dir dir, const Pos& pos);

} //MapPatterns

#endif

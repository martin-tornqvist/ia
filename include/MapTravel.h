#ifndef MAP_TRAVEL_H
#define MAP_TRAVEL_H

#include <vector>
#include <string>

#include "Map.h"

using namespace std;

//This includes forest intro level, rats in the walls level, etc (every level that
//increments the DLVL number).
enum IsMainDungeon {no, yes};

struct MapData
{
    MapType       type;
    IsMainDungeon isMainDungeon;
};

namespace MapTravel
{

extern std::vector<MapData> mapList;

void init();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

void tryUseDownStairs();

void goToNxt();

MapType getMapType();

} //MapTravel

#endif

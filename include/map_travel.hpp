#ifndef MAP_TRAVEL_HPP
#define MAP_TRAVEL_HPP

#include <vector>
#include <string>

#include "map.hpp"

//This includes forest intro level, rats in the walls level, etc (every level
//that increments the dlvl number).
enum IsMainDungeon
{
    no,
    yes
};

struct MapData
{
    MapType        type;
    IsMainDungeon is_main_dungeon;
};

namespace map_travel
{

extern std::vector<MapData> map_list;

void init();

void save();
void load();

void try_use_down_stairs();

void go_to_nxt();

MapType map_type();

} //map_travel

#endif

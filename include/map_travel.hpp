#ifndef MAP_TRAVEL_H
#define MAP_TRAVEL_H

#include <vector>
#include <string>

#include "map.hpp"

//This includes forest intro level, rats in the walls level, etc (every level that
//increments the DLVL number).
enum Is_main_dungeon {no, yes};

struct map_data
{
    Map_type        type;
    Is_main_dungeon is_main_dungeon;
};

namespace map_travel
{

extern std::vector<map_data> map_list;

void init();

void save();
void load();

void try_use_down_stairs();

void go_to_nxt();

Map_type map_type();

} //map_travel

#endif

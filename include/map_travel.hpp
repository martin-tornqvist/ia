#ifndef MAP_TRAVEL_H
#define MAP_TRAVEL_H

#include <vector>
#include <string>

#include "map.hpp"

using namespace std;

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

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

void try_use_down_stairs();

void go_to_nxt();

Map_type get_map_type();

} //map_travel

#endif

#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

#include <vector>

#include "cmn_data.hpp"
#include "cmn_types.hpp"

//TODO: This should be merged into map_parsing
//Also, most/all things in here really needs clearer purpose/naming

class Room;

enum class Placement_rule
{
    adj_to_walls, away_from_walls, either
};

namespace map_patterns
{

void get_cells_in_room(const Room& room, std::vector<Pos>& adj_to_walls,
                       std::vector<Pos>& away_from_walls);

int get_walk_blockers_in_dir(const Dir dir, const Pos& pos);

} //map_patterns

#endif

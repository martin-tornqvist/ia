#ifndef MAP_PATTERNS_H
#define MAP_PATTERNS_H

#include <vector>

#include "Cmn_data.h"
#include "Cmn_types.h"

//TODO: This should be merged into Map_parsing
//Also, most/all things in here really needs clearer purpose/naming

class Room;

enum class Placement_rule
{
    adj_to_walls, away_from_walls, either
};

namespace Map_patterns
{

void get_cells_in_room(const Room& room, std::vector<Pos>& adj_to_walls,
                    std::vector<Pos>& away_from_walls);

int get_walk_blockers_in_dir(const Dir dir, const Pos& pos);

} //Map_patterns

#endif

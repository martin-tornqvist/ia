#ifndef MAP_PATTERNS_HPP
#define MAP_PATTERNS_HPP

#include <vector>

#include "rl_utils.hpp"

//TODO: This should be be merged into mapgen_utils or something
//Also, most/all things in here really needs clearer purpose/naming

class Room;

enum class PlacementRule
{
    adj_to_walls,
    away_from_walls,
    either
};

namespace map_patterns
{

void cells_in_room(const Room& room,
                   std::vector<P>& adj_to_walls,
                   std::vector<P>& away_from_walls);

int walk_blockers_in_dir(const Dir dir, const P& pos);

} //map_patterns

#endif

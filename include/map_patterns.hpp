#ifndef MAP_PATTERNS_HPP
#define MAP_PATTERNS_HPP

#include <vector>

#include "rl_utils.hpp"

//TODO: This should be be merged into other files

class Room;

enum class FeaturePlacement
{
    adj_to_walls,
    away_from_walls,
    either
};

namespace map_patterns
{

//TODO: This should be a member function in the room class
void cells_in_room(const Room& room,
                   std::vector<P>& adj_to_walls,
                   std::vector<P>& away_from_walls);

} //map_patterns

#endif

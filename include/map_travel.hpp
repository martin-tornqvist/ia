#ifndef MAP_TRAVEL_HPP
#define MAP_TRAVEL_HPP

#include <vector>
#include <string>

#include "map_builder.hpp"

enum class AllowSpawnMonOverTime
{
        no,
        yes
};

struct MapData
{
        MapType type = MapType::std;

        AllowSpawnMonOverTime allow_spawn_mon_over_time =
                AllowSpawnMonOverTime::yes;
};

namespace map_travel
{

extern std::vector<MapData> map_list;

void init();

void save();
void load();

void try_use_down_stairs();

void go_to_nxt();

MapData current_map_data();

} // map_travel

#endif // MAP_TRAVEL_HPP

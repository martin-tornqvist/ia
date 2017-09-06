#ifndef MAP_TRAVEL_HPP
#define MAP_TRAVEL_HPP

#include <vector>
#include <string>

#include "map.hpp"

// This includes forest intro level, rats in the walls level, etc (every level
// that increments the dlvl number).
enum class IsMainDungeon
{
    no,
    yes
};

enum class AllowSpawnMonOverTime
{
    no,
    yes
};

struct MapData
{
    MapData(MapType type ,
            IsMainDungeon is_main_dungeon,
            AllowSpawnMonOverTime allow_spawn_mon_over_time) :
        type                        (type),
        is_main_dungeon             (is_main_dungeon),
        allow_spawn_mon_over_time   (allow_spawn_mon_over_time) {}

    MapData() :
        type                        (MapType::std),
        is_main_dungeon             (IsMainDungeon::yes),
        allow_spawn_mon_over_time   (AllowSpawnMonOverTime::yes) {}

    MapType type;

    IsMainDungeon is_main_dungeon;

    AllowSpawnMonOverTime allow_spawn_mon_over_time;
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

#endif

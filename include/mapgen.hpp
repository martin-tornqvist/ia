#ifndef MAPBUILD_HPP
#define MAPBUILD_HPP

#include <vector>

#include "map.hpp"
#include "map_templates.hpp"

class Room;

struct Region
{
public:
        Region(const R& r) :
                main_room(nullptr),
                r(r),
                is_free(true) {}

        Region() :
                main_room(nullptr),
                r(),
                is_free(true) {}

        R rnd_room_rect() const;

        Room* main_room;
        R r;
        bool is_free;
};

namespace mapgen
{

// This variable is checked at certain points to see if the current map
// has been flagged as "failed". Setting is_map_valid to false will generally
// stop map generation, discard the map, and trigger generation of a new map.
extern bool is_map_valid;

// All cells marked as true in this array will be considered for door placement
extern bool door_proposals[map_w][map_h];

// Standard dungeon level
// TODO: Consider moving to MapBuilderStd
bool make_std_lvl();

//------------------------------------------------------------------------------
// Map generation steps (in no particular order)
//------------------------------------------------------------------------------
void merge_regions(Region regions[3][3]);

void make_aux_rooms(Region regions[3][3]);

void reserve_river(Region regions[3][3]);

void make_sub_rooms();

void decorate();

void make_doors();

void make_metal_doors_and_levers();

void make_monoliths();

void make_pylons_and_levers();

//------------------------------------------------------------------------------
// Room reshaping utils (called by the room objects)
//------------------------------------------------------------------------------
// NOTE: Some reshape functions below will not change the boundaries of the
// room, but may affect which cells belong to the room. This only affects the
// room map (in the "map" namespace), so the room parameter should be a const
// reference. For other reshape functions, the room may expand beyond its
// initial rectangle, so in those cases the functions need to modify the data of
// the room object.
void cut_room_corners(const Room& room);
void make_pillars_in_room(const Room& room);
void cavify_room(Room& room);

//------------------------------------------------------------------------------
// Room creation
//------------------------------------------------------------------------------
// NOTE: All "make_room..." functions handle all the necessary steps such as
// creating floor on the map, creating room objects and registering them, et c.
Room* make_room(Region& region);

Room* make_room(const R& r, const IsSubRoom is_sub_room);

// Low level functions related to room creation - these are only necessary when
// creating rooms by other methods than the "make_room" functions above.
void register_room(Room& room);

void make_floor(const Room& room);

//------------------------------------------------------------------------------
// Misc utils
//------------------------------------------------------------------------------
void connect_rooms();

void valid_corridor_entries(
        const Room& room,
        std::vector<P>& out);

bool is_choke_point(
        const P& p,
        const bool blocked[map_w][map_h],
        ChokePointData* out);

void make_pathfind_corridor(
        Room& r0,
        Room& r1,
        bool door_proposals[map_w][map_h] = nullptr);

void rnd_walk(
        const P& p0,
        int len,
        std::vector<P>& pos_list_ref,
        const bool allow_diagonal = true,
        R area = R(1, 1, map_w - 2, map_h - 2));

void pathfinder_walk(
        const P& p0,
        const P& p1,
        std::vector<P>& pos_list_ref,
        const bool is_smooth);

// Generates a map of spawn chance weights, with emphasis on hidden, optional,
// or hard to reach areas - this can be used e.g. to place items or levers.
void make_explore_spawn_weights(
        const bool blocked[map_w][map_h],
        std::vector<P>& positions_out,
        std::vector<int>& weights_out);

void allowed_stair_cells(bool out[map_w][map_h]);

void move_player_to_nearest_allowed_pos();

P make_stairs_at_random_pos();

void reveal_doors_on_path_to_stairs(const P& stairs_pos);

} // mapgen

#endif // MAPBUILD_HPP

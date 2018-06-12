#ifndef MAP_HPP
#define MAP_HPP

#include <vector>

#include "colors.hpp"
#include "config.hpp"
#include "fov.hpp"
#include "game.hpp"
#include "item_data.hpp"
#include "rl_utils.hpp"

class Rigid;
class Mob;
class Player;

// TODO: This should probably be removed, and multiple smaller arrays should be
// used instead
struct Cell
{
    Cell();
    ~Cell();

    void reset();

    bool is_explored, is_seen_by_player;
    LosResult player_los; // Updated when player updates FOV
    Item* item;
    Rigid* rigid;
};

struct ChokePointData
{
    ChokePointData() :
        p(),
        player_side(-1),
        stairs_side(-1)
    {
        sides[0].resize(0);
        sides[1].resize(0);
    }

    ChokePointData& operator=(const ChokePointData& other)
    {
        p = other.p;

        sides[0] = other.sides[0];
        sides[1] = other.sides[1];

        return *this;
    }

    P p;

    // These shall only ever have a value of 0 or 1
    int player_side;
    int stairs_side;

    std::vector<P> sides[2];
};

namespace map
{

extern Player* player;

extern int dlvl;

extern Array2<Cell> cells;

extern Array2<bool> light;
extern Array2<bool> dark;

extern Color wall_color;

// This vector is the room owner
extern std::vector<Room*> room_list;

// Helper array, for convenience and optimization
extern Array2<Room*> room_map;

// NOTE: This data is only intended to be used for the purpose of map generation
// (and placing items etc), it is NOT updated while playing the map.
extern std::vector<ChokePointData> choke_point_data;

void init();
void cleanup();

void save();
void load();

void reset(const P& dims);

int w();

int h();

P dims();

R rect();

size_t nr_cells();

Rigid* put(Rigid* const rigid);

// This should be called when e.g. a door closes, or a wall is destoyed -
// updates light map, player fov (etc).
void update_vision();

void make_blood(const P& origin);
void make_gore(const P& origin);

void delete_and_remove_room_from_list(Room* const room);

bool is_pos_seen_by_player(const P& p);

Actor* actor_at_pos(const P& pos, ActorState state = ActorState::alive);

Mob* first_mob_at_pos(const P& pos);

void actor_cells(const std::vector<Actor*>& actors, std::vector<P>& out);

Array2<Actor*> get_actor_array();

Actor* random_closest_actor(const P& c, const std::vector<Actor*>& actors);

bool is_pos_inside_map(const P& pos);

bool is_pos_inside_outer_walls(const P& pos);

bool is_area_inside_map(const R& area);

} // map

#endif // MAP_HPP

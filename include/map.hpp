#ifndef MAP_HPP
#define MAP_HPP

#include <vector>

#include "colors.hpp"
#include "item_data.hpp"
#include "feature.hpp"
#include "config.hpp"
#include "actor_player.hpp"
#include "fov.hpp"
#include "render.hpp"

class Rigid;
class Mob;

struct Cell
{
    Cell();
    ~Cell();

    void reset();

    bool is_explored, is_seen_by_player, is_lit, is_dark;
    LosResult player_los; //Updated when player updates FOV
    Item* item;
    Rigid* rigid;
    CellRenderData player_visual_memory;
    P pos;
};

enum class MapType
{
    intro,
    std,
    egypt,
    leng,
    rats_in_the_walls,
    boss,
    trapez
};

struct ChokePointData
{
    ChokePointData() :
        p           (),
        player_side (-1),
        stairs_side (-1)
    {
        sides[0].resize(0);
        sides[1].resize(0);
    }

    ChokePointData& operator=(const ChokePointData& other)
    {
        p           = other.p;
        sides[0]    = other.sides[0];
        sides[1]    = other.sides[1];
        return *this;
    }

    P p;

    //These shall have value 0 or 1
    int player_side;
    int stairs_side;

    std::vector<P> sides[2];
};

namespace map
{

extern Player*                      player;
extern int                          dlvl;
extern Cell                         cells[map_w][map_h];

extern Clr                          wall_clr;

//This vector is the room owner
extern std::vector<Room*>           room_list;

//Helper array, for convenience and optimization
extern Room*                        room_map[map_w][map_h];

//NOTE: This data is only intended to be used for the purpose of map generation
//      (and placing items etc), it is NOT updated while playing the map.
extern std::vector<ChokePointData>  choke_point_data;

void init();
void cleanup();

void save();
void load();

void reset_map();

Rigid* put(Rigid* const rigid);

//Makes a copy of the renderers current array
//TODO: This is weird, and it's unclear how it should be used. Remove?
//Can it not be copied in the map drawing function instead?
void cpy_render_array_to_visual_memory();

void mk_blood(const P& origin);
void mk_gore(const P& origin);

void delete_and_remove_room_from_list(Room* const room);

bool is_pos_seen_by_player(const P& p);

Actor* actor_at_pos(const P& pos, ActorState state = ActorState::alive);

Mob* first_mob_at_pos(const P& pos);

void actor_cells(const std::vector<Actor*>& actors, std::vector<P>& out);

void mk_actor_array(Actor* a[map_w][map_h]);

Actor* random_closest_actor(const P& c, const std::vector<Actor*>& actors);

bool is_pos_inside_map(const P& pos, const bool count_edge_as_inside = true);

bool is_area_inside_map(const R& area);

} //map

#endif

#ifndef MAP_HPP
#define MAP_HPP

#include <vector>

#include "colors.hpp"
#include "item_data.hpp"
#include "feature.hpp"
#include "config.hpp"
#include "actor_player.hpp"
#include "fov.hpp"

class Save_handler;
class Rigid;
class Mob;

struct Cell
{
    Cell();
    ~Cell();

    void reset();

    bool                is_explored, is_seen_by_player, is_lit, is_dark;
    Los_result          player_los; //Updated when player updates FOV
    Item*               item;
    Rigid*              rigid;
    Cell_render_data    player_visual_memory;
    P                   pos;
};

enum class Map_type
{
    intro,
    std,
    egypt,
    leng,
    rats_in_the_walls,
    boss,
    trapez
};

namespace map
{

extern Player*              player;
extern int                  dlvl;
extern Cell                 cells[MAP_W][MAP_H];
extern std::vector<Room*>   room_list;              //Owns the rooms
extern Room*                room_map[MAP_W][MAP_H]; //Helper array

extern Clr                  wall_clr;

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

Actor* actor_at_pos(const P& pos, Actor_state state = Actor_state::alive);

Mob* first_mob_at_pos(const P& pos);

void actor_cells(const std::vector<Actor*>& actors, std::vector<P>& out);

void mk_actor_array(Actor* a[MAP_W][MAP_H]);

Actor* random_closest_actor(const P& c, const std::vector<Actor*>& actors);

bool is_pos_inside_map(const P& pos, const bool COUNT_EDGE_AS_INSIDE = true);

bool is_area_inside_map(const R& area);

} //map

#endif

#ifndef ATTACK_H
#define ATTACK_H

#include <vector>
#include <math.h>

#include "ability_values.hpp"
#include "item_data.hpp"
#include "actor_data.hpp"
#include "art.hpp"

class Actor;
class Wpn;

class Att_data
{
public:
    Actor*              attacker;
    Actor*              defender;
    Ability_roll_result att_result;
    int                 dmg;
    bool                is_intrinsic_att;
    bool                is_ethereal_defender_missed;

protected:
    //Never use this class directly (only through inheritance)
    Att_data(Actor* const attacker,
             Actor* const defender,
             const Item& att_item);
};

class Melee_att_data: public Att_data
{
public:
    Melee_att_data(Actor* const attacker,
                   Actor& defender,
                   const Wpn& wpn);
    bool is_defender_dodging;
    bool is_backstab;
    bool is_weak_attack;
};

class Ranged_att_data: public Att_data
{
public:
    Ranged_att_data(Actor* const attacker,
                    const Pos& attacker_orign,
                    const Pos& aim_pos,
                    const Pos& cur_pos,
                    const Wpn& wpn,
                    Actor_size aim_lvl = Actor_size::none);
    int         hit_chance_tot;
    Actor_size  intended_aim_lvl;
    Actor_size  defender_size;
    std::string verb_player_attacks;
    std::string verb_other_attacks;
};

class Throw_att_data: public Att_data
{
public:
    Throw_att_data(Actor* const attacker,
                   const Pos& aim_pos,
                   const Pos& cur_pos,
                   const Item& item,
                   Actor_size aim_lvl = Actor_size::none);
    int         hit_chance_tot;
    Actor_size  intended_aim_lvl;
    Actor_size  defender_size;
};

struct Projectile
{
    Projectile() :
        pos                     (Pos(-1, -1)),
        is_obstructed           (false),
        is_seen_by_player       (true),
        actor_hit               (nullptr),
        obstructed_in_element   (-1),
        is_done_rendering       (false),
        glyph                   (-1),
        tile                    (Tile_id::empty),
        clr                     (clr_white),
        att_data                (nullptr) {}

    ~Projectile()
    {
        delete att_data;
    }

    void set_att_data(Ranged_att_data* new_att_data)
    {
        delete att_data;

        att_data = new_att_data;
    }

    void set_tile(const Tile_id tile_to_render, const Clr& clr_to_render)
    {
        tile  = tile_to_render;
        clr   = clr_to_render;
    }

    void set_glyph(const char GLYPH_TO_RENDER, const Clr& clr_to_render)
    {
        glyph = GLYPH_TO_RENDER;
        clr   = clr_to_render;
    }

    Pos                 pos;
    bool                is_obstructed;
    bool                is_seen_by_player;
    Actor*              actor_hit;
    int                 obstructed_in_element;
    bool                is_done_rendering;
    char                glyph;
    Tile_id             tile;
    Clr                 clr;
    Ranged_att_data*    att_data;
};

enum class Melee_hit_size
{
    small,
    medium,
    hard
};

namespace attack
{

//NOTE: Attacker origin is needed since attacker may be a NULL pointer.
void melee(Actor* const attacker, const Pos& attacker_origin, Actor& defender, const Wpn& wpn);

bool ranged(Actor* const attacker, const Pos& origin, const Pos& aim_pos, Wpn& wpn);

void ranged_hit_chance(const Actor& attacker, const Actor& defender, const Wpn& wpn);

} //Attack

#endif


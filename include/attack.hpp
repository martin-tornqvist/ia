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
    Actor* attacker;
    Actor* defender;
    Ability_roll_result attack_result;
    int nr_dmg_rolls, nr_dmg_sides, dmg_plus;
    int dmg_roll, dmg;
    bool is_intrinsic_att;
    bool is_ethereal_defender_missed;

protected:
    Att_data(Actor& attacker_, const Item& att_item_);
};

class Melee_att_data: public Att_data
{
public:
    Melee_att_data(Actor& attacker_, const Wpn& wpn_, Actor& defender_);
    bool is_defender_dodging;
    bool is_backstab;
    bool is_weak_attack;
};

class Ranged_att_data: public Att_data
{
public:
    Ranged_att_data(Actor& attacker_, const Wpn& wpn_, const Pos& aim_pos_,
                  const Pos& cur_pos_, Actor_size intended_aim_lvl_ = Actor_size::none);
    int           hit_chance_tot;
    Actor_size   intended_aim_lvl;
    Actor_size   defender_size;
    std::string verb_player_attacks;
    std::string verb_other_attacks;
};

class Throw_att_data: public Att_data
{
public:
    Throw_att_data(Actor& attacker_, const Item& item_, const Pos& aim_pos_,
                 const Pos& cur_pos_, Actor_size intended_aim_lvl_ = Actor_size::none);
    int       hit_chance_tot;
    Actor_size intended_aim_lvl;
    Actor_size defender_size;
};

struct Projectile
{
    Projectile() : pos(Pos(-1, -1)), is_obstructed(false),
        is_visible_to_player(true), actor_hit(nullptr), obstructed_in_element(-1),
        is_done_rendering(false), glyph(-1), tile(Tile_id::empty), clr(clr_white),
        attack_data(nullptr) {}

    ~Projectile() {if (attack_data) {delete attack_data;}}

    void set_att_data(Ranged_att_data* attack_data_)
    {
        if (attack_data) {delete attack_data;}
        attack_data = attack_data_;
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

    Pos pos;
    bool is_obstructed;
    bool is_visible_to_player;
    Actor* actor_hit;
    int obstructed_in_element;
    bool is_done_rendering;
    char glyph;
    Tile_id tile;
    Clr clr;
    Ranged_att_data* attack_data;
};

enum class Melee_hit_size {small, medium, hard};

namespace attack
{

void melee(Actor& attacker, const Wpn& wpn, Actor& defender);

bool ranged(Actor& attacker, Wpn& wpn, const Pos& aim_pos);

void get_ranged_hit_chance(const Actor& attacker, const Actor& defender,
                        const Wpn& wpn);

} //Attack

#endif


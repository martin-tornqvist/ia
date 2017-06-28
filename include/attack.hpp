#ifndef ATTACK_HPP
#define ATTACK_HPP

#include <vector>
#include <math.h>

#include "ability_values.hpp"
#include "item_data.hpp"
#include "actor_data.hpp"
#include "art.hpp"

class Actor;
class Wpn;

class AttData
{
public:
    virtual ~AttData() {}

    Actor* attacker;
    Actor* defender;
    int skill_mod;
    int wpn_mod;
    int dodging_mod;
    int state_mod;
    int hit_chance_tot;
    ActionResult att_result;
    int dmg;
    bool is_intrinsic_att;

protected:
    // Never use this class directly (only through inheritance)
    AttData(Actor* const attacker,
            Actor* const defender,
            const Item& att_item);

    bool is_defender_aware(const Actor* const attacker,
                           const Actor& defender) const;
};

class MeleeAttData: public AttData
{
public:
    MeleeAttData(Actor* const attacker,
                 Actor& defender,
                 const Wpn& wpn);

    ~MeleeAttData() {}

    bool is_backstab;
    bool is_weak_attack;
};

class RangedAttData: public AttData
{
public:
    RangedAttData(Actor* const attacker,
                  const P& attacker_origin,
                  const P& aim_pos,
                  const P& current_pos,
                  const Wpn& wpn,
                  ActorSize aim_lvl = ActorSize::undefined);

    ~RangedAttData() {}

    P aim_pos;
    ActorSize intended_aim_lvl;
    ActorSize defender_size;
    std::string verb_player_attacks;
    std::string verb_other_attacks;
    int dist_mod;
};

class ThrowAttData: public AttData
{
public:
    ThrowAttData(Actor* const attacker,
                 const P& aim_pos,
                 const P& current_pos,
                 const Item& item,
                 ActorSize aim_lvl = ActorSize::undefined);

    ActorSize intended_aim_lvl;
    ActorSize defender_size;
    int dist_mod;
};

struct Projectile
{
    Projectile() :
        pos                     (P(-1, -1)),
        is_dead                 (false),
        is_seen_by_player       (true),
        actor_hit               (nullptr),
        obstructed_in_element   (-1),
        is_done_rendering       (false),
        glyph                   (-1),
        tile                    (TileId::empty),
        clr                     (clr_white),
        att_data                (nullptr) {}

    ~Projectile()
    {
        delete att_data;
    }

    void set_att_data(RangedAttData* new_att_data)
    {
        delete att_data;

        att_data = new_att_data;
    }

    void set_tile(const TileId tile_to_render, const Clr& clr_to_render)
    {
        tile  = tile_to_render;
        clr   = clr_to_render;
    }

    void set_glyph(const char glyph_to_render, const Clr& clr_to_render)
    {
        glyph = glyph_to_render;
        clr   = clr_to_render;
    }

    P pos;
    bool is_dead;
    bool is_seen_by_player;
    Actor* actor_hit;
    int obstructed_in_element;
    bool is_done_rendering;
    char glyph;
    TileId tile;
    Clr clr;
    RangedAttData* att_data;
};

enum class MeleeHitSize
{
    small,
        medium,
        hard
        };

namespace attack
{

// NOTE: Attacker origin is needed since attacker may be a NULL pointer.
void melee(Actor* const attacker,
           const P& attacker_origin,
           Actor& defender,
           Wpn& wpn);

bool ranged(Actor* const attacker,
            const P& origin,
            const P& aim_pos,
            Wpn& wpn);

void ranged_hit_chance(const Actor& attacker,
                       const Actor& defender,
                       const Wpn& wpn);

} // attack

#endif // ATTACK_HPP

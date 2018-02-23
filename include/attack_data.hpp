#ifndef ATTACK_DATA_HPP
#define ATTACK_DATA_HPP

#include "ability_values.hpp"
#include "actor_data.hpp"

class Actor;
class Item;
class P;
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
                      const Wpn& wpn);

        ~RangedAttData() {}

        P aim_pos;
        ActorSize aim_lvl;
        ActorSize defender_size;
        int dist_mod;
};

class ThrowAttData: public AttData
{
public:
        ThrowAttData(Actor* const attacker,
                     const P& aim_pos,
                     const P& current_pos,
                     const Item& item);

        ActorSize aim_lvl;
        ActorSize defender_size;
        int dist_mod;
};

#endif // ATTACK_DATA_HPP

#include "ability_values.hpp"

#include <math.h>

#include "actor_player.hpp"
#include "player_bon.hpp"
#include "utils.hpp"
#include "properties.hpp"
#include "map.hpp"

int Ability_vals::val(const Ability_id id,
                      const bool IS_AFFECTED_BY_PROPS,
                      const Actor& actor) const
{
    int ret = ability_list[size_t(id)];

    if (IS_AFFECTED_BY_PROPS)
    {
        ret += actor.prop_handler().ability_mod(id);
    }

    if (actor.is_player())
    {
        for (const Inv_slot& slot : actor.inv().slots_)
        {
            if (slot.item)
            {
                ret += slot.item->data().ability_mods_while_equipped[int(id)];
            }
        }

        const int HP_PCT  = (actor.hp() * 100) / actor.hp_max(true);

        switch (id)
        {
        case Ability_id::searching:
            ret += 8;

            if (player_bon::traits[size_t(Trait::observant)])
            {
                ret += 4;
            }

            if (player_bon::traits[size_t(Trait::perceptive)])
            {
                ret += 4;
            }
            break;

        case Ability_id::melee:
            ret += 60;

            if (player_bon::traits[size_t(Trait::adept_melee_fighter)])
            {
                ret += 10;
            }

            if (player_bon::traits[size_t(Trait::expert_melee_fighter)])
            {
                ret += 10;
            }

            if (player_bon::traits[size_t(Trait::master_melee_fighter)])
            {
                ret += 10;
            }

            if (player_bon::traits[size_t(Trait::perseverant)] && HP_PCT <= 25)
            {
                ret += 30;
            }
            break;

        case Ability_id::ranged:
            ret += 50;

            if (player_bon::traits[size_t(Trait::adept_marksman)])
            {
                ret += 10;
            }

            if (player_bon::traits[size_t(Trait::expert_marksman)])
            {
                ret += 10;
            }

            if (player_bon::traits[size_t(Trait::master_marksman)])
            {
                ret += 10;
            }

            if (player_bon::traits[size_t(Trait::perseverant)] && HP_PCT <= 25)
            {
                ret += 30;
            }

            if (player_bon::bg() == Bg::ghoul)
            {
                ret -= 15;
            }
            break;

        case Ability_id::dodge_trap:
            ret += 5;

            if (player_bon::traits[size_t(Trait::dexterous)])
            {
                ret += 25;
            }

            if (player_bon::traits[size_t(Trait::lithe)])
            {
                ret += 25;
            }
            break;

        case Ability_id::dodge_att:
            ret += 10;

            if (player_bon::traits[size_t(Trait::dexterous)])
            {
                ret += 25;
            }

            if (player_bon::traits[size_t(Trait::lithe)])
            {
                ret += 25;
            }

            if (player_bon::traits[size_t(Trait::perseverant)] && HP_PCT <= 25)
            {
                ret += 50;
            }
            break;

        case Ability_id::stealth:
            ret += 20;

            if (player_bon::traits[size_t(Trait::stealthy)])
            {
                ret += 30;
            }

            if (player_bon::traits[size_t(Trait::imperceptible)])
            {
                ret += 30;
            }
            break;

        case Ability_id::empty:
        case Ability_id::END:
            break;
        }

        if (id == Ability_id::searching)
        {
            //Searching must always be at least 1 to avoid trapping the player
            ret = std::max(ret, 1);
        }
        else if (id == Ability_id::dodge_att)
        {
            //It should not be possible to dodge every attack
            ret = std::min(ret, 95);
        }
    }

    ret = std::max(0, ret);

    return ret;
}

void Ability_vals::reset()
{
    for (int i = 0; i < int(Ability_id::END); ++i)
    {
        ability_list[i] = 0;
    }
}

void Ability_vals::set_val(const Ability_id ability, const int VAL)
{
    ability_list[int(ability)] = VAL;
}

void Ability_vals::change_val(const Ability_id ability, const int CHANGE)
{
    ability_list[int(ability)] += CHANGE;
}

namespace ability_roll
{

Ability_roll_result roll(const int TOT_SKILL_VALUE, const Actor* const actor_rolling)
{
    const int ROLL = rnd::percent();

    const bool IS_CURSED    = actor_rolling && actor_rolling->has_prop(Prop_id::cursed);
    const bool IS_BLESSED   = actor_rolling && actor_rolling->has_prop(Prop_id::blessed);

    //Critical success?
    Range crit_success_range(1, 1);

    if (IS_BLESSED)
    {
        //Increase critical success range while blessed
        crit_success_range.max = 5;
    }
    else if (IS_CURSED)
    {
        //Never critically succeed while cursed
        crit_success_range.set(-1, -1);
    }

    if (utils::is_val_in_range(ROLL, crit_success_range))
    {
        return Ability_roll_result::success_critical;
    }

    //Critical fail?
    Range crit_fail_range(100, 100);

    if (IS_BLESSED)
    {
        //Never critically fail while blessed
        crit_fail_range.set(-1, -1);
    }
    else if (IS_CURSED)
    {
        //Increase critical fail range while cursed
        crit_fail_range.min = 95;
    }

    if (utils::is_val_in_range(ROLL, crit_fail_range))
    {
        return Ability_roll_result::fail_critical;
    }

    return ROLL <= TOT_SKILL_VALUE ?
           Ability_roll_result::success : Ability_roll_result::fail;
}

} //ability_roll

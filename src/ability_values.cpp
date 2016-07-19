#include "ability_values.hpp"

#include <math.h>

#include "actor_player.hpp"
#include "player_bon.hpp"
#include "properties.hpp"
#include "map.hpp"

int AbilityVals::val(const AbilityId id,
                      const bool is_affected_by_props,
                      const Actor& actor) const
{
    int ret = ability_list[size_t(id)];

    if (is_affected_by_props)
    {
        ret += actor.prop_handler().ability_mod(id);
    }

    if (actor.is_player())
    {
        for (const InvSlot& slot : actor.inv().slots_)
        {
            if (slot.item)
            {
                ret += slot.item->data().ability_mods_while_equipped[int(id)];
            }
        }

        const int hp_pct  = (actor.hp() * 100) / actor.hp_max(true);

        const int perseverant_bon_hp_pct = 30;

        switch (id)
        {
        case AbilityId::searching:
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

        case AbilityId::melee:
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

            if (
                player_bon::traits[size_t(Trait::perseverant)] &&
                hp_pct <= perseverant_bon_hp_pct)
            {
                ret += 30;
            }
            break;

        case AbilityId::ranged:
            ret += 70;

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

            if (
                player_bon::traits[size_t(Trait::perseverant)] &&
                hp_pct <= perseverant_bon_hp_pct)
            {
                ret += 30;
            }

            if (player_bon::bg() == Bg::ghoul)
            {
                ret -= 15;
            }
            break;

        case AbilityId::dodge_trap:
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

        case AbilityId::dodge_att:
            ret += 10;

            if (player_bon::traits[size_t(Trait::dexterous)])
            {
                ret += 25;
            }

            if (player_bon::traits[size_t(Trait::lithe)])
            {
                ret += 25;
            }

            if (
                player_bon::traits[size_t(Trait::perseverant)] &&
                hp_pct <= perseverant_bon_hp_pct)
            {
                ret += 50;
            }
            break;

        case AbilityId::stealth:
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

        case AbilityId::empty:
        case AbilityId::END:
            break;
        }

        if (id == AbilityId::searching)
        {
            //Searching must always be at least 1 to avoid trapping the player
            ret = std::max(ret, 1);
        }
        else if (id == AbilityId::dodge_att)
        {
            //It should not be possible to dodge every attack
            ret = std::min(ret, 95);
        }
    }

    ret = std::max(0, ret);

    return ret;
}

void AbilityVals::reset()
{
    for (int i = 0; i < int(AbilityId::END); ++i)
    {
        ability_list[i] = 0;
    }
}

void AbilityVals::set_val(const AbilityId ability, const int val)
{
    ability_list[int(ability)] = val;
}

void AbilityVals::change_val(const AbilityId ability, const int change)
{
    ability_list[int(ability)] += change;
}

namespace ability_roll
{

AbilityRollResult roll(const int tot_skill_value, const Actor* const actor_rolling)
{
    const int roll = rnd::percent();

    const bool is_cursed    = actor_rolling && actor_rolling->has_prop(PropId::cursed);
    const bool is_blessed   = actor_rolling && actor_rolling->has_prop(PropId::blessed);

    //Critical success?
    Range crit_success_range(1, 1);

    if (is_blessed)
    {
        //Increase critical success range while blessed
        crit_success_range.max = 5;
    }
    else if (is_cursed)
    {
        //Never critically succeed while cursed
        crit_success_range.set(-1, -1);
    }

    if (is_val_in_range(roll, crit_success_range))
    {
        return AbilityRollResult::success_critical;
    }

    //Critical fail?
    Range crit_fail_range(100, 100);

    if (is_blessed)
    {
        //Never critically fail while blessed
        crit_fail_range.set(-1, -1);
    }
    else if (is_cursed)
    {
        //Increase critical fail range while cursed
        crit_fail_range.min = 95;
    }

    if (is_val_in_range(roll, crit_fail_range))
    {
        return AbilityRollResult::fail_critical;
    }

    return roll <= tot_skill_value ?
           AbilityRollResult::success : AbilityRollResult::fail;
}

} //ability_roll

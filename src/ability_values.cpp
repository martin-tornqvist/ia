#include "ability_values.hpp"

#include <math.h>

#include "actor_player.hpp"
#include "player_bon.hpp"
#include "utils.hpp"
#include "properties.hpp"
#include "map.hpp"

using namespace std;

int Ability_vals::get_val(const Ability_id ability_id,
                          const bool IS_AFFECTED_BY_PROPS, Actor& actor) const
{
    int ret = ability_list[int(ability_id)];

    if (IS_AFFECTED_BY_PROPS)
    {
        ret += actor.get_prop_handler().get_ability_mod(ability_id);
    }

    if (actor.is_player())
    {
        for (const Inv_slot& slot : actor.get_inv().slots_)
        {
            if (slot.item)
            {
                ret += slot.item->get_data().ability_mods_while_equipped[int(ability_id)];
            }
        }

        const int HP_PCT  = (actor.get_hp() * 100) / actor.get_hp_max(true);

        switch (ability_id)
        {
        case Ability_id::searching:
            ret += 8;
            if (player_bon::traits[int(Trait::observant)])   ret += 4;
            if (player_bon::traits[int(Trait::perceptive)])  ret += 4;
            break;

        case Ability_id::melee:
            ret += 45;
            if (player_bon::traits[int(Trait::adept_melee_fighter)])   ret += 10;
            if (player_bon::traits[int(Trait::expert_melee_fighter)])  ret += 10;
            if (player_bon::traits[int(Trait::master_melee_fighter)])  ret += 10;
            if (player_bon::traits[int(Trait::perseverant)] && HP_PCT <= 25) ret += 30;
            break;

        case Ability_id::ranged:
            ret += 50;
            if (player_bon::traits[int(Trait::adept_marksman)])   ret += 10;
            if (player_bon::traits[int(Trait::expert_marksman)])  ret += 10;
            if (player_bon::traits[int(Trait::master_marksman)])  ret += 10;
            if (player_bon::traits[int(Trait::perseverant)] && HP_PCT <= 25) ret += 30;
            break;

        case Ability_id::dodge_trap:
            ret += 5;
            if (player_bon::traits[int(Trait::dexterous)]) ret += 25;
            if (player_bon::traits[int(Trait::lithe)])     ret += 25;
            break;

        case Ability_id::dodge_att:
            ret += 10;
            if (player_bon::traits[int(Trait::dexterous)]) ret += 25;
            if (player_bon::traits[int(Trait::lithe)])     ret += 25;
            if (player_bon::traits[int(Trait::perseverant)] && HP_PCT <= 25) ret += 50;
            break;

        case Ability_id::stealth:
            ret += 10;
            if (player_bon::traits[int(Trait::stealthy)])      ret += 50;
            if (player_bon::traits[int(Trait::imperceptible)]) ret += 30;
            break;

        case Ability_id::empty:
        case Ability_id::END: {} break;
        }

        if (ability_id == Ability_id::searching)
        {
            //Searching must always be at least 1 to avoid trapping the player
            ret = max(ret, 1);
        }
        else if (ability_id == Ability_id::dodge_att)
        {
            //It should not be possible to dodge every attack
            ret = min(ret, 95);
        }
    }

    ret = max(0, ret);

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

ability_roll_result roll(const int TOT_SKILL_VALUE)
{
    const int ROLL = rnd::percent();

    const int SUCC_CRI_LMT = int(ceil(float(TOT_SKILL_VALUE) / 20.0));
    const int SUCC_BIG_LMT = int(ceil(float(TOT_SKILL_VALUE) / 5.0));
    const int SUCC_NRM_LMT = int(ceil(float(TOT_SKILL_VALUE) * 4.0 / 5.0));
    const int SUCC_SML_LMT = TOT_SKILL_VALUE;
    const int FAIL_SML_LMT = 2 * TOT_SKILL_VALUE - SUCC_NRM_LMT;
    const int FAIL_NRM_LMT = 2 * TOT_SKILL_VALUE - SUCC_BIG_LMT;
    const int FAIL_BIG_LMT = 98;

    if (ROLL <= SUCC_CRI_LMT) return success_critical;
    if (ROLL <= SUCC_BIG_LMT) return success_big;
    if (ROLL <= SUCC_NRM_LMT) return success_normal;
    if (ROLL <= SUCC_SML_LMT) return success_small;
    if (ROLL <= FAIL_SML_LMT) return fail_small;
    if (ROLL <= FAIL_NRM_LMT) return fail_normal;
    if (ROLL <= FAIL_BIG_LMT) return fail_big;

    return fail_critical;

    /* Example:
    -----------
    Ability = 50

    Roll:
    1  -   3: Critical  success
    4  -  10: Big       success
    11 -  40: Normal    success
    41 -  50: Small     Success
    51 -  60: Small     Fail
    61 -  90: Normal    Fail
    91 -  98: Big       Fail
    99 - 100: Critical  Fail */
}

} //ability_roll

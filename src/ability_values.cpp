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
    int ret = ability_list[(size_t)id];

    if (is_affected_by_props)
    {
        ret += actor.prop_handler().ability_mod(id);
    }

    if (actor.is_player())
    {
        for (const InvSlot& slot : actor.inv().slots_)
        {
            if (!slot.item)
            {
                continue;
            }

            auto& d = slot.item->data();

            ret += d.ability_mods_while_equipped[(size_t)id];
        }

        const int hp_pct = (actor.hp() * 100) / actor.hp_max(true);

        const int perseverant_bon_hp_pct = 50;

        switch (id)
        {
        case AbilityId::searching:
        {
            ret += 10;

            if (player_bon::traits[(size_t)Trait::observant])
            {
                ret += 10;
            }
        }
        break;

        case AbilityId::melee:
        {
            ret += 60;

            if (player_bon::traits[(size_t)Trait::adept_melee_fighter])
            {
                ret += 10;
            }

            if (player_bon::traits[(size_t)Trait::expert_melee_fighter])
            {
                ret += 10;
            }

            if (player_bon::traits[(size_t)Trait::master_melee_fighter])
            {
                ret += 10;
            }

            if (player_bon::traits[(size_t)Trait::perseverant] &&
                (hp_pct < perseverant_bon_hp_pct))
            {
                ret += 30;
            }
        }
        break;

        case AbilityId::ranged:
        {
            ret += 60;

            if (player_bon::traits[(size_t)Trait::adept_marksman])
            {
                ret += 10;
            }

            if (player_bon::traits[(size_t)Trait::expert_marksman])
            {
                ret += 10;
            }

            if (player_bon::traits[(size_t)Trait::master_marksman])
            {
                ret += 10;
            }

            if (player_bon::traits[(size_t)Trait::perseverant] &&
                (hp_pct < perseverant_bon_hp_pct))
            {
                ret += 30;
            }

            if (player_bon::bg() == Bg::ghoul)
            {
                ret -= 15;
            }
        }
        break;

        case AbilityId::dodging:
        {
            if (player_bon::traits[(size_t)Trait::dexterous])
            {
                ret += 15;
            }

            if (player_bon::traits[(size_t)Trait::lithe])
            {
                ret += 15;
            }

            if (player_bon::traits[(size_t)Trait::perseverant] &&
                (hp_pct < perseverant_bon_hp_pct))
            {
                ret += 30;
            }
        }
        break;

        case AbilityId::stealth:
        {
            ret += 25;

            if (player_bon::traits[(size_t)Trait::stealthy])
            {
                ret += 45;
            }

            if (player_bon::traits[(size_t)Trait::imperceptible])
            {
                ret += 20;
            }
        }
        break;

        case AbilityId::empty:
        case AbilityId::END:
            break;
        }

        if (id == AbilityId::searching)
        {
            // Searching must ALWAYS be at least 1, to avoid trapping the player
            ret = std::max(1, ret);
        }
    }

    ret = std::max(0, ret);

    return ret;
}

void AbilityVals::reset()
{
    for (size_t i = 0; i < (size_t)AbilityId::END; ++i)
    {
        ability_list[i] = 0;
    }
}

void AbilityVals::set_val(const AbilityId ability, const int val)
{
    ability_list[(size_t)ability] = val;
}

void AbilityVals::change_val(const AbilityId ability, const int change)
{
    ability_list[(size_t)ability] += change;
}

namespace ability_roll
{

ActionResult roll(const int skill_value,
                  const Actor* const actor_rolling)
{
    /*
      Examples:
      ------------
      Skill value = 50, not blessed, not cursed

        1 -   1     Critical success
        2 -  25     Big success
       26 -  50     Normal success
       51 -  75     Normal fail
       76 -  99     Big fail
      100 - 100     Critical fail


      Skill value = 90, not blessed, not cursed

        1 -   1     Critical success
        2 -  45     Big success
       46 -  90     Normal success
       91 -  95     Normal fail
       96 -  99     Big fail
      100 - 100     Critical fail


      Skill value = 10, not blessed, not cursed

        1 -   1     Critical success
        2 -   5     Big success
        6 -  10     Normal success
       11 -  55     Normal fail
       56 -  99     Big fail
      100 - 100     Critical fail
    */

    const int roll = rnd::range(1, 100);

    const bool is_cursed =
        actor_rolling &&
        actor_rolling->has_prop(PropId::cursed);

    const bool is_blessed =
        actor_rolling &&
        actor_rolling->has_prop(PropId::blessed);

    int succ_cri_lmt = 1;

    if (is_blessed)
    {
        succ_cri_lmt = 5;
    }

    if (is_cursed)
    {
        succ_cri_lmt = -1;
    }

    int fail_big_lmt = 99;

    if (is_blessed)
    {
        fail_big_lmt = 100;
    }

    if (is_cursed)
    {
        fail_big_lmt = 95;
    }

    const int succ_big_lmt = ceil((double)skill_value / 2.0);
    const int succ_nrm_lmt = skill_value;
    const int fail_nrm_lmt = ceil(100 - ((double)(100 - skill_value) / 2.0));

    //
    // NOTE: We check for critical fail/success first, since they should not be
    //       completely unaffected by the skill value (you can always critically
    //       fail/succeed, unless cursed/blessed)
    //
    if (roll <= succ_cri_lmt)   return ActionResult::success_critical;

    if (roll > fail_big_lmt)    return ActionResult::fail_critical;

    if (roll <= succ_big_lmt)   return ActionResult::success_big;
    if (roll <= succ_nrm_lmt)   return ActionResult::success;
    if (roll <= fail_nrm_lmt)   return ActionResult::fail;

    // Sanity check:
    ASSERT(roll <= fail_big_lmt);

    return ActionResult::fail_big;
}

} // ability_roll

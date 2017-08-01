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
            ret += 70;

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
            ret += 40;

            if (player_bon::traits[(size_t)Trait::stealthy])
            {
                ret += 35;
            }

            if (player_bon::traits[(size_t)Trait::imperceptible])
            {
                ret += 35;
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

ActionResult roll(const int skill_value)
{
    /*
      Example:
      ------------
      Skill value = 50

        1 -   2     Critical success
        3 -  25     Big success
       26 -  50     Normal success
       51 -  75     Normal fail
       76 -  98     Big fail
       99 - 100     Critical fail
    */

    const int succ_cri_lmt = 2;
    const int succ_big_lmt = ceil((double)skill_value / 2.0);
    const int succ_nrm_lmt = skill_value;
    const int fail_nrm_lmt = ceil(100.0 - ((double)(100 - skill_value) / 2.0));
    const int fail_big_lmt = 98;

    const int roll = rnd::range(1, 100);

    //
    // NOTE: We check critical success and fail first, since they should
    //       be completely unaffected by skill values - they can always
    //       happen, and always have the same chance to happen, regardless
    //       of skills
    //
    if (roll <= succ_cri_lmt)
    {
        return ActionResult::success_critical;
    }

    if (roll > fail_big_lmt)
    {
        return ActionResult::fail_critical;
    }

    if (roll <= succ_big_lmt)
    {
        return ActionResult::success_big;
    }

    if (roll <= succ_nrm_lmt)
    {
        return ActionResult::success;
    }

    if (roll <= fail_nrm_lmt)
    {
        return ActionResult::fail;
    }

    // Sanity check:
    ASSERT(roll <= fail_big_lmt);

    return ActionResult::fail_big;
}

} // ability_roll

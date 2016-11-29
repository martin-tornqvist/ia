#ifndef ABILITY_VALUES_HPP
#define ABILITY_VALUES_HPP

class Actor;

enum class AbilityId
{
    empty,
    searching,
    ranged,
    melee,
    dodging,
    stealth,
    END
};

//Each actor has an instance of this class
class AbilityVals
{
public:
    AbilityVals()
    {
        reset();
    }

    AbilityVals& operator=(const AbilityVals& other)
    {
        for (int i = 0; i < int(AbilityId::END); ++i)
        {
            ability_list[i] = other.ability_list[i];
        }

        return *this;
    }

    void reset();

    int val(const AbilityId id,
            const bool is_affected_by_props,
            const Actor& actor) const;

    int raw_val(const AbilityId id)
    {
        return ability_list[int(id)];
    }

    void set_val(const AbilityId id, const int val);

    void change_val(const AbilityId id, const int change);

private:
    int ability_list[int(AbilityId::END)];
};

enum AbilityRollResult
{
    fail_critical,
    fail,
    success,
    success_critical
};

namespace ability_roll
{

AbilityRollResult roll(const int tot_skill_value,
                       const Actor* const actor_rolling);

} //ability_roll

#endif

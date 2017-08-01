#ifndef ABILITY_VALUES_HPP
#define ABILITY_VALUES_HPP

#include <cstddef>

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

enum class ActionResult
{
    fail_critical,
    fail_big,
    fail,
    success,
    success_big,
    success_critical
};

// Each actor has an instance of this class
class AbilityVals
{
public:
    AbilityVals()
    {
        reset();
    }

    AbilityVals& operator=(const AbilityVals& other)
    {
        for (size_t i = 0; i < (size_t)AbilityId::END; ++i)
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
        return ability_list[(size_t)id];
    }

    void set_val(const AbilityId id, const int val);

    void change_val(const AbilityId id, const int change);

private:
    int ability_list[(size_t)AbilityId::END];
};

namespace ability_roll
{

ActionResult roll(const int skill_value);

} // ability_roll

#endif // ABILITY_VALUES_HPP

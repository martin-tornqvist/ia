#ifndef ABILITY_VALUES_HPP
#define ABILITY_VALUES_HPP

class Actor;

enum class Ability_id
{
    empty,
    searching,
    ranged,
    melee,
    dodge_trap,
    dodge_att,
    stealth,
    END
};

//Each actor has an instance of this class
class Ability_vals
{
public:
    Ability_vals() {reset();}

    Ability_vals& operator=(const Ability_vals& other)
    {
        for (int i = 0; i < int(Ability_id::END); ++i)
        {
            ability_list[i] = other.ability_list[i];
        }

        return *this;
    }

    void reset();

    int val(const Ability_id id, const bool IS_AFFECTED_BY_PROPS, const Actor& actor) const;

    int raw_val(const Ability_id id)
    {
        return ability_list[int(id)];
    }

    void set_val(const Ability_id id, const int VAL);

    void change_val(const Ability_id id, const int CHANGE);

private:
    int ability_list[int(Ability_id::END)];
};

enum Ability_roll_result
{
    fail_critical,
    fail,
    success,
    success_critical
};

namespace ability_roll
{

Ability_roll_result roll(const int TOT_SKILL_VALUE, const Actor* const actor_rolling);

} //ability_roll

#endif

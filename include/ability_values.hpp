#ifndef ABILITY_VALUES_H
#define ABILITY_VALUES_H

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

    int get_val(const Ability_id ability_id, const bool IS_AFFECTED_BY_PROPS,
               Actor& actor) const;

    int get_raw_val(const Ability_id ability)
    {
        return ability_list[int(ability)];
    }

    void set_val(const Ability_id ability, const int VAL);

    void change_val(const Ability_id ability, const int CHANGE);

private:
    int ability_list[int(Ability_id::END)];
};

//TODO: Is this really necessary? Most functionality nowadays just roll their own chances.
//Probably the only case where fail_small and fail_big is used is for melee attack messages.
//It seems simpler and more transparent to just use the Rnd functions for rolling,
//together with Ability_vals::get_val() for retrieving abilities to roll against.
enum Ability_roll_result
{
    fail_critical,
    fail_big,
    fail_normal,
    fail_small,
    success_small,
    success_normal,
    success_big,
    success_critical
};

//TODO: See comment above for Ability_roll_result
namespace ability_roll
{

Ability_roll_result roll(const int TOT_SKILL_VALUE);

} //Ability_roll

#endif

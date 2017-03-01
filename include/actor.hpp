#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <string>
#include <vector>

#include "global.hpp"

#include "actor_data.hpp"
#include "sound.hpp"
#include "config.hpp"
#include "art.hpp"

class PropHandler;
class Inventory;

enum class ActorDied
{
    no,
    yes
};

class Actor
{
public:
    Actor();
    virtual ~Actor();

    PropHandler& prop_handler()
    {
        return *prop_handler_;
    }

    const PropHandler& prop_handler() const
    {
        return *prop_handler_;
    }

    // This is just a shortcut to the same function in the property handler
    bool has_prop(const PropId id) const;

    ActorDataT& data()
    {
        return *data_;
    }

    const ActorDataT& data() const
    {
        return *data_;
    }

    Inventory& inv()
    {
        return *inv_;
    }

    const Inventory& inv() const
    {
        return *inv_;
    }

    int ability(const AbilityId id, const bool is_affected_by_props) const;

    // NOTE: This function is not concerned with whether actors are within FOV,
    //       or if they are actually hidden or not. It merely performs a skill
    //       check, taking various conditions such as light/dark into concern.
    //       It has no side effects.
    ActionResult roll_sneak(const Actor& actor) const;

    void place(const P& pos_, ActorDataT& data);

    virtual void place_hook() {}

    ActorDied hit(int dmg,
                  const DmgType dmg_type,
                  const DmgMethod method = DmgMethod::END,
                  const AllowWound allow_wound = AllowWound::yes);

    ActorDied hit_spi(const int dmg,
                      const Verbosity verbosity = Verbosity::verbose);

    bool restore_hp(const int hp_restored,
                    const bool is_allowed_above_max = false,
                    const Verbosity verbosity = Verbosity::verbose);

    bool restore_spi(const int spi_restored,
                     const bool is_allowed_above_max = false,
                     const Verbosity verbosity = Verbosity::verbose);

    void set_hp_and_spi_to_max();

    void change_max_hp(const int change,
                       const Verbosity verbosity = Verbosity::verbose);

    void change_max_spi(const int change,
                        const Verbosity verbosity = Verbosity::verbose);

    void die(const bool is_destroyed,
             const bool allow_gore,
             const bool allow_drop_items);

    void destroy();

    // Silently mark the monster as body-destroyed, without running any hooks
    // etc (used e.g. by the Horn of Banishment)
    void destroy_silent()
    {
        state_ = ActorState::destroyed;
    }

    // Used by Ghoul class and Ghoul monsters
    DidAction try_eat_corpse();

    void on_feed();

    void on_std_turn_common();

    virtual void act() {}

    virtual void move(Dir dir) = 0;

    virtual void on_actor_turn() {}
    virtual void on_std_turn() {}

    virtual TileId tile() const;

    virtual char glyph() const;

    virtual Clr clr() const = 0;

    virtual std::vector<Actor*> seen_actors() const = 0;

    virtual std::vector<Actor*> seen_foes() const = 0;

    ActorId id() const
    {
        return data_->id;
    }

    int hp() const
    {
        return hp_;
    }

    int spi() const
    {
        return spi_;
    }

    int hp_max(const bool with_modifiers) const;

    int spi_max() const
    {
        return spi_max_;
    }

    int speed_pct() const;

    virtual int spell_skill(const SpellId id) const = 0;

    virtual std::string name_the() const
    {
        return data_->name_the;
    }

    virtual std::string name_a() const
    {
        return data_->name_a;
    }

    std::string corpse_name_a() const
    {
        return data_->corpse_name_a;
    }

    std::string corpse_name_the() const
    {
        return data_->corpse_name_the;
    }

    virtual std::string descr() const
    {
        return data_->descr;
    }

    bool is_humanoid() const
    {
        return data_->is_humanoid;
    }

    void add_light(bool light_map[map_w][map_h]) const;

    virtual void add_light_hook(bool light[map_w][map_h]) const
    {
        (void)light;
    }

    void teleport();

    void teleport(const P& p);

    bool is_alive() const
    {
        return state_ == ActorState::alive;
    }

    bool is_corpse() const
    {
        return state_ == ActorState::corpse;
    }

    ActorState state() const
    {
        return state_;
    }

    virtual bool is_leader_of(const Actor* const actor) const = 0;
    virtual bool is_actor_my_leader(const Actor* const actor) const = 0;

    bool is_player() const;

    P pos;

    int delay_;

protected:
    // TODO: Try to get rid of these friend declarations
    friend class AbilityVals;
    friend class PropDiseased;
    friend class PropPossByZuul;
    friend class Trap;

    virtual void on_death() {}

    virtual void on_destroyed() {}

    virtual std::string death_msg() const;

    virtual void on_hit(int& dmg,
                        const DmgType dmg_type,
                        const DmgMethod method,
                        const AllowWound allow_wound)
    {
        (void)dmg;
        (void)dmg_type;
        (void)method;
        (void)allow_wound;
    }

    virtual void mk_start_items() {}

    ActorState  state_;

    int hp_;
    int hp_max_;
    int spi_;
    int spi_max_;

    P lair_pos_;

    PropHandler* prop_handler_;
    ActorDataT* data_;
    Inventory* inv_;
};

#endif // ACTOR_HPP

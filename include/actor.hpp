#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <vector>

#include "cmn.hpp"

#include "actor_data.hpp"
#include "sound.hpp"
#include "config.hpp"
#include "art.hpp"

class Prop_handler;
class Inventory;

enum class Actor_died
{
    no,
    yes
};

class Actor
{
public:
    Actor();
    virtual ~Actor();

    Prop_handler& prop_handler()
    {
        return *prop_handler_;
    }

    const Prop_handler& prop_handler() const
    {
        return *prop_handler_;
    }

    //This is just a shortcut to the same function in the property handler
    bool has_prop(const Prop_id id) const;

    Actor_data_t& data()
    {
        return *data_;
    }

    const Actor_data_t& data() const
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

    int ability(const Ability_id id, const bool IS_AFFECTED_BY_PROPS) const;

    //NOTE: This function is not concerned with whether the parameter actor is
    //within FOV, or if the actor is actually hidden or not. It merely tests the
    //sneak skill of the actor, and various conditions such as light/dark. It
    //has no side effects - it merely does a randomized check.
    bool is_spotting_sneaking_actor(Actor& actor);

    void place(const P& pos_, Actor_data_t& data);

    virtual void place_hook() {}

    Actor_died hit(int dmg,
                   const Dmg_type dmg_type,
                   const Dmg_method method = Dmg_method::END,
                   const Allow_wound allow_wound = Allow_wound::yes);

    Actor_died hit_spi(const int DMG, const Verbosity verbosity = Verbosity::verbose);

    bool restore_hp(const int HP_RESTORED,
                    const bool IS_ALLOWED_ABOVE_MAX = false,
                    const Verbosity verbosity = Verbosity::verbose);

    bool restore_spi(const int SPI_RESTORED,
                     const bool IS_ALLOWED_ABOVE_MAX = false,
                     const Verbosity verbosity = Verbosity::verbose);

    void set_hp_and_spi_to_max();

    void change_max_hp(const int CHANGE, const Verbosity verbosity = Verbosity::verbose);

    void change_max_spi(const int CHANGE, const Verbosity verbosity = Verbosity::verbose);

    void die(const bool IS_DESTROYED, const bool ALLOW_GORE, const bool ALLOW_DROP_ITEMS);

    //Used by Ghoul class and Ghoul monsters
    Did_action try_eat_corpse();
    void on_feed();

    void on_std_turn_common();

    virtual void act() {}

    virtual void move(Dir dir) = 0;

    virtual void on_actor_turn() {}
    virtual void on_std_turn() {}

    virtual void update_clr();

    void seen_actors(std::vector<Actor*>& out);

    void seen_foes(std::vector<Actor*>& out);

    Actor_id id() const
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

    int hp_max(const bool WITH_MODIFIERS) const;

    int spi_max() const
    {
        return spi_max_;
    }

    Actor_speed speed() const;

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

    virtual char glyph() const
    {
        return glyph_;
    }

    virtual Clr clr()
    {
        return clr_;
    }

    virtual Tile_id tile() const
    {
        return tile_;
    }

    void add_light(bool light_map[MAP_W][MAP_H]) const;

    virtual void add_light_hook(bool light[MAP_W][MAP_H]) const
    {
        (void)light;
    }

    void teleport();

    bool is_alive() const
    {
        return state_ == Actor_state::alive;
    }

    bool is_corpse() const
    {
        return state_ == Actor_state::corpse;
    }

    Actor_state state() const
    {
        return state_;
    }

    virtual bool is_leader_of(const Actor* const actor) const = 0;
    virtual bool is_actor_my_leader(const Actor* const actor) const = 0;

    bool is_player() const;

    P pos;

protected:
    //TODO: Try to get rid of these friend declarations
    friend class Ability_vals;
    friend class Prop_diseased;
    friend class Prop_poss_by_zuul;
    friend class Trap;

    virtual void on_death() {}

    virtual std::string death_msg() const;

    virtual void on_hit(int& dmg,
                        const Dmg_type dmg_type,
                        const Dmg_method method,
                        const Allow_wound allow_wound)
    {
        (void)dmg;
        (void)dmg_type;
        (void)method;
        (void)allow_wound;
    }

    virtual void mk_start_items() {}

    Actor_state  state_;
    Clr clr_;
    char glyph_;
    Tile_id tile_;

    int hp_, hp_max_, spi_, spi_max_;

    P lair_pos_;

    Prop_handler* prop_handler_;
    Actor_data_t* data_;
    Inventory* inv_;
};

#endif

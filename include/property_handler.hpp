#ifndef PROPERTY_HANDLER_HPP
#define PROPERTY_HANDLER_HPP

#include <memory>
#include <string>

#include "ability_values.hpp"
#include "global.hpp"
#include "property_data.hpp"
#include "rl_utils.hpp"

class Actor;
class Item;
class Prop;
class Wpn;

enum class PropSrc
{
        // Properties applied by potions, spells, etc, or "natural" properties
        // for monsters (e.g. flying), or player properties gained by traits
        intr,

        // Properties applied by items carried in inventory
        inv,

        END
};

struct PropTextListEntry
{
        PropTextListEntry() :
                title(),
                descr(),
                prop(nullptr) {}

        ColoredString title;

        std::string descr;

        const Prop* prop;
};

// Each actor has an instance of this
class PropHandler
{
public:
        PropHandler(Actor* owner);

        ~PropHandler();

        void save() const;

        void load();

        // All properties must be added through this function (can also be done
        // via the other "apply" methods, which will then call "apply")
        void apply(Prop* const prop,
                   PropSrc src = PropSrc::intr,
                   const bool force_effect = false,
                   const Verbosity verbosity = Verbosity::verbose);

        void apply_natural_props_from_actor_data();

        // The following two methods are supposed to be called by items
        void add_prop_from_equipped_item(const Item* const item,
                                         Prop* const prop,
                                         const Verbosity verbosity);

        void remove_props_for_item(const Item* const item);

        // Fast method for checking if a certain property id is applied
        bool has_prop(const PropId id) const
        {
                return prop_count_cache_[(size_t)id] > 0;
        }

        Prop* prop(const PropId id) const;

        bool end_prop(const PropId id);

        bool end_prop_silent(const PropId id);

        std::vector<ColoredString> property_names_short() const;

        std::vector<PropTextListEntry> property_names_and_descr() const;

        std::vector<PropTextListEntry> property_names_temporary_negative();

        bool has_temporary_negative_prop_mon() const;

        //----------------------------------------------------------------------
        // Hooks called from various places
        //----------------------------------------------------------------------
        void affect_move_dir(const P& actor_pos, Dir& dir) const;

        int affect_max_hp(const int hp_max) const;
        int affect_max_spi(const int spi_max) const;
        int affect_shock(const int shock) const;

        bool allow_attack(const Verbosity verbosity) const;
        bool allow_attack_melee(const Verbosity verbosity) const;
        bool allow_attack_ranged(const Verbosity verbosity) const;
        bool allow_see() const;
        bool allow_move() const;
        bool allow_act() const;
        bool allow_speak(const Verbosity verbosity) const;
        bool allow_eat(const Verbosity verbosity) const; // Also for drinking

        // NOTE: The allow_*_absolute methods below answer if some action could
        // EVER be performed, and the allow_*_chance methods allows the action
        // with a random chance. For example, blindness never allows the player
        // to read scrolls, and the game won't let the player try. But burning
        // will allow the player to try, with a certain percent chance of
        // success, and the scroll will be wasted on failure. (All plain
        // allow_* methods above are also considered "absolute".)
        bool allow_read_absolute(const Verbosity verbosity) const;
        bool allow_read_chance(const Verbosity verbosity) const;
        bool allow_cast_intr_spell_absolute(const Verbosity verbosity) const;
        bool allow_cast_intr_spell_chance(const Verbosity verbosity) const;

        void on_hit();
        void on_death();
        void on_destroyed();

        int ability_mod(const AbilityId ability) const;

        bool affect_actor_color(Color& color) const;

        void on_placed();

        // Called when the actors turn begins/ends
        void on_turn_begin();
        void on_turn_end();

        void on_std_turn();

        // Called just before an actor is supposed to do an action (move,
        // attack,...). This may "take over" the actor and do some special
        // behavior instead (e.g. a Zombie rising, or a Vortex pulling),
        // possibly ticking game time - if time is ticked, this method returns
        // 'DidAction::yes' (each property implementing this callback must
        // make sure to do this).
        DidAction on_act();

        bool is_resisting_dmg(const DmgType dmg_type,
                              const Verbosity verbosity) const;

private:
        void print_resist_msg(const Prop& prop);
        void print_start_msg(const Prop& prop);

        bool try_apply_more_on_existing_intr_prop(const Prop& new_prop);

        bool is_temporary_negative_prop(const Prop& prop) const;

        bool is_resisting_prop(const PropId id) const;

        // A hook that prints messages, updates FOV, etc, and also calls the
        // on_end() property hook.
        // NOTE: It does NOT remove the property from the vector or decrement
        // the active property info. The caller is responsible for this.
        void on_prop_end(Prop* const prop);

        void incr_prop_count(const PropId id);
        void decr_prop_count(const PropId id);

        std::vector< std::unique_ptr<Prop> > props_;

        // This array is only used as an optimization when requesting which
        // properties are currently active (see the "has_prop()" method above).
        int prop_count_cache_[(size_t)PropId::END];

        Actor* owner_;
};

#endif // PROPERTY_HANDLER_HPP

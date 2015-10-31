#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>
#include <vector>

#include "ability_values.hpp"
#include "cmn_data.hpp"
#include "converters.hpp"
#include "cmn_types.hpp"

enum class Prop_id
{
    rPhys,
    rFire,
    rPoison,
    rElec,
    rAcid,
    rSleep,
    rFear,
    rConf,
    rBreath,
    rDisease,
    rSpell,
    lgtSens,
    blind,
    fainted,
    burning,
    radiant,
    invis,
    see_invis, //Can see invisible and sneaking actors
    infravis,
    poisoned,
    paralyzed,
    terrified,
    confused,
    stunned,
    slowed,
    hasted,
    infected,
    diseased,
    weakened,
    frenzied,
    blessed,
    cursed,
    tele_ctrl, //NOTE: This only makes sense for the player
    spell_reflect,
    strangled,
    conflict,
    descend,

    //Properties describing the actors body and/or method of moving around
    flying,
    ethereal,
    ooze,
    burrowing,

    //Properties used for AI control
    waiting,
    disabled_attack,
    disabled_melee,
    disabled_ranged,

    //Special (for supporting very specific game mechanics)
    poss_by_zuul,
    fast_shooting,
    nailed,
    flared,
    warlock_charged,
    wound,

    END
};

enum class Prop_turn_mode
{
    std,
    actor
};

enum class Prop_turns
{
    std,
    specific,
    indefinite
};

enum class Prop_msg
{
    start_player,
    start_mon,
    end_player,
    end_mon,
    res_player,
    res_mon,
    END
};

enum class Prop_alignment
{
    good,
    bad,
    neutral
};

enum class Prop_src
{
    //Properties applied by potions, spells, etc, "natural" properties that monsters can
    //start with (e.g. flying), or player properties gained by traits
    intr,

    //Properties applied by items carried in inventory
    inv,

    END
};

struct Prop_data_t
{
    Prop_data_t() :
        id                              (Prop_id::END),
        std_rnd_turns                   (Range(10, 10)),
        name                            (""),
        name_short                      (""),
        is_making_mon_aware             (false),
        allow_display_turns             (true),
        allow_apply_more_while_active   (true),
        update_vision_when_start_or_end (false),
        allow_test_on_bot               (false),
        alignment                       (Prop_alignment::bad)
    {
        for (int i = 0; i < int(Prop_msg::END); ++i)
        {
            msg[i] = "";
        }
    }

    Prop_id         id;
    Range           std_rnd_turns;
    std::string     name;
    std::string     name_short;
    std::string     msg[size_t(Prop_msg::END)];
    bool            is_making_mon_aware;
    bool            allow_display_turns;
    bool            allow_apply_more_while_active;
    bool            update_vision_when_start_or_end;
    bool            allow_test_on_bot;
    Prop_alignment  alignment;
};

namespace prop_data
{

extern Prop_data_t data[size_t(Prop_id::END)];

void init();

} //Prop_data

class Actor;
class Wpn;
class Prop;
class Item;

//Each actor has an instance of this
class Prop_handler
{
public:
    Prop_handler(Actor* owning_actor);

    ~Prop_handler();

    //Adds all natural properties set in the actor data
    void init_natural_props();

    void save() const;

    void load();

    //All properties must be added through this function (can also be done via the other "add"
    //methods, which will then call "try_add_prop()")
    void try_add_prop(Prop* const prop,
                      Prop_src src = Prop_src::intr,
                      const bool FORCE_EFFECT = false,
                      const Verbosity verbosity = Verbosity::verbose);

    void try_add_prop_from_att(const Wpn& wpn, const bool IS_MELEE);

    //The following two methods are supposed to be called by items
    void add_prop_from_equipped_item(const Item* const item,
                                     Prop* const prop,
                                     const Verbosity verbosity);

    void remove_props_for_item(const Item* const item);

    //Fast method for checking if a certain property id is applied
    bool has_prop(const Prop_id id) const
    {
        return active_props_info_[size_t(id)] > 0;
    }

    Prop* prop(const Prop_id id) const;

    bool end_prop(const Prop_id id, const bool RUN_PROP_END_EFFECTS = true);

    void props_interface_line(std::vector<Str_and_clr>& line) const;

    void apply_actor_turn_prop_buffer();

    Prop* mk_prop(const Prop_id id, Prop_turns turns_init, const int NR_TURNS = -1) const;

    //-----------------------------------------------------------------------------
    // Hooks called from various places
    //-----------------------------------------------------------------------------
    void affect_move_dir(const P& actor_pos, Dir& dir) const;

    int affect_max_hp(const int HP_MAX) const;

    bool allow_attack(const Verbosity verbosity) const;
    bool allow_attack_melee(const Verbosity verbosity) const;
    bool allow_attack_ranged(const Verbosity verbosity) const;
    bool allow_see() const;
    bool allow_move() const;
    bool allow_act() const;
    bool allow_read(const Verbosity verbosity) const;
    bool allow_cast_spell(const Verbosity verbosity) const;
    bool allow_speak(const Verbosity verbosity) const;
    bool allow_eat(const Verbosity verbosity) const; //Also used for drinking

    void on_hit();
    void on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR);

    int ability_mod(const Ability_id ability) const;

    bool affect_actor_clr(Clr& clr) const;

    void tick(const Prop_turn_mode turn_mode);

    bool try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const;

private:
    bool try_resist_prop(const Prop_id id) const;

    //This prints messages, updates FOV, etc, and also calls the on_end() property hook. It does
    //NOT remove the property from the vector or decrement the active property info. The caller is
    //responsible for this.
    void on_prop_end(Prop* const prop);

    void incr_active_props_info(const Prop_id id);
    void decr_active_props_info(const Prop_id id);

    std::vector<Prop*> props_;
    std::vector<Prop*> actor_turn_prop_buffer_;

    //This array is only used for optimization and convenience of asking the property handler which
    //properties are currently active (see the "has_prop()" method above). It is used as a cache,
    //so that we need to search through the vector as little as possible.
    int active_props_info_[size_t(Prop_id::END)];

    Actor* owning_actor_;
};

class Prop
{
public:
    Prop(Prop_id id, Prop_turns turns_init, int nr_turns = -1);

    virtual ~Prop() {}

    virtual void save() const {}

    virtual void load() {}

    Prop_id id() const
    {
        return id_;
    }

    int nr_turns_left() const
    {
        return nr_turns_left_;
    }

    void set_nr_turns_left(const int NR_TURNS)
    {
        nr_turns_left_ = NR_TURNS;
    }

    virtual bool is_finished() const
    {
        return nr_turns_left_ == 0;
    }

    virtual Prop_alignment alignment() const
    {
        return data_.alignment;
    }

    virtual bool allow_display_turns() const
    {
        return data_.allow_display_turns;
    }

    virtual bool is_making_mon_aware() const
    {
        return data_.is_making_mon_aware;
    }

    virtual std::string name() const
    {
        return data_.name;
    }

    virtual std::string name_short() const
    {
        return data_.name_short;
    }

    virtual void msg(const Prop_msg msg_type, std::string& msg_ref) const
    {
        msg_ref = data_.msg[size_t(msg_type)];
    }

    virtual bool allow_apply_more_while_active() const
    {
        return data_.allow_apply_more_while_active;
    }

    virtual bool need_update_vision_when_start_or_end() const
    {
        return data_.update_vision_when_start_or_end;
    }

    virtual bool allow_see() const
    {
        return true;
    }

    virtual bool allow_move() const
    {
        return true;
    }

    virtual bool allow_act() const
    {
        return true;
    }

    virtual void on_hit() {}

    //Returns a pointer to the property if it's still active, otherwise nullptr is returned
    virtual Prop* on_new_turn()
    {
        return this;
    }

    virtual void on_start() {}
    virtual void on_end() {}
    virtual void on_more() {}

    virtual void on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR)
    {
        (void)IS_PLAYER_SEE_OWNING_ACTOR;
    }

    virtual int affect_max_hp(const int HP_MAX) const
    {
        return HP_MAX;
    }

    virtual bool affect_actor_clr(Clr& clr) const
    {
        (void)clr;
        return false;
    }

    virtual bool allow_attack_melee(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_attack_ranged(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_read(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_cast_spell(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_speak(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_eat(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual int ability_mod(const Ability_id ability) const
    {
        (void)ability;
        return 0;
    }

    virtual void affect_move_dir(const P& actor_pos, Dir& dir)
    {
        (void)actor_pos;
        (void)dir;
    }

    virtual bool is_resisting_other_prop(const Prop_id prop_id) const
    {
        (void)prop_id;
        return false;
    }

    virtual bool try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const
    {
        (void)dmg_type;
        (void)verbosity;
        return false;
    }

    virtual Prop_turn_mode turn_mode() const
    {
        return Prop_turn_mode::std;
    }

    Prop_turns turns_init_type() const
    {
        return turns_init_type_;
    }

    Prop_src src() const
    {
        return src_;
    }

protected:
    friend class Prop_handler;

    const Prop_id id_;
    const Prop_data_t& data_;

    int nr_turns_left_;

    //How the prop turns was inited (std, specific, indefinite). This is used for example to make
    //copies of a property to apply on melee attacks.
    Prop_turns turns_init_type_;

    Actor* owning_actor_;
    Prop_src src_;
    const Item* item_applying_;
};

class Prop_terrified: public Prop
{
public:
    Prop_terrified(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::terrified, turns_init, nr_turns) {}

    int ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::dodge_att:
            return 20;

        case Ability_id::ranged:
            return -20;

        default:
            break;
        }

        return 0;
    }

    bool allow_attack_melee(const Verbosity verbosity) const override;

    bool allow_attack_ranged(const Verbosity verbosity) const override;

    void on_start() override;
};

class Prop_weakened: public Prop
{
public:
    Prop_weakened(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::weakened, turns_init, nr_turns) {}
};

class Prop_infected: public Prop
{
public:
    Prop_infected(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::infected, turns_init, nr_turns) {}

    Prop* on_new_turn() override;
};

class Prop_diseased: public Prop
{
public:
    Prop_diseased(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::diseased, turns_init, nr_turns) {}

    int affect_max_hp(const int HP_MAX) const override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;

    void on_start() override;
    void on_end() override;
};

class Prop_descend: public Prop
{
public:
    Prop_descend(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::descend, turns_init, nr_turns) {}

    Prop* on_new_turn() override;
};

class Prop_flying: public Prop
{
public:
    Prop_flying(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::flying, turns_init, nr_turns) {}
};

class Prop_ethereal: public Prop
{
public:
    Prop_ethereal(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::ethereal, turns_init, nr_turns) {}
};

class Prop_ooze: public Prop
{
public:
    Prop_ooze(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::ooze, turns_init, nr_turns) {}
};

class Prop_burrowing: public Prop
{
public:
    Prop_burrowing(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::burrowing, turns_init, nr_turns) {}

    Prop* on_new_turn() override;
};

class Prop_poss_by_zuul: public Prop
{
public:
    Prop_poss_by_zuul(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::poss_by_zuul, turns_init, nr_turns) {}

    void on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR) override;

    int affect_max_hp(const int HP_MAX) const override
    {
        return HP_MAX * 2;
    }
};

class Prop_poisoned: public Prop
{
public:
    Prop_poisoned(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::poisoned, turns_init, nr_turns) {}

    Prop* on_new_turn() override;
};

class Prop_fast_shooting: public Prop
{
public:
    Prop_fast_shooting(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::fast_shooting, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }
};

class Prop_blind: public Prop
{
public:
    Prop_blind(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::blind, turns_init, nr_turns) {}

    bool need_update_vision_when_start_or_end() const override;

    bool allow_see() const override
    {
        return false;
    }

    int ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::searching:
            return -9999;

        case Ability_id::ranged:
            return -50;

        case Ability_id::melee:
            return -25;

        case Ability_id::dodge_trap:
        case Ability_id::dodge_att:
            return -50;

        default:
            break;
        }

        return 0;
    }
};

class Prop_radiant: public Prop
{
public:
    Prop_radiant(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::radiant, turns_init, nr_turns) {}
};

class Prop_invisible: public Prop
{
public:
    Prop_invisible(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::invis, turns_init, nr_turns) {}

    bool affect_actor_clr(Clr& clr) const override
    {
        clr = clr_gray;
        return true;
    }
};

class Prop_see_invis: public Prop
{
public:
    Prop_see_invis(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::see_invis, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;
};

class Prop_infravis: public Prop
{
public:
    Prop_infravis(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::infravis, turns_init, nr_turns) {}
};

class Prop_blessed: public Prop
{
public:
    Prop_blessed(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::blessed, turns_init, nr_turns) {}

    void on_start() override;

    int ability_mod(const Ability_id ability) const override
    {
        if (ability == Ability_id::searching)
        {
            return 0;
        }

        return 10;
    }
};

class Prop_cursed: public Prop
{
public:
    Prop_cursed(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::cursed, turns_init, nr_turns) {}

    int ability_mod(const Ability_id ability) const override
    {
        (void)ability;
        return -10;
    }

    void on_start() override;

    void on_end() override;
};

class Prop_burning: public Prop
{
public:
    Prop_burning(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::burning, turns_init, nr_turns) {}

    bool allow_read(const Verbosity verbosity) const override;
    bool allow_cast_spell(const Verbosity verbosity) const override;

    bool affect_actor_clr(Clr& clr) const override
    {
        clr = clr_red_lgt;
        return true;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override;

    Prop* on_new_turn() override;
};

class Prop_flared: public Prop
{
public:
    Prop_flared(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::flared, turns_init, nr_turns) {}

    Prop* on_new_turn() override;
};

class Prop_warlock_charged: public Prop
{
public:
    Prop_warlock_charged(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::warlock_charged, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }
};

class Prop_confused: public Prop
{
public:
    Prop_confused(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::confused, turns_init, nr_turns) {}

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    bool allow_read(const Verbosity verbosity) const override;
    bool allow_cast_spell(const Verbosity verbosity) const override;
    bool allow_attack_melee(const Verbosity verbosity) const override;
    bool allow_attack_ranged(const Verbosity verbosity) const override;
};

class Prop_stunned: public Prop
{
public:
    Prop_stunned(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::stunned, turns_init, nr_turns) {}
};

class Prop_nailed: public Prop
{
public:
    Prop_nailed(Prop_turns turns_init, int nr_turns = -1) :
        Prop        (Prop_id::nailed, turns_init, nr_turns),
        nr_spikes_  (1) {}

    std::string name_short() const override
    {
        return "Nailed(" + to_str(nr_spikes_) + ")";
    }

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    void on_more() override
    {
        ++nr_spikes_;
    }

    bool is_finished() const override
    {
        return nr_spikes_ <= 0;
    }

private:
    int nr_spikes_;
};

class Prop_wound: public Prop
{
public:
    Prop_wound(Prop_turns turns_init, int nr_turns = -1) :
        Prop        (Prop_id::wound, turns_init, nr_turns),
        nr_wounds_  (1) {}

    void save() const;

    void load();

    void msg(const Prop_msg msg_type, std::string& msg_ref) const override;

    std::string name_short() const override
    {
        return "Wound(" + to_str(nr_wounds_) + ")";
    }

    int ability_mod(const Ability_id ability) const override;

    void on_more() override;

    bool is_finished() const override
    {
        return nr_wounds_ <= 0;
    }

    int nr_wounds() const
    {
        return nr_wounds_;
    }

    void heal_one_wound();

private:
    int nr_wounds_;
};

class Prop_waiting: public Prop
{
public:
    Prop_waiting(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::waiting, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }

    bool allow_move() const override
    {
        return false;
    }

    bool allow_act() const override
    {
        return false;
    }

    bool allow_attack_melee(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }
};

class Prop_disabled_attack: public Prop
{
public:
    Prop_disabled_attack(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::disabled_attack, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }

    bool allow_attack_melee(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }
};

class Prop_disabled_melee: public Prop
{
public:
    Prop_disabled_melee(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::disabled_melee, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }

    bool allow_attack_melee(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }
};

class Prop_disabled_ranged: public Prop
{
public:
    Prop_disabled_ranged(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::disabled_ranged, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }
};

class Prop_paralyzed: public Prop
{
public:
    Prop_paralyzed(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::paralyzed, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }

    void on_start() override;

    bool allow_act() const override
    {
        return false;
    }

    int ability_mod(const Ability_id ability) const override
    {
        if (ability == Ability_id::dodge_trap || ability == Ability_id::dodge_att)
        {
            return -999;
        }

        return 0;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }

    bool allow_attack_melee(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }
};

class Prop_fainted: public Prop
{
public:
    Prop_fainted(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::fainted, turns_init, nr_turns) {}

    bool need_update_vision_when_start_or_end() const override;

    bool allow_act() const override
    {
        return false;
    }

    bool allow_see() const override
    {
        return false;
    }

    int ability_mod(const Ability_id ability) const override
    {
        if (ability == Ability_id::dodge_trap || ability == Ability_id::dodge_att)
        {
            return -999;
        }

        return 0;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }

    bool allow_attack_melee(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }

    void on_hit() override
    {
        nr_turns_left_ = 0;
    }
};

class Prop_slowed: public Prop
{
public:
    Prop_slowed(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::slowed, turns_init, nr_turns) {}

    void on_start() override;

    int ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::dodge_att:
            return -30;

        case Ability_id::ranged:
            return -10;

        case Ability_id::melee:
            return -10;

        default:
            break;
        }

        return 0;
    }
};

class Prop_hasted: public Prop
{
public:
    Prop_hasted(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::hasted, turns_init, nr_turns) {}

    void on_start() override;

    int ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::dodge_att:
            return 10;

        case Ability_id::ranged:
            return 5;

        case Ability_id::melee:
            return 5;

        default:
            break;
        }

        return 0;
    }
};

class Prop_frenzied: public Prop
{
public:
    Prop_frenzied(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::frenzied, turns_init, nr_turns) {}

    void on_start() override;
    void on_end() override;

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    bool allow_read(const Verbosity verbosity) const override;
    bool allow_cast_spell(const Verbosity verbosity) const override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;

    int ability_mod(const Ability_id ability) const override
    {
        if (ability == Ability_id::melee)
        {
            return 999;
        }

        if (ability == Ability_id::dodge_att || ability == Ability_id::dodge_trap)
        {
            return -20;
        }

        return 0;
    }
};

class Prop_rAcid: public Prop
{
public:
    Prop_rAcid(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rAcid, turns_init, nr_turns) {}

    bool try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const override;
};

class Prop_rConf: public Prop
{
public:
    Prop_rConf(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rConf, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;
};

class Prop_rElec: public Prop
{
public:
    Prop_rElec(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rElec, turns_init, nr_turns) {}

    bool try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const override;
};

class Prop_rFear: public Prop
{
public:
    Prop_rFear(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rFear, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;
};

class Prop_rPhys: public Prop
{
public:
    Prop_rPhys(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rPhys, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;

    bool try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const override;
};

class Prop_rFire: public Prop
{
public:
    Prop_rFire(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rFire, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;

    bool try_resist_dmg(const Dmg_type dmg_type, const Verbosity verbosity) const override;
};

class Prop_rPoison: public Prop
{
public:
    Prop_rPoison(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rPoison, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;
};

class Prop_rSleep: public Prop
{
public:
    Prop_rSleep(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rSleep, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;
};

class Prop_rDisease: public Prop
{
public:
    Prop_rDisease(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rDisease, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id prop_id) const override;
};

class Prop_rBreath: public Prop
{
public:
    Prop_rBreath(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rBreath, turns_init, nr_turns) {}
};

class Prop_lgtSens: public Prop
{
public:
    Prop_lgtSens(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::lgtSens, turns_init, nr_turns) {}
    ~Prop_lgtSens() override {}
};

class Prop_tele_control: public Prop
{
public:
    Prop_tele_control(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::tele_ctrl, turns_init, nr_turns) {}
    ~Prop_tele_control() override {}
};

class Prop_rSpell: public Prop
{
public:
    Prop_rSpell(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::rSpell, turns_init, nr_turns) {}
};

class Prop_spell_reflect: public Prop
{
public:
    Prop_spell_reflect(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::spell_reflect, turns_init, nr_turns) {}

    ~Prop_spell_reflect() override {}
};

class Prop_conflict: public Prop
{
public:
    Prop_conflict(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::conflict, turns_init, nr_turns) {}
    ~Prop_conflict() override {}
};

class Prop_strangled: public Prop
{
public:
    Prop_strangled(Prop_turns turns_init, int nr_turns = -1) :
        Prop(Prop_id::strangled, turns_init, nr_turns) {}

    Prop_turn_mode turn_mode() const override
    {
        return Prop_turn_mode::actor;
    }

    Prop* on_new_turn() override;

    bool allow_speak(const Verbosity verbosity) const override;
    bool allow_eat(const Verbosity verbosity) const override;
};

#endif

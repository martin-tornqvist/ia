#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include <string>
#include <vector>

#include "ability_values.hpp"
#include "rl_utils.hpp"
#include "global.hpp"

class Actor;
class Wpn;
class Prop;
class Item;

enum class PropId
{
    r_phys,
    r_fire,
    r_poison,
    r_elec,
    r_acid,
    r_sleep,
    r_fear,
    r_slow,
    r_conf,
    r_breath,
    r_disease,
    //
    // NOTE: The purpose of this is only to prevent blindness for "eyeless"
    //       monsters (e.g. constructs such as animated weapons), and is only
    //       intended as a natural property - not for e.g. gas masks.
    //
    r_blind,
    r_para, // Mostly intended as a natural property for monsters
    lgt_sens,
    blind,
    deaf,
    fainted,
    burning,
    radiant,
    invis,
    cloaked,
    see_invis,
    darkvis,
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
    tele_ctrl,
    spell_reflect,
    strangled,
    conflict,

    // Properties describing the actors body and/or method of moving around
    flying,
    ethereal,
    ooze,
    burrowing,

    // Properties mostly used for AI control
    waiting,
    disabled_attack,
    disabled_melee,
    disabled_ranged,

    // Special (for supporting very specific game mechanics)
    descend,
    poss_by_zuul,
    aiming,
    nailed,
    flared,
    wound,
    r_spell,
    clockwork_hasted, // For the Arcane Clockwork artifact
    summoned,
    hp_sap,
    spi_sap,
    mind_sap,

    END
};

enum class PropTurns
{
    std,
    specific,
    indefinite
};

enum class PropMsg
{
    start_player,
    start_mon,
    end_player,
    end_mon,
    res_player,
    res_mon,
    END
};

enum class PropAlignment
{
    good,
    bad,
    neutral
};

enum class PropSrc
{
    // Properties applied by potions, spells, etc, or "natural" properties for
    // monsters (e.g. flying), or player properties gained by traits
    intr,

    //Properties applied by items carried in inventory
    inv,

    END
};

struct PropDataT
{
    PropDataT() :
        id                              (PropId::END),
        std_rnd_turns                   (Range(10, 10)),
        name                            (""),
        name_short                      (""),
        descr                           (""),
        is_making_mon_aware             (false),
        allow_display_turns             (true),
        update_vision_when_start_or_end (false),
        allow_test_on_bot               (false),
        alignment                       (PropAlignment::bad)
    {
        for (size_t i = 0; i < (size_t)PropMsg::END; ++i)
        {
            msg[i] = "";
        }
    }

    PropId id;
    Range std_rnd_turns;
    std::string name;
    std::string name_short;
    std::string descr;
    std::string msg[(size_t)PropMsg::END];
    bool is_making_mon_aware;
    bool allow_display_turns;
    bool update_vision_when_start_or_end;
    bool allow_test_on_bot;
    PropAlignment alignment;
};

struct DmgResistData
{
    DmgResistData() :
        is_resisted         (false),
        resist_msg_player   (),
        resist_msg_mon      () {}

    bool is_resisted;
    std::string resist_msg_player;
    // Not including monster name, e.g. " seems unaffected"
    std::string resist_msg_mon;
};

struct PropListEntry
{
    PropListEntry() :
        title   (),
        descr   (),
        prop    (nullptr) {}

    StrAndClr title;

    std::string descr;

    const Prop* prop;
};

namespace prop_data
{

extern PropDataT data[(size_t)PropId::END];

void init();

} // prop_data

// Each actor has an instance of this
class PropHandler
{
public:
    PropHandler(Actor* owning_actor);

    ~PropHandler();

    // Adds all natural properties set in the actor data
    void init_natural_props();

    void save() const;

    void load();

    // All properties must be added through this function (can also be done via
    // the other "apply" methods, which will then call "apply")
    void apply(Prop* const prop,
               PropSrc src = PropSrc::intr,
               const bool force_effect = false,
               const Verbosity verbosity = Verbosity::verbose);

    void apply_from_att(const Wpn& wpn, const bool is_melee);

    // The following two methods are supposed to be called by items
    void add_prop_from_equipped_item(const Item* const item,
                                     Prop* const prop,
                                     const Verbosity verbosity);

    void remove_props_for_item(const Item* const item);

    // Fast method for checking if a certain property id is applied
    bool has_prop(const PropId id) const
    {
        return active_props_info_[(size_t)id] > 0;
    }

    Prop* prop(const PropId id) const;

    bool end_prop(const PropId id,
                  const bool run_prop_end_effects = true);

    // A line of property names of the short form
    std::vector<StrAndClr> props_line() const;

    // A list of properties names of the full form, with descriptions
    std::vector<PropListEntry> props_list() const;

    Prop* mk_prop(const PropId id,
                  PropTurns turns_init,
                  const int nr_turns = -1) const;

    // Used for monster description property list
    std::vector<PropListEntry> temporary_negative_prop_list();
    bool has_temporary_negative_prop_mon();

    //--------------------------------------------------------------------------
    // Hooks called from various places
    //--------------------------------------------------------------------------
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
    bool allow_eat(const Verbosity verbosity) const; // Also used for drinking

    //
    // NOTE: The allow_*_absolute methods below answer if some action could
    //       EVER be performed, and the allow_*_chance methods allows the
    //       action with a random chance.
    //       For example, blindness never allows the player to read scrolls, and
    //       the game won't let the player try, and waste a scroll. But burning
    //       will allow the player to try, with a certain percent chance of
    //       success, and the scroll will be wasted.
    //       (All plain allow_* methods above are also considered "absolute".)
    //
    bool allow_read_absolute(const Verbosity verbosity) const;
    bool allow_read_chance(const Verbosity verbosity) const;
    bool allow_cast_intr_spell_absolute(const Verbosity verbosity) const;
    bool allow_cast_intr_spell_chance(const Verbosity verbosity) const;

    void on_hit();
    void on_death(const bool is_player_see_owning_actor);

    int ability_mod(const AbilityId ability) const;

    bool affect_actor_clr(Clr& clr) const;

    // Called when the actors turn begins
    void on_turn_begin();

    // Called when the actors turn ends
    void on_turn_end();

    bool is_resisting_dmg(const DmgType dmg_type,
                          const Actor* const attacker,
                          const Verbosity verbosity) const;

private:
    bool is_temporary_negative_prop_mon(const Prop& prop);

    bool is_resisting_prop(const PropId id) const;

    // This prints messages, updates FOV, etc, and also calls the on_end()
    // property hook. It does NOT remove the property from the vector or
    // decrement the active property info. The caller is responsible for this.
    void on_prop_end(Prop* const prop);

    void incr_active_props_info(const PropId id);
    void decr_active_props_info(const PropId id);

    std::vector<Prop*> props_;

    // This array is only used for optimization and convenience of asking the
    // property handler which properties are currently active (see the
    // "has_prop()" method above). It is used as a cache, so that we need to
    // search through the vector as little as possible.
    int active_props_info_[(size_t)PropId::END];

    Actor* owning_actor_;
};

class Prop
{
public:
    Prop(PropId id, PropTurns turns_init, int nr_turns = -1);

    virtual ~Prop() {}

    virtual void save() const {}

    virtual void load() {}

    PropId id() const
    {
        return id_;
    }

    int nr_turns_left() const
    {
        return nr_turns_left_;
    }

    void set_nr_turns_left(const int nr_turns)
    {
        nr_turns_left_ = nr_turns;
    }

    virtual bool is_finished() const
    {
        return nr_turns_left_ == 0;
    }

    virtual PropAlignment alignment() const
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

    std::string descr() const
    {
        return data_.descr;
    }

    virtual void msg(const PropMsg msg_type, std::string& msg_ref) const
    {
        msg_ref = data_.msg[size_t(msg_type)];
    }

    //
    // TODO: This is ridiculous! Can't the properties just call vision updating
    //       themselves (whatever is needed per case)???
    //
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

    // Returns a pointer to the property if it's still active, otherwise
    // nullptr is returned
    virtual Prop* on_tick()
    {
        return this;
    }

    virtual void on_start() {}
    virtual void on_end() {}
    virtual void on_more(const Prop& new_prop)
    {
        (void)new_prop;
    }

    virtual void on_death(const bool is_player_see_owning_actor)
    {
        (void)is_player_see_owning_actor;
    }

    virtual int affect_max_hp(const int hp_max) const
    {
        return hp_max;
    }

    virtual int affect_max_spi(const int spi_max) const
    {
        return spi_max;
    }

    virtual int affect_shock(const int shock) const
    {
        return shock;
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

    virtual bool allow_read_absolute(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_read_chance(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_cast_intr_spell_absolute(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual bool allow_cast_intr_spell_chance(const Verbosity verbosity) const
    {
        (void)verbosity;
        return true;
    }

    virtual int ability_mod(const AbilityId ability) const
    {
        (void)ability;
        return 0;
    }

    virtual void affect_move_dir(const P& actor_pos, Dir& dir)
    {
        (void)actor_pos;
        (void)dir;
    }

    virtual bool is_resisting_other_prop(const PropId prop_id) const
    {
        (void)prop_id;
        return false;
    }

    virtual DmgResistData is_resisting_dmg(const DmgType dmg_type) const
    {
        (void)dmg_type;

        return DmgResistData();
    }

    PropTurns turns_init_type() const
    {
        return turns_init_type_;
    }

    PropSrc src() const
    {
        return src_;
    }

protected:
    friend class PropHandler;

    const PropId id_;
    const PropDataT& data_;

    int nr_turns_left_;

    // How the prop turns was inited (std, specific, indefinite). This is used
    // for example to make copies of a property to apply on melee attacks.
    PropTurns turns_init_type_;

    Actor* owning_actor_;
    PropSrc src_;
    const Item* item_applying_;
};

class PropTerrified: public Prop
{
public:
    PropTerrified(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::terrified, turns_init, nr_turns) {}

    int ability_mod(const AbilityId ability) const override
    {
        switch (ability)
        {
        case AbilityId::dodging:
            return 20;

        case AbilityId::ranged:
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

class PropWeakened: public Prop
{
public:
    PropWeakened(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::weakened, turns_init, nr_turns) {}
};

class PropInfected: public Prop
{
public:
    PropInfected(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::infected, turns_init, nr_turns) {}

    Prop* on_tick() override;
};

class PropDiseased: public Prop
{
public:
    PropDiseased(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::diseased, turns_init, nr_turns) {}

    int affect_max_hp(const int hp_max) const override;

    bool is_resisting_other_prop(const PropId prop_id) const override;

    void on_start() override;
    void on_end() override;
};

class PropDescend: public Prop
{
public:
    PropDescend(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::descend, turns_init, nr_turns) {}

    Prop* on_tick() override;
};

class PropFlying: public Prop
{
public:
    PropFlying(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::flying, turns_init, nr_turns) {}
};

class PropEthereal: public Prop
{
public:
    PropEthereal(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::ethereal, turns_init, nr_turns) {}
};

class PropOoze: public Prop
{
public:
    PropOoze(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::ooze, turns_init, nr_turns) {}
};

class PropBurrowing: public Prop
{
public:
    PropBurrowing(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::burrowing, turns_init, nr_turns) {}

    Prop* on_tick() override;
};

class PropPossByZuul: public Prop
{
public:
    PropPossByZuul(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::poss_by_zuul, turns_init, nr_turns) {}

    void on_death(const bool is_player_see_owning_actor) override;

    int affect_max_hp(const int hp_max) const override
    {
        return hp_max * 2;
    }
};

class PropPoisoned: public Prop
{
public:
    PropPoisoned(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::poisoned, turns_init, nr_turns) {}

    Prop* on_tick() override;
};

class PropAiming: public Prop
{
public:
    PropAiming(PropTurns turns_init, int nr_turns = -1) :
        Prop                (PropId::aiming, turns_init, nr_turns),
        nr_turns_aiming_    (1) {}

    std::string name_short() const override
    {
        return data_.name_short + ((nr_turns_aiming_ >= 3) ? "(3)" : "");
    }

    int ability_mod(const AbilityId ability) const override
    {
        if (ability == AbilityId::ranged)
        {
            return nr_turns_aiming_ >= 3 ? 999 : 20;
        }

        return 0;
    }

    bool is_max_ranged_dmg() const
    {
        return nr_turns_aiming_ >= 3;
    }

    int nr_turns_aiming_;
};

class PropBlind: public Prop
{
public:
    PropBlind(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::blind, turns_init, nr_turns) {}

    bool need_update_vision_when_start_or_end() const override;

    bool allow_read_absolute(const Verbosity verbosity) const override;

    bool allow_see() const override
    {
        return false;
    }

    int ability_mod(const AbilityId ability) const override
    {
        switch (ability)
        {
        case AbilityId::searching:
            return -9999;

        case AbilityId::ranged:
            return -20;

        case AbilityId::melee:
            return -20;

        case AbilityId::dodging:
            return -50;

        default:
            break;
        }

        return 0;
    }
};

class PropDeaf: public Prop
{
public:
    PropDeaf(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::deaf, turns_init, nr_turns) {}
};

class PropRadiant: public Prop
{
public:
    PropRadiant(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::radiant, turns_init, nr_turns) {}
};

class PropInvisible: public Prop
{
public:
    PropInvisible(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::invis, turns_init, nr_turns) {}
};

class PropCloaked: public Prop
{
public:
    PropCloaked(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::cloaked, turns_init, nr_turns) {}
};

class PropSeeInvis: public Prop
{
public:
    PropSeeInvis(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::see_invis, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropDarkvis: public Prop
{
public:
    PropDarkvis(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::darkvis, turns_init, nr_turns) {}
};

class PropBlessed: public Prop
{
public:
    PropBlessed(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::blessed, turns_init, nr_turns) {}

    void on_start() override;

    void on_more(const Prop& new_prop) override;

    int ability_mod(const AbilityId ability) const override
    {
        (void)ability;

        return 5;
    }

private:
    void bless_adjacent() const;
};

class PropCursed: public Prop
{
public:
    PropCursed(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::cursed, turns_init, nr_turns) {}

    int ability_mod(const AbilityId ability) const override
    {
        (void)ability;

        return -5;
    }

    void on_start() override;

    void on_more(const Prop& new_prop) override;

    void on_end() override;

private:
    void curse_adjacent() const;
};

class PropBurning: public Prop
{
public:
    PropBurning(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::burning, turns_init, nr_turns) {}

    bool allow_read_chance(const Verbosity verbosity) const override;
    bool allow_cast_intr_spell_chance(const Verbosity verbosity) const override;

    int ability_mod(const AbilityId ability) const override
    {
        (void)ability;
        return -30;
    }

    bool affect_actor_clr(Clr& clr) const override
    {
        clr = clr_red_lgt;
        return true;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override;

    Prop* on_tick() override;
};

class PropFlared: public Prop
{
public:
    PropFlared(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::flared, turns_init, nr_turns) {}

    Prop* on_tick() override;
};

class PropConfused: public Prop
{
public:
    PropConfused(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::confused, turns_init, nr_turns) {}

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    bool allow_attack_melee(const Verbosity verbosity) const override;
    bool allow_attack_ranged(const Verbosity verbosity) const override;
    bool allow_read_absolute(const Verbosity verbosity) const override;
    bool allow_cast_intr_spell_absolute(
        const Verbosity verbosity) const override;
};

class PropStunned: public Prop
{
public:
    PropStunned(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::stunned, turns_init, nr_turns) {}
};

class PropNailed: public Prop
{
public:
    PropNailed(PropTurns turns_init, int nr_turns = -1) :
        Prop        (PropId::nailed, turns_init, nr_turns),
        nr_spikes_  (1) {}

    std::string name_short() const override
    {
        return "Nailed(" + std::to_string(nr_spikes_) + ")";
    }

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    void on_more(const Prop& new_prop) override
    {
        (void)new_prop;

        ++nr_spikes_;
    }

    bool is_finished() const override
    {
        return nr_spikes_ <= 0;
    }

private:
    int nr_spikes_;
};

class PropWound: public Prop
{
public:
    PropWound(PropTurns turns_init, int nr_turns = -1) :
        Prop        (PropId::wound, turns_init, nr_turns),
        nr_wounds_  (1) {}

    void save() const override;

    void load() override;

    void msg(const PropMsg msg_type, std::string& msg_ref) const override;

    std::string name_short() const override
    {
        return "Wound(" + std::to_string(nr_wounds_) + ")";
    }

    int ability_mod(const AbilityId ability) const override;

    void on_more(const Prop& new_prop) override;

    bool is_finished() const override
    {
        return nr_wounds_ <= 0;
    }

    int affect_max_hp(const int hp_max) const override;

    int nr_wounds() const
    {
        return nr_wounds_;
    }

    void heal_one_wound();

private:
    int nr_wounds_;
};

class PropHpSap: public Prop
{
public:
    PropHpSap(PropTurns turns_init, int nr_turns = -1);

    void save() const override;

    void load() override;

    std::string name_short() const override
    {
        return "hp(" + std::to_string(nr_drained_) + ")";
    }

    void on_more(const Prop& new_prop) override;

    int affect_max_hp(const int hp_max) const override;

private:
    int nr_drained_;
};

class PropSpiSap: public Prop
{
public:
    PropSpiSap(PropTurns turns_init, int nr_turns = -1);

    void save() const override;

    void load() override;

    std::string name_short() const override
    {
        return "spi(" + std::to_string(nr_drained_) + ")";
    }

    void on_more(const Prop& new_prop) override;

    int affect_max_spi(const int spi_max) const override;

private:
    int nr_drained_;
};

class PropMindSap: public Prop
{
public:
    PropMindSap(PropTurns turns_init, int nr_turns = -1);

    void save() const override;

    void load() override;

    std::string name_short() const override
    {
        return "shock(" + std::to_string(nr_drained_) + "%)";
    }

    void on_more(const Prop& new_prop) override;

    int affect_shock(const int shock) const override;

private:
    int nr_drained_;
};

class PropWaiting: public Prop
{
public:
    PropWaiting(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::waiting, turns_init, nr_turns) {}

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

class PropDisabledAttack: public Prop
{
public:
    PropDisabledAttack(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::disabled_attack, turns_init, nr_turns) {}

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

class PropDisabledMelee: public Prop
{
public:
    PropDisabledMelee(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::disabled_melee, turns_init, nr_turns) {}

    bool allow_attack_melee(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }
};

class PropDisabledRanged: public Prop
{
public:
    PropDisabledRanged(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::disabled_ranged, turns_init, nr_turns) {}

    bool allow_attack_ranged(const Verbosity verbosity) const override
    {
        (void)verbosity;
        return false;
    }
};

class PropParalyzed: public Prop
{
public:
    PropParalyzed(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::paralyzed, turns_init, nr_turns) {}

    void on_start() override;

    int ability_mod(const AbilityId ability) const override
    {
        if (ability == AbilityId::dodging)
        {
            return -999;
        }

        return 0;
    }

    bool allow_act() const override
    {
        return false;
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

class PropFainted: public Prop
{
public:
    PropFainted(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::fainted, turns_init, nr_turns) {}

    bool need_update_vision_when_start_or_end() const override;

    int ability_mod(const AbilityId ability) const override
    {
        if (ability == AbilityId::dodging)
        {
            return -999;
        }

        return 0;
    }

    bool allow_act() const override
    {
        return false;
    }

    bool allow_see() const override
    {
        return false;
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

class PropSlowed: public Prop
{
public:
    PropSlowed(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::slowed, turns_init, nr_turns) {}

    void on_start() override;
};

class PropHasted: public Prop
{
public:
    PropHasted(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::hasted, turns_init, nr_turns) {}

    void on_start() override;
};

class PropClockworkHasted: public Prop
{
public:
    PropClockworkHasted(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::clockwork_hasted, turns_init, nr_turns) {}
};

class PropSummoned: public Prop
{
public:
    PropSummoned(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::summoned, turns_init, nr_turns) {}

    void on_end() override;
};

class PropFrenzied: public Prop
{
public:
    PropFrenzied(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::frenzied, turns_init, nr_turns) {}

    void on_start() override;
    void on_end() override;

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    bool allow_read_absolute(const Verbosity verbosity) const override;
    bool allow_cast_intr_spell_absolute(
        const Verbosity verbosity) const override;

    bool is_resisting_other_prop(const PropId prop_id) const override;

    int ability_mod(const AbilityId ability) const override
    {
        if (ability == AbilityId::melee)
        {
            return 10;
        }

        return 0;
    }
};

class PropRAcid: public Prop
{
public:
    PropRAcid(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_acid, turns_init, nr_turns) {}

    DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRConf: public Prop
{
public:
    PropRConf(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_conf, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRElec: public Prop
{
public:
    PropRElec(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_elec, turns_init, nr_turns) {}

    DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRFear: public Prop
{
public:
    PropRFear(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_fear, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRSlow: public Prop
{
public:
    PropRSlow(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_slow, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRPhys: public Prop
{
public:
    PropRPhys(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_phys, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;

    DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRFire: public Prop
{
public:
    PropRFire(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_fire, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;

    DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRPoison: public Prop
{
public:
    PropRPoison(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_poison, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRSleep: public Prop
{
public:
    PropRSleep(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_sleep, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRDisease: public Prop
{
public:
    PropRDisease(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_disease, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRBlind: public Prop
{
public:
    PropRBlind(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_blind, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRPara: public Prop
{
public:
    PropRPara(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_para, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRBreath: public Prop
{
public:
    PropRBreath(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_breath, turns_init, nr_turns) {}
};

class PropLgtSens: public Prop
{
public:
    PropLgtSens(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::lgt_sens, turns_init, nr_turns) {}
    ~PropLgtSens() override {}
};

class PropTeleControl: public Prop
{
public:
    PropTeleControl(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::tele_ctrl, turns_init, nr_turns) {}
    ~PropTeleControl() override {}
};

class PropRSpell: public Prop
{
public:
    PropRSpell(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::r_spell, turns_init, nr_turns) {}
};

class PropSpellReflect: public Prop
{
public:
    PropSpellReflect(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::spell_reflect, turns_init, nr_turns) {}

    ~PropSpellReflect() override {}
};

class PropConflict: public Prop
{
public:
    PropConflict(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::conflict, turns_init, nr_turns) {}
    ~PropConflict() override {}
};

class PropStrangled: public Prop
{
public:
    PropStrangled(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::strangled, turns_init, nr_turns) {}

    Prop* on_tick() override;

    bool allow_speak(const Verbosity verbosity) const override;
    bool allow_eat(const Verbosity verbosity) const override;
};

#endif // PROPERTIES_HPP

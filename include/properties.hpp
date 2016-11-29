#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include <string>
#include <vector>

#include "ability_values.hpp"
#include "rl_utils.hpp"
#include "global.hpp"

enum class PropId
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
    // NOTE: The purpose of this is only to prevent blindness for "eyeless"
    //       monsters (e.g. constructs such as animated weapons), and is only
    //       intended as a natural property.
    rBlind,
    lgtSens,
    blind,
    fainted,
    burning,
    radiant,
    invis,
    see_invis,
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
    tele_ctrl,
    spell_reflect,
    strangled,
    conflict,
    descend,

    // Properties describing the actors body and/or method of moving around
    flying,
    ethereal,
    ooze,
    burrowing,

    // Properties used for AI control
    waiting,
    disabled_attack,
    disabled_melee,
    disabled_ranged,

    // Special (for supporting very specific game mechanics)
    poss_by_zuul,
    aiming,
    nailed,
    flared,
    warlock_charged,
    wound,
    rSpell,

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
        is_making_mon_aware             (false),
        allow_display_turns             (true),
        allow_apply_more_while_active   (true),
        update_vision_when_start_or_end (false),
        allow_test_on_bot               (false),
        alignment                       (PropAlignment::bad)
    {
        for (int i = 0; i < (int)PropMsg::END; ++i)
        {
            msg[i] = "";
        }
    }

    PropId          id;
    Range           std_rnd_turns;
    std::string     name;
    std::string     name_short;
    std::string     msg[(size_t)PropMsg::END];
    bool            is_making_mon_aware;
    bool            allow_display_turns;
    bool            allow_apply_more_while_active;
    bool            update_vision_when_start_or_end;
    bool            allow_test_on_bot;
    PropAlignment   alignment;
};

namespace prop_data
{

extern PropDataT data[(size_t)PropId::END];

void init();

} // prop_data

class Actor;
class Wpn;
class Prop;
class Item;

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
    // the other "add" methods, which will then call "try_add()")
    void try_add(Prop* const prop,
                 PropSrc src = PropSrc::intr,
                 const bool force_effect = false,
                 const Verbosity verbosity = Verbosity::verbose);

    void try_add_from_att(const Wpn& wpn, const bool is_melee);

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

    bool end_prop(const PropId id, const bool run_prop_end_effects = true);

    void props_interface_line(std::vector<StrAndClr>& line) const;

    Prop* mk_prop(const PropId id,
                  PropTurns turns_init,
                  const int nr_turns = -1) const;

    //--------------------------------------------------------------------------
    // Hooks called from various places
    //--------------------------------------------------------------------------
    void affect_move_dir(const P& actor_pos, Dir& dir) const;

    int affect_max_hp(const int hp_max) const;

    bool allow_attack(const Verbosity verbosity) const;
    bool allow_attack_melee(const Verbosity verbosity) const;
    bool allow_attack_ranged(const Verbosity verbosity) const;
    bool allow_see() const;
    bool allow_move() const;
    bool allow_act() const;
    bool allow_read(const Verbosity verbosity) const;
    bool allow_cast_spell(const Verbosity verbosity) const;
    bool allow_speak(const Verbosity verbosity) const;
    bool allow_eat(const Verbosity verbosity) const; // Also used for drinking

    void on_hit();
    void on_death(const bool is_player_see_owning_actor);

    int ability_mod(const AbilityId ability) const;

    bool affect_actor_clr(Clr& clr) const;

    // Called when the actors turn begins
    void on_turn_begin();

    // Called when the actors turn ends
    void on_turn_end();

    bool try_resist_dmg(const DmgType dmg_type,
                        const Verbosity verbosity) const;

private:
    bool try_resist_prop(const PropId id) const;

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

    virtual void msg(const PropMsg msg_type, std::string& msg_ref) const
    {
        msg_ref = data_.msg[size_t(msg_type)];
    }

    virtual bool allow_apply_more_while_active() const
    {
        return data_.allow_apply_more_while_active;
    }

    // TODO: This is ridiculous! Can't the properties just call vision updating
    //       themselves (whatever is needed per case)???
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
    virtual void on_more() {}

    virtual void on_death(const bool is_player_see_owning_actor)
    {
        (void)is_player_see_owning_actor;
    }

    virtual int affect_max_hp(const int hp_max) const
    {
        return hp_max;
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

    virtual bool try_resist_dmg(const DmgType dmg_type,
                                const Verbosity verbosity) const
    {
        (void)dmg_type;
        (void)verbosity;
        return false;
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

class PropSeeInvis: public Prop
{
public:
    PropSeeInvis(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::see_invis, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropInfravis: public Prop
{
public:
    PropInfravis(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::infravis, turns_init, nr_turns) {}
};

class PropBlessed: public Prop
{
public:
    PropBlessed(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::blessed, turns_init, nr_turns) {}

    void on_start() override;

    void on_more() override;

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

    void on_more() override;

    void on_end() override;

private:
    void curse_adjacent() const;
};

class PropBurning: public Prop
{
public:
    PropBurning(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::burning, turns_init, nr_turns) {}

    bool allow_read(const Verbosity verbosity) const override;
    bool allow_cast_spell(const Verbosity verbosity) const override;

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

class PropWarlockCharged: public Prop
{
public:
    PropWarlockCharged(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::warlock_charged, turns_init, nr_turns) {}

};

class PropConfused: public Prop
{
public:
    PropConfused(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::confused, turns_init, nr_turns) {}

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    bool allow_read(const Verbosity verbosity) const override;
    bool allow_cast_spell(const Verbosity verbosity) const override;
    bool allow_attack_melee(const Verbosity verbosity) const override;
    bool allow_attack_ranged(const Verbosity verbosity) const override;
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

    void on_more() override;

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

class PropFrenzied: public Prop
{
public:
    PropFrenzied(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::frenzied, turns_init, nr_turns) {}

    void on_start() override;
    void on_end() override;

    void affect_move_dir(const P& actor_pos, Dir& dir) override;

    bool allow_read(const Verbosity verbosity) const override;
    bool allow_cast_spell(const Verbosity verbosity) const override;

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
        Prop(PropId::rAcid, turns_init, nr_turns) {}

    bool try_resist_dmg(const DmgType dmg_type,
                        const Verbosity verbosity) const override;
};

class PropRConf: public Prop
{
public:
    PropRConf(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rConf, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRElec: public Prop
{
public:
    PropRElec(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rElec, turns_init, nr_turns) {}

    bool try_resist_dmg(const DmgType dmg_type,
                        const Verbosity verbosity) const override;
};

class PropRFear: public Prop
{
public:
    PropRFear(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rFear, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRPhys: public Prop
{
public:
    PropRPhys(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rPhys, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;

    bool try_resist_dmg(const DmgType dmg_type,
                        const Verbosity verbosity) const override;
};

class PropRFire: public Prop
{
public:
    PropRFire(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rFire, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;

    bool try_resist_dmg(const DmgType dmg_type,
                        const Verbosity verbosity) const override;
};

class PropRPoison: public Prop
{
public:
    PropRPoison(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rPoison, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRSleep: public Prop
{
public:
    PropRSleep(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rSleep, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRDisease: public Prop
{
public:
    PropRDisease(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rDisease, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRBlind: public Prop
{
public:
    PropRBlind(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rBlind, turns_init, nr_turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRBreath: public Prop
{
public:
    PropRBreath(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::rBreath, turns_init, nr_turns) {}
};

class PropLgtSens: public Prop
{
public:
    PropLgtSens(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::lgtSens, turns_init, nr_turns) {}
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
        Prop(PropId::rSpell, turns_init, nr_turns) {}
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

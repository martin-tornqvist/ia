#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <string>

#include "property_handler.hpp"
#include "global.hpp"

// -----------------------------------------------------------------------------
// Support types
// -----------------------------------------------------------------------------
enum class PropTurns
{
    std,
    specific,
    indefinite
};

struct DmgResistData
{
    DmgResistData() :
        is_resisted(false),
        msg_resist_player(),
        msg_resist_mon() {}

    bool is_resisted;
    std::string msg_resist_player;
    // Not including monster name, e.g. " seems unaffected"
    std::string msg_resist_mon;
};

enum class PropEnded
{
    no,
    yes
};

struct PropActResult
{
    PropActResult() :
        did_action(DidAction::no),
        prop_ended(PropEnded::no) {}

    PropActResult(DidAction did_action, PropEnded prop_ended) :
        did_action(did_action),
        prop_ended(prop_ended) {}

    DidAction did_action;
    PropEnded prop_ended;
};

// -----------------------------------------------------------------------------
// Property base class
// -----------------------------------------------------------------------------
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

    virtual std::string msg_end_player() const
    {
        return data_.msg_end_player;
    }

    virtual bool should_update_vision_on_toggled() const
    {
        return data_.update_vision_on_toggled;
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

    virtual PropEnded on_tick()
    {
        return PropEnded::no;
    }

    virtual void on_std_turn() {}

    virtual PropActResult on_act()
    {
        return PropActResult();
    }

    virtual void on_start() {}

    virtual void on_end() {}

    virtual void on_more(const Prop& new_prop)
    {
        (void)new_prop;
    }

    virtual void on_death() {}

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

    virtual bool affect_actor_color(Color& color) const
    {
        (void)color;
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
    // for example to make copies of a property.
    PropTurns turns_init_type_;

    Actor* owner_;
    PropSrc src_;
    const Item* item_applying_;
};

// -----------------------------------------------------------------------------
// Specific properties
// -----------------------------------------------------------------------------
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

    PropEnded on_tick() override;
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

    PropEnded on_tick() override;
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

    PropEnded on_tick() override;
};

class PropPossByZuul: public Prop
{
public:
    PropPossByZuul(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::poss_by_zuul, turns_init, nr_turns) {}

    void on_death() override;

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

    PropEnded on_tick() override;
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

    bool should_update_vision_on_toggled() const override;

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

class PropRecloaks: public Prop
{
public:
    PropRecloaks(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::recloaks, turns_init, nr_turns) {}

    PropActResult on_act() override;
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
        Prop(PropId::darkvision, turns_init, nr_turns) {}
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

    bool affect_actor_color(Color& color) const override
    {
        color = colors::light_red();
        return true;
    }

    bool allow_attack_ranged(const Verbosity verbosity) const override;

    PropEnded on_tick() override;
};

class PropFlared: public Prop
{
public:
    PropFlared(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::flared, turns_init, nr_turns) {}

    PropEnded on_tick() override;
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

    std::string msg_end_player() const override
    {
        return
            (nr_wounds_ > 1) ?
            "All my wounds are healed!" :
            "A wound is healed!";
    }

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

    bool should_update_vision_on_toggled() const override;

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
        Prop(PropId::light_sensitive, turns_init, nr_turns) {}
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

class PropVortex: public Prop
{
public:
    PropVortex(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::vortex, turns_init, nr_turns),
        pull_cooldown(0) {}

    PropActResult on_act() override;

private:
    int pull_cooldown;
};

class PropExplodesOnDeath: public Prop
{
public:
    PropExplodesOnDeath(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::explodes_on_death, turns_init, nr_turns) {}

    void on_death() override;
};

class PropSplitsOnDeath: public Prop
{
public:
    PropSplitsOnDeath(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::splits_on_death, turns_init, nr_turns) {}

    void on_death() override;
};

class PropCorpseEater: public Prop
{
public:
    PropCorpseEater(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::corpse_eater, turns_init, nr_turns) {}

    PropActResult on_act() override;
};

class PropTeleports: public Prop
{
public:
    PropTeleports(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::teleports, turns_init, nr_turns) {}

    PropActResult on_act() override;
};

class PropCorruptsEnvColor: public Prop
{
public:
    PropCorruptsEnvColor(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::corrupts_env_color, turns_init, nr_turns) {}

    PropActResult on_act() override;
};

class PropRegenerates: public Prop
{
public:
    PropRegenerates(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::regenerates, turns_init, nr_turns) {}

    void on_std_turn() override;
};

class PropCorpseRises: public Prop
{
public:
    PropCorpseRises(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::corpse_rises, turns_init, nr_turns),
        rise_one_in_n_(8) {}

    PropActResult on_act() override;

private:
    int rise_one_in_n_;
};

class PropBreeds: public Prop
{
public:
    PropBreeds(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::breeds, turns_init, nr_turns) {}

    void on_std_turn() override;
};

class PropConfusesAdjacent: public Prop
{
public:
    PropConfusesAdjacent(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::confuses_adjacent, turns_init, nr_turns) {}

    void on_std_turn() override;
};

class PropSpeaksCurses: public Prop
{
public:
    PropSpeaksCurses(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::speaks_curses, turns_init, nr_turns) {}

    PropActResult on_act() override;
};

class PropMajorClaphamSummon: public Prop
{
public:
    PropMajorClaphamSummon(PropTurns turns_init, int nr_turns = -1) :
        Prop(PropId::major_clapham_summon, turns_init, nr_turns) {}

    PropActResult on_act() override;
};

#endif // PROPERTY_HPP

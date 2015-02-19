#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>

#include "Ability_values.h"
#include "Cmn_data.h"
#include "Converters.h"
#include "Cmn_types.h"

class Actor;
class Wpn;

enum class Prop_id
{
    r_phys,
    r_fire,
    r_cold,
    r_poison,
    r_elec,
    r_acid,
    r_sleep,
    r_fear,
    r_conf,
    r_breath,
    r_disease,
    lgt_sens,
    blind,
    fainted,
    burning,
    radiant,
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

    //Properties describing the actors body and/or method of moving around
    flying,
    ethereal,
    ooze,
    burrowing,

    //The following are used for AI control
    waiting,
    disabled_attack,
    disabled_melee,
    disabled_ranged,

    //Special (for supporting very specific game mechanics)
    poss_by_zuul,
    aiming,
    nailed,
    flared,
    warlock_charged,

    END
};

enum class Prop_turn_mode {std, actor};
enum class Prop_turns    {std, specific, indefinite};

enum Prop_msg_type
{
    prop_msg_on_start_player,
    prop_msg_on_start_mon,
    prop_msg_on_end_player,
    prop_msg_on_end_mon,
    prop_msg_on_res_player,
    prop_msg_on_res_mon,
    end_of_prop_msg
};

enum Prop_alignment
{
    prop_alignment_good, prop_alignment_bad, prop_alignment_neutral
};

enum class Prop_src {applied, inv, END};

struct Prop_data_t
{
    Prop_data_t() :
        id                                  (Prop_id::END),
        std_rnd_turns                         (Range(10, 10)),
        name                                (""),
        name_short                           (""),
        is_making_mon_aware                    (false),
        allow_display_turns                   (true),
        allow_apply_more_while_active           (true),
        update_player_visual_when_start_or_end    (false),
        is_ended_by_magic_healing               (false),
        allow_test_on_bot                      (false),
        alignment                           (prop_alignment_bad)
    {
        for (int i = 0; i < end_of_prop_msg; ++i) {msg[i] = "";}
    }

    Prop_id          id;
    Range           std_rnd_turns;
    std::string     name;
    std::string     name_short;
    std::string     msg[end_of_prop_msg];
    bool            is_making_mon_aware;
    bool            allow_display_turns;
    bool            allow_apply_more_while_active;
    bool            update_player_visual_when_start_or_end;
    bool            is_ended_by_magic_healing;
    bool            allow_test_on_bot;
    Prop_alignment   alignment;
};

namespace Prop_data
{

extern Prop_data_t data[size_t(Prop_id::END)];

void init();

} //Prop_data

class Prop;

//Each actor has an instance of this
class Prop_handler
{
public:
    Prop_handler(Actor* owning_actor);

    ~Prop_handler();

    void try_apply_prop(Prop* const   prop,
                      const bool    FORCE_EFFECT                = false,
                      const bool    NO_MESSAGES                 = false,
                      const bool    DISABLE_REDRAW              = false,
                      const bool    DISABLE_PROP_START_EFFECTS  = false);

    void try_apply_prop_from_att(const Wpn& wpn, const bool IS_MELEE);

    void change_move_dir(const Pos& actor_pos, Dir& dir) const;

    int get_changed_max_hp(const int HP_MAX) const;

    bool allow_attack        (const bool ALLOW_MSG)  const;
    bool allow_attack_melee   (const bool ALLOW_MSG)  const;
    bool allow_attack_ranged  (const bool ALLOW_MSG)  const;
    bool allow_see()                                 const;
    bool allow_move()                                const;
    bool allow_act()                                 const;
    bool allow_read          (const bool ALLOW_MSG)  const;
    bool allow_cast_spell     (const bool ALLOW_MSG)  const;
    bool allow_speak         (const bool ALLOW_MSG)  const;
    bool allow_eat           (const bool ALLOW_MSG)  const; //Also used for drinking

    void on_hit();
    void on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR);

    int get_ability_mod(const Ability_id ability) const;

    void get_prop_ids(bool out[size_t(Prop_id::END)]) const;

    Prop* get_prop(const Prop_id id, const Prop_src source) const;

    bool end_applied_prop(const Prop_id id, const bool RUN_PROP_END_EFFECTS = true);

    void end_applied_props_by_magic_healing();

    bool change_actor_clr(Clr& clr) const;

    std::vector<Prop*> applied_props_;
    std::vector<Prop*> actor_turn_prop_buffer_;

    void apply_actor_turn_prop_buffer();

    void tick(const Prop_turn_mode turn_mode);

    void get_props_interface_line(std::vector<Str_and_clr>& line) const;

    Prop* mk_prop(const Prop_id id, Prop_turns turns_init,
                 const int NR_TURNS = -1) const;

    bool try_resist_dmg(const Dmg_type dmg_type, const bool ALLOW_MSG) const;

private:
    //These two functions are responsible for collecting properties from all possible
    //specified sources. They must be "mirrored" in that they collect information from
    //exactly the same places - except one will return a list of Props, and the other
    //will return a list of Prop_ids. This is an optimization. The reasoning is that in
    //many cases (such as the very frequent Actor::add_light) it is sufficient to know
    //the ids. It would then be wasteful to first gather all Props, then gather the ids
    //from those. It's more efficient to just gather the ids at once.
    void get_props_from_sources(std::vector<Prop*>& out,
                             bool sources[int(Prop_src::END)]) const;

    void get_prop_ids_from_sources(bool out[size_t(Prop_id::END)],
                               bool sources[int(Prop_src::END)]) const;

    bool try_resist_prop(const Prop_id id, const std::vector<Prop*>& prop_list) const;

    Actor* owning_actor_;
};

class Prop
{
public:
    Prop(Prop_id id, Prop_turns turns_init, int turns);

    virtual ~Prop() {}

    virtual void store_to_save_lines(std::vector<std::string>& lines) const
    {
        (void)lines;
    }

    virtual void setup_from_save_lines(std::vector<std::string>& lines)
    {
        (void)lines;
    }

    Prop_id get_id() const {return id_;}

    virtual bool is_finished() const {return turns_left_ == 0;}
    virtual Prop_alignment get_alignment() const {return data_->alignment;}
    virtual bool allow_display_turns() const {return data_->allow_display_turns;}

    virtual bool is_making_mon_aware() const
    {
        return data_->is_making_mon_aware;
    }

    virtual std::string get_name() const {return data_->name;}
    virtual std::string get_name_short() const {return data_->name_short;}
    virtual void get_msg(const Prop_msg_type msg_type, std::string& msg_ref) const
    {
        msg_ref = data_->msg[msg_type];
    }

    virtual bool allow_apply_more_while_active() const
    {
        return data_->allow_apply_more_while_active;
    }

    virtual bool should_update_player_visual_when_start_or_end() const
    {
        return data_->update_player_visual_when_start_or_end;
    }

    virtual bool is_ended_by_magic_healing() const
    {
        return data_->is_ended_by_magic_healing;
    }

    virtual bool allow_see()   const {return true;}
    virtual bool allow_move()  const {return true;}
    virtual bool allow_act()   const {return true;}
    virtual void on_hit()            {}
    virtual void on_new_turn()        {}
    virtual void on_start()          {}
    virtual void on_end()            {}
    virtual void on_more()           {}

    virtual void on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR)
    {
        (void)IS_PLAYER_SEE_OWNING_ACTOR;
    }

    virtual int get_changed_max_hp(const int HP_MAX) const {return HP_MAX;}

    virtual bool change_actor_clr(Clr& clr) const {(void)clr; return false;}

    virtual bool allow_attack_melee(const bool ALLOW_MSG) const
    {
        (void)ALLOW_MSG;
        return true;
    }

    virtual bool allow_attack_ranged(const bool ALLOW_MSG) const
    {
        (void)ALLOW_MSG;
        return true;
    }

    virtual bool allow_read(const bool ALLOW_MSG) const
    {
        (void)ALLOW_MSG;
        return true;
    }

    virtual bool allow_cast_spell(const bool ALLOW_MSG) const
    {
        (void)ALLOW_MSG;
        return true;
    }

    virtual bool allow_speak(const bool ALLOW_MSG) const
    {
        (void)ALLOW_MSG;
        return true;
    }

    virtual bool allow_eat(const bool ALLOW_MSG) const
    {
        (void)ALLOW_MSG;
        return true;
    }

    virtual int get_ability_mod(const Ability_id ability) const
    {
        (void)ability;
        return 0;
    }

    virtual void change_move_dir(const Pos& actor_pos, Dir& dir)
    {
        (void)actor_pos;
        (void)dir;
    }

    virtual bool is_resisting_other_prop(const Prop_id id) const
    {
        (void)id;
        return false;
    }

    virtual bool try_resist_dmg(const Dmg_type dmg_type, const bool ALLOW_MSG) const
    {
        (void)dmg_type;
        (void)ALLOW_MSG;
        return false;
    }

    virtual Prop_turn_mode get_turn_mode() const {return Prop_turn_mode::std;}

    Prop_turns get_turns_init_type() const {return turns_init_type_;}

    int turns_left_;

    Actor* owning_actor_;

protected:
    const Prop_id id_;

    const Prop_data_t* const data_;

    //How the prop turns was inited (std, specific, indefinite). This is used for example
    //to make copies of a property to apply on melee attacks.
    Prop_turns turns_init_type_;
};

class Prop_terrified: public Prop
{
public:
    Prop_terrified(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::terrified, turns_init, turns) {}

    int get_ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::dodge_att:   return 20;
        case Ability_id::ranged:     return -20;
        default: {} break;
        }

        return 0;
    }

    bool allow_attack_melee(const bool ALLOW_MSG) const override;

    bool allow_attack_ranged(const bool ALLOW_MSG) const override;
};

class Prop_weakened: public Prop
{
public:
    Prop_weakened(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::weakened, turns_init, turns) {}
};

class Prop_infected: public Prop
{
public:
    Prop_infected(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::infected, turns_init, turns) {}

    void on_new_turn() override;
};

class Prop_diseased: public Prop
{
public:
    Prop_diseased(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::diseased, turns_init, turns) {}

    int get_changed_max_hp(const int HP_MAX) const override;

    bool is_resisting_other_prop(const Prop_id id) const override;

    void on_start() override;
};

class Prop_flying: public Prop
{
public:
    Prop_flying(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::flying, turns_init, turns) {}
};

class Prop_ethereal: public Prop
{
public:
    Prop_ethereal(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::ethereal, turns_init, turns) {}
};

class Prop_ooze: public Prop
{
public:
    Prop_ooze(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::ooze, turns_init, turns) {}
};

class Prop_burrowing: public Prop
{
public:
    Prop_burrowing(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::burrowing, turns_init, turns) {}

    void on_new_turn() override;
};

class Prop_poss_by_zuul: public Prop
{
public:
    Prop_poss_by_zuul(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::poss_by_zuul, turns_init, turns) {}

    void on_death(const bool IS_PLAYER_SEE_OWNING_ACTOR) override;

    int get_changed_max_hp(const int HP_MAX) const override
    {
        return HP_MAX * 2;
    }
};

class Prop_poisoned: public Prop
{
public:
    Prop_poisoned(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::poisoned, turns_init, turns) {}

    void on_new_turn() override;
};

class Prop_aiming: public Prop
{
public:
    Prop_aiming(Prop_turns turns_init, int turns = -1) :
        Prop          (Prop_id::aiming, turns_init, turns),
        nr_turns_aiming (1) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}

    std::string get_name_short() const override
    {
        return data_->name_short + (nr_turns_aiming >= 3 ? "(3)" : "");
    }

    int get_ability_mod(const Ability_id ability) const override
    {
        if (ability == Ability_id::ranged)
        {
            return nr_turns_aiming >= 3 ? 999 : 20;
        }
        return 0;
    }

    bool is_max_ranged_dmg() const {return nr_turns_aiming >= 3;}

    int nr_turns_aiming;
};

class Prop_blind: public Prop
{
public:
    Prop_blind(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::blind, turns_init, turns) {}

    bool should_update_player_visual_when_start_or_end() const override;

    bool allow_see() const override {return false;}

    int get_ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::searching:  return -9999;
        case Ability_id::ranged:     return -50;
        case Ability_id::melee:      return -25;
        case Ability_id::dodge_trap:
        case Ability_id::dodge_att:   return -50;
        default: {} break;
        }

        return 0;
    }
};

class Prop_radiant: public Prop
{
public:
    Prop_radiant(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::radiant, turns_init, turns) {}
};

class Prop_blessed: public Prop
{
public:
    Prop_blessed(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::blessed, turns_init, turns) {}

    void on_start() override;

    int get_ability_mod(const Ability_id ability) const override
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
    Prop_cursed(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::cursed, turns_init, turns) {}

    void on_start() override;

    int get_ability_mod(const Ability_id ability) const override
    {
        (void)ability;
        return -10;
    }
};

class Prop_burning: public Prop
{
public:
    Prop_burning(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::burning, turns_init, turns) {}

    bool allow_read      (const bool ALLOW_MSG) const override;
    bool allow_cast_spell (const bool ALLOW_MSG) const override;

    bool change_actor_clr(Clr& clr) const override
    {
        clr = clr_red_lgt;
        return true;
    }

    bool allow_attack_ranged(const bool ALLOW_MSG) const override;

    void on_start()      override;
    void on_new_turn()    override;
};

class Prop_flared: public Prop
{
public:
    Prop_flared(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::flared, turns_init, turns) {}

    void on_new_turn() override;
};

class Prop_warlock_charged: public Prop
{
public:
    Prop_warlock_charged(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::warlock_charged, turns_init, turns) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}
};

class Prop_confused: public Prop
{
public:
    Prop_confused(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::confused, turns_init, turns) {}

    void change_move_dir(const Pos& actor_pos, Dir& dir) override;

    bool allow_read          (const bool ALLOW_MSG) const override;
    bool allow_cast_spell     (const bool ALLOW_MSG) const override;
    bool allow_attack_melee   (const bool ALLOW_MSG) const override;
    bool allow_attack_ranged  (const bool ALLOW_MSG) const override;
};

class Prop_stunned: public Prop
{
public:
    Prop_stunned(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::stunned, turns_init, turns) {}
};

class Prop_nailed: public Prop
{
public:
    Prop_nailed(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::nailed, turns_init, turns), nr_spikes_(1) {}

    std::string get_name_short() const override
    {
        return "Nailed(" + to_str(nr_spikes_) + ")";
    }

    void change_move_dir(const Pos& actor_pos, Dir& dir) override;

    void on_more() override {nr_spikes_++;}

    bool is_finished() const override {return nr_spikes_ <= 0;}

private:
    int nr_spikes_;
};

class Prop_waiting: public Prop
{
public:
    Prop_waiting(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::waiting, turns_init, turns) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}

    bool allow_move() const override  {return false;}
    bool allow_act() const override   {return false;}

    bool allow_attack_melee(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }

    bool allow_attack_ranged(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }
};

class Prop_disabled_attack: public Prop
{
public:
    Prop_disabled_attack(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::disabled_attack, turns_init, turns) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}

    bool allow_attack_ranged(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }

    bool allow_attack_melee(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }
};

class Prop_disabled_melee: public Prop
{
public:
    Prop_disabled_melee(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::disabled_melee, turns_init, turns) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}

    bool allow_attack_melee(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }
};

class Prop_disabled_ranged: public Prop
{
public:
    Prop_disabled_ranged(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::disabled_ranged, turns_init, turns) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}

    bool allow_attack_ranged(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }
};

class Prop_paralyzed: public Prop
{
public:
    Prop_paralyzed(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::paralyzed, turns_init, turns) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}

    void on_start() override;

    bool allow_act() const override {return false;}

    int get_ability_mod(const Ability_id ability) const override
    {
        if (ability == Ability_id::dodge_trap || ability == Ability_id::dodge_att)
        {
            return -999;
        }
        return 0;
    }

    bool allow_attack_ranged(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }

    bool allow_attack_melee(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }
};

class Prop_fainted: public Prop
{
public:
    Prop_fainted(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::fainted, turns_init, turns) {}

    bool should_update_player_visual_when_start_or_end() const override;

    bool allow_act() const override {return false;}

    bool allow_see() const override {return false;}

    int get_ability_mod(const Ability_id ability) const override
    {
        if (ability == Ability_id::dodge_trap || ability == Ability_id::dodge_att)
        {
            return -999;
        }
        return 0;
    }

    bool allow_attack_ranged(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }

    bool allow_attack_melee(const bool ALLOW_MSG) const override
    {
        (void)ALLOW_MSG;
        return false;
    }

    void on_hit() override {turns_left_ = 0;}
};

class Prop_slowed: public Prop
{
public:
    Prop_slowed(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::slowed, turns_init, turns) {}

    void on_start() override;

    int get_ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::dodge_att:   return -30;
        case Ability_id::ranged:     return -10;
        case Ability_id::melee:      return -10;
        default: {} break;
        }
        return 0;
    }
};

class Prop_hasted: public Prop
{
public:
    Prop_hasted(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::hasted, turns_init, turns) {}

    void on_start() override;

    int get_ability_mod(const Ability_id ability) const override
    {
        switch (ability)
        {
        case Ability_id::dodge_att:   return 10;
        case Ability_id::ranged:     return 5;
        case Ability_id::melee:      return 5;
        default: {} break;
        }
        return 0;
    }
};

class Prop_frenzied: public Prop
{
public:
    Prop_frenzied(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::frenzied, turns_init, turns) {}

    void on_start() override;
    void on_end() override;

    void change_move_dir(const Pos& actor_pos, Dir& dir) override;

    bool allow_read(const bool ALLOW_MSG) const override;
    bool allow_cast_spell(const bool ALLOW_MSG) const override;

    bool is_resisting_other_prop(const Prop_id id) const override;

    int get_ability_mod(const Ability_id ability) const override
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
    Prop_rAcid(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_acid, turns_init, turns) {}

    bool try_resist_dmg(const Dmg_type dmg_type, const bool ALLOW_MSG) const override;
};

class Prop_rCold: public Prop
{
public:
    Prop_rCold(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_cold, turns_init, turns) {}

    bool try_resist_dmg(const Dmg_type dmg_type, const bool ALLOW_MSG) const override;
};

class Prop_rConfusion: public Prop
{
public:
    Prop_rConfusion(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_conf, turns_init, turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id id) const override;
};

class Prop_rElec: public Prop
{
public:
    Prop_rElec(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_elec, turns_init, turns) {}

    bool try_resist_dmg(const Dmg_type dmg_type, const bool ALLOW_MSG) const override;
};

class Prop_rFear: public Prop
{
public:
    Prop_rFear(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_fear, turns_init, turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id id) const override;
};

class Prop_rPhys: public Prop
{
public:
    Prop_rPhys(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_phys, turns_init, turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id id) const override;

    bool try_resist_dmg(const Dmg_type dmg_type, const bool ALLOW_MSG) const override;
};

class Prop_rFire: public Prop
{
public:
    Prop_rFire(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_fire, turns_init, turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id id) const override;

    bool try_resist_dmg(const Dmg_type dmg_type, const bool ALLOW_MSG) const override;
};

class Prop_rPoison: public Prop
{
public:
    Prop_rPoison(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_poison, turns_init, turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id id) const override;
};

class Prop_rSleep: public Prop
{
public:
    Prop_rSleep(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_sleep, turns_init, turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id id) const override;
};

class Prop_rDisease: public Prop
{
public:
    Prop_rDisease(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_disease, turns_init, turns) {}

    void on_start() override;

    bool is_resisting_other_prop(const Prop_id id) const override;
};

class Prop_rBreath: public Prop
{
public:
    Prop_rBreath(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::r_breath, turns_init, turns) {}
};

class Prop_lgt_sens: public Prop
{
public:
    Prop_lgt_sens(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::lgt_sens, turns_init, turns) {}
    ~Prop_lgt_sens() override {}
};

class Prop_tele_control: public Prop
{
public:
    Prop_tele_control(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::tele_ctrl, turns_init, turns) {}
    ~Prop_tele_control() override {}
};

class Prop_spell_reflect: public Prop
{
public:
    Prop_spell_reflect(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::spell_reflect, turns_init, turns) {}
    ~Prop_spell_reflect() override {}
};

class Prop_conflict: public Prop
{
public:
    Prop_conflict(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::conflict, turns_init, turns) {}
    ~Prop_conflict() override {}
};

class Prop_strangled: public Prop
{
public:
    Prop_strangled(Prop_turns turns_init, int turns = -1) :
        Prop(Prop_id::strangled, turns_init, turns) {}

    Prop_turn_mode get_turn_mode() const override {return Prop_turn_mode::actor;}

    void on_new_turn() override;

    bool allow_speak(const bool ALLOW_MSG) const override;
    bool allow_eat  (const bool ALLOW_MSG) const override;
};

#endif

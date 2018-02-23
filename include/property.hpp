#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <string>

#include "property_handler.hpp"
#include "global.hpp"

// -----------------------------------------------------------------------------
// Support types
// -----------------------------------------------------------------------------
enum class PropDurationMode
{
        standard,
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
        Prop(PropId id);

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

        void set_duration(const int nr_turns)
        {
                ASSERT(nr_turns > 0);

                duration_mode_ = PropDurationMode::specific;

                nr_turns_left_ = nr_turns;
        }

        void set_indefinite()
        {
                duration_mode_ = PropDurationMode::indefinite;

                nr_turns_left_ = -1;
        }

        virtual bool is_finished() const
        {
                return nr_turns_left_ == 0;
        }

        PropDurationMode duration_mode() const
        {
                return duration_mode_;
        }

        PropSrc src() const
        {
                return src_;
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

        virtual bool allow_cast_intr_spell_absolute(
                const Verbosity verbosity) const
        {
                (void)verbosity;
                return true;
        }

        virtual bool allow_cast_intr_spell_chance(
                const Verbosity verbosity) const
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

protected:
        friend class PropHandler;

        const PropId id_;
        const PropData& data_;

        int nr_turns_left_;

        PropDurationMode duration_mode_;

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
        PropTerrified() :
                Prop(PropId::terrified) {}

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
        PropWeakened() :
                Prop(PropId::weakened) {}
};

class PropInfected: public Prop
{
public:
        PropInfected() :
                Prop(PropId::infected) {}

        PropEnded on_tick() override;
};

class PropDiseased: public Prop
{
public:
        PropDiseased() :
                Prop(PropId::diseased) {}

        int affect_max_hp(const int hp_max) const override;

        bool is_resisting_other_prop(const PropId prop_id) const override;

        void on_start() override;
        void on_end() override;
};

class PropDescend: public Prop
{
public:
        PropDescend() :
                Prop(PropId::descend) {}

        PropEnded on_tick() override;
};

class PropFlying: public Prop
{
public:
        PropFlying() :
                Prop(PropId::flying) {}
};

class PropEthereal: public Prop
{
public:
        PropEthereal() :
                Prop(PropId::ethereal) {}
};

class PropOoze: public Prop
{
public:
        PropOoze() :
                Prop(PropId::ooze) {}
};

class PropBurrowing: public Prop
{
public:
        PropBurrowing() :
                Prop(PropId::burrowing) {}

        PropEnded on_tick() override;
};

class PropPossByZuul: public Prop
{
public:
        PropPossByZuul() :
                Prop(PropId::poss_by_zuul) {}

        void on_death() override;

        int affect_max_hp(const int hp_max) const override
        {
                return hp_max * 2;
        }
};

class PropPoisoned: public Prop
{
public:
        PropPoisoned() :
                Prop(PropId::poisoned) {}

        PropEnded on_tick() override;
};

class PropAiming: public Prop
{
public:
        PropAiming() :
                Prop(PropId::aiming),
                nr_turns_aiming_(1) {}

        std::string name_short() const override
        {
                return
                        data_.name_short +
                        ((nr_turns_aiming_ >= 3) ? "(3)" : "");
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
        PropBlind() :
                Prop(PropId::blind) {}

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
        PropDeaf() :
                Prop(PropId::deaf) {}
};

class PropRadiant: public Prop
{
public:
        PropRadiant() :
                Prop(PropId::radiant) {}
};

class PropInvisible: public Prop
{
public:
        PropInvisible() :
                Prop(PropId::invis) {}
};

class PropCloaked: public Prop
{
public:
        PropCloaked() :
                Prop(PropId::cloaked) {}
};

class PropRecloaks: public Prop
{
public:
        PropRecloaks() :
                Prop(PropId::recloaks) {}

        PropActResult on_act() override;
};

class PropSeeInvis: public Prop
{
public:
        PropSeeInvis() :
                Prop(PropId::see_invis) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropDarkvis: public Prop
{
public:
        PropDarkvis() :
                Prop(PropId::darkvision) {}
};

class PropBlessed: public Prop
{
public:
        PropBlessed() :
                Prop(PropId::blessed) {}

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
        PropCursed() :
                Prop(PropId::cursed) {}

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
        PropBurning() :
                Prop(PropId::burning) {}

        bool allow_read_chance(const Verbosity verbosity) const override;

        bool allow_cast_intr_spell_chance(
                const Verbosity verbosity) const override;

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
        PropFlared() :
                Prop(PropId::flared) {}

        PropEnded on_tick() override;
};

class PropConfused: public Prop
{
public:
        PropConfused() :
                Prop(PropId::confused) {}

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
        PropStunned() :
                Prop(PropId::stunned) {}
};

class PropNailed: public Prop
{
public:
        PropNailed() :
                Prop(PropId::nailed),
                nr_spikes_(1) {}

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
        PropWound() :
                Prop(PropId::wound),
                nr_wounds_(1) {}

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
        PropHpSap();

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
        PropSpiSap();

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
        PropMindSap();

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
        PropWaiting() :
                Prop(PropId::waiting) {}

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
        PropDisabledAttack() :
                Prop(PropId::disabled_attack) {}

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
        PropDisabledMelee() :
                Prop(PropId::disabled_melee) {}

        bool allow_attack_melee(const Verbosity verbosity) const override
        {
                (void)verbosity;
                return false;
        }
};

class PropDisabledRanged: public Prop
{
public:
        PropDisabledRanged() :
                Prop(PropId::disabled_ranged) {}

        bool allow_attack_ranged(const Verbosity verbosity) const override
        {
                (void)verbosity;
                return false;
        }
};

class PropParalyzed: public Prop
{
public:
        PropParalyzed() :
                Prop(PropId::paralyzed) {}

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
        PropFainted() :
                Prop(PropId::fainted) {}

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
        PropSlowed() :
                Prop(PropId::slowed) {}

        void on_start() override;
};

class PropHasted: public Prop
{
public:
        PropHasted() :
                Prop(PropId::hasted) {}

        void on_start() override;
};

class PropClockworkHasted: public Prop
{
public:
        PropClockworkHasted() :
                Prop(PropId::clockwork_hasted) {}
};

class PropSummoned: public Prop
{
public:
        PropSummoned() :
                Prop(PropId::summoned) {}

        void on_end() override;
};

class PropFrenzied: public Prop
{
public:
        PropFrenzied() :
                Prop(PropId::frenzied) {}

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
        PropRAcid() :
                Prop(PropId::r_acid) {}

        DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRConf: public Prop
{
public:
        PropRConf() :
                Prop(PropId::r_conf) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRElec: public Prop
{
public:
        PropRElec() :
                Prop(PropId::r_elec) {}

        DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRFear: public Prop
{
public:
        PropRFear() :
                Prop(PropId::r_fear) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRSlow: public Prop
{
public:
        PropRSlow() :
                Prop(PropId::r_slow) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRPhys: public Prop
{
public:
        PropRPhys() :
                Prop(PropId::r_phys) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;

        DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRFire: public Prop
{
public:
        PropRFire() :
                Prop(PropId::r_fire) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;

        DmgResistData is_resisting_dmg(const DmgType dmg_type) const override;
};

class PropRPoison: public Prop
{
public:
        PropRPoison() :
                Prop(PropId::r_poison) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRSleep: public Prop
{
public:
        PropRSleep() :
                Prop(PropId::r_sleep) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRDisease: public Prop
{
public:
        PropRDisease() :
                Prop(PropId::r_disease) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRBlind: public Prop
{
public:
        PropRBlind() :
                Prop(PropId::r_blind) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRPara: public Prop
{
public:
        PropRPara() :
                Prop(PropId::r_para) {}

        void on_start() override;

        bool is_resisting_other_prop(const PropId prop_id) const override;
};

class PropRBreath: public Prop
{
public:
        PropRBreath() :
                Prop(PropId::r_breath) {}
};

class PropLgtSens: public Prop
{
public:
        PropLgtSens() :
                Prop(PropId::light_sensitive) {}
        ~PropLgtSens() override {}
};

class PropTeleControl: public Prop
{
public:
        PropTeleControl() :
                Prop(PropId::tele_ctrl) {}
        ~PropTeleControl() override {}
};

class PropRSpell: public Prop
{
public:
        PropRSpell() :
                Prop(PropId::r_spell) {}
};

class PropSpellReflect: public Prop
{
public:
        PropSpellReflect() :
                Prop(PropId::spell_reflect) {}

        ~PropSpellReflect() override {}
};

class PropConflict: public Prop
{
public:
        PropConflict() :
                Prop(PropId::conflict) {}
        ~PropConflict() override {}
};

class PropVortex: public Prop
{
public:
        PropVortex() :
                Prop(PropId::vortex),
                pull_cooldown(0) {}

        PropActResult on_act() override;

private:
        int pull_cooldown;
};

class PropExplodesOnDeath: public Prop
{
public:
        PropExplodesOnDeath() :
                Prop(PropId::explodes_on_death) {}

        void on_death() override;
};

class PropSplitsOnDeath: public Prop
{
public:
        PropSplitsOnDeath() :
                Prop(PropId::splits_on_death) {}

        void on_death() override;
};

class PropCorpseEater: public Prop
{
public:
        PropCorpseEater() :
                Prop(PropId::corpse_eater) {}

        PropActResult on_act() override;
};

class PropTeleports: public Prop
{
public:
        PropTeleports() :
                Prop(PropId::teleports) {}

        PropActResult on_act() override;
};

class PropCorruptsEnvColor: public Prop
{
public:
        PropCorruptsEnvColor() :
                Prop(PropId::corrupts_env_color) {}

        PropActResult on_act() override;
};

class PropRegenerates: public Prop
{
public:
        PropRegenerates() :
                Prop(PropId::regenerates) {}

        void on_std_turn() override;
};

class PropCorpseRises: public Prop
{
public:
        PropCorpseRises() :
                Prop(PropId::corpse_rises),
                rise_one_in_n_(8) {}

        PropActResult on_act() override;

private:
        int rise_one_in_n_;
};

class PropBreeds: public Prop
{
public:
        PropBreeds() :
                Prop(PropId::breeds) {}

        void on_std_turn() override;
};

class PropConfusesAdjacent: public Prop
{
public:
        PropConfusesAdjacent() :
                Prop(PropId::confuses_adjacent) {}

        void on_std_turn() override;
};

class PropSpeaksCurses: public Prop
{
public:
        PropSpeaksCurses() :
                Prop(PropId::speaks_curses) {}

        PropActResult on_act() override;
};

class PropMajorClaphamSummon: public Prop
{
public:
        PropMajorClaphamSummon() :
                Prop(PropId::major_clapham_summon) {}

        PropActResult on_act() override;
};

#endif // PROPERTY_HPP

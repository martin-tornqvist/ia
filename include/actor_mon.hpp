#ifndef MON_HPP
#define MON_HPP

#include "global.hpp"
#include "actor.hpp"
#include "sound.hpp"
#include "spells.hpp"

struct AiAttData
{
        Wpn* wpn = nullptr;
        bool is_melee = false;
};

struct AiAvailAttacksData
{
        std::vector<Wpn*> weapons = {};
        bool should_reload = false;
        bool is_melee = false;
};

struct MonSpell
{
        MonSpell() :
                spell(nullptr),
                skill((SpellSkill)0),
                cooldown(-1) {}

        Spell* spell;
        SpellSkill skill;
        int cooldown;
};

class Mon: public Actor
{
public:
        Mon();
        virtual ~Mon();

        bool can_see_actor(const Actor& other,
                           const bool hard_blocked_los[map_w][map_h]) const;

        std::vector<Actor*> seen_actors() const override;

        std::vector<Actor*> seen_foes() const override;

        // Actors which are possible to see (i.e. not impossible due to
        // invisibility, etc), but may or may not currently be seen due to
        // (lack of) awareness
        std::vector<Actor*> seeable_foes() const;

        bool is_sneaking() const;

        void act() override;

        void move(Dir dir) override;

        virtual Color color() const override;

        SpellSkill spell_skill(const SpellId id) const override;

        AiAvailAttacksData avail_attacks(Actor& defender) const;

        AiAttData choose_attack(const AiAvailAttacksData& avail_attacks) const;

        DidAction try_attack(Actor& defender);

        void hear_sound(const Snd& snd);

        void become_aware_player(const bool is_from_seeing,
                                 const int factor = 1);

        void become_wary_player();

        void set_player_aware_of_me(int duration_factor = 1);

        void on_actor_turn() override;

        void on_std_turn() override final;

        virtual std::string aware_msg_mon_seen() const
        {
                return data_->aware_msg_mon_seen;
        }

        virtual std::string aware_msg_mon_hidden() const
        {
                return data_->aware_msg_mon_hidden;
        }

        virtual SfxId aware_sfx_mon_seen() const
        {
                return data_->aware_sfx_mon_seen;
        }

        virtual SfxId aware_sfx_mon_hidden() const
        {
                return data_->aware_sfx_mon_hidden;
        }

        void speak_phrase(const AlertsMon alerts_others);

        bool is_leader_of(const Actor* const actor) const override;
        bool is_actor_my_leader(const Actor* const actor) const override;

        void add_spell(SpellSkill skill, Spell* const spell);

        int wary_of_player_counter_;
        int aware_of_player_counter_;
        int player_aware_of_me_counter_;
        bool is_msg_mon_in_view_printed_;
        bool is_player_feeling_msg_allowed_;
        Dir last_dir_moved_;
        MonRoamingAllowed is_roaming_allowed_;
        Actor* leader_;
        Actor* target_;
        bool is_target_seen_;
        bool waiting_;

        std::vector<MonSpell> spells_;

protected:
        std::vector<Actor*> unseen_foes_aware_of() const;

        // Return value 'true' means it is possible to see the other actor (i.e.
        // it's not impossible due to invisibility, etc), but the actor may or
        // may not currently be seen due to (lack of) awareness
        bool is_actor_seeable(const Actor& other,
                              const bool hard_blocked_los[map_w][map_h]) const;

        void make_leader_aware_silent() const;

        void print_player_see_mon_become_aware_msg() const;

        void print_player_see_mon_become_wary_msg() const;

        bool is_friend_blocking_ranged_attack(const P& target_pos) const;

        Wpn* avail_wielded_melee() const;
        Wpn* avail_wielded_ranged() const;
        std::vector<Wpn*> avail_intr_melee() const;
        std::vector<Wpn*> avail_intr_ranged() const;

        bool should_reload(const Wpn& wpn) const;

        virtual void on_hit(int& dmg,
                            const DmgType dmg_type,
                            const DmgMethod method,
                            const AllowWound allow_wound) override;

        virtual DidAction on_act()
        {
                return DidAction::no;
        }

        virtual void on_std_turn_hook() {}

        int nr_mon_in_group();
};

class Zombie: public Mon
{
public:
        Zombie() : Mon() {}

        virtual ~Zombie() {}

protected:
        void on_destroyed() override;
};

class KeziahMason: public Mon
{
public:
        KeziahMason() : Mon(), has_summoned_jenkin(false) {}
        ~KeziahMason() {}


private:
        DidAction on_act() override;

        bool has_summoned_jenkin;
};

class Cultist: public Mon
{
public:
        Cultist() : Mon() {}


        static std::string cultist_phrase();

        std::string aware_msg_mon_seen() const override
        {
                return name_the() + ": " + cultist_phrase();
        }

        std::string aware_msg_mon_hidden() const override
        {
                return "Voice: " + cultist_phrase();
        }

        virtual ~Cultist() {}
};

class BogTcher: public Cultist
{
public:
        BogTcher() : Cultist() {}
        ~BogTcher() {}
};

class CultistPriest: public Cultist
{
public:
        CultistPriest() : Cultist() {}
        ~CultistPriest() {}
};

class CultistWizard: public Cultist
{
public:
        CultistWizard() : Cultist() {}
        ~CultistWizard() {}
};

class CultistArchWizard: public Cultist
{
public:
        CultistArchWizard() : Cultist() {}
        ~CultistArchWizard() {}
};

class Zuul: public Mon
{
public:
        Zuul() : Mon() {}
        ~Zuul() {}

        std::string death_msg() const override
        {
                return "Zuul vanishes...";
        }
};

class Ghost: public Mon
{
public:
        Ghost() : Mon() {}

        ~Ghost() {}

        std::string death_msg() const override
        {
                return "The Ghost is put to rest.";
        }
};

class Ape: public Mon
{
public:
        Ape() :
                Mon(),
                frenzy_cooldown_(0) {}

        ~Ape() {}

private:
        DidAction on_act() override;

        int frenzy_cooldown_;
};

class Khephren: public Mon
{
public:
        Khephren() :
                Mon(),
                has_summoned_locusts(false) {}
        ~Khephren() {}

private:
        DidAction on_act() override;

        bool has_summoned_locusts;
};

class Shadow: public Mon
{
public:
        Shadow() : Mon() {}
        ~Shadow() {}

        std::string death_msg() const override
        {
                return "The shadow fades.";
        }
};

class Ooze: public Mon
{
public:
        Ooze() : Mon() {}

        ~Ooze() {}

        std::string death_msg() const override
        {
                return "The Ooze disintegrates.";
        }
};

class StrangeColor: public Mon
{
public:
        StrangeColor() : Mon() {}

        ~StrangeColor() {}

        Color color() const override;

        std::string death_msg() const override
        {
                return "The Color disintegrates.";
        }
};

class Mold: public Mon
{
public:
        Mold() : Mon() {}

        ~Mold() {}

        std::string death_msg() const override
        {
                return "The Mold is destroyed.";
        }
};

class TheHighPriest: public Mon
{
public:
        TheHighPriest();
        ~TheHighPriest() {}


private:
        DidAction on_act() override;

        void on_death() override;

        bool has_become_aware_;
};

class AnimatedWpn: public Mon
{
public:
        AnimatedWpn();

        ~AnimatedWpn() {}

        void on_death() override;

        std::string name_the() const override;

        std::string name_a() const override;

        char character() const override;

        Color color() const override;

        TileId tile() const override;

        std::string descr() const override;

        std::string death_msg() const override;

        void on_std_turn_hook() override;

        void drop();

private:
        int nr_turns_until_drop_;
};

#endif // MON_HPP

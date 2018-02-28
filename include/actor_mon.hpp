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

class Rat: public Mon
{
public:
        Rat() : Mon() {}
        ~Rat() {}
};

class RatThing: public Rat
{
public:
        RatThing() : Rat() {}
        ~RatThing() {}
};

class BrownJenkin: public RatThing
{
public:
        BrownJenkin() : RatThing() {}
        ~BrownJenkin() {}
};

class Spider: public Mon
{
public:
        Spider() : Mon() {}
        virtual ~Spider() {}
};

class GreenSpider: public Spider
{
public:
        GreenSpider() : Spider() {}
        ~GreenSpider() {}
};

class WhiteSpider: public Spider
{
public:
        WhiteSpider() : Spider() {}
        ~WhiteSpider() {}
};

class RedSpider: public Spider
{
public:
        RedSpider() : Spider() {}
        ~RedSpider() {}
};

class ShadowSpider: public Spider
{
public:
        ShadowSpider() : Spider() {}
        ~ShadowSpider() {}
};

class LengSpider: public Spider
{
public:
        LengSpider() : Spider() {}
        ~LengSpider() {}
};

class PitViper: public Mon
{
public:
        PitViper() : Mon() {}
        virtual ~PitViper() {}
};

class SpittingCobra: public Mon
{
public:
        SpittingCobra() : Mon() {}
        virtual ~SpittingCobra() {}
};

class BlackMamba: public Mon
{
public:
        BlackMamba() : Mon() {}
        virtual ~BlackMamba() {}
};

class Zombie: public Mon
{
public:
        Zombie() : Mon() {}

        virtual ~Zombie() {}

protected:
        void on_death() override;

        void on_destroyed() override;
};

class ZombieClaw: public Zombie
{
public:
        ZombieClaw() : Zombie() {}
        ~ZombieClaw() {}
};

class ZombieAxe: public Zombie
{
public:
        ZombieAxe() : Zombie() {}
        ~ZombieAxe() {}
};

class BloatedZombie: public Zombie
{
public:
        BloatedZombie() : Zombie() {}
        ~BloatedZombie() {}

};

class MajorClaphamLee: public ZombieClaw
{
public:
        MajorClaphamLee() :
                ZombieClaw() {}
        ~MajorClaphamLee() {}
};

class DeanHalsey: public ZombieClaw
{
public:
        DeanHalsey() : ZombieClaw() {}
        ~DeanHalsey() {}
};

class CrawlingIntestines: public Mon
{
public:
        CrawlingIntestines() : Mon() {}
        ~CrawlingIntestines() {}

};

class CrawlingHand: public Mon
{
public:
        CrawlingHand() : Mon() {}
        ~CrawlingHand() {}

};

class Thing: public Mon
{
public:
        Thing() : Mon() {}
        ~Thing() {}

};

class FloatingSkull: public Mon
{
public:
        FloatingSkull() : Mon() {}
        ~FloatingSkull() {}

};

class MindLeech: public Mon
{
public:
        MindLeech() : Mon() {}
        ~MindLeech() {}

};

class SpiritLeech: public Mon
{
public:
        SpiritLeech() : Mon() {}
        ~SpiritLeech() {}

};

class LifeLeech: public Mon
{
public:
        LifeLeech() : Mon() {}
        ~LifeLeech() {}

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

class LengElder: public Mon
{
public:
        LengElder() :
                Mon(),
                has_given_item_to_player_(false),
                nr_turns_to_hostile_(-1) {}
        ~LengElder() {}


private:
        void on_std_turn_hook()    override;
        bool  has_given_item_to_player_;
        int   nr_turns_to_hostile_;
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

class LordOfShadows: public Mon
{
public:
        LordOfShadows() : Mon() {}
        ~LordOfShadows() {}
};

class LordOfSpiders: public Mon
{
public:
        LordOfSpiders() : Mon() {}
        ~LordOfSpiders() {}

};

class LordOfSpirits: public Mon
{
public:
        LordOfSpirits() : Mon() {}
        ~LordOfSpirits() {}

};

class LordOfPestilence: public Mon
{
public:
        LordOfPestilence() : Mon() {}
        ~LordOfPestilence() {}
};

class FireHound: public Mon
{
public:
        FireHound() : Mon() {}
        ~FireHound() {}
};

class EnergyHound: public Mon
{
public:
        EnergyHound() : Mon() {}
        ~EnergyHound() {}
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

protected:
        DidAction on_act() override;
};

class Phantasm: public Ghost
{
public:
        Phantasm() : Ghost() {}

        ~Phantasm() {}


        std::string death_msg() const override
        {
                return "The Phantasm is put to rest.";
        }
};

class Wraith: public Ghost
{
public:
        Wraith() : Ghost() {}

        ~Wraith() {}


        std::string death_msg() const override
        {
                return "The Wraith is put to rest.";
        }
};

class Raven: public Mon
{
public:
        Raven() : Mon() {}
        ~Raven() {}
};

class GiantBat: public Mon
{
public:
        GiantBat() : Mon() {}
        ~GiantBat() {}
};

class VampireBat: public Mon
{
public:
        VampireBat() : Mon() {}
        ~VampireBat() {}
};

class Abaxu: public Mon
{
public:
        Abaxu() : Mon() {}
        ~Abaxu() {}
};

class Byakhee: public GiantBat
{
public:
        Byakhee() : GiantBat() {}
        ~Byakhee() {}
};

class GiantMantis: public Mon
{
public:
        GiantMantis() : Mon() {}
        ~GiantMantis() {}
};

class Chthonian: public Mon
{
public:
        Chthonian() : Mon() {}
        ~Chthonian() {}
};

class DeathFiend: public Mon
{
public:
        DeathFiend() : Mon() {}
        ~DeathFiend() {}
};

class HuntingHorror: public GiantBat
{
public:
        HuntingHorror() : GiantBat() {}
        ~HuntingHorror() {}
};

class Wolf: public Mon
{
public:
        Wolf() : Mon() {}
        ~Wolf() {}
};

class MiGo: public Mon
{
public:
        MiGo() : Mon() {}
        ~MiGo() {}
};

class MiGoCommander: public MiGo
{
public:
        MiGoCommander() : MiGo() {}
        ~MiGoCommander() {}
};

class SentryDrone: public Mon
{
public:
        SentryDrone() : Mon() {}
        ~SentryDrone() {}

};

class FlyingPolyp: public Mon
{
public:
        FlyingPolyp() : Mon() {}
        ~FlyingPolyp() {}

};

class GreaterPolyp: public FlyingPolyp
{
public:
        GreaterPolyp() : FlyingPolyp() {}
        ~GreaterPolyp() {}
};

class Ghoul: public Mon
{
public:
        Ghoul() : Mon() {}
        ~Ghoul() {}

        virtual void init_hook() override;
};

class VoidTraveler: public Mon
{
public:
        VoidTraveler() : Mon() {}
        ~VoidTraveler() {}

};

class ElderVoidTraveler: public VoidTraveler
{
public:
        ElderVoidTraveler() : VoidTraveler() {}
        ~ElderVoidTraveler() {}

};

class DeepOne: public Mon
{
public:
        DeepOne() : Mon() {}
        ~DeepOne() {}

};

class Ape: public Mon
{
public:
        Ape() : Mon(), frenzy_cooldown_(0) {}
        ~Ape() {}


private:
        DidAction on_act() override;

        int frenzy_cooldown_;
};

class Mummy: public Mon
{
public:
        Mummy() : Mon() {}
        ~Mummy() {}
};

class MummyCrocHead: public Mummy
{
public:
        MummyCrocHead() : Mummy() {}
        ~MummyCrocHead() {}

};

class MummyUnique: public Mummy
{
public:
        MummyUnique() : Mummy() {}
        ~MummyUnique() {}

};

class Khephren: public MummyUnique
{
public:
        Khephren() :
                MummyUnique(),
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

class InvisStalker: public Mon
{
public:
        InvisStalker() : Mon() {}
        ~InvisStalker() {}

};

class WormMass: public Mon
{
public:
        WormMass() :
                Mon() {}

        ~WormMass() {}

};

class MindWorms: public WormMass
{
public:
        MindWorms() :
                WormMass() {}

        ~MindWorms() {}

};

class GiantLocust: public Mon
{
public:
        GiantLocust() :
                Mon() {}

        ~GiantLocust() {}

};

class Vortex: public Mon
{
public:
        Vortex() :
                Mon() {}

        virtual ~Vortex() {}
};

class DustVortex: public Vortex
{
public:
        DustVortex() : Vortex() {}
        ~DustVortex() {}
};

class FireVortex: public Vortex
{
public:
        FireVortex() : Vortex() {}
        ~FireVortex() {}
};

class EnergyVortex: public Vortex
{
public:
        EnergyVortex() : Vortex() {}
        ~EnergyVortex() {}
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

class OozeBlack: public Ooze
{
public:
        OozeBlack() : Ooze() {}
        ~OozeBlack() {}
};

class OozeClear: public Ooze
{
public:
        OozeClear() : Ooze() {}
        ~OozeClear() {}
};

class OozePutrid: public Ooze
{
public:
        OozePutrid() : Ooze() {}
        ~OozePutrid() {}
};

class OozePoison: public Ooze
{
public:
        OozePoison() : Ooze() {}
        ~OozePoison() {}
};

class StrangeColor: public Ooze
{
public:
        StrangeColor() : Ooze() {}

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
        Mold() :
                Mon() {}

        ~Mold() {}


        std::string death_msg() const override
        {
                return "The Mold is destroyed.";
        }
};

class GasSpore: public Mon
{
public:
        GasSpore() : Mon() {}
        ~GasSpore() {}
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

class HighPriestGuardWarVet: public Mon
{
public:
        HighPriestGuardWarVet() {}
        ~HighPriestGuardWarVet() {}

};

class HighPriestGuardRogue: public Mon
{
public:
        HighPriestGuardRogue() {}
        ~HighPriestGuardRogue() {}

};

class HighPriestGuardGhoul: public Ghoul
{
public:
        HighPriestGuardGhoul() {}
        ~HighPriestGuardGhoul() {}

        // Don't become allied to player Ghouls
        void init_hook() override {}

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

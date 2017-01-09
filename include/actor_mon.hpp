#ifndef MON_HPP
#define MON_HPP

#include "global.hpp"
#include "actor.hpp"
#include "sound.hpp"
#include "spells.hpp"

struct AiAttData
{
    AiAttData(Wpn* wpn, bool is_melee) :
        wpn         (wpn),
        is_melee    (is_melee) {}

    Wpn* wpn;
    bool is_melee;
};

struct AiAvailAttacksData
{
    AiAvailAttacksData() :
        is_reload_needed    (false),
        is_melee            (true) {}

    AiAvailAttacksData(const AiAvailAttacksData& other) :
        weapons             (other.weapons),
        is_reload_needed    (other.is_reload_needed),
        is_melee            (other.is_melee) {}

    AiAvailAttacksData& operator=(const AiAvailAttacksData& other)
    {
        weapons             = other.weapons;
        is_reload_needed    = other.is_reload_needed;
        is_melee            = other.is_melee;
        return *this;
    }

    std::vector<Wpn*> weapons;
    bool is_reload_needed, is_melee;
};

class Wpn;

class Mon: public Actor
{
public:
    Mon();
    virtual ~Mon();

    bool can_see_actor(const Actor& other,
                       const bool hard_blocked_los[map_w][map_h]) const;

    void act() override;

    void move(Dir dir) override;

    virtual Clr clr() const override;

    int spell_skill(const SpellId id) const override;

    void avail_attacks(Actor& defender, AiAvailAttacksData& dst);

    AiAttData choose_att(const AiAvailAttacksData& mon_avail_attacks);

    bool try_attack(Actor& defender);

    void hear_sound(const Snd& snd);

    void become_aware_player(const bool is_from_seeing,
                             const int factor = 1);

    void set_player_aware_of_me(int duration_factor = 1);

    void on_actor_turn() override;

    void on_std_turn() override final;

    virtual std::string aggro_phrase_mon_seen() const
    {
        return data_->aggro_text_mon_seen;
    }

    virtual std::string aggro_phrase_mon_hidden() const
    {
        return data_->aggro_text_mon_hidden;
    }

    virtual SfxId aggro_sfx_mon_seen() const
    {
        return data_->aggro_sfx_mon_seen;
    }

    virtual SfxId aggro_sfx_mon_hidden() const
    {
        return data_->aggro_sfx_mon_hidden;
    }

    void speak_phrase(const AlertsMon alerts_others);

    bool is_leader_of(const Actor* const actor) const override;
    bool is_actor_my_leader(const Actor* const actor) const override;

    int                 aware_counter_, player_aware_of_me_counter_;
    bool                is_msg_mon_in_view_printed_;
    Dir                 last_dir_moved_;
    std::vector<Spell*> spells_known_;
    int                 spell_cooldown_current_;
    bool                is_roaming_allowed_;
    bool                is_sneaking_;
    Actor*              leader_;
    Actor*              tgt_;
    bool                waiting_;

protected:
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
    virtual void mk_start_items() override;
};

class RatThing: public Rat
{
public:
    RatThing() : Rat() {}
    ~RatThing() {}
    void mk_start_items() override;
};

class BrownJenkin: public RatThing
{
public:
    BrownJenkin() : RatThing() {}
    ~BrownJenkin() {}
    void mk_start_items() override;
};

class Spider: public Mon
{
public:
    Spider() : Mon() {}
    virtual ~Spider() {}

private:
    DidAction on_act() override;
};

class GreenSpider: public Spider
{
public:
    GreenSpider() : Spider() {}
    ~GreenSpider() {}
    void mk_start_items() override;
};

class WhiteSpider: public Spider
{
public:
    WhiteSpider() : Spider() {}
    ~WhiteSpider() {}
    void mk_start_items() override;
};

class RedSpider: public Spider
{
public:
    RedSpider() : Spider() {}
    ~RedSpider() {}
    void mk_start_items() override;
};

class ShadowSpider: public Spider
{
public:
    ShadowSpider() : Spider() {}
    ~ShadowSpider() {}
    void mk_start_items() override;
};

class LengSpider: public Spider
{
public:
    LengSpider() : Spider() {}
    ~LengSpider() {}
    void mk_start_items() override;
};

class PitViper: public Mon
{
public:
    PitViper() : Mon() {}
    virtual ~PitViper() {}
    void mk_start_items() override;
};

class SpittingCobra: public Mon
{
public:
    SpittingCobra() : Mon() {}
    virtual ~SpittingCobra() {}
    void mk_start_items() override;
};

class BlackMamba: public Mon
{
public:
    BlackMamba() : Mon() {}
    virtual ~BlackMamba() {}
    void mk_start_items() override;
};

class Zombie: public Mon
{
public:
    Zombie() : Mon()
    {
        dead_turn_counter = 0;
        has_resurrected = false;
    }

    virtual ~Zombie() {}

protected:
    virtual DidAction on_act() override;

    void on_death() override;

    DidAction try_resurrect();

    int dead_turn_counter;
    bool has_resurrected;
};

class ZombieClaw: public Zombie
{
public:
    ZombieClaw() : Zombie() {}
    ~ZombieClaw() {}
    void mk_start_items() override;
};

class ZombieAxe: public Zombie
{
public:
    ZombieAxe() : Zombie() {}
    ~ZombieAxe() {}
    void mk_start_items() override;
};

class BloatedZombie: public Zombie
{
public:
    BloatedZombie() : Zombie() {}
    ~BloatedZombie() {}

    void mk_start_items() override;
};

class MajorClaphamLee: public ZombieClaw
{
public:
    MajorClaphamLee() :
        ZombieClaw(), has_summoned_tomb_legions(false) {}
    ~MajorClaphamLee() {}

private:
    DidAction on_act() override;

    bool has_summoned_tomb_legions;
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

    void mk_start_items() override;
};

class CrawlingHand: public Mon
{
public:
    CrawlingHand() : Mon() {}
    ~CrawlingHand() {}

    void mk_start_items() override;
};

class Thing: public Mon
{
public:
    Thing() : Mon() {}
    ~Thing() {}

    void mk_start_items() override;
};

class FloatingSkull: public Mon
{
public:
    FloatingSkull() : Mon() {}
    ~FloatingSkull() {}

    void mk_start_items() override;

private:
    DidAction on_act() override;
};

class KeziahMason: public Mon
{
public:
    KeziahMason() : Mon(), has_summoned_jenkin(false) {}
    ~KeziahMason() {}

    void mk_start_items() override;

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

    void mk_start_items() override;

private:
    void on_std_turn_hook()    override;
    bool  has_given_item_to_player_;
    int   nr_turns_to_hostile_;
};

class Cultist: public Mon
{
public:
    Cultist() : Mon() {}

    virtual void mk_start_items() override;

    static std::string cultist_phrase();

    std::string aggro_phrase_mon_seen() const override
    {
        return name_the() + ": " + cultist_phrase();
    }
    std::string aggro_phrase_mon_hidden() const override
    {
        return "Voice: " + cultist_phrase();
    }

    virtual ~Cultist() {}
};

class CultistElectric: public Cultist
{
public:
    CultistElectric() : Cultist() {}
    ~CultistElectric() {}
    void mk_start_items() override;
};

class CultistSpikeGun: public Cultist
{
public:
    CultistSpikeGun() : Cultist() {}
    ~CultistSpikeGun() {}
    void mk_start_items() override;
};

class CultistPriest: public Cultist
{
public:
    CultistPriest() : Cultist() {}
    ~CultistPriest() {}
    void mk_start_items() override;
};

class LordOfShadows: public Mon
{
public:
    LordOfShadows() : Mon() {}
    ~LordOfShadows() {}
    void mk_start_items() override;

private:
    DidAction on_act() override;
};

class LordOfSpiders: public Mon
{
public:
    LordOfSpiders() : Mon() {}
    ~LordOfSpiders() {}

    void mk_start_items() override;

private:
    DidAction on_act() override;
};

class LordOfSpirits: public Mon
{
public:
    LordOfSpirits() : Mon() {}
    ~LordOfSpirits() {}

    void mk_start_items() override;

private:
    DidAction on_act() override;
};

class LordOfPestilence: public Mon
{
public:
    LordOfPestilence() : Mon() {}
    ~LordOfPestilence() {}
    void mk_start_items() override;

private:
    DidAction on_act() override;
};

class FireHound: public Mon
{
public:
    FireHound() : Mon() {}
    ~FireHound() {}
    void mk_start_items() override;
};

class Zuul: public Mon
{
public:
    Zuul() : Mon() {}
    ~Zuul() {}

    void place_hook() override;

    void mk_start_items() override;

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

    virtual void mk_start_items() override;

    std::string death_msg() const override
    {
        return "The Ghost is put to rest.";
    }

private:
    DidAction on_act() override;
};

class Phantasm: public Ghost
{
public:
    Phantasm() : Ghost() {}
    ~Phantasm() {}
    void mk_start_items() override;

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
    void mk_start_items() override;

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
    void mk_start_items() override;
};

class GiantBat: public Mon
{
public:
    GiantBat() : Mon() {}
    ~GiantBat() {}
    void mk_start_items() override;
};

class Byakhee: public GiantBat
{
public:
    Byakhee() : GiantBat() {}
    ~Byakhee() {}
    void mk_start_items() override;
};

class GiantMantis: public Mon
{
public:
    GiantMantis() : Mon() {}
    ~GiantMantis() {}
    void mk_start_items() override;
};

class Chthonian: public Mon
{
public:
    Chthonian() : Mon() {}
    ~Chthonian() {}
    void mk_start_items() override;
};

class DeathFiend: public Mon
{
public:
    DeathFiend() : Mon() {}
    ~DeathFiend() {}
    void mk_start_items() override;
};

class HuntingHorror: public GiantBat
{
public:
    HuntingHorror() : GiantBat() {}
    ~HuntingHorror() {}
    void mk_start_items() override;
};

class Wolf: public Mon
{
public:
    Wolf() : Mon() {}
    ~Wolf() {}
    void mk_start_items() override;
};

class MiGo: public Mon
{
public:
    MiGo() : Mon() {}
    ~MiGo() {}
    void mk_start_items() override;
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

    void mk_start_items() override;
};

class FlyingPolyp: public Mon
{
public:
    FlyingPolyp() : Mon() {}
    ~FlyingPolyp() {}

    void mk_start_items() override;
};

class GreaterPolyp: public FlyingPolyp
{
public:
    GreaterPolyp() : FlyingPolyp() {}
    ~GreaterPolyp() {}
    void mk_start_items() override;
};

class Ghoul: public Mon
{
public:
    Ghoul() : Mon() {}
    ~Ghoul() {}

    void mk_start_items() override;

    void place_hook() override;

private:
    DidAction on_act() override;
};

class DeepOne: public Mon
{
public:
    DeepOne() : Mon() {}
    ~DeepOne() {}

    void mk_start_items() override;
};

class Ape: public Mon
{
public:
    Ape() : Mon(), frenzy_cooldown_(0) {}
    ~Ape() {}
    void mk_start_items() override;

private:
    DidAction on_act() override;

    int frenzy_cooldown_;
};

class Mummy: public Mon
{
public:
    Mummy() : Mon() {}
    ~Mummy() {}
    virtual void mk_start_items() override;

private:
    virtual DidAction on_act() override;
};

class MummyCrocHead: public Mummy
{
public:
    MummyCrocHead() : Mummy() {}
    ~MummyCrocHead() {}
    virtual void mk_start_items() override;
};

class MummyUnique: public Mummy
{
public:
    MummyUnique() : Mummy() {}
    ~MummyUnique() {}
    void mk_start_items() override;
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

    virtual void mk_start_items() override;

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

    virtual void mk_start_items() override;
};

class WormMass: public Mon
{
public:
    WormMass() :
        Mon             (),
        allow_split_    (true) {}

    ~WormMass() {}

    void mk_start_items() override;

private:
    bool allow_split_;

    void on_death() override;
};

class MindWorms: public WormMass
{
public:
    MindWorms() :
        WormMass() {}

    ~MindWorms() {}

    void mk_start_items() override;
};

class GiantLocust: public Mon
{
public:
    GiantLocust() :
        Mon(),
        spawn_new_one_in_n(45) {}

    ~GiantLocust() {}

    void mk_start_items() override;

private:
    DidAction on_act() override;
    int spawn_new_one_in_n;
};

class Vortex: public Mon
{
public:
    Vortex() :
        Mon(),
        pull_cooldown(0) {}

    virtual ~Vortex() {}

    virtual void mk_start_items() override = 0;

private:
    DidAction on_act() override;

    int pull_cooldown;
};

class DustVortex: public Vortex
{
public:
    DustVortex() : Vortex() {}
    ~DustVortex() {}
    void mk_start_items() override;
};

class FireVortex: public Vortex
{
public:
    FireVortex() : Vortex() {}
    ~FireVortex() {}
    void mk_start_items() override;
};

class Ooze: public Mon
{
public:
    Ooze() : Mon() {}
    ~Ooze() {}
    virtual void mk_start_items() override = 0;

    std::string death_msg() const override
    {
        return "The Ooze disintegrates.";
    }

private:
    virtual void on_std_turn_hook() override;
};

class OozeBlack: public Ooze
{
public:
    OozeBlack() : Ooze() {}
    ~OozeBlack() {}
    void mk_start_items() override;
};

class OozeClear: public Ooze
{
public:
    OozeClear() : Ooze() {}
    ~OozeClear() {}
    void mk_start_items() override;
};

class OozePutrid: public Ooze
{
public:
    OozePutrid() : Ooze() {}
    ~OozePutrid() {}
    void mk_start_items() override;
};

class OozePoison: public Ooze
{
public:
    OozePoison() : Ooze() {}
    ~OozePoison() {}
    void mk_start_items() override;
};

class ColorOoSpace: public Ooze
{
public:
    ColorOoSpace() : Ooze() {}

    ~ColorOoSpace() {}

    void mk_start_items() override;

    Clr clr() const override;

    std::string death_msg() const override
    {
        return "The Color disintegrates.";
    }

private:
    DidAction on_act() override;
    void on_std_turn_hook() override;
};

class Mold: public Mon
{
public:
    Mold() : Mon(), spawn_new_one_in_n(40) {}
    ~Mold() {}

    void mk_start_items() override;

    std::string death_msg() const override
    {
        return "The Mold is destroyed.";
    }

private:
    DidAction on_act() override;

    int spawn_new_one_in_n;
};

class GasSpore: public Mon
{
public:
    GasSpore() : Mon() {}
    ~GasSpore() {}

    void on_death() override;

    void mk_start_items() override {}
};

class TheHighPriest: public Mon
{
public:
    TheHighPriest();
    ~TheHighPriest() {}

    void mk_start_items() override;

private:
    DidAction on_act() override;

    void on_death() override;

    void on_std_turn_hook() override;

    bool has_greeted_player_;
};

class AnimatedWpn: public Mon
{
public:
    AnimatedWpn();

    ~AnimatedWpn() {}

    std::string name_the() const override;

    std::string name_a() const override;

    char glyph() const override;

    Clr clr() const override;

    TileId tile() const override;

    std::string descr() const override;

    std::string death_msg() const override;

    void on_std_turn_hook() override;

private:
    int nr_turns_until_drop_;
};

#endif

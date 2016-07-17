#ifndef MON_HPP
#define MON_HPP

#include "global.hpp"
#include "actor.hpp"
#include "sound.hpp"
#include "spells.hpp"

struct Ai_att_data
{
    Ai_att_data(Wpn* wpn, bool is_melee) :
        wpn         (wpn),
        is_melee    (is_melee) {}

    Wpn* wpn;
    bool is_melee;
};

struct Ai_avail_attacks_data
{
    Ai_avail_attacks_data() :
        is_reload_needed    (false),
        is_melee            (true) {}

    Ai_avail_attacks_data(const Ai_avail_attacks_data& other) :
        weapons             (other.weapons),
        is_reload_needed    (other.is_reload_needed),
        is_melee            (other.is_melee) {}

    Ai_avail_attacks_data& operator=(const Ai_avail_attacks_data& other)
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

    void avail_attacks(Actor& defender, Ai_avail_attacks_data& dst);

    Ai_att_data choose_att(const Ai_avail_attacks_data& mon_avail_attacks);

    bool try_attack(Actor& defender);

    void hear_sound(const Snd& snd);

    void become_aware_player(const bool is_from_seeing);

    void set_player_aware_of_me(const int duration_factor = 1);

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

    virtual Sfx_id aggro_sfx_mon_seen() const
    {
        return data_->aggro_sfx_mon_seen;
    }

    virtual Sfx_id aggro_sfx_mon_hidden() const
    {
        return data_->aggro_sfx_mon_hidden;
    }

    void speak_phrase(const Alerts_mon alerts_others);

    bool is_leader_of(const Actor* const actor) const override;
    bool is_actor_my_leader(const Actor* const actor) const override;

    int                 aware_counter_, player_aware_of_me_counter_;
    bool                is_msg_mon_in_view_printed_;
    Dir                 last_dir_moved_;
    std::vector<Spell*> spells_known_;
    int                 spell_cool_down_cur_;
    bool                is_roaming_allowed_;
    bool                is_sneaking_;
    Actor*              leader_;
    Actor*              tgt_;
    bool                waiting_;
    double              shock_caused_cur_;
    bool                has_given_xp_for_spotting_;

protected:
    virtual void on_hit(int& dmg,
                        const Dmg_type dmg_type,
                        const Dmg_method method,
                        const Allow_wound allow_wound) override;

    virtual Did_action on_act()
    {
        return Did_action::no;
    }

    virtual void on_std_turn_hook() {}

    int group_size();
};

class Rat: public Mon
{
public:
    Rat() : Mon() {}
    ~Rat() {}
    virtual void mk_start_items() override;
};

class Rat_thing: public Rat
{
public:
    Rat_thing() : Rat() {}
    ~Rat_thing() {}
    void mk_start_items() override;
};

class Brown_jenkin: public Rat_thing
{
public:
    Brown_jenkin() : Rat_thing() {}
    ~Brown_jenkin() {}
    void mk_start_items() override;
};

class Spider: public Mon
{
public:
    Spider() : Mon() {}
    virtual ~Spider() {}

private:
    Did_action on_act() override;
};

class Green_spider: public Spider
{
public:
    Green_spider() : Spider() {}
    ~Green_spider() {}
    void mk_start_items() override;
};

class White_spider: public Spider
{
public:
    White_spider() : Spider() {}
    ~White_spider() {}
    void mk_start_items() override;
};

class Red_spider: public Spider
{
public:
    Red_spider() : Spider() {}
    ~Red_spider() {}
    void mk_start_items() override;
};

class Shadow_spider: public Spider
{
public:
    Shadow_spider() : Spider() {}
    ~Shadow_spider() {}
    void mk_start_items() override;
};

class Leng_spider: public Spider
{
public:
    Leng_spider() : Spider() {}
    ~Leng_spider() {}
    void mk_start_items() override;
};

class Pit_viper: public Mon
{
public:
    Pit_viper() : Mon() {}
    virtual ~Pit_viper() {}
    void mk_start_items() override;
};

class Spitting_cobra: public Mon
{
public:
    Spitting_cobra() : Mon() {}
    virtual ~Spitting_cobra() {}
    void mk_start_items() override;
};

class Black_mamba: public Mon
{
public:
    Black_mamba() : Mon() {}
    virtual ~Black_mamba() {}
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
    virtual Did_action on_act() override;

    void on_death() override;

    Did_action try_resurrect();

    int dead_turn_counter;
    bool has_resurrected;
};

class Zombie_claw: public Zombie
{
public:
    Zombie_claw() : Zombie() {}
    ~Zombie_claw() {}
    void mk_start_items() override;
};

class Zombie_axe: public Zombie
{
public:
    Zombie_axe() : Zombie() {}
    ~Zombie_axe() {}
    void mk_start_items() override;
};

class Bloated_zombie: public Zombie
{
public:
    Bloated_zombie() : Zombie() {}
    ~Bloated_zombie() {}

    void mk_start_items() override;
};

class Major_clapham_lee: public Zombie_claw
{
public:
    Major_clapham_lee() :
        Zombie_claw(), has_summoned_tomb_legions(false) {}
    ~Major_clapham_lee() {}

private:
    Did_action on_act() override;

    bool has_summoned_tomb_legions;
};

class Dean_halsey: public Zombie_claw
{
public:
    Dean_halsey() : Zombie_claw() {}
    ~Dean_halsey() {}
};

class Crawling_intestines: public Mon
{
public:
    Crawling_intestines() : Mon() {}
    ~Crawling_intestines() {}

    void mk_start_items() override;
};

class Crawling_hand: public Mon
{
public:
    Crawling_hand() : Mon() {}
    ~Crawling_hand() {}

    void mk_start_items() override;
};

class Thing: public Mon
{
public:
    Thing() : Mon() {}
    ~Thing() {}

    void mk_start_items() override;
};

class Floating_skull: public Mon
{
public:
    Floating_skull() : Mon() {}
    ~Floating_skull() {}

    void mk_start_items() override;

private:
    Did_action on_act() override;
};

class Keziah_mason: public Mon
{
public:
    Keziah_mason() : Mon(), has_summoned_jenkin(false) {}
    ~Keziah_mason() {}

    void mk_start_items() override;

private:
    Did_action on_act() override;

    bool has_summoned_jenkin;
};

class Leng_elder: public Mon
{
public:
    Leng_elder() :
        Mon(),
        has_given_item_to_player_(false),
        nr_turns_to_hostile_(-1) {}
    ~Leng_elder() {}

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

class Cultist_electric: public Cultist
{
public:
    Cultist_electric() : Cultist() {}
    ~Cultist_electric() {}
    void mk_start_items() override;
};

class Cultist_spike_gun: public Cultist
{
public:
    Cultist_spike_gun() : Cultist() {}
    ~Cultist_spike_gun() {}
    void mk_start_items() override;
};

class Cultist_priest: public Cultist
{
public:
    Cultist_priest() : Cultist() {}
    ~Cultist_priest() {}
    void mk_start_items() override;
};

class Lord_of_shadows: public Mon
{
public:
    Lord_of_shadows() : Mon() {}
    ~Lord_of_shadows() {}
    void mk_start_items() override;

private:
    Did_action on_act() override;
};

class Lord_of_spiders: public Mon
{
public:
    Lord_of_spiders() : Mon() {}
    ~Lord_of_spiders() {}

    void mk_start_items() override;

private:
    Did_action on_act() override;
};

class Lord_of_spirits: public Mon
{
public:
    Lord_of_spirits() : Mon() {}
    ~Lord_of_spirits() {}

    void mk_start_items() override;

private:
    Did_action on_act() override;
};

class Lord_of_pestilence: public Mon
{
public:
    Lord_of_pestilence() : Mon() {}
    ~Lord_of_pestilence() {}
    void mk_start_items() override;

private:
    Did_action on_act() override;
};

class Fire_hound: public Mon
{
public:
    Fire_hound() : Mon() {}
    ~Fire_hound() {}
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
    Did_action on_act() override;
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

class Giant_bat: public Mon
{
public:
    Giant_bat() : Mon() {}
    ~Giant_bat() {}
    void mk_start_items() override;
};

class Byakhee: public Giant_bat
{
public:
    Byakhee() : Giant_bat() {}
    ~Byakhee() {}
    void mk_start_items() override;
};

class Giant_mantis: public Mon
{
public:
    Giant_mantis() : Mon() {}
    ~Giant_mantis() {}
    void mk_start_items() override;
};

class Chthonian: public Mon
{
public:
    Chthonian() : Mon() {}
    ~Chthonian() {}
    void mk_start_items() override;
};

class Death_fiend: public Mon
{
public:
    Death_fiend() : Mon() {}
    ~Death_fiend() {}
    void mk_start_items() override;
};

class Hunting_horror: public Giant_bat
{
public:
    Hunting_horror() : Giant_bat() {}
    ~Hunting_horror() {}
    void mk_start_items() override;
};

class Wolf: public Mon
{
public:
    Wolf() : Mon() {}
    ~Wolf() {}
    void mk_start_items() override;
};

class Mi_go: public Mon
{
public:
    Mi_go() : Mon() {}
    ~Mi_go() {}
    void mk_start_items() override;
};

class Mi_go_commander: public Mi_go
{
public:
    Mi_go_commander() : Mi_go() {}
    ~Mi_go_commander() {}
};

class Sentry_drone: public Mon
{
public:
    Sentry_drone() : Mon() {}
    ~Sentry_drone() {}

    void mk_start_items() override;
};

class Flying_polyp: public Mon
{
public:
    Flying_polyp() : Mon() {}
    ~Flying_polyp() {}

    void mk_start_items() override;
};

class Greater_polyp: public Flying_polyp
{
public:
    Greater_polyp() : Flying_polyp() {}
    ~Greater_polyp() {}
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
    Did_action on_act() override;
};

class Deep_one: public Mon
{
public:
    Deep_one() : Mon() {}
    ~Deep_one() {}

    void mk_start_items() override;
};

class Ape: public Mon
{
public:
    Ape() : Mon(), frenzy_cool_down_(0) {}
    ~Ape() {}
    void mk_start_items() override;

private:
    Did_action on_act() override;

    int frenzy_cool_down_;
};

class Mummy: public Mon
{
public:
    Mummy() : Mon() {}
    ~Mummy() {}
    virtual void mk_start_items() override;

private:
    virtual Did_action on_act() override;
};

class Mummy_croc_head: public Mummy
{
public:
    Mummy_croc_head() : Mummy() {}
    ~Mummy_croc_head() {}
    virtual void mk_start_items() override;
};

class Mummy_unique: public Mummy
{
public:
    Mummy_unique() : Mummy() {}
    ~Mummy_unique() {}
    void mk_start_items() override;
};

class Khephren: public Mummy_unique
{
public:
    Khephren() :
        Mummy_unique(),
        has_summoned_locusts(false) {}
    ~Khephren() {}

private:
    Did_action on_act() override;

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

class Invis_stalker: public Mon
{
public:
    Invis_stalker() : Mon() {}
    ~Invis_stalker() {}

    virtual void mk_start_items() override;
};

class Worm_mass: public Mon
{
public:
    Worm_mass() :
        Mon             (),
        allow_split_    (true) {}

    ~Worm_mass() {}

    void mk_start_items() override;

private:
    bool allow_split_;

    void on_death() override;
};

class Mind_worms: public Worm_mass
{
public:
    Mind_worms() :
        Worm_mass() {}

    ~Mind_worms() {}

    void mk_start_items() override;
};

class Giant_locust: public Mon
{
public:
    Giant_locust() :
        Mon(),
        spawn_new_one_in_n(45) {}

    ~Giant_locust() {}

    void mk_start_items() override;

private:
    Did_action on_act() override;
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
    Did_action on_act() override;

    int pull_cooldown;
};

class Dust_vortex: public Vortex
{
public:
    Dust_vortex() : Vortex() {}
    ~Dust_vortex() {}
    void mk_start_items() override;
};

class Fire_vortex: public Vortex
{
public:
    Fire_vortex() : Vortex() {}
    ~Fire_vortex() {}
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

class Ooze_black: public Ooze
{
public:
    Ooze_black() : Ooze() {}
    ~Ooze_black() {}
    void mk_start_items() override;
};

class Ooze_clear: public Ooze
{
public:
    Ooze_clear() : Ooze() {}
    ~Ooze_clear() {}
    void mk_start_items() override;
};

class Ooze_putrid: public Ooze
{
public:
    Ooze_putrid() : Ooze() {}
    ~Ooze_putrid() {}
    void mk_start_items() override;
};

class Ooze_poison: public Ooze
{
public:
    Ooze_poison() : Ooze() {}
    ~Ooze_poison() {}
    void mk_start_items() override;
};

class Color_oo_space: public Ooze
{
public:
    Color_oo_space() : Ooze() {}

    ~Color_oo_space() {}

    void mk_start_items() override;

    Clr clr() override;

    std::string death_msg() const override
    {
        return "The Color disintegrates.";
    }

private:
    Did_action on_act() override;
    void on_std_turn_hook() override;
};

class Mold: public Mon
{
public:
    Mold() : Mon(), spawn_new_one_in_n(30) {}
    ~Mold() {}

    void mk_start_items() override;

    std::string death_msg() const override
    {
        return "The Mold is destroyed.";
    }

private:
    Did_action on_act() override;

    int spawn_new_one_in_n;
};

class Gas_spore: public Mon
{
public:
    Gas_spore() : Mon() {}
    ~Gas_spore() {}

    void on_death() override;

    void mk_start_items() override {}
};

class The_high_priest: public Mon
{
public:
    The_high_priest();
    ~The_high_priest() {}

    void mk_start_items() override;

private:
    Did_action on_act() override;

    void on_death() override;

    void on_std_turn_hook() override;

    bool has_greeted_player_;
};

class Animated_wpn: public Mon
{
public:
    Animated_wpn();

    ~Animated_wpn() {}

    std::string name_the() const override;

    std::string name_a() const override;

    char glyph() const override;

    Clr clr() override;

    Tile_id tile() const override;

    std::string descr() const override;

    std::string death_msg() const override;

    void on_std_turn_hook() override;

private:
    int nr_turns_until_drop_;
};

#endif

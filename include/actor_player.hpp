#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <math.h>

#include "actor.hpp"
#include "global.hpp"
#include "insanity.hpp"

enum class Phobia
{
    rat,
    spider,
    dog,
    undead,
    open_place,
    cramped_place,
    deep_places,
    dark,
    END
};

enum class Obsession
{
    sadism,
    masochism,
    END
};

enum class ShockSrc
{
    see_mon,
    use_strange_item,
    cast_intr_spell,
    time,
    misc,
    END
};

class MedicalBag;
class Explosive;
class Item;
class Wpn;

class Player: public Actor
{
public:
    Player();
    ~Player();

    void save() const;
    void load();

    void update_fov();

    void update_mon_awareness();

    bool can_see_actor(const Actor& other) const;

    std::vector<Actor*> seen_actors() const override;

    std::vector<Actor*> seen_foes() const override;

    bool is_seeing_burning_feature() const;

    void act() override;

    void move(Dir dir) override;

    Color color() const override;

    SpellSkill spell_skill(const SpellId id) const override;

    void on_actor_turn() override;
    void on_std_turn() override;

    void hear_sound(const Snd& snd,
                    const bool is_origin_seen_by_player,
                    const Dir dir_to_origin,
                    const int percent_audible_distance);

    void incr_shock(const ShockLvl shock_lvl, ShockSrc shock_src);

    void incr_shock(const double shock, ShockSrc shock_src);

    double shock_lvl_to_value(const ShockLvl shock_lvl) const;

    void restore_shock(const int amount_restored,
                       const bool is_temp_shock_restored);

    // Used for determining if '!'-marks should be drawn over the player
    double perm_shock_taken_current_turn() const
    {
        return perm_shock_taken_current_turn_;
    }

    int shock_tot() const;

    int ins() const;

    void auto_melee();

    Wpn& unarmed_wpn();

    void set_unarmed_wpn(Wpn* wpn)
    {
        unarmed_wpn_ = wpn;
    }

    void kick_mon(Actor& defender);
    void hand_att(Actor& defender);

    void add_light_hook(bool light_map[map_w][map_h]) const override;

    void on_log_msg_printed();  // Aborts e.g. searching and quick move
    void interrupt_actions();   // Aborts e.g. healing

    int enc_percent() const;

    int carry_weight_lmt() const;

    void set_quick_move(const Dir dir);

    bool is_leader_of(const Actor* const actor) const override;
    bool is_actor_my_leader(const Actor* const actor) const override;

    void on_new_dlvl_reached();

    // Randomly prints a message such as "I sense an object of great power here"
    // if there is a major treasure on the map (on the floor or in a container),
    // and the player is observant
    void item_feeling();

    // Randomly prints a message such as "A chill runs down my spine" if there
    // are unique monsters on the map, and the player is observant
    void mon_feeling();

    MedicalBag* active_medical_bag_;
    int handle_armor_countdown_;
    int armor_putting_on_backpack_idx_;
    bool is_dropping_armor_from_body_slot_;
    Explosive* active_explosive_;
    Actor* tgt_;
    int wait_turns_left;
    int ins_;
    double shock_, shock_tmp_, perm_shock_taken_current_turn_;

private:
    void incr_insanity();

    void reset_perm_shock_taken_current_turn()
    {
        perm_shock_taken_current_turn_ = 0.0;
    }

    int shock_resistance(const ShockSrc shock_src) const;

    double shock_taken_after_mods(const double base_shock,
                                  const ShockSrc shock_src) const;

    void add_shock_from_seen_monsters();

    void update_tmp_shock();

    void on_hit(int& dmg,
                const DmgType dmg_type,
                const DmgMethod method,
                const AllowWound allow_wound) override;

    void fov_hack();

    int nr_turns_until_ins_;

    Dir quick_move_dir_;
    bool has_taken_quick_move_step_;

    int nr_turns_until_rspell_;

    Wpn* unarmed_wpn_;
};

#endif // PLAYER_HPP

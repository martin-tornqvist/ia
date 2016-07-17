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
class Wpn;

class Player: public Actor
{
public:
    Player();
    ~Player();

    void save() const;
    void load();

    void update_fov();

    bool can_see_actor(const Actor& other) const;

    void act() override;

    void move(Dir dir) override;

    void mk_start_items() override;

    void on_actor_turn() override;
    void on_std_turn() override;

    void hear_sound(const Snd& snd, const bool is_origin_seen_by_player,
                    const Dir dir_to_origin,
                    const int percent_audible_distance);

    void incr_shock(const ShockLvl shock_value, ShockSrc shock_src);
    void incr_shock(const int shock, ShockSrc shock_src);
    void restore_shock(const int amount_restored, const bool is_temp_shock_restored);
    void update_tmp_shock();

    int shock_tot() const
    {
        return int(floor(shock_ + shock_tmp_));
    }

    int ins() const;

    //Used for determining if '!'-marks should be drawn on the player map symbol
    double perm_shock_taken_cur_turn() const
    {
        return perm_shock_taken_cur_turn_;
    }

    void reset_perm_shock_taken_cur_turn()
    {
        perm_shock_taken_cur_turn_ = 0.0;
    }

    int shock_resistance(const ShockSrc shock_src) const;
    double shock_taken_after_mods(const int base_shock, const ShockSrc shock_src) const;

    void auto_melee();

    Wpn& unarmed_wpn();

    void kick_mon(Actor& defender);
    void hand_att(Actor& defender);

    void update_clr() override;

    void add_light_hook(bool light_map[map_w][map_h]) const override;

    void on_log_msg_printed();  //Aborts e.g. searching and quick move
    void interrupt_actions();   //Aborts e.g. healing

    int enc_percent() const;

    int carry_weight_lmt() const;

    void set_quick_move(const Dir dir);

    bool is_leader_of(const Actor* const actor) const override;
    bool is_actor_my_leader(const Actor* const actor) const override;

    bool is_standing_in_open_place() const;

    bool is_standing_in_cramped_place() const;

    bool is_free_step_turn() const;

    MedicalBag* active_medical_bag;
    Explosive* active_explosive;
    Actor* tgt_;
    int wait_turns_left;
    int ins_;
    double shock_, shock_tmp_, perm_shock_taken_cur_turn_;

private:
    void incr_insanity();

    void on_hit(int& dmg,
                const DmgType dmg_type,
                const DmgMethod method,
                const AllowWound allow_wound) override;

    void fov_hack();

    int nr_steps_until_free_action_;
    int nr_turns_until_ins_;

    int nr_quick_move_steps_left_;
    Dir quick_move_dir_;

    int nr_turns_until_rspell_;

    Wpn* unarmed_wpn_;
};


#endif

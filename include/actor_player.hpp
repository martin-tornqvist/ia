#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <math.h>

#include "actor.hpp"
#include "cmn.hpp"
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

enum class Shock_src
{
    see_mon,
    use_strange_item,
    cast_intr_spell,
    time,
    misc,
    END
};

class Medical_bag;
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

    void move(Dir dir);

    void mk_start_items() override;

    void on_actor_turn() override;
    void on_std_turn() override;

    void hear_sound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                    const Dir dir_to_origin,
                    const int PERCENT_AUDIBLE_DISTANCE);

    void incr_shock(const Shock_lvl shock_value, Shock_src shock_src);
    void incr_shock(const int SHOCK, Shock_src shock_src);
    void restore_shock(const int amount_restored, const bool IS_TEMP_SHOCK_RESTORED);
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

    int shock_resistance(const Shock_src shock_src) const;
    double shock_taken_after_mods(const int BASE_SHOCK, const Shock_src shock_src) const;

    void auto_melee();

    Wpn& unarmed_wpn();

    void kick_mon(Actor& defender);
    void hand_att(Actor& defender);

    void update_clr();

    void add_light_hook(bool light_map[MAP_W][MAP_H]) const;

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

    Medical_bag* active_medical_bag;
    Explosive* active_explosive;
    Actor* tgt_;
    int wait_turns_left;
    int ins_;
    double shock_, shock_tmp_, perm_shock_taken_cur_turn_;

private:
    void incr_insanity();

    void on_hit(int& dmg,
                const Dmg_type dmg_type,
                const Dmg_method method,
                const Allow_wound allow_wound) override;

    void fov_hack();

    int nr_steps_until_free_action_;
    int nr_turns_until_ins_;

    int nr_quick_move_steps_left_;
    Dir quick_move_dir_;

    int nr_turns_until_rspell_;

    Wpn* unarmed_wpn_;
};


#endif

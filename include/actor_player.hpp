#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "actor.hpp"
#include "cmn_data.hpp"

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

class Mon;
class Medical_bag;
class Explosive;

class Player: public Actor
{
public:
    Player();
    ~Player();

    void update_fov();

    void move_dir(Dir dir);

    void mk_start_items() override;
    void on_actor_turn() override;
    void on_std_turn() override;

    void hear_sound(const Snd& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                    const Dir dir_to_origin,
                    const int PERCENT_AUDIBLE_DISTANCE);

    void incr_shock(const Shock_lvl shock_value, Shock_src shock_src);
    void incr_shock(const int SHOCK, Shock_src shock_src);
    void restore_shock(const int amount_restored,
                       const bool IS_TEMP_SHOCK_RESTORED);
    void update_tmp_shock();

    int shock_tot() const
    {
        return int(floor(shock_ + shock_tmp_));
    }

    int ins() const
    {
        return std::min(100, ins_);
    }

    //The following is used for determining if '!'-marks should be drawn on the
    //player map symbol
    double perm_shock_taken_cur_turn() const
    {
        return perm_shock_taken_cur_turn_;
    }

    void reset_perm_shock_taken_cur_turn()
    {
        perm_shock_taken_cur_turn_ = 0.0;
    }

    int shock_resistance(const Shock_src shock_src) const;
    double shock_taken_after_mods(const int BASE_SHOCK,
                                  const Shock_src shock_src) const;

    void store_to_save_lines(std::vector<std::string>& lines) const;
    void setup_from_save_lines(std::vector<std::string>& lines);

    void auto_melee();

    void kick_mon(Actor& actor_to_kick);
    void punch_mon(Actor& actor_to_punch);

    void update_clr();

    void add_light_hook(bool light_map[MAP_W][MAP_H]) const;

    void on_log_msg_printed(); //Aborts e.g. searching and quick move
    void interrupt_actions(); //Aborts e.g. healing

    int enc_percent() const;

    int carry_weight_lmt() const;

    void set_quick_move(const Dir dir);

    bool is_leader_of(const Actor* const actor) const override;
    bool is_actor_my_leader(const Actor* const actor) const override;

    bool phobias[int(Phobia::END)];
    bool obsessions[int(Obsession::END)];

    Medical_bag* active_medical_bag;
    Explosive* active_explosive;
    Actor* tgt_;
    int wait_turns_left;
    int ins_;
    double shock_, shock_tmp_, perm_shock_taken_cur_turn_;

private:
    void incr_insanity();

    void test_phobias();

    void on_hit(int& dmg) override;

    void FOVhack();

    bool is_standing_in_open_space() const;

    bool is_standing_in_cramped_space() const;

    int nr_moves_until_free_action_;
    int nr_turns_until_ins_;

    int nr_quick_move_steps_left_;
    Dir quick_move_dir_;

    const int CARRY_WEIGHT_BASE_;
};


#endif

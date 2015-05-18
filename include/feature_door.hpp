#ifndef FEATURE_DOOR_H
#define FEATURE_DOOR_H

#include "feature_rigid.hpp"

enum class Door_spawn_state
{
    open,
    closed,
    stuck,
    secret,
    secret_and_stuck,
    any
};

class Door: public Rigid
{
public:
    Door(const Pos& feature_pos, const Rigid* const mimic_feature,
         Door_spawn_state spawn_state = Door_spawn_state::any);

//Spawn-by-id compliant ctor (do not use for normal cases):
    Door(const Pos& feature_pos) :
        Rigid(feature_pos),
        mimic_feature_(nullptr),
        nr_spikes_(0),
        is_open_(false),
        is_stuck_(false),
        is_secret_(false),
        is_handled_externally_(false),
        matl_(Matl::wood) {}

    Door() = delete;

    ~Door() override;

    Feature_id id() const override {return Feature_id::door;}

    std::string name(const Article article) const override;
    Was_destroyed on_finished_burning() override;
    char glyph() const override;
    Tile_id tile() const override;
    void bump(Actor& actor_bumping) override;
    bool can_move_cmn() const override;
    bool can_move(const bool actors_prop_ids[size_t(Prop_id::END)]) const override;
    bool is_los_passable() const override;
    bool is_projectile_passable() const override;
    bool is_smoke_passable() const override;

    void try_open(Actor* actor_trying);
    void try_close(Actor* actor_trying);
    bool try_spike(Actor* actor_trying);

    bool is_open() const {return is_open_;}
    bool is_secret() const {return is_secret_;}
    bool is_stuck() const {return is_stuck_;}
    bool is_handled_externally() const {return is_handled_externally_;}

    Matl matl() const;

    void reveal(const bool ALLOW_MESSAGE);

    void set_to_secret() {is_open_ = is_secret_ = false;}

    virtual Did_open open(Actor* const actor_opening) override;

    static bool is_tile_any_door(const Tile_id tile)
    {
        return tile == Tile_id::door_closed || tile == Tile_id::door_open;
    }

    void player_try_spot_hidden();

    const Rigid* mimic() const {return mimic_feature_;}

private:
    Clr clr_() const override;

    void on_hit(const Dmg_type dmg_type, const Dmg_method dmg_method,
                Actor* const actor) override;

    const Rigid* const mimic_feature_;
    int nr_spikes_;

    bool is_open_, is_stuck_, is_secret_, is_handled_externally_;

    Matl matl_;
};

#endif


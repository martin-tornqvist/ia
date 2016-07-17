#ifndef FEATURE_HPP
#define FEATURE_HPP

#include <vector>
#include <string>

#include "feature_data.hpp"

class Actor;

class Feature
{
public:
    Feature(const P& feature_pos) :
        pos_(feature_pos) {}

    virtual ~Feature() {}

    virtual Feature_id id() const = 0;
    virtual std::string name(const Article article) const = 0;
    virtual Clr clr() const = 0;
    virtual Clr clr_bg() const = 0;

    const Feature_data_t& data() const;

    virtual void hit(const Dmg_type dmg_type,
                     const Dmg_method dmg_method,
                     Actor* const actor);

    virtual void bump(Actor& actor_bumping);
    virtual void on_new_turn() {}
    virtual bool can_move_cmn() const;
    virtual bool can_move(Actor& actor) const;
    virtual bool is_sound_passable() const;
    virtual bool is_los_passable() const;
    virtual bool is_projectile_passable() const;
    virtual bool is_smoke_passable() const;
    virtual bool is_bottomless() const;
    virtual char glyph() const;
    virtual Tile_id tile() const;
    virtual bool can_have_corpse() const;
    virtual bool can_have_rigid() const;
    virtual bool can_have_blood() const;
    virtual bool can_have_gore() const;
    virtual bool can_have_item() const;
    virtual int dodge_modifier() const;
    virtual Matl matl() const;

    int shock_when_adj() const;

    virtual void add_light(bool light[map_w][map_h]) const;

    P pos() const
    {
        return pos_;
    }

protected:
    P pos_;
};

#endif

#ifndef FEATURE_H
#define FEATURE_H

#include <vector>
#include <string>

#include "cmn_types.hpp"
#include "feature_data.hpp"

class Actor;

class Feature
{
public:
    Feature(const Pos& pos) : pos_(pos) {}

    virtual ~Feature() {}

    virtual Feature_id     get_id()                         const = 0;
    virtual std::string   get_name(const Article article)  const = 0;
    virtual Clr           get_clr()                        const = 0;
    virtual Clr           get_clr_bg()                      const = 0;

    const Feature_data_t&   get_data() const;

    virtual void hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor);

    virtual void    bump(Actor& actor_bumping);
    virtual void    on_new_turn() {}
    virtual bool    can_move_cmn()                        const;
    virtual bool    can_move(const bool actor_prop_ids[size_t(Prop_id::END)]) const;
    virtual bool    is_sound_passable()                   const;
    virtual bool    is_los_passable()                     const;
    virtual bool    is_projectile_passable()              const;
    virtual bool    is_smoke_passable()                   const;
    virtual bool    is_bottomless()                      const;
    virtual char    get_glyph()                          const;
    virtual Tile_id  get_tile()                           const;
    virtual void    add_light(bool light[MAP_W][MAP_H])  const;
    virtual bool    can_have_corpse()                     const;
    virtual bool    can_have_rigid()                      const;
    virtual bool    can_have_blood()                      const;
    virtual bool    can_have_gore()                       const;
    virtual bool    can_have_item()                       const;
    virtual int     get_dodge_modifier()                  const;
    virtual Matl    get_matl()                           const;

    int get_shock_when_adj() const;
    Pos get_pos()          const {return pos_;}

protected:
    Pos pos_;
};

#endif

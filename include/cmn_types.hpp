#ifndef CMN_TYPES_H
#define CMN_TYPES_H

#include <algorithm>
#include <string>
#include <cassert>

#include "colors.hpp"
#include "art.hpp"

//This is typically used for functions such as item identification and property applying to
//enable/disable printing to the message log and other such "side effects". For example when
//loading a saved game, we may want to do these things silently.
enum class Verbosity
{
    silent,
    verbose,
};

enum class Inv_type
{
    slots,
    backpack
};

enum class Did_action
{
    yes,
    no
};

enum class Pass_time
{
    yes,
    no
};

enum class Consume_item
{
    yes,
    no
};

enum class Unequip_allowed
{
    yes,
    no
};

enum class Item_ref_type
{
    plain,
    a,
    plural,
    END
};

enum class Item_ref_inf
{
    none,
    yes
};

enum class Item_ref_att_inf
{
    none,
    wpn_context,
    melee,
    ranged,
    thrown
};

enum class Article
{
    a,
    the
};

enum class Matl
{
    empty,
    stone,
    metal,
    plant,  //Grass, bushes, reeds, vines, fungi...
    wood,   //Trees, doors, benches...
    cloth,  //Carpet, silk (cocoons)...
    fluid
};

enum class Liquid_type
{
    water,
    mud,
    blood,
    acid,
    lava
};

enum class Condition
{
    breaking,
    shoddy,
    fine
};

enum class Dmg_type
{
    physical,
    fire,
    acid,
    electric,
    spirit,
    light,
    pure,
    END
};

enum class Dmg_method
{
    piercing,
    slashing,
    blunt_medium,
    blunt_heavy,
    kick,
    explosion,
    shotgun,
    elemental,
    forced, //Guaranteed to detroy the feature (silently - no messages)
    END
};

enum class Att_mode
{
    none,
    melee,
    thrown,
    ranged
};

enum class Allow_wound
{
    no,
    yes
};

enum class Shock_lvl
{
    none   = 0,
    mild   = 2,
    some   = 4,
    heavy  = 12
};

enum class Game_entry_mode
{
    new_game,
    load_game
};

enum class Spawn_rate
{
    never,
    extremely_rare,
    very_rare,
    rare,
    common,
    very_common
};

enum class Dir
{
    down_left   = 1,
    down        = 2,
    down_right  = 3,
    left        = 4,
    center      = 5,
    right       = 6,
    up_left     = 7,
    up          = 8,
    up_right    = 9,
    END
};

enum class Ver_dir
{
    up,
    down
};

enum class Actor_state
{
    alive,
    corpse,
    destroyed
};

enum class Axis
{
    hor,
    ver
};

enum class Lgt_size
{
    none,
    small, //3x3
    fov
};

enum class More_prompt_on_msg
{
    no,
    yes
};

enum class Inf_screen_type
{
    scrolling,
    single_screen
};

struct Cell_render_data
{
    Cell_render_data();

    Clr     clr;
    Clr     clr_bg;
    Tile_id  tile;
    char    glyph;
    int     lifebar_length;
    bool    is_light_fade_allowed, is_marked_lit, is_living_actor_seen_here, is_aware_of_mon_here;
};


struct Str_and_clr
{
    Str_and_clr();

    Str_and_clr(const std::string& text, const Clr& color);

    Str_and_clr& operator=(const Str_and_clr& other);

    std::string str;
    Clr clr;
};

class P
{
public:
    P();

    P(const int X, const int Y);

    P(const P& p);

    P(const int V);

    P& operator=        (const P&   p);
    P& operator/=       (const int  V);
    P& operator+=       (const P&   o);
    P& operator-=       (const P&   o);

    P       operator+   (const P&   p) const;
    P       operator+   (const int  V) const;
    P       operator-   (const P&   p) const;
    P       operator-   (const int  V) const;
    P       operator/   (const int  V) const;
    P       operator/   (const P&   p) const;
    P       operator*   (const int  V) const;
    P       operator*   (const P&   p) const;

    bool    operator==  (const P&   p) const;
    bool    operator!=  (const P&   p) const;
    bool    operator!=  (const int  V) const;
    bool    operator>   (const P&   p) const;
    bool    operator>   (const int  V) const;
    bool    operator<   (const P&   p) const;
    bool    operator<   (const int  V) const;
    bool    operator>=  (const P&   p) const;
    bool    operator>=  (const int  V) const;
    bool    operator<=  (const P&   p) const;
    bool    operator<=  (const int  V) const;

    P signs() const;

    void set(const int x_, const int y_);
    void set(const P& p);
    void swap(P& p);

    int x, y;
};

class Rect
{
public:
    Rect();

    Rect(const P& p0_, const P& p1_);

    Rect(const int X0, const int Y0, const int X1, const int Y1);

    Rect(const Rect& r);

    int w()         const;
    int h()         const;
    P dims()        const;
    int min_dim()   const;
    int max_dim()   const;
    P center_pos()  const;

    P p0;
    P p1;
};

struct Pos_and_val
{
    Pos_and_val();

    Pos_and_val(const P& pos_, const int val_);

    Pos_and_val(const Pos_and_val& other);

    P pos;
    int val;
};

struct Dice_param
{
    Dice_param();

    Dice_param(const int ROLLS, const int SIDES, const int PLUS = 0);

    Dice_param(const Dice_param& other);

    Dice_param& operator=(const Dice_param& other);

    int max() const;

    int rolls, sides, plus;
};

struct Range
{
    Range();

    Range(const int MIN, const int MAX);

    Range(const Range& other);

    int len() const;

    bool is_in_range(const int V) const;

    void set(const int MIN, const int MAX);

    int min, max;
};

struct Fraction
{
    Fraction();

    Fraction(const int NUMERATOR, const int DENOMINATOR);

    void set(const int NUMERATOR, const int DENOMINATOR);

    int numerator, denominator;
};

enum class Item_type
{
    general,
    melee_wpn,
    ranged_wpn,
    throwing_wpn,
    ammo,
    ammo_mag,
    scroll,
    potion,
    device,
    rod,
    armor,
    head_wear,
    amulet,
    explosive,

    END_OF_EXTR_ITEMS,
    melee_wpn_intr,
    ranged_wpn_intr
};

struct Item_name
{
    Item_name(const std::string& name,
              const std::string& name_pl,
              const std::string& name_a);

    Item_name();

    std::string names[size_t(Item_ref_type::END)];
};

struct Item_att_msgs
{
    Item_att_msgs();

    Item_att_msgs(const std::string& player_, const std::string& other_);

    std::string player, other;
};

#endif

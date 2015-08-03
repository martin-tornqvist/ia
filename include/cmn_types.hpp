#ifndef CMN_TYPES_H
#define CMN_TYPES_H

#include <algorithm>
#include <string>

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
    cold,
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

enum class Shock_lvl
{
    none   = 0,
    mild   = 2,
    some   = 4,
    heavy  = 12
};

enum class Game_entry_mode {new_game, load_game};

struct Cell_render_data
{
    Cell_render_data() :
        clr(clr_black),
        clr_bg(clr_black),
        tile(Tile_id::empty),
        glyph(' '),
        lifebar_length(-1),
        is_light_fade_allowed(true),
        is_marked_lit(false),
        is_living_actor_seen_here(false),
        is_aware_of_mon_here(false) {}

    Clr     clr;
    Clr     clr_bg;
    Tile_id  tile;
    char    glyph;
    int     lifebar_length;
    bool    is_light_fade_allowed, is_marked_lit, is_living_actor_seen_here, is_aware_of_mon_here;
};


struct Str_and_clr
{
    Str_and_clr() : str(""), clr(clr_black) {}

    Str_and_clr(const std::string& text, const Clr& color) :
        str(text), clr(color) {}

    Str_and_clr& operator=(const Str_and_clr& other)
    {
        str = other.str;
        clr = other.clr;
        return *this;
    }

    std::string str;
    Clr clr;
};

struct Pos
{
    Pos() : x(0), y(0) {}
    Pos(const int X, const int Y) : x(X), y(Y) {}
    Pos(const Pos& p) : x(p.x), y(p.y) {}
    Pos(const int V) : x(V), y(V) {}

    Pos& operator=(const Pos& p) {x = p.x; y = p.y; return *this;}

    Pos& operator/=(const int  V)       {x /= V;   y /= V;   return *this;}
    Pos& operator+=(const Pos& o)       {x += o.x; y += o.y; return *this;}
    Pos& operator-=(const Pos& o)       {x -= o.x; y -= o.y; return *this;}
    Pos operator+(const Pos& p)     const {return Pos(x + p.x,  y + p.y);}
    Pos operator+(const int V)      const {return Pos(x + V,    y + V);}
    Pos operator-(const Pos& p)     const {return Pos(x - p.x,  y - p.y);}
    Pos operator-(const int V)      const {return Pos(x - V,    y - V);}
    Pos operator/(const int V)      const {return Pos(x / V,    y / V);}
    Pos operator/(const Pos& p)     const {return Pos(x / p.x,  y / p.y);}
    Pos operator*(const int V)      const {return Pos(x * V,    y * V);}
    Pos operator*(const Pos& p)     const {return Pos(x * p.x,  y * p.y);}
    bool operator==(const Pos& p)   const {return x == p.x  && y == p.y;}
    bool operator!=(const Pos& p)   const {return x != p.x  || y != p.y;}
    bool operator!=(const int V)    const {return x != V    || y != V;}
    bool operator>(const Pos& p)    const {return x > p.x   && y > p.y;}
    bool operator>(const int  V)    const {return x > V     && y > V;}
    bool operator<(const Pos& p)    const {return x < p.x   && y < p.y;}
    bool operator<(const int  V)    const {return x < V     && y < V;}
    bool operator>=(const Pos&  p)  const {return x >= p.x  && y >= p.y;}
    bool operator>=(const int   V)  const {return x >= V    && y >= V;}
    bool operator<=(const Pos&  p)  const {return x <= p.x  && y <= p.y;}
    bool operator<=(const int   V)  const {return x <= V    && y <= V;}

    Pos signs() const
    {
        return Pos(x == 0 ? 0 : x > 0 ? 1 : -1,
                   y == 0 ? 0 : y > 0 ? 1 : -1);
    }

    void set(const int x_, const int y_)  {x = x_;  y = y_;}
    void set(const Pos& p)                {x = p.x; y = p.y;}

    void swap(Pos& p) {Pos p_temp(p); p = *this; set(p_temp);}

    int x, y;
};

struct Rect
{
    Rect() : p0(Pos()), p1(Pos()) {}

    Rect(const Pos& p0_, const Pos& p1_) :  p0(p0_), p1(p1_) {}

    Rect(const int X0, const int Y0, const int X1, const int Y1) :
        p0(Pos(X0, Y0)), p1(Pos(X1, Y1)) {}

    Rect(const Rect& r) : p0(r.p0), p1(r.p1) {}

    int w()                 const {return p1.x - p0.x + 1;}
    int h()                 const {return p1.y - p0.y + 1;}
    Pos dims()          const {return {w(), h()};}
    int min_dim()       const {return std::min(w(), h());}
    int max_dim()       const {return std::max(w(), h());}
    Pos center_pos()    const {return ((p1 + p0) / 2);}

    Pos p0;
    Pos p1;
};

struct Pos_and_val
{
    Pos_and_val() : pos(Pos()), val(-1) {}

    Pos_and_val(const Pos& pos_, const int val_) : pos(pos_), val(val_) {}

    Pos_and_val(const Pos_and_val& other) : pos(other.pos), val(other.val) {}

    Pos pos;
    int val;
};

struct Dice_param
{
    Dice_param() : rolls(0), sides(0), plus(0) {}

    Dice_param(const int ROLLS, const int SIDES, const int PLUS = 0) :
        rolls(ROLLS), sides(SIDES), plus(PLUS) {}

    Dice_param(const Dice_param& other) :
        rolls(other.rolls), sides(other.sides), plus(other.plus) {}

    Dice_param& operator=(const Dice_param& other)
    {
        rolls = other.rolls;
        sides = other.sides;
        plus  = other.plus;
        return *this;
    }

    int highest() const {return (rolls * sides) + plus;}

    int rolls, sides, plus;
};

struct Range
{
    Range() : lower(-1), upper(-1) {}

    Range(const int LOWER, const int UPPER) : lower(LOWER), upper(UPPER) {}

    Range(const Range& other) : lower(other.lower), upper(other.upper) {}

    int lower, upper;
};

struct Fraction
{
    Fraction() : numerator(-1), denominator(-1) {}

    Fraction(const int NUMERATOR, const int DENOMINATOR) :
        numerator(NUMERATOR), denominator(DENOMINATOR) {}

    void set(const int NUMERATOR, const int DENOMINATOR)
    {
        numerator   = NUMERATOR;
        denominator = DENOMINATOR;
    }

    int numerator, denominator;
};

enum class Item_type
{
    general,
    melee_wpn,
    ranged_wpn,
    throwing_wpn,
    ammo,
    ammo_clip,
    scroll,
    potion,
    device,
    armor,
    head_wear,
    amulet,
    ring,
    explosive,

    END_OF_EXTR_ITEMS,
    melee_wpn_intr,
    ranged_wpn_intr
};

struct Item_name
{
    Item_name(const std::string& NAME, const std::string& NAME_PL,
              const std::string& NAME_A)
    {
        names[int(Item_ref_type::plain)]   = NAME;
        names[int(Item_ref_type::plural)]  = NAME_PL;
        names[int(Item_ref_type::a)]       = NAME_A;
    }

    Item_name()
    {
        for (int i = 0; i < int(Item_ref_type::END); i++)
        {
            names[i] = "";
        }
    }

    std::string names[int(Item_ref_type::END)];
};

struct Item_att_msgs
{
    Item_att_msgs() :
        player(""),
        other("") {}

    Item_att_msgs(const std::string& player_, const std::string& other_) :
        player(player_),
        other(other_) {}

    std::string player, other;
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
    down       = 2,
    down_right  = 3,
    left       = 4,
    center     = 5,
    right      = 6,
    up_left     = 7,
    up         = 8,
    up_right    = 9,
    END
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

#endif

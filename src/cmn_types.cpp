#include "cmn_types.hpp"

//============ Cell_render_data ===============================
Cell_render_data::Cell_render_data() :
        clr                         (clr_black),
        clr_bg                      (clr_black),
        tile                        (Tile_id::empty),
        glyph                       (' '),
        lifebar_length              (-1),
        is_light_fade_allowed       (true),
        is_marked_lit               (false),
        is_living_actor_seen_here   (false),
        is_aware_of_mon_here        (false) {}

//============ Str_and_clr ====================================
Str_and_clr::Str_and_clr() : str(""), clr(clr_black) {}

Str_and_clr::Str_and_clr(const std::string& text, const Clr& color) :
    str(text), clr(color) {}

Str_and_clr& Str_and_clr::operator=(const Str_and_clr& other)
{
    str = other.str;
    clr = other.clr;
    return *this;
}

//============ P ==============================================
P::P() :
    x(0),
    y(0) {}

P::P(const int X, const int Y) :
    x(X),
    y(Y) {}

P::P(const P& p) :
    x(p.x),
    y(p.y) {}

P::P(const int V) :
    x(V),
    y(V) {}

P&      P::operator=   (const P&   p) {x = p.x;  y = p.y;  return *this;}
P&      P::operator/=  (const int  V) {x /= V;   y /= V;   return *this;}
P&      P::operator+=  (const P&   o) {x += o.x; y += o.y; return *this;}
P&      P::operator-=  (const P&   o) {x -= o.x; y -= o.y; return *this;}

P       P::operator+   (const P&   p) const {return P(x + p.x, y + p.y);}
P       P::operator+   (const int  V) const {return P(x + V,   y + V);}
P       P::operator-   (const P&   p) const {return P(x - p.x, y - p.y);}
P       P::operator-   (const int  V) const {return P(x - V,   y - V);}
P       P::operator/   (const int  V) const {return P(x / V,   y / V);}
P       P::operator/   (const P&   p) const {return P(x / p.x, y / p.y);}
P       P::operator*   (const int  V) const {return P(x * V,   y * V);}
P       P::operator*   (const P&   p) const {return P(x * p.x, y * p.y);}

bool    P::operator==  (const P&   p) const {return x == p.x && y == p.y;}
bool    P::operator!=  (const P&   p) const {return x != p.x || y != p.y;}
bool    P::operator!=  (const int  V) const {return x != V   || y != V;}
bool    P::operator>   (const P&   p) const {return x >  p.x && y > p.y;}
bool    P::operator>   (const int  V) const {return x >  V   && y > V;}
bool    P::operator<   (const P&   p) const {return x <  p.x && y < p.y;}
bool    P::operator<   (const int  V) const {return x <  V   && y < V;}
bool    P::operator>=  (const P&   p) const {return x >= p.x && y >= p.y;}
bool    P::operator>=  (const int  V) const {return x >= V   && y >= V;}
bool    P::operator<=  (const P&   p) const {return x <= p.x && y <= p.y;}
bool    P::operator<=  (const int  V) const {return x <= V   && y <= V;}

P P::signs() const
{
    return P(x == 0 ? 0 : x > 0 ? 1 : -1,
             y == 0 ? 0 : y > 0 ? 1 : -1);
}

void P::set(const int x_, const int y_)    {x = x_;  y = y_;}
void P::set(const P& p)                    {x = p.x; y = p.y;}
void P::swap(P& p)                         {P p_temp(p); p = *this; set(p_temp);}

//============ Rect ===========================================
Rect::Rect() :
    p0(P()),
    p1(P()) {}

Rect::Rect(const P& p0_, const P& p1_) :
    p0(p0_),
    p1(p1_) {}

Rect::Rect(const int X0, const int Y0, const int X1, const int Y1) :
    p0(P(X0, Y0)),
    p1(P(X1, Y1)) {}

Rect::Rect(const Rect& r) :
    p0(r.p0),
    p1(r.p1) {}

int Rect::w()         const {return p1.x - p0.x + 1;}
int Rect::h()         const {return p1.y - p0.y + 1;}
P Rect::dims()        const {return {w(), h()};}
int Rect::min_dim()   const {return std::min(w(), h());}
int Rect::max_dim()   const {return std::max(w(), h());}
P Rect::center_pos()  const {return ((p1 + p0) / 2);}

//============ Pos_and_val ====================================
Pos_and_val::Pos_and_val() : pos(P()), val(-1) {}

Pos_and_val::Pos_and_val(const P& pos_, const int val_) :
    pos     (pos_),
    val     (val_) {}

Pos_and_val::Pos_and_val(const Pos_and_val& other) :
    pos     (other.pos),
    val     (other.val) {}

//============ Dice_param =====================================
Dice_param::Dice_param() :
    rolls   (0),
    sides   (0),
    plus    (0) {}

Dice_param::Dice_param(const int ROLLS, const int SIDES, const int PLUS) :
    rolls   (ROLLS),
    sides   (SIDES),
    plus    (PLUS) {}

Dice_param::Dice_param(const Dice_param& other) :
    rolls   (other.rolls),
    sides   (other.sides),
    plus    (other.plus) {}

Dice_param& Dice_param::operator=(const Dice_param& other)
{
    rolls = other.rolls;
    sides = other.sides;
    plus  = other.plus;
    return *this;
}

int Dice_param::max() const
{
    return (rolls * sides) + plus;
}

//============ Range ==========================================
Range::Range() :
    min(-1),
    max(-1) {}

Range::Range(const int MIN, const int MAX) :
    min(MIN),
    max(MAX) {}

Range::Range(const Range& other) :
    Range(other.min, other.max) {}

int Range::len() const
{
    return max - min + 1;
}

bool Range::is_in_range(const int V) const
{
    assert(max >= min);

    return V >= min && V <= max;
}

void Range::set(const int MIN, const int MAX)
{
    min = MIN;
    max = MAX;
}

//============ Fraction =======================================
Fraction::Fraction() : numerator(-1), denominator(-1) {}

Fraction::Fraction(const int NUMERATOR, const int DENOMINATOR) :
    numerator(NUMERATOR), denominator(DENOMINATOR) {}

void Fraction::set(const int NUMERATOR, const int DENOMINATOR)
{
    numerator   = NUMERATOR;
    denominator = DENOMINATOR;
}

//============ Item_name =======================================
Item_name::Item_name(const std::string& name,
                     const std::string& name_pl,
                     const std::string& name_a)
{
    names[size_t(Item_ref_type::plain)]   = name;
    names[size_t(Item_ref_type::plural)]  = name_pl;
    names[size_t(Item_ref_type::a)]       = name_a;
}

Item_name::Item_name()
{
    for (size_t i = 0; i < size_t(Item_ref_type::END); i++)
    {
        names[i] = "";
    }
}

//============ Item_att_msgs ===================================
Item_att_msgs::Item_att_msgs() :
    player(""),
    other("") {}

Item_att_msgs::Item_att_msgs(const std::string& player_, const std::string& other_) :
    player(player_),
    other(other_) {}

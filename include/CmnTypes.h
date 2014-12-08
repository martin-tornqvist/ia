#ifndef CMN_TYPES_H
#define CMN_TYPES_H

#include <algorithm>
#include <string>

#include "Colors.h"
#include "Art.h"

enum class InvList        {slots, general};

enum class ItemRefType    {plain, a, plural, END};
enum class ItemRefInf     {none, yes};
enum class ItemRefAttInf  {none, wpnContext, melee, ranged, thrown};

enum class Article        {a, the};

enum class Matl
{
  empty,
  stone,
  metal,
  plant,  //Grass, bushes, reeds, vines, fungi
  wood,   //Trees, doors, benches
  cloth,  //Carpet, silk (cocoons)
  fluid
};

enum class Condition {breaking, shoddy, fine};

enum class DmgType
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

enum class DmgMethod
{
  piercing,
  slashing,
  bluntMedium,
  bluntHeavy,
  kick,
  explosion,
  shotgun,
  elemental,
  forced, //Guaranteed to detroy the feature (silently - no messages)
  END
};

enum class ShockLvl
{
  none   = 0,
  mild   = 2,
  some   = 4,
  heavy  = 12
};

enum class GameEntryMode {newGame, loadGame};

struct CellRenderData
{
  CellRenderData() :
    clr(clrBlack),
    clrBg(clrBlack),
    tile(TileId::empty),
    glyph(' '),
    lifebarLength(-1),
    isLightFadeAllowed(true),
    isMarkedLit(false),
    isLivingActorSeenHere(false),
    isAwareOfMonHere(false) {}

  Clr     clr;
  Clr     clrBg;
  TileId  tile;
  char    glyph;
  int     lifebarLength;
  bool    isLightFadeAllowed, isMarkedLit, isLivingActorSeenHere, isAwareOfMonHere;
};


struct StrAndClr
{
  StrAndClr() : str(""), clr(clrBlack) {}

  StrAndClr(const std::string& text, const Clr& color) :
    str(text), clr(color) {}

  StrAndClr& operator=(const StrAndClr& other)
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

  Pos getSigns() const
  {
    return Pos(x == 0 ? 0 : x > 0 ? 1 : -1,
               y == 0 ? 0 : y > 0 ? 1 : -1);
  }

  void set(const int x_, const int y_)  {x = x_;  y = y_;}
  void set(const Pos& p)                {x = p.x; y = p.y;}

  void swap(Pos& p) {Pos pTemp(p); p = *this; set(pTemp);}

  int x, y;
};

struct Rect
{
  Rect() : p0(Pos()), p1(Pos()) {}

  Rect(const Pos& p0_, const Pos& p1_) :  p0(p0_), p1(p1_) {}

  Rect(const int X0, const int Y0, const int X1, const int Y1) :
    p0(Pos(X0, Y0)), p1(Pos(X1, Y1)) {}

  Rect(const Rect& r) : p0(r.p0), p1(r.p1) {}

  int getW()          const {return p1.x - p0.x + 1;}
  int getH()          const {return p1.y - p0.y + 1;}
  Pos getDims()       const {return {getW(), getH()};}
  int getMinDim()     const {return std::min(getW(), getH());}
  int getMaxDim()     const {return std::max(getW(), getH());}
  Pos getCenterPos()  const {return ((p1 + p0) / 2);}

  Pos p0;
  Pos p1;
};

struct PosAndVal
{
  PosAndVal() : pos(Pos()), val(-1) {}

  PosAndVal(const Pos& pos_, const int val_) : pos(pos_), val(val_) {}

  PosAndVal(const PosAndVal& other) : pos(other.pos), val(other.val) {}

  Pos pos;
  int val;
};

struct DiceParam
{
  DiceParam() : rolls(0), sides(0), plus(0) {}

  DiceParam(const int ROLLS, const int SIDES, const int PLUS = 0) :
    rolls(ROLLS), sides(SIDES), plus(PLUS) {}

  DiceParam(const DiceParam& other) :
    rolls(other.rolls), sides(other.sides), plus(other.plus) {}

  DiceParam& operator=(const DiceParam& other)
  {
    rolls = other.rolls;
    sides = other.sides;
    plus  = other.plus;
    return *this;
  }

  int getHighest() const {return (rolls * sides) + plus;}

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

struct ItemName
{
  ItemName(const std::string& NAME, const std::string& NAME_PL,
           const std::string& NAME_A)
  {
    names[int(ItemRefType::plain)]   = NAME;
    names[int(ItemRefType::plural)]  = NAME_PL;
    names[int(ItemRefType::a)]       = NAME_A;
  }

  ItemName()
  {
    for (int i = 0; i < int(ItemRefType::END); i++)
    {
      names[i] = "";
    }
  }

  std::string names[int(ItemRefType::END)];
};

struct ItemAttMsgs
{
public:
  ItemAttMsgs() : player(""), other("") {}

  ItemAttMsgs(const std::string& player_, const std::string& other_) :
    player(player_), other(other_) {}

  std::string player, other;
};

enum SpawnRate
{
  spawnNever, spawnExtremelyRare, spawnVeryRare,
  spawnRare, spawnCommon, spawnVeryCommon
};

enum class Dir
{
  downLeft   = 1,
  down       = 2,
  downRight  = 3,
  left       = 4,
  center     = 5,
  right      = 6,
  upLeft     = 7,
  up         = 8,
  upRight    = 9,
  END
};

enum class ActorState {alive, corpse, destroyed};

enum HorizontalVertical {hor, ver};

#endif

#ifndef CMN_TYPES_H
#define CMN_TYPES_H

#include <string>

#include "Colors.h"
#include "Art.h"

enum class Condition {breaking, shoddy, fine};

enum class DmgType {
  physical,
  fire,
  cold,
  acid,
  electric,
  spirit,
  light,
  pure,
  endOfDmgTypes
};

enum class ShockValue {
  shockValue_none   = 0,
  shockValue_mild   = 2,
  shockValue_some   = 4,
  shockValue_heavy  = 12
};

enum class MarkerTask  {
  look, aimRangedWeapon, aimThrownWeapon, aimLitExplosive
};

enum class GameEntryMode {newGame, loadGame};

struct CellRenderData {
  CellRenderData() {clear();}

  inline void clear() {
    clr                   = clrBlack;
    clrBg                 = clrBlack;
    tile                  = TileId::empty;
    glyph                 = ' ';
    lifebarLength         = -1;
    isFadeEffectAllowed   = true;
    isMarkedAsLit         = false;
    isLivingActorSeenHere = false;
    isAwareOfMonsterHere  = false;
  }

  SDL_Color clr;
  SDL_Color clrBg;
  TileId tile;
  char glyph;
  int lifebarLength;
  bool isFadeEffectAllowed;
  bool isMarkedAsLit;
  bool isLivingActorSeenHere;
  bool isAwareOfMonsterHere;
};


struct StrAndClr {
  StrAndClr() : str(""), clr(clrBlack) {}

  StrAndClr(const std::string& text, const SDL_Color& color) :
    str(text), clr(color) {}

  StrAndClr& operator=(const StrAndClr& other) {
    str = other.str;
    clr = other.clr;
    return *this;
  }

  std::string str;
  SDL_Color clr;
};

struct Pos {
  Pos() : x(0), y(0) {}
  Pos(const int x_, const int y_) : x(x_), y(y_) {}
  Pos(const Pos& p) : x(p.x), y(p.y) {}

  Pos& operator/=(const int div)  {x /= div; y /= div; return *this;}
  Pos& operator+=(const Pos& o)   {x += o.x; y += o.y; return *this;}
  Pos& operator-=(const Pos& o)   {x -= o.x; y -= o.y; return *this;}

  Pos operator+(const Pos& p)     const {return Pos(x + p.x, y + p.y);}
  Pos operator-(const Pos& p)     const {return Pos(x - p.x, y - p.y);}
  Pos operator/(const int DIV)    const {return Pos(x / DIV, y / DIV);}
  Pos operator*(const int FACTOR) const {return Pos(x * FACTOR, y * FACTOR);}
  Pos operator*(const Pos& p)     const {return Pos(x * p.x, y * p.y);}
  bool operator==(const Pos& p)   const {return x == p.x && y == p.y;}
  bool operator!=(const Pos& p)   const {return x != p.x || y != p.y;}

  Pos getSigns() const {
    return Pos(x == 0 ? 0 : x > 0 ? 1 : -1,
               y == 0 ? 0 : y > 0 ? 1 : -1);
  }

  void set(const int x_, const int y_)  {x = x_; y = y_;}
  void set(const Pos& p)                {x = p.x; y = p.y;}

  void swap(Pos& p) {Pos pTemp(p); p.set(*this); set(pTemp);}

  int x, y;
};

struct Rect {
  Rect() : x0y0(Pos()), x1y1(Pos()) {}

  Rect(const Pos& x0y0_, const Pos& x1y1_) :  x0y0(x0y0_), x1y1(x1y1_) {}

  Rect(const int X0, const int Y0, const int X1, const int Y1) :
    x0y0(Pos(X0, Y0)), x1y1(Pos(X1, Y1)) {}

  Rect(const Rect& r) : x0y0(r.x0y0), x1y1(r.x1y1) {}

  Pos x0y0;
  Pos x1y1;
};

struct PosAndVal {
  PosAndVal() : pos(Pos()), val(-1) {}

  PosAndVal(const Pos& pos_, const int val_) : pos(pos_), val(val_) {}

  PosAndVal(const PosAndVal& other) : pos(other.pos), val(other.val) {}

  Pos pos;
  int val;
};

struct DiceParam {
public:
  DiceParam() : rolls(1), sides(100), plus(0) {}

  DiceParam(const int ROLLS, const int SIDES, const int PLUS = 0) :
    rolls(ROLLS), sides(SIDES), plus(PLUS) {}

  DiceParam(const DiceParam& other) :
    rolls(other.rolls), sides(other.sides), plus(other.plus) {}

  DiceParam& operator=(const DiceParam& other) {
    rolls = other.rolls;
    sides = other.sides;
    plus  = other.plus;
    return *this;
  }

  inline int getHighest() const {return (rolls * sides) + plus;}

  int rolls, sides, plus;
};

struct Range {
  Range() : lower(-1), upper(-1) {}

  Range(const int LOWER, const int UPPER) :
    lower(LOWER), upper(UPPER) {}

  int lower, upper;
};

struct ItemName {
public:
  ItemName() : name(""), name_plural(""), name_a("") {}

  ItemName(const std::string& NAME, const std::string& NAME_PL,
           const std::string& NAME_A) :
    name(NAME), name_plural(NAME_PL), name_a(NAME_A) {}

  std::string name, name_plural, name_a;
};

struct ItemAttMsgs {
public:
  ItemAttMsgs() : player(""), other("") {}

  ItemAttMsgs(const std::string& PLAYER, const std::string& OTHER) :
    player(PLAYER), other(OTHER) {}

  std::string player, other;
};

enum SpawnRate {
  spawnNever, spawnExtremelyRare, spawnVeryRare,
  spawnRare, spawnCommon, spawnVeryCommon
};

enum class Dir {
  downLeft   = 1,
  down       = 2,
  downRight  = 3,
  left       = 4,
  center     = 5,
  right      = 6,
  upLeft     = 7,
  up         = 8,
  upRight    = 9,
  endOfDirs
};

enum class ActorDeadState {alive, corpse, destroyed};

enum EntityType {
  entityActor,
  entityItem,
  entityFeatureStatic,
  entityFeatureMob
};

#endif

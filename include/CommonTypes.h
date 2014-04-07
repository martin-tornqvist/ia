#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

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
    tile                  = tile_empty;
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
    str(text), clr(color) {
  }

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
  Pos(const Pos& other) : x(other.x), y(other.y) {}

  Pos& operator/=(const int div) {
    x /= div;
    y /= div;
    return *this;
  }
  Pos& operator+=(const Pos& offset) {
    x += offset.x;
    y += offset.y;
    return *this;
  }
  Pos& operator-=(const Pos& offset) {
    x -= offset.x;
    y -= offset.y;
    return *this;
  }
  Pos operator+(const Pos& other) const {
    return Pos(x + other.x, y + other.y);
  }
  Pos operator-(const Pos& other) const {
    return Pos(x - other.x, y - other.y);
  }
  Pos operator/(const int DIV) const {
    return Pos(x / DIV, y / DIV);
  }
  Pos operator*(const int FACTOR) const {
    return Pos(x * FACTOR, y * FACTOR);
  }
  Pos operator*(const Pos& other) const {
    return Pos(x * other.x, y * other.y);
  }

  bool operator==(const Pos& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Pos& other) const {
    return x != other.x || y != other.y;
  }

  Pos getSigns() const {
    return Pos(x == 0 ? 0 : x > 0 ? 1 : -1,
               y == 0 ? 0 : y > 0 ? 1 : -1);
  }

  void set(const int x_, const int y_) {
    x = x_;
    y = y_;
  }
  void set(const Pos& other) {
    x = other.x;
    y = other.y;
  }

  void swap(Pos& other) {
    Pos otherTemp(other);
    other.set(*this);
    set(otherTemp);
  }

  int x, y;
};

struct Rect {
  Rect() : x0y0(Pos()), x1y1(Pos()) {}
  Rect(const Pos& x0y0_, const Pos& x1y1_) :  x0y0(x0y0_), x1y1(x1y1_) {}
  Rect(const int X0, const int Y0, const int X1, const int Y1) :
    x0y0(Pos(X0, Y0)), x1y1(Pos(X1, Y1)) {}
  Rect(const Rect& other) : x0y0(other.x0y0), x1y1(other.x1y1) {}

  Pos x0y0;
  Pos x1y1;
};

struct PosAndVal {
  PosAndVal() :
    pos(Pos()), val(-1) {
  }
  PosAndVal(const Pos& pos_, const int val_) :
    pos(pos_), val(val_) {
  }
  PosAndVal(const PosAndVal& other) :
    pos(other.pos), val(other.val) {
  }

  Pos pos;
  int val;
};

struct PosAndVal_compareForVal {
public:
  PosAndVal_compareForVal() {}
  bool operator()(const PosAndVal& p1, const PosAndVal& p2) {
    return p1.val < p2.val;
  }
};

struct DiceParam {
public:
  DiceParam() : rolls(1), sides(100), plus(0) {}

  DiceParam(const int ROLLS, const int SIDES, const int PLUS = 0) :
    rolls(ROLLS), sides(SIDES), plus(PLUS) {
  }

  DiceParam(const DiceParam& other) :
    rolls(other.rolls), sides(other.sides), plus(other.plus) {
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

struct ItemAttackMessages {
public:
  ItemAttackMessages() : player(""), other("") {}

  ItemAttackMessages(const std::string& PLAYER, const std::string& OTHER) :
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

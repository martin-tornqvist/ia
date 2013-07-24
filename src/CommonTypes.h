#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <string>
#include <stdlib.h>

#include "Colors.h"
#include "Art.h"

using namespace std;

enum Font_t {font_interface, font_big};

enum MoveType_t {
  moveType_walk, moveType_fly, moveType_ethereal, moveType_ooze, endOfMoveType
};

enum DmgTypes_t {
  dmgType_physical, dmgType_fire, dmgType_cold, dmgType_acid, dmgType_electric,
  dmgType_spirit, dmgType_light, dmgType_pure, endOfDmgTypes
};

enum ShockValues_t {
  shockValue_none, shockValue_mild, shockValue_some, shockValue_heavy
};

enum MarkerTask_t {
  markerTask_aimRangedWeapon, markerTask_look,
  markerTask_aimThrownWeapon, markerTask_aimLitExplosive
};

enum GameEntry_t {gameEntry_new, gameEntry_load};

struct CellRenderDataAscii {
  CellRenderDataAscii() :
    color(clrBlack), colorBg(clrBlack), glyph(' '),
    lifebarLength(-1), isFadeEffectAllowed(true), isMarkedAsLit(false) {
  }
  void clear() {
    color = clrBlack;
    colorBg = clrBlack;
    glyph = ' ';
    lifebarLength = -1;
    isFadeEffectAllowed = true;
    isMarkedAsLit = false;
  }
  SDL_Color color;
  SDL_Color colorBg;
  char glyph;
  int lifebarLength;
  bool isFadeEffectAllowed, isMarkedAsLit;
};

struct CellRenderDataTiles {
  CellRenderDataTiles() :
    color(clrBlack), colorBg(clrBlack), tile(tile_empty), lifebarLength(-1),
    isFadeEffectAllowed(true), isLivingActorSeenHere(false),
    isMarkedAsLit(false) {
  }
  void clear() {
    color = clrBlack;
    colorBg = clrBlack;
    tile = tile_empty;
    lifebarLength = -1;
    isFadeEffectAllowed = true;
    isLivingActorSeenHere = isMarkedAsLit = false;
  }
  SDL_Color color;
  SDL_Color colorBg;
  Tile_t tile;
  int lifebarLength;
  bool isFadeEffectAllowed, isLivingActorSeenHere, isMarkedAsLit;
};

struct StringAndClr {
  StringAndClr() : str(""), clr(clrBlack) {}

  StringAndClr(const string& text, const SDL_Color& color) :
    str(text), clr(color) {
  }

  StringAndClr& operator=(const StringAndClr& other) {
    str = other.str;
    clr = other.clr;
    return *this;
  }

  string str;
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
    return Pos(x == 0 ? 0 : x / abs(x), y == 0 ? 0 : y / abs(y));
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

class DiceParam {
public:
  DiceParam() :
    rolls(1), sides(100), plus(0) {
  }
  DiceParam(const int ROLLS, const int SIDES, const int PLUS = 0) :
    rolls(ROLLS), sides(SIDES), plus(PLUS) {
  }
  DiceParam(const DiceParam& other) :
    rolls(other.rolls), sides(other.sides), plus(other.plus) {
  }

  int rolls, sides, plus;
};

class ItemName {
public:
  ItemName() :
    name(""), name_plural(""), name_a("") {
  }
  ItemName(const string& NAME, const string& NAME_PLURAL, const string& NAME_A) :
    name(NAME), name_plural(NAME_PLURAL), name_a(NAME_A) {
  }

  string name, name_plural, name_a;
};

class ItemAttackMessages {
public:
  ItemAttackMessages() :
    player(""), other("") {
  }
  ItemAttackMessages(const string& PLAYER, const string& OTHER) :
    player(PLAYER), other(OTHER) {
  }

  string player, other;
};

enum SpawnRate_t {
  spawnNever, spawnExtremelyRare, spawnVeryRare,
  spawnRare, spawnCommon, spawnVeryCommon
};

struct DirectionNames {
  DirectionNames() {
    directions[0][0] = "NW";
    directions[0][1] = "W";
    directions[0][2] = "SW";
    directions[1][0] = "N";
    directions[1][1] = "";
    directions[1][2] = "S";
    directions[2][0] = "NE";
    directions[2][1] = "E";
    directions[2][2] = "SE";
  }

  string directions[3][3];
};

enum EntityStrength_t {
  weak, normal, strong, superStrong
};

class EntityStrength {
public:
  static double getFactor(EntityStrength_t const strength) {
    if(strength == weak)        return 0.7;
    if(strength == strong)      return 1.6;
    if(strength == superStrong) return 2.5;
    return 1.0;
  }
protected:
  EntityStrength() {}
};

enum Directions_t {
  direction_down = 2, direction_left = 4, direction_right = 6, direction_up = 8
};

enum ActorDeadState_t {
  actorDeadState_alive = 0, actorDeadState_corpse = 1, actorDeadState_mangled = 2
};

#endif

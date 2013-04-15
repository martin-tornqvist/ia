#ifndef CONST_TYPES_H
#define CONST_TYPES_H

#include <string>
#include <stdlib.h>

#include "Colors.h"
#include "Art.h"

using namespace std;

enum MoveType_t {
  moveType_walk, moveType_fly, moveType_ethereal, moveType_ooze, endOfMoveType
};

enum DamageTypes_t {
  damageType_physical,
  damageType_fire,
  damageType_cold,
  damageType_acid,
  damageType_electric,
  damageType_spirit,
  damageType_light,
  damageType_pure,
  endOfDamageTypes
};

enum ShockValues_t
{
  shockValue_none,
  shockValue_mild,
  shockValue_some,
  shockValue_heavy
};

enum GameEntry_t {
  gameEntry_new, gameEntry_load
};

struct CellRenderDataAscii {
  CellRenderDataAscii() :
    color(clrBlack), colorBg(clrBlack), drawBgColor(false), glyph(' '),
    lifebarLength(-1), isFadeEffectAllowed(true) {
  }
  void clear() {
    color = clrBlack;
    colorBg = clrBlack;
    drawBgColor = false;
    glyph = ' ';
    lifebarLength = -1;
    isFadeEffectAllowed = true;
  }
  sf::Color color;
  sf::Color colorBg;
  bool drawBgColor;
  char glyph;
  int lifebarLength;
  bool isFadeEffectAllowed;
};

struct CellRenderDataTile {
  CellRenderDataTile() :
    color(clrBlack), colorBg(clrBlack), drawBgColor(false), tile(tile_empty),
    lifebarLength(-1), isFadeEffectAllowed(true), isLivingActorSeenHere(false) {
  }
  void clear() {
    color = clrBlack;
    colorBg = clrBlack;
    drawBgColor = false;
    tile = tile_empty;
    lifebarLength = -1;
    isFadeEffectAllowed = true;
    isLivingActorSeenHere = false;
  }
  sf::Color color;
  sf::Color colorBg;
  bool drawBgColor;
  Tile_t tile;
  int lifebarLength;
  bool isFadeEffectAllowed;
  bool isLivingActorSeenHere;
};

struct StringAndColor {
  StringAndColor() :
    str(""), color(clrBlack) {
  }

  StringAndColor(const string& text, const sf::Color& clr) :
    str(text), color(clr) {
  }

  StringAndColor& operator=(const StringAndColor& other) {
    str = other.str;
    color = other.color;
    return *this;
  }

  string str;
  sf::Color color;
};

struct coord {
  coord() :
    x(0), y(0) {
  }
  coord(const int x_, const int y_) :
    x(x_), y(y_) {
  }
  coord(const coord& other) :
    x(other.x), y(other.y) {
  }

  coord& operator/=(const int div) {
    x /= div;
    y /= div;
    return *this;
  }
  coord& operator+=(const coord& offset) {
    x += offset.x;
    y += offset.y;
    return *this;
  }
  coord& operator-=(const coord& offset) {
    x -= offset.x;
    y -= offset.y;
    return *this;
  }
  coord operator+(const coord& other) const {
    return coord(x + other.x, y + other.y);
  }
  coord operator-(const coord& other) const {
    return coord(x - other.x, y - other.y);
  }
  coord operator/(const int div) const {
    return coord(x / div, y / div);
  }
  coord operator*(const int factor) const {
    return coord(x * factor, y * factor);
  }

  bool operator==(const coord& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const coord& other) const {
    return x != other.x || y != other.y;
  }

  coord getSigns() const {
    return coord(x == 0 ? 0 : x / abs(x), y == 0 ? 0 : y / abs(y));
  }

  void set(const int x_, const int y_) {
    x = x_;
    y = y_;
  }
  void set(const coord& other) {
    x = other.x;
    y = other.y;
  }

  void swap(coord& other) {
    coord otherTemp(other);
    other.set(*this);
    set(otherTemp);
  }

  int x, y;
};

struct Rect {
  Rect() :
    x0y0(coord()), x1y1(coord()) {
  }
  Rect(const coord& x0y0_, const coord& x1y1_) :
    x0y0(x0y0_), x1y1(x1y1_) {
  }
  Rect(const Rect& other) :
    x0y0(other.x0y0), x1y1(other.x1y1) {
  }

  coord x0y0;
  coord x1y1;
};

struct PosAndVal {
  PosAndVal() :
    pos(coord()), val(-1) {
  }
  PosAndVal(const coord& pos_, const int val_) :
    pos(pos_), val(val_) {
  }
  PosAndVal(const PosAndVal& other) :
    pos(other.pos), val(other.val) {
  }

  coord pos;
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
  spawnNever, spawnExtremelyRare, spawnVeryRare, spawnRare, spawnCommon, spawnVeryCommon
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

//enum InventoryPurpose_t {
//  inventoryPurpose_look,
//  inventoryPurpose_wieldWear,
//  inventoryPurpose_wieldAlt,
//  inventoryPurpose_missileSelect,
//  inventoryPurpose_use,
////  inventoryPurpose_read,
//  inventoryPurpose_quaff,
//  inventoryPurpose_eat,
//  inventoryPurpose_selectDrop,
//  inventoryPurpose_readyExplosive
//};

//enum GeneralInventoryFilters_t {
//  generalInventoryFilter_wieldable,
//  generalInventoryFilter_wieldableAndWearable,
//  generalInventoryFilter_missile,
//  generalInventoryFilter_usable,
////  generalInventoryFilter_readable,
//  generalInventoryFilter_quaffable,
//  generalInventoryFilter_eatable,
//  generalInventoryFilter_explosives
//};

enum EntityStrength_t {
  weak, normal, strong, superStrong
};

class EntityStrength {
public:
  static double getFactor(EntityStrength_t const strength) {
    if(strength == weak)
      return 0.7;
    if(strength == strong)
      return 1.6;
    if(strength == superStrong)
      return 2.5;
    return 1.0;
  }

protected:
  EntityStrength() {
  }
};

enum Directions_t {
  direction_down = 2, direction_left = 4, direction_right = 6, direction_up = 8
};

enum ActorDeadState_t {
  actorDeadState_alive = 0, actorDeadState_corpse = 1, actorDeadState_mangled = 2
};

#endif

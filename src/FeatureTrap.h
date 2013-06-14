#ifndef FEATURE_TRAPS_H
#define FEATURE_TRAPS_H

#include "Feature.h"
#include "AbilityValues.h"
#include "Art.h"
#include "Feature.h"
#include "CommonTypes.h"

class Engine;
class SpecificTrapBase;
class TrapSpawnData;

enum Trap_t {
  trap_blinding,
  trap_dart,
  trap_gasConfusion,
  trap_gasFear,
  trap_gasParalyze,
  trap_smoke,
  trap_alarm,
  trap_spear,
  trap_spiderWeb,
  trap_teleport,
  trap_summonMonster,
  endOfTraps,
  trap_any
};

class Trap: public FeatureStatic {
public:
  ~Trap();

  void bump(Actor* actorBumping);
  SDL_Color getColor() const;
  char getGlyph() const;
  Tile_t getTile() const;
  string getDescription(const bool DEFINITE_ARTICLE) const;
  bool canHaveBlood() const;
  bool canHaveGore() const;
  bool canHaveCorpse() const;
  bool canHaveItem() const;

  bool isMagical() const;
  bool isDisarmable() const;

  void triggerOnPurpose(Actor* actorTriggering);

  void reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES);

  bool isHidden() const {return isHidden_;}

  MaterialType_t getMaterialType() const;

  coord actorTryLeave(Actor* const actor, const coord& pos, const coord& dest);

  Trap_t getTrapType() const;

  const SpecificTrapBase* getSpecificTrap() const {return specificTrap_;}

protected:
  friend class FeatureFactory;
  friend class Disarm;
  Trap(Feature_t id, coord pos, Engine* engine, TrapSpawnData* spawnData);

  void trigger(Actor* const actor);

  void setSpecificTrapFromId(const Trap_t id);

  const FeatureDef* const mimicFeature_;
  bool isHidden_;
  SpecificTrapBase* specificTrap_;

  friend class Player;
  void playerTrySpotHidden();
};

class SpecificTrapBase {
public:

protected:
  friend class Trap;
  SpecificTrapBase(coord pos, Trap_t trapType, Engine* engine) :
    pos_(pos), trapType_(trapType), eng(engine) {
  }
  SpecificTrapBase() {}
  virtual ~SpecificTrapBase() {}

  virtual coord specificTrapActorTryLeave(Actor* const actor, const coord& pos, const coord& dest) {
    (void)actor;
    (void)pos;
    return dest;
  }

  virtual void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult) = 0;
  virtual string getTrapSpecificTitle() const = 0;
  virtual SDL_Color getTrapSpecificColor() const = 0;
  virtual char getTrapSpecificGlyph() const = 0;
  virtual Tile_t getTrapSpecificTile() const = 0;
  virtual bool isMagical() const = 0;
  virtual bool isDisarmable() const = 0;

  coord pos_;
  Trap_t trapType_;
  Engine* eng;
};

class TrapDart: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapDart(coord pos, Engine* engine);
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrWhiteHigh;}
  string getTrapSpecificTitle() const {return "Dart trap";}
  char getTrapSpecificGlyph() const {return '^';}
  Tile_t getTrapSpecificTile() const {return tile_trapGeneral;}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  bool isPoisoned;
};

class TrapSpear: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapSpear(coord pos, Engine* engine);
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrWhiteHigh;}
  string getTrapSpecificTitle() const {return "Spear trap";}
  char getTrapSpecificGlyph() const {return '^';}
  Tile_t getTrapSpecificTile() const {return tile_trapGeneral;}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  bool isPoisoned;
};

class TrapGasConfusion: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapGasConfusion(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_gasConfusion, engine) {}
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrMagenta;}
  string getTrapSpecificTitle() const {return "Gas trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const {return tile_trapGeneral;}
};

class TrapGasParalyzation: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapGasParalyzation(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_gasParalyze, engine) {
  }
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrMagenta;}
  string getTrapSpecificTitle() const {return "Gas trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const {return tile_trapGeneral;}
};

class TrapGasFear: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapGasFear(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_gasFear, engine) {}
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrMagenta;}
  string getTrapSpecificTitle() const {return "Gas trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const {return tile_trapGeneral;}
};

class TrapBlindingFlash: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapBlindingFlash(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_blinding, engine) {}
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrYellow;}
  string getTrapSpecificTitle() const {return "Blinding trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const {return tile_trapGeneral;}
};

class TrapTeleport: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapTeleport(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_teleport, engine) {
  }
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrCyan;}
  string getTrapSpecificTitle() const {return "Teleporter trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return true;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const {return tile_elderSign;}
};

class TrapSummonMonster: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapSummonMonster(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_summonMonster, engine) {
  }
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrBrownDark;}
  string getTrapSpecificTitle() const {return "Monster summoning trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return true;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const {return tile_elderSign;}
};

class TrapSmoke: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapSmoke(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_smoke, engine) {}
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrGray;}
  string getTrapSpecificTitle() const {return "Smoke trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const { return tile_trapGeneral;}
};

class TrapAlarm: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapAlarm(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_alarm, engine) {}
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrBrown;}
  string getTrapSpecificTitle() const {return "Alarm trap";}
  char getTrapSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getTrapSpecificTile() const { return tile_trapGeneral;}
};


class TrapSpiderWeb: public SpecificTrapBase {
public:
  coord specificTrapActorTryLeave(Actor* const actor, const coord& pos, const coord& dest);

  bool isHolding() const {return isHoldingActor;}

private:
  friend class Trap;
  TrapSpiderWeb(coord pos, Engine* engine) :
    SpecificTrapBase(pos, trap_spiderWeb, engine), isHoldingActor(false) {}
  void trapSpecificTrigger(Actor* const actor, const AbilityRollResult_t dodgeResult);
  SDL_Color getTrapSpecificColor() const {return clrWhiteHigh;}
  string getTrapSpecificTitle() const {return "Spider web";}
  char getTrapSpecificGlyph() const {return '*';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return false;}
  Tile_t getTrapSpecificTile() const {return tile_spiderWeb;}

  bool isHoldingActor;
};

#endif

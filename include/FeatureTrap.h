#ifndef FEATURE_TRAPS_H
#define FEATURE_TRAPS_H

#include "Feature.h"
#include "AbilityValues.h"
#include "Art.h"
#include "Feature.h"
#include "CommonData.h"

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

  void bump(Actor& actorBumping);
  SDL_Color getColor() const;
  char getGlyph() const;
  Tile_t getTile() const;
  string getDescr(const bool DEFINITE_ARTICLE) const override;
  bool canHaveBlood() const;
  bool canHaveGore() const;
  bool canHaveCorpse() const;
  bool canHaveItem() const;

  bool isMagical() const;
  bool isDisarmable() const;

  void triggerOnPurpose(Actor& actorTriggering);

  void reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES);

  bool isHidden() const {return isHidden_;}

  MaterialType_t getMaterialType() const;

  Dir_t actorTryLeave(Actor& actor, const Dir_t dir);

  Trap_t getTrapType() const;

  const SpecificTrapBase* getSpecificTrap() const {return specificTrap_;}

protected:
  friend class FeatureFactory;
  friend class Disarm;
  Trap(Feature_t id, Pos pos, Engine& engine, TrapSpawnData* spawnData);

  void trigger(Actor& actor);

  void setSpecificTrapFromId(const Trap_t id);

  const FeatureData* const mimicFeature_;
  bool isHidden_;
  SpecificTrapBase* specificTrap_;

  friend class Player;
  void playerTrySpotHidden();
};

class SpecificTrapBase {
public:

protected:
  friend class Trap;
  SpecificTrapBase(Pos pos, Trap_t trapType, Engine& engine) :
    pos_(pos), trapType_(trapType), eng(engine) {}
//  SpecificTrapBase() {}
  virtual ~SpecificTrapBase() {}

  virtual Dir_t specificTrapActorTryLeave(Actor& actor, const Dir_t dir) {
    (void)actor;
    return dir;
  }

  virtual void specificTrigger(
    Actor& actor, const AbilityRollResult_t dodgeResult) = 0;
  virtual string getSpecificTitle() const = 0;
  virtual SDL_Color getSpecificColor() const = 0;
  virtual char getSpecificGlyph() const = 0;
  virtual Tile_t getSpecificTile() const = 0;
  virtual bool isMagical() const = 0;
  virtual bool isDisarmable() const = 0;

  Pos pos_;
  Trap_t trapType_;
  Engine& eng;
};

class TrapDart: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapDart(Pos pos, Engine& engine);
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrWhiteHigh;}
  string getSpecificTitle() const {return "Dart trap";}
  char getSpecificGlyph() const {return '^';}
  Tile_t getSpecificTile() const {return tile_trapGeneral;}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  bool isPoisoned;
};

class TrapSpear: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapSpear(Pos pos, Engine& engine);
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrWhiteHigh;}
  string getSpecificTitle() const {return "Spear trap";}
  char getSpecificGlyph() const {return '^';}
  Tile_t getSpecificTile() const {return tile_trapGeneral;}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  bool isPoisoned;
};

class TrapGasConfusion: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapGasConfusion(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_gasConfusion, engine) {}
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrMagenta;}
  string getSpecificTitle() const {return "Gas trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const {return tile_trapGeneral;}
};

class TrapGasParalyzation: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapGasParalyzation(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_gasParalyze, engine) {
  }
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrMagenta;}
  string getSpecificTitle() const {return "Gas trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const {return tile_trapGeneral;}
};

class TrapGasFear: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapGasFear(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_gasFear, engine) {}
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrMagenta;}
  string getSpecificTitle() const {return "Gas trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const {return tile_trapGeneral;}
};

class TrapBlindingFlash: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapBlindingFlash(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_blinding, engine) {}
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrYellow;}
  string getSpecificTitle() const {return "Blinding trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const {return tile_trapGeneral;}
};

class TrapTeleport: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapTeleport(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_teleport, engine) {
  }
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrCyan;}
  string getSpecificTitle() const {return "Teleporter trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return true;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const {return tile_elderSign;}
};

class TrapSummonMonster: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapSummonMonster(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_summonMonster, engine) {
  }
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrBrownDark;}
  string getSpecificTitle() const {return "Monster summoning trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return true;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const {return tile_elderSign;}
};

class TrapSmoke: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapSmoke(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_smoke, engine) {}
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrGray;}
  string getSpecificTitle() const {return "Smoke trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const { return tile_trapGeneral;}
};

class TrapAlarm: public SpecificTrapBase {
public:
private:
  friend class Trap;
  TrapAlarm(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_alarm, engine) {}
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrBrown;}
  string getSpecificTitle() const {return "Alarm trap";}
  char getSpecificGlyph() const {return '^';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return true;}
  Tile_t getSpecificTile() const { return tile_trapGeneral;}
};


class TrapSpiderWeb: public SpecificTrapBase {
public:
  Dir_t specificTrapActorTryLeave(Actor& actor, const Dir_t dir);

  bool isHolding() const {return isHoldingActor;}

private:
  friend class Trap;
  TrapSpiderWeb(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_spiderWeb, engine), isHoldingActor(false) {}
  void specificTrigger(Actor& actor,
                           const AbilityRollResult_t dodgeResult);
  SDL_Color getSpecificColor() const {return clrWhiteHigh;}
  string getSpecificTitle() const {return "Spider web";}
  char getSpecificGlyph() const {return '*';}
  bool isMagical() const {return false;}
  bool isDisarmable() const {return false;}
  Tile_t getSpecificTile() const {return tile_spiderWeb;}

  bool isHoldingActor;
};

#endif

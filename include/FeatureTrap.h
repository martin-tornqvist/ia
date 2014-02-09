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

enum TrapId {
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

  void  bump(Actor& actorBumping) override;
  SDL_Color getClr()                          const override;
  char getGlyph()                               const override;
  TileId getTile()                              const override;
  string getDescr(const bool DEFINITE_ARTICLE)  const override;
  void disarm()                                 override;
  bool canHaveCorpse()  const override {return isHidden_;}
  bool canHaveBlood()   const override {return isHidden_;}
  bool canHaveGore()    const override {return isHidden_;}
  bool canHaveItem()    const override {return isHidden_;}

  bool isMagical() const;

  void triggerOnPurpose(Actor& actorTriggering);

  void reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES);

  bool isHidden() const {return isHidden_;}

  MaterialType getMaterialType() const;

  Dir actorTryLeave(Actor& actor, const Dir dir);

  TrapId getTrapType() const;

  const SpecificTrapBase* getSpecificTrap() const {return specificTrap_;}

protected:
  friend class FeatureFactory;
  Trap(FeatureId id, Pos pos, Engine& engine, TrapSpawnData* spawnData);

  void triggerTrap(Actor& actor) override;

  void setSpecificTrapFromId(const TrapId id);

  const FeatureData* const mimicFeature_;
  bool isHidden_;
  SpecificTrapBase* specificTrap_;

  friend class Player;
  void playerTrySpotHidden();
};

class SpecificTrapBase {
protected:
  friend class Trap;
  SpecificTrapBase(Pos pos, TrapId trapType, Engine& engine) :
    pos_(pos), trapType_(trapType), eng(engine) {}

  virtual ~SpecificTrapBase() {}

  virtual Dir actorTryLeave(Actor& actor, const Dir dir) {
    (void)actor;
    return dir;
  }

  virtual void trigger(
    Actor& actor, const AbilityRollResult dodgeResult) = 0;
  virtual string getTitle()     const = 0;
  virtual SDL_Color getClr()  const = 0;
  virtual char getGlyph()       const = 0;
  virtual TileId getTile()      const = 0;
  virtual bool isMagical()      const = 0;

  Pos pos_;
  TrapId trapType_;
  Engine& eng;
};

class TrapDart: public SpecificTrapBase {
private:
  friend class Trap;
  TrapDart(Pos pos, Engine& engine);
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrWhiteHigh;}
  string      getTitle()    const override {return "Dart trap";}
  char        getGlyph()    const override {return '^';}
  TileId      getTile()     const override {return tile_trapGeneral;}
  bool        isMagical()   const override {return false;}
  bool        isPoisoned;
};

class TrapSpear: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSpear(Pos pos, Engine& engine);
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrWhiteHigh;}
  string      getTitle()    const override {return "Spear trap";}
  char        getGlyph()    const override {return '^';}
  TileId      getTile()     const override {return tile_trapGeneral;}
  bool        isMagical()   const override {return false;}
  bool isPoisoned;
};

class TrapGasConfusion: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasConfusion(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_gasConfusion, engine) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrMagenta;}
  string      getTitle()    const override {return "Gas trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return false;}
  TileId      getTile()     const override {return tile_trapGeneral;}
};

class TrapGasParalyzation: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasParalyzation(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_gasParalyze, engine) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrMagenta;}
  string      getTitle()    const override {return "Gas trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return false;}
  TileId      getTile()     const override {return tile_trapGeneral;}
};

class TrapGasFear: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasFear(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_gasFear, engine) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrMagenta;}
  string      getTitle()    const override {return "Gas trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return false;}
  TileId      getTile()     const override {return tile_trapGeneral;}
};

class TrapBlindingFlash: public SpecificTrapBase {
private:
  friend class Trap;
  TrapBlindingFlash(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_blinding, engine) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrYellow;}
  string      getTitle()    const override {return "Blinding trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return false;}
  TileId      getTile()     const override {return tile_trapGeneral;}
};

class TrapTeleport: public SpecificTrapBase {
private:
  friend class Trap;
  TrapTeleport(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_teleport, engine) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrCyan;}
  string      getTitle()    const override {return "Teleporter trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return true;}
  TileId      getTile()     const override {return tile_elderSign;}
};

class TrapSummonMonster: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSummonMonster(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_summonMonster, engine) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrBrownDrk;}
  string      getTitle()    const override {return "Monster summoning trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return true;}
  TileId      getTile()     const override {return tile_elderSign;}
};

class TrapSmoke: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSmoke(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_smoke, engine) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrGray;}
  string      getTitle()    const override {return "Smoke trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return false;}
  TileId      getTile()     const override { return tile_trapGeneral;}
};

class TrapAlarm: public SpecificTrapBase {
private:
  friend class Trap;
  TrapAlarm(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_alarm, engine) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrBrown;}
  string      getTitle()    const override {return "Alarm trap";}
  char        getGlyph()    const override {return '^';}
  bool        isMagical()   const override {return false;}
  TileId      getTile()     const override { return tile_trapGeneral;}
};


class TrapSpiderWeb: public SpecificTrapBase {
public:
  Dir actorTryLeave(Actor& actor, const Dir dir);

  bool isHolding() const {return isHoldingActor;}

private:
  friend class Trap;
  TrapSpiderWeb(Pos pos, Engine& engine) :
    SpecificTrapBase(pos, trap_spiderWeb, engine), isHoldingActor(false) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()    const override {return clrWhiteHigh;}
  string      getTitle()    const override {return "Spider web";}
  char        getGlyph()    const override {return '*';}
  bool        isMagical()   const override {return false;}
  TileId      getTile()     const override {return tile_spiderWeb;}

  bool isHoldingActor;
};

#endif

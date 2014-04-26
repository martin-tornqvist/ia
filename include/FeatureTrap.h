#ifndef FEATURE_TRAPS_H
#define FEATURE_TRAPS_H

#include "Feature.h"
#include "AbilityValues.h"
#include "Art.h"
#include "Feature.h"
#include "CmnData.h"

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
  SDL_Color getClr()                            const override;
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
  Trap(FeatureId id, Pos pos, TrapSpawnData* spawnData);

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
  SpecificTrapBase(Pos pos, TrapId trapType) :
    pos_(pos), trapType_(trapType) {}

  virtual ~SpecificTrapBase() {}

  virtual Dir actorTryLeave(Actor& actor, const Dir dir) {
    (void)actor;
    return dir;
  }

  virtual void trigger(
    Actor& actor, const AbilityRollResult dodgeResult) = 0;
  virtual string getTitle()     const = 0;
  virtual SDL_Color getClr()    const = 0;
  virtual char getGlyph()       const = 0;
  virtual TileId getTile()      const = 0;
  virtual bool isMagical()      const = 0;
  virtual bool isDisarmable()   const = 0;
  virtual string getDisarmMsg() const {
    return isMagical() ? "I dispel a magic trap." : "I disarm a trap.";
  }
  virtual string getDisarmFailMsg() const {
    return isMagical() ?
           "I fail to dispel a magic trap." :
           "I fail to disarm a trap.";
  }

  Pos pos_;
  TrapId trapType_;
};

class TrapDart: public SpecificTrapBase {
private:
  friend class Trap;
  TrapDart(Pos pos);
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrWhiteHigh;}
  string      getTitle()      const override {return "Dart trap";}
  TileId      getTile()       const override {return tile_trapGeneral;}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  bool        isDisarmable()  const override {return true;}

  bool        isPoisoned;
};

class TrapSpear: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSpear(Pos pos);
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrWhiteHigh;}
  string      getTitle()      const override {return "Spear trap";}
  TileId      getTile()       const override {return tile_trapGeneral;}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  bool        isDisarmable()  const override {return true;}

  bool isPoisoned;
};

class TrapGasConfusion: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasConfusion(Pos pos) :
    SpecificTrapBase(pos, trap_gasConfusion) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrMagenta;}
  string      getTitle()      const override {return "Gas trap";}
  TileId      getTile()       const override {return tile_trapGeneral;}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  bool        isDisarmable()  const override {return true;}
};

class TrapGasParalyzation: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasParalyzation(Pos pos) :
    SpecificTrapBase(pos, trap_gasParalyze) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrMagenta;}
  string      getTitle()      const override {return "Gas trap";}
  TileId      getTile()       const override {return tile_trapGeneral;}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  bool        isDisarmable()  const override {return true;}
};

class TrapGasFear: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasFear(Pos pos) :
    SpecificTrapBase(pos, trap_gasFear) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrMagenta;}
  string      getTitle()      const override {return "Gas trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override {return tile_trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};

class TrapBlindingFlash: public SpecificTrapBase {
private:
  friend class Trap;
  TrapBlindingFlash(Pos pos) :
    SpecificTrapBase(pos, trap_blinding) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrYellow;}
  string      getTitle()      const override {return "Blinding trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override {return tile_trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};

class TrapTeleport: public SpecificTrapBase {
private:
  friend class Trap;
  TrapTeleport(Pos pos) :
    SpecificTrapBase(pos, trap_teleport) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrCyan;}
  string      getTitle()      const override {return "Teleporter trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return true;}
  TileId      getTile()       const override {return tile_elderSign;}
  bool        isDisarmable()  const override {return true;}
};

class TrapSummonMonster: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSummonMonster(Pos pos) :
    SpecificTrapBase(pos, trap_summonMonster) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrBrownDrk;}
  string      getTitle()      const override {return "Monster summoning trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return true;}
  TileId      getTile()       const override {return tile_elderSign;}
  bool        isDisarmable()  const override {return true;}
};

class TrapSmoke: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSmoke(Pos pos) :
    SpecificTrapBase(pos, trap_smoke) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrGray;}
  string      getTitle()      const override {return "Smoke trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override { return tile_trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};

class TrapAlarm: public SpecificTrapBase {
private:
  friend class Trap;
  TrapAlarm(Pos pos) :
    SpecificTrapBase(pos, trap_alarm) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrBrown;}
  string      getTitle()      const override {return "Alarm trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override { return tile_trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};


class TrapSpiderWeb: public SpecificTrapBase {
public:
  Dir actorTryLeave(Actor& actor, const Dir dir);

  bool isHolding() const {return isHoldingActor;}

private:
  friend class Trap;
  TrapSpiderWeb(Pos pos) :
    SpecificTrapBase(pos, trap_spiderWeb), isHoldingActor(false) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  SDL_Color   getClr()        const override {return clrWhiteHigh;}
  string      getTitle()      const override {return "Spider web";}
  char        getGlyph()      const override {return '*';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override {return tile_spiderWeb;}
  bool        isDisarmable()  const override {return true;}
  string      getDisarmMsg()  const override {
    return "I tear down a spider web.";
  }
  string      getDisarmFailMsg() const override {
    return "I fail to tear down a spider web.";
  }

  bool isHoldingActor;
};

#endif

#ifndef FEATURE_TRAPS_H
#define FEATURE_TRAPS_H

#include "Feature.h"
#include "AbilityValues.h"
#include "Art.h"
#include "FeatureStatic.h"
#include "CmnData.h"

class SpecificTrapBase;

enum class TrapId {
  blinding,
  dart,
  gasConfusion,
  gasFear,
  gasParalyze,
  smoke,
  alarm,
  spear,
  spiderWeb,
  teleport,
  summonMonster,
  END,
  any
};

class Trap: public FeatureStatic {
public:
  Trap(const Pos& pos, const FeatureDataT& mimicFeature, TrapId type);

  //Spawn by id compliant ctor (do not use for normal cases):
  Trap(const Pos& pos) : FeatureStatic(pos), mimicFeature_(nullptr), isHidden_(false) {}

  Trap() = delete;

  ~Trap();

  FeatureId getId() const override {return FeatureId::trap;}

  void        bump(Actor& actorBumping)                   override;
  Clr   getClr()                              const override;
  char        getGlyph()                            const override;
  TileId      getTile()                             const override;
  std::string getName(const bool DEFINITE_ARTICLE) const override;
  void        disarm()                                    override;
  bool        canHaveCorpse()                       const override {return isHidden_;}
  bool        canHaveBlood()                        const override {return isHidden_;}
  bool        canHaveGore()                         const override {return isHidden_;}
  bool        canHaveItem()                         const override {return isHidden_;}

  bool isMagical() const;

  void triggerOnPurpose(Actor& actorTriggering);

  void reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES);

  bool isHidden() const {return isHidden_;}

  Matl getMatl() const;

  Dir actorTryLeave(Actor& actor, const Dir dir);

  TrapId getTrapType() const;

  const SpecificTrapBase* getSpecificTrap() const {return specificTrap_;}

  void  playerTrySpotHidden();

protected:
  void triggerTrap(Actor& actor) override;

  void setSpecificTrapFromId(const TrapId id);

  const FeatureDataT* mimicFeature_;
  bool isHidden_;
  SpecificTrapBase* specificTrap_;
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
  virtual std::string getTitle()  const = 0;
  virtual Clr getClr()      const = 0;
  virtual char getGlyph()         const = 0;
  virtual TileId getTile()        const = 0;
  virtual bool isMagical()        const = 0;
  virtual bool isDisarmable()     const = 0;
  virtual std::string getDisarmMsg() const {
    return isMagical() ? "I dispel a magic trap." : "I disarm a trap.";
  }
  virtual std::string getDisarmFailMsg() const {
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
  Clr   getClr()        const override {return clrWhiteHigh;}
  std::string      getTitle()      const override {return "Dart trap";}
  TileId      getTile()       const override {return TileId::trapGeneral;}
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
  Clr   getClr()        const override {return clrWhiteHigh;}
  std::string      getTitle()      const override {return "Spear trap";}
  TileId      getTile()       const override {return TileId::trapGeneral;}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  bool        isDisarmable()  const override {return true;}

  bool isPoisoned;
};

class TrapGasConfusion: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasConfusion(Pos pos) :
    SpecificTrapBase(pos, TrapId::gasConfusion) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrMagenta;}
  std::string      getTitle()      const override {return "Gas trap";}
  TileId      getTile()       const override {return TileId::trapGeneral;}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  bool        isDisarmable()  const override {return true;}
};

class TrapGasParalyzation: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasParalyzation(Pos pos) :
    SpecificTrapBase(pos, TrapId::gasParalyze) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrMagenta;}
  std::string      getTitle()      const override {return "Gas trap";}
  TileId      getTile()       const override {return TileId::trapGeneral;}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  bool        isDisarmable()  const override {return true;}
};

class TrapGasFear: public SpecificTrapBase {
private:
  friend class Trap;
  TrapGasFear(Pos pos) :
    SpecificTrapBase(pos, TrapId::gasFear) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrMagenta;}
  std::string      getTitle()      const override {return "Gas trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override {return TileId::trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};

class TrapBlindingFlash: public SpecificTrapBase {
private:
  friend class Trap;
  TrapBlindingFlash(Pos pos) :
    SpecificTrapBase(pos, TrapId::blinding) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrYellow;}
  std::string      getTitle()      const override {return "Blinding trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override {return TileId::trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};

class TrapTeleport: public SpecificTrapBase {
private:
  friend class Trap;
  TrapTeleport(Pos pos) :
    SpecificTrapBase(pos, TrapId::teleport) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrCyan;}
  std::string      getTitle()      const override {return "Teleporter trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return true;}
  TileId      getTile()       const override {return TileId::elderSign;}
  bool        isDisarmable()  const override {return true;}
};

class TrapSummonMonster: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSummonMonster(Pos pos) :
    SpecificTrapBase(pos, TrapId::summonMonster) {
  }
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrBrownDrk;}
  std::string      getTitle()      const override {return "Monster summoning trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return true;}
  TileId      getTile()       const override {return TileId::elderSign;}
  bool        isDisarmable()  const override {return true;}
};

class TrapSmoke: public SpecificTrapBase {
private:
  friend class Trap;
  TrapSmoke(Pos pos) :
    SpecificTrapBase(pos, TrapId::smoke) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrGray;}
  std::string      getTitle()      const override {return "Smoke trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override { return TileId::trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};

class TrapAlarm: public SpecificTrapBase {
private:
  friend class Trap;
  TrapAlarm(Pos pos) :
    SpecificTrapBase(pos, TrapId::alarm) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrBrown;}
  std::string      getTitle()      const override {return "Alarm trap";}
  char        getGlyph()      const override {return '^';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override { return TileId::trapGeneral;}
  bool        isDisarmable()  const override {return true;}
};


class TrapSpiderWeb: public SpecificTrapBase {
public:
  Dir actorTryLeave(Actor& actor, const Dir dir);

  bool isHolding() const {return isHoldingActor;}

private:
  friend class Trap;
  TrapSpiderWeb(Pos pos) :
    SpecificTrapBase(pos, TrapId::spiderWeb), isHoldingActor(false) {}
  void trigger(Actor& actor, const AbilityRollResult dodgeResult);
  Clr   getClr()        const override {return clrWhiteHigh;}
  std::string      getTitle()      const override {return "Spider web";}
  char        getGlyph()      const override {return '*';}
  bool        isMagical()     const override {return false;}
  TileId      getTile()       const override {return TileId::spiderWeb;}
  bool        isDisarmable()  const override {return true;}
  std::string      getDisarmMsg()  const override {
    return "I tear down a spider web.";
  }
  std::string      getDisarmFailMsg() const override {
    return "I fail to tear down a spider web.";
  }

  bool isHoldingActor;
};

#endif

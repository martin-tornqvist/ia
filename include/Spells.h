#ifndef SPELLS_H
#define SPELLS_H

#include <vector>

#include "CommonTypes.h"
#include "Properties.h"

using namespace std;

class Engine;
class Actor;
class Monster;

enum Spell_t {
  //Player and monster available spells
  spell_darkbolt,
  spell_azathothsWrath,
  spell_enfeeble,
  spell_teleport,
  spell_bless, //TODO Add a curse spell as well, available for player

  //Player only
  spell_mayhem,
  spell_pestilence,
  spell_detectItems,
  spell_detectTraps,
  spell_clairvoyance,
  spell_opening,
  spell_mthPower,
  spell_sacrificeLife,
  spell_sacrificeSpirit,

  spell_rogueHide,

  //Monsters only
  spell_disease,
  spell_summonRandom,
  spell_healSelf, //TODO Make it heal over time, and avail for player too
  spell_knockBack,
  spell_miGoHypnosis,

  endOfSpells
};

class Spell;

class SpellHandler {
public:
  SpellHandler(Engine& engine) : eng(engine) {}
  Spell* getRandomSpellForMonster();
  Spell* getSpellFromId(const Spell_t spellId) const;
private:
  Engine& eng;
};

struct SpellCastRetData {
public:
  SpellCastRetData(bool isCastIdentifyingSpell) :
    isCastIdenifying(isCastIdentifyingSpell) {}
  bool isCastIdenifying;
};

class Spell {
public:
  Spell() {}
  virtual ~Spell() {}
  SpellCastRetData cast(Actor* const caster, const bool IS_INTRINSIC,
                        Engine& eng);
  virtual bool isGoodForMonsterToCastNow(
    Monster* const monster, Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  virtual bool isAvailForAllMonsters() const = 0;
  virtual bool isAvailForPlayer() const = 0;
  virtual string getName() const = 0;
  virtual Spell_t getId() const = 0;

  Range getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                   Engine& eng) const;

  int getShockFromIntrCast() const;
protected:
  virtual SpellCastRetData cast_(Actor* const caster, Engine& eng) = 0;

  virtual int getMaxSpiCost_() const = 0;
  virtual int getShockFromIntrCast_() const = 0;
};

class SpellDarkbolt: public Spell {
public:
  SpellDarkbolt() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Darkbolt";}
  Spell_t getId()               const override {return spell_darkbolt;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI - 2;}
  int getShockFromIntrCast_()   const override {return 5;}
};

class SpellAzathothsWrath: public Spell {
public:
  SpellAzathothsWrath() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Azathoths Wrath";}
  Spell_t getId()               const override {return spell_azathothsWrath;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellMayhem: public Spell {
public:
  SpellMayhem() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Mayhem";}
  Spell_t getId()               const override {return spell_mayhem;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 11;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellPestilence: public Spell {
public:
  SpellPestilence() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Pestilence";}
  Spell_t getId()               const override {return spell_pestilence;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 4;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellDetectItems: public Spell {
public:
  SpellDetectItems() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Items";}
  Spell_t getId()               const override {return spell_detectItems;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 5;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellDetectTraps: public Spell {
public:
  SpellDetectTraps() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Traps";}
  Spell_t getId()               const override {return spell_detectTraps;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 5;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellClairvoyance: public Spell {
public:
  SpellClairvoyance() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Clairvoyance";}
  Spell_t getId()               const override {return spell_clairvoyance;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 9;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellOpening: public Spell {
public:
  SpellOpening() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Opening";}
  Spell_t getId()               const override {return spell_opening;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 9;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellSacrificeLife: public Spell {
public:
  SpellSacrificeLife() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Sacrifice Life Force";}
  Spell_t getId()               const override {return spell_sacrificeLife;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellSacrificeSpirit: public Spell {
public:
  SpellSacrificeSpirit() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Sacrifice Spirit";}
  Spell_t getId()               const override {return spell_sacrificeSpirit;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI - 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellRogueHide: public Spell {
public:
  SpellRogueHide() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Hide [name?]";}
  Spell_t getId()               const override {return spell_rogueHide;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI - 1;}
  int getShockFromIntrCast_()   const override {return 5;}
};

class SpellMthPower: public Spell {
public:
  SpellMthPower() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {
    return "Thaumaturgic Alteration";
  }
  Spell_t getId()               const override {return spell_mthPower;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 4;}
  int getShockFromIntrCast_()   const override {return 10;}

  bool doSpecialAction(Engine& eng) const;
  void castRandomOtherSpell(Engine& eng) const;
};

class SpellBless: public Spell {
public:
  SpellBless() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Bless";}
  Spell_t getId()               const override {return spell_bless;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 11;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellKnockBack: public Spell {
public:
  SpellKnockBack() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Knockback";}
  Spell_t getId()               const override {return spell_knockBack;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellTeleport: public Spell {
public:
  SpellTeleport() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Teleport";}
  Spell_t getId()               const override {return spell_teleport;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellEnfeeble: public Spell {
public:
  SpellEnfeeble() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Enfeeble";}
  Spell_t getId()               const override {return spell_enfeeble;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}

  PropId_t getPropId(Engine& eng) const;
};

class SpellDisease: public Spell {
public:
  SpellDisease() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Disease";}
  Spell_t getId()               const override {return spell_disease;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellSummonRandom: public Spell {
public:
  SpellSummonRandom() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Summon monster";}
  Spell_t getId()               const override {return spell_summonRandom;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellHealSelf: public Spell {
public:
  SpellHealSelf() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Healing";}
  Spell_t getId()               const override {return spell_healSelf;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

class SpellMiGoHypnosis: public Spell {
public:
  SpellMiGoHypnosis() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "MiGo Hypnosis";}
  Spell_t getId()               const override {return spell_miGoHypnosis;}
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
  int getShockFromIntrCast_()   const override {return 10;}
};

#endif

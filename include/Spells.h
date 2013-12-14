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

  //Monsters only
  spell_disease,
  spell_summonRandom,
  spell_healSelf, //TODO Make it heal over time, and avail for player too
  spell_knockBack,

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
  virtual bool isGoodForMonsterToCastNow(Monster* const monster,
                                         Engine& eng) = 0;
  virtual bool isLearnableForMonsters() const = 0;
  virtual bool isLearnableForPlayer() const = 0;
  virtual string getName() const = 0;
  virtual Spell_t getId() const = 0;

  Range getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                   Engine& eng) const;
protected:
  virtual SpellCastRetData specificCast(Actor* const caster,
                                        Engine& eng) = 0;

  virtual int getSpecificMaxSpiCost() const = 0;
};

class SpellAzathothsWrath: public Spell {
public:
  SpellAzathothsWrath() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Azathoths Wrath";}
  Spell_t getId()               const {return spell_azathothsWrath;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 8;}
};

class SpellMayhem: public Spell {
public:
  SpellMayhem() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Mayhem";}
  Spell_t getId()               const {return spell_mayhem;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 17;}
};

class SpellPestilence: public Spell {
public:
  SpellPestilence() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Pestilence";}
  Spell_t getId()               const {return spell_pestilence;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 10;}
};

class SpellDetectItems: public Spell {
public:
  SpellDetectItems() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Detect Items";}
  Spell_t getId()               const {return spell_detectItems;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 11;}
};

class SpellDetectTraps: public Spell {
public:
  SpellDetectTraps() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Detect Traps";}
  Spell_t getId()               const {return spell_detectTraps;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 11;}
};

class SpellClairvoyance: public Spell {
public:
  SpellClairvoyance() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Clairvoyance";}
  Spell_t getId()               const {return spell_clairvoyance;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 15;}
};

class SpellOpening: public Spell {
public:
  SpellOpening() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Opening";}
  Spell_t getId()               const {return spell_opening;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 15;}
};

class SpellSacrificeLife: public Spell {
public:
  SpellSacrificeLife() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Sacrifice Life Force";}
  Spell_t getId()               const {return spell_sacrificeLife;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 8;}
};

class SpellSacrificeSpirit: public Spell {
public:
  SpellSacrificeSpirit() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Sacrifice Spirit";}
  Spell_t getId()               const {return spell_sacrificeSpirit;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 4;}
};

class SpellMthPower: public Spell {
public:
  SpellMthPower() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Thaumaturgic Alteration";}
  Spell_t getId()               const {return spell_mthPower;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 10;}

  bool doSpecialAction(Engine& eng) const;
  void castRandomOtherSpell(Engine& eng) const;
};

class SpellBless: public Spell {
public:
  SpellBless() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Bless";}
  Spell_t getId()               const {return spell_bless;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 17;}
};

class SpellKnockBack: public Spell {
public:
  SpellKnockBack() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Knockback";}
  Spell_t getId()               const {return spell_knockBack;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 8;}
};

class SpellTeleport: public Spell {
public:
  SpellTeleport() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Teleport";}
  Spell_t getId()               const {return spell_teleport;}
  int getSpecificMaxSpiCost()   const {return 8;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
};

class SpellEnfeeble: public Spell {
public:
  SpellEnfeeble() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Enfeeble";}
  Spell_t getId()               const {return spell_enfeeble;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 8;}

  PropId_t getPropId(Engine& eng) const;
};

class SpellDisease: public Spell {
public:
  SpellDisease() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Disease";}
  Spell_t getId()               const {return spell_disease;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 8;}
};

class SpellSummonRandom: public Spell {
public:
  SpellSummonRandom() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Summon monster";}
  Spell_t getId()               const {return spell_summonRandom;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 8;}
};

class SpellHealSelf: public Spell {
public:
  SpellHealSelf() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Healing";}
  Spell_t getId()               const {return spell_healSelf;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine& eng);
  int getSpecificMaxSpiCost()   const {return 8;}
};

#endif

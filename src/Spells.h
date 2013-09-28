#ifndef SPELLS_H
#define SPELLS_H

#include <vector>

#include "CommonTypes.h"
#include "Properties.h"

using namespace std;

class Engine;
class Actor;
class Monster;

enum Spells_t {
  //Player and monster available spells
  spell_azathothsBlast,
  spell_enfeeble,
  spell_teleport,
  spell_bless, //TODO Add a curse spell as well, available for player

  //Player only
  spell_mayhem,
  spell_pestilence,
  spell_descent,
  spell_detectItems,
  spell_detectTraps,
  spell_identify,
  spell_clairvoyance,
  spell_opening,
  spell_mthPower,

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
  SpellHandler(Engine* const engine) : eng(engine) {}
  Spell* getRandomSpellForMonster();
  Spell* getSpellFromId(const Spells_t spellId) const;
private:
  Engine* eng;
};

struct SpellCastRetData {
public:
  SpellCastRetData(bool isCastIdentifying) :
    IS_CAST_IDENTIFYING(isCastIdentifying) {}
  const bool IS_CAST_IDENTIFYING;
};

class Spell {
public:
  Spell() {}
  virtual ~Spell() {}
  SpellCastRetData cast(Actor* const caster, const bool IS_INTRINSIC,
                        Engine* const eng);
  virtual bool isGoodForMonsterToCastNow(Monster* const monster,
                                         Engine* const eng) = 0;
  virtual bool isLearnableForMonsters() const = 0;
  virtual bool isLearnableForPlayer() const = 0;
  virtual string getName() const = 0;
  virtual Spells_t getId() const = 0;

  Range getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                   Engine* const eng) const;
protected:
  virtual SpellCastRetData specificCast(Actor* const caster,
                                        Engine* const eng) = 0;

  virtual int getSpecificMaxSpiCost() const = 0;
};

class SpellIdentify: public Spell {
public:
  SpellIdentify() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Identify";}
  Spells_t getId()              const {return spell_identify;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 17;}
};

class SpellAzathothsBlast: public Spell {
public:
  SpellAzathothsBlast() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Azathoths Blast";}
  Spells_t getId()              const {return spell_azathothsBlast;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 8;}
};

class SpellMayhem: public Spell {
public:
  SpellMayhem() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Mayhem";}
  Spells_t getId()              const {return spell_mayhem;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 17;}
};

class SpellPestilence: public Spell {
public:
  SpellPestilence() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Pestilence";}
  Spells_t getId()              const {return spell_pestilence;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 10;}
};

class SpellDescent: public Spell {
public:
  SpellDescent() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Descent";}
  Spells_t getId()              const {return spell_descent;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 21;}
};

class SpellDetectItems: public Spell {
public:
  SpellDetectItems() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Detect Items";}
  Spells_t getId()              const {return spell_detectItems;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 10;}
};

class SpellDetectTraps: public Spell {
public:
  SpellDetectTraps() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Detect Traps";}
  Spells_t getId()              const {return spell_detectTraps;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 10;}
};

class SpellClairvoyance: public Spell {
public:
  SpellClairvoyance() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Clairvoyance";}
  Spells_t getId()              const {return spell_clairvoyance;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 15;}
};

class SpellOpening: public Spell {
public:
  SpellOpening() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Opening";}
  Spells_t getId()              const {return spell_opening;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 15;}
};

enum MthPowerAction_t {
  mthPowerAction_slayMonsters,
  mthPowerAction_heal,
  mthPowerAction_findStairs,
//  mthPowerAction_sorcery,
  mthPowerAction_mendArmor,
  mthPowerAction_improveWeapon
//  mthPowerAction_purgeEffects //TODO Reimplement
};

class SpellMthPower: public Spell {
public:
  SpellMthPower() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng) {
    (void)monster;
    (void)eng;
    return false;
  }
  bool isLearnableForMonsters() const {return false;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Thaumaturgic Alteration";}
  Spells_t getId()              const {return spell_mthPower;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 10;}

  void getPossibleActions(
    vector<MthPowerAction_t>& possibleActions, Engine* const eng) const;

  void doAction(const MthPowerAction_t action, Engine* const eng) const;
};

class SpellBless: public Spell {
public:
  SpellBless() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Bless";}
  Spells_t getId()              const {return spell_bless;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 17;}
};

class SpellKnockBack: public Spell {
public:
  SpellKnockBack() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Knockback";}
  Spells_t getId()              const {return spell_knockBack;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 8;}
};

class SpellTeleport: public Spell {
public:
  SpellTeleport() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Teleport";}
  Spells_t getId()              const {return spell_teleport;}
  int getSpecificMaxSpiCost()      const {return 8;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
};

class SpellEnfeeble: public Spell {
public:
  SpellEnfeeble() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return true;}
  string getName()              const {return "Enfeeble";}
  Spells_t getId()              const {return spell_enfeeble;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 8;}

  PropId_t getPropId(Engine* const eng) const;
};

class SpellDisease: public Spell {
public:
  SpellDisease() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Disease";}
  Spells_t getId()              const {return spell_disease;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 8;}
};

class SpellSummonRandom: public Spell {
public:
  SpellSummonRandom() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Summon monster";}
  Spells_t getId()              const {return spell_summonRandom;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 8;}
};

class SpellHealSelf: public Spell {
public:
  SpellHealSelf() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine* const eng);
  bool isLearnableForMonsters() const {return true;}
  bool isLearnableForPlayer()   const {return false;}
  string getName()              const {return "Healing";}
  Spells_t getId()              const {return spell_healSelf;}
private:
  SpellCastRetData specificCast(Actor* const caster, Engine* const eng);
  int getSpecificMaxSpiCost()      const {return 8;}
};

#endif

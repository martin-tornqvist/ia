#ifndef SPELLS_H
#define SPELLS_H

#include <vector>

#include "CommonTypes.h"
#include "Properties.h"

using namespace std;

class Engine;
class Actor;
class Monster;

enum SpellId {
  //Player and monster available spells
  spell_darkbolt,
  spell_azathothsWrath,
  spell_enfeeble,
  spell_teleport,
  spell_bless,

  //Player only
  spell_mayhem,
  spell_pestilence,
  spell_detectItems,
  spell_detectTraps,
  spell_detectMonsters,
  spell_opening,
  spell_sacrificeLife,
  spell_sacrificeSpirit,

  spell_cloudMinds,

  //Monsters only
  spell_disease,
  spell_summonRandom,
  spell_healSelf,
  spell_knockBack,
  spell_miGoHypnosis,

  endOfSpellId
};

enum IntrSpellShock {
  intrSpellShockMild,
  intrSpellShockDisturbing,
  intrSpellShockSevere,
};

class Spell;

class SpellHandler {
public:
  SpellHandler(Engine& engine) : eng(engine) {}
  Spell* getRandomSpellForMonster();
  Spell* getSpellFromId(const SpellId spellId) const;
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
  virtual bool isAvailForAllMonsters()  const = 0;
  virtual bool isAvailForPlayer()       const = 0;
  virtual string getName()              const = 0;
  virtual SpellId getId()               const = 0;

  Range getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                   Engine& eng) const;

  int getShockValueIntrCast() {
    const IntrSpellShock shockType = getShockTypeIntrCast();

    switch(shockType) {
      case intrSpellShockMild:        return 4;
      case intrSpellShockDisturbing:  return 10;
      case intrSpellShockSevere:      return 15;
    }
    return -1;
  }

  virtual IntrSpellShock getShockTypeIntrCast() const = 0;
protected:
  virtual SpellCastRetData cast_(Actor* const caster, Engine& eng) = 0;

  virtual int getMaxSpiCost_() const = 0;
};

class SpellDarkbolt: public Spell {
public:
  SpellDarkbolt() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Darkbolt";}
  SpellId getId()               const override {return spell_darkbolt;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockMild;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI - 2;}
};

class SpellAzathothsWrath: public Spell {
public:
  SpellAzathothsWrath() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Azathoths Wrath";}
  SpellId getId()               const override {return spell_azathothsWrath;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellMayhem: public Spell {
public:
  SpellMayhem() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Mayhem";}
  SpellId getId()               const override {return spell_mayhem;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockSevere;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 11;}
};

class SpellPestilence: public Spell {
public:
  SpellPestilence() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Pestilence";}
  SpellId getId()               const override {return spell_pestilence;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 4;}
};

class SpellDetectItems: public Spell {
public:
  SpellDetectItems() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Items";}
  SpellId getId()               const override {return spell_detectItems;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 5;}
};

class SpellDetectTraps: public Spell {
public:
  SpellDetectTraps() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Traps";}
  SpellId getId()               const override {return spell_detectTraps;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 5;}
};

class SpellDetectMonsters: public Spell {
public:
  SpellDetectMonsters() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Monsters";}
  SpellId getId()               const override {return spell_detectMonsters;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI;}
};

class SpellOpening: public Spell {
public:
  SpellOpening() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Opening";}
  SpellId getId()               const override {return spell_opening;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 9;}
};

class SpellSacrificeLife: public Spell {
public:
  SpellSacrificeLife() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Sacrifice Life Force";}
  SpellId getId()               const override {return spell_sacrificeLife;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellSacrificeSpirit: public Spell {
public:
  SpellSacrificeSpirit() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Sacrifice Spirit";}
  SpellId getId()               const override {return spell_sacrificeSpirit;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI - 2;}
};

class SpellCloudMinds: public Spell {
public:
  SpellCloudMinds() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Cloud Minds";}
  SpellId getId()               const override {return spell_cloudMinds;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockMild;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI - 1;}
};

class SpellBless: public Spell {
public:
  SpellBless() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Bless";}
  SpellId getId()               const override {return spell_bless;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 11;}
};

class SpellKnockBack: public Spell {
public:
  SpellKnockBack() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Knockback";}
  SpellId getId()               const override {return spell_knockBack;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellTeleport: public Spell {
public:
  SpellTeleport() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Teleport";}
  SpellId getId()               const override {return spell_teleport;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellEnfeeble: public Spell {
public:
  SpellEnfeeble() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Enfeeble";}
  SpellId getId()               const override {return spell_enfeeble;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}

  PropId getPropId(Engine& eng) const;
};

class SpellDisease: public Spell {
public:
  SpellDisease() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Disease";}
  SpellId getId()               const override {return spell_disease;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellSummonRandom: public Spell {
public:
  SpellSummonRandom() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Summon monster";}
  SpellId getId()               const override {return spell_summonRandom;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellHealSelf: public Spell {
public:
  SpellHealSelf() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Healing";}
  SpellId getId()               const override {return spell_healSelf;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellMiGoHypnosis: public Spell {
public:
  SpellMiGoHypnosis() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster,
                                 Engine& eng) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "MiGo Hypnosis";}
  SpellId getId()               const override {return spell_miGoHypnosis;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return intrSpellShockDisturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster, Engine& eng) override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

#endif

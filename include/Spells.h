#ifndef SPELLS_H
#define SPELLS_H

#include <vector>

#include "CmnTypes.h"
#include "Properties.h"
#include "PlayerBon.h"

using namespace std;

class Actor;
class Monster;

enum class SpellId {
  //Player and monster available spells
  darkbolt,
  azathothsWrath,
  slowEnemies,
  terrifyEnemies,
  paralyzeEnemies,
  teleport,
  bless,

  //Player only
  mayhem,
  pestilence,
  detItems,
  detTraps,
  detMon,
  opening,
  sacrLife,
  sacrSpi,
  elemRes,

  cloudMinds,

  //Monsters only
  disease,
  summonRandom,
  healSelf,
  knockBack,
  miGoHypnosis,
  immolation,

  endOfSpellId
};

enum class IntrSpellShock {mild, disturbing, severe};

class Spell;

class SpellHandler {
public:
  SpellHandler() {}
  Spell* getRandomSpellForMonster();
  Spell* getSpellFromId(const SpellId spellId) const;
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
  SpellCastRetData cast(Actor* const caster, const bool IS_INTRINSIC) const;

  virtual bool isGoodForMonsterToCastNow(
    Monster* const monster) {
    (void)monster;
    return false;
  }
  virtual bool isAvailForAllMonsters()  const = 0;
  virtual bool isAvailForPlayer()       const = 0;
  virtual string getName()              const = 0;
  virtual SpellId getId()               const = 0;

  Range getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster) const;

  int getShockValueIntrCast() const {
    const IntrSpellShock shockType = getShockTypeIntrCast();

    switch(shockType) {
      case IntrSpellShock::mild:        return 2;
      case IntrSpellShock::disturbing:  return 8;
      case IntrSpellShock::severe:      return 16;
    }
    return -1;
  }

  virtual IntrSpellShock getShockTypeIntrCast() const = 0;
protected:
  virtual SpellCastRetData cast_(Actor* const caster) const = 0;

  virtual int getMaxSpiCost_() const = 0;
};

class SpellDarkbolt: public Spell {
public:
  SpellDarkbolt() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Darkbolt";}
  SpellId getId()               const override {return SpellId::darkbolt;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::mild;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(1) - (SPI_PER_LVL * 3);
  }
};

class SpellAzathothsWrath: public Spell {
public:
  SpellAzathothsWrath() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Azathoths Wrath";}
  SpellId getId()               const override {return SpellId::azathothsWrath;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

class SpellMayhem: public Spell {
public:
  SpellMayhem() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Mayhem";}
  SpellId getId()               const override {return SpellId::mayhem;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::severe;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(11);
  }
};

class SpellPestilence: public Spell {
public:
  SpellPestilence() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Pestilence";}
  SpellId getId()               const override {return SpellId::pestilence;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(4);
  }
};

class SpellDetItems: public Spell {
public:
  SpellDetItems() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Items";}
  SpellId getId()               const override {return SpellId::detItems;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(6);
  }
};

class SpellDetTraps: public Spell {
public:
  SpellDetTraps() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Traps";}
  SpellId getId()               const override {return SpellId::detTraps;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(4);
  }
};

class SpellDetMon: public Spell {
public:
  SpellDetMon() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Detect Monsters";}
  SpellId getId()               const override {return SpellId::detMon;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(6);
  }
};

class SpellOpening: public Spell {
public:
  SpellOpening() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Opening";}
  SpellId getId()               const override {return SpellId::opening;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(9);
  }
};

class SpellSacrLife: public Spell {
public:
  SpellSacrLife() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Sacrifice Life Force";}
  SpellId getId()               const override {return SpellId::sacrLife;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

class SpellSacrSpi: public Spell {
public:
  SpellSacrSpi() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Sacrifice Spirit";}
  SpellId getId()               const override {return SpellId::sacrSpi;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(1) - (SPI_PER_LVL * 3);
  }
};

class SpellCloudMinds: public Spell {
public:
  SpellCloudMinds() : Spell() {}
  bool isAvailForAllMonsters()  const override {return false;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Cloud Minds";}
  SpellId getId()               const override {return SpellId::cloudMinds;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::mild;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(1) - (SPI_PER_LVL * 2);
  }
};

class SpellBless: public Spell {
public:
  SpellBless() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Bless";}
  SpellId getId()               const override {return SpellId::bless;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(11);
  }
};

class SpellKnockBack: public Spell {
public:
  SpellKnockBack() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Knockback";}
  SpellId getId()               const override {return SpellId::knockBack;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_()          const override {return PLAYER_START_SPI + 2;}
};

class SpellTeleport: public Spell {
public:
  SpellTeleport() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Teleport";}
  SpellId getId()               const override {return SpellId::teleport;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

class SpellElemRes: public Spell {
public:
  SpellElemRes() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  string getName()              const override {return "Elemental Resistance";}
  SpellId getId()               const override {return SpellId::elemRes;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(4);
  }
};

class SpellPropOnEnemies: public Spell {
public:
  SpellPropOnEnemies() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return true;}
  virtual string getName()      const override = 0;
  virtual SpellId getId()       const override = 0;
  virtual IntrSpellShock getShockTypeIntrCast() const override = 0;
protected:
  SpellCastRetData cast_(Actor* const caster) const override;
  virtual PropId getPropId()    const           = 0;
  virtual int getMaxSpiCost_()  const override  = 0;
};

class SpellSlowEnemies: public SpellPropOnEnemies {
public:
  SpellSlowEnemies() : SpellPropOnEnemies() {}
  string getName()  const override {return "Slow Enemies";}
  SpellId getId()   const override {return SpellId::slowEnemies;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  PropId getPropId() const override {return propSlowed;}
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(1);
  }
};

class SpellTerrifyMon: public SpellPropOnEnemies {
public:
  SpellTerrifyMon() : SpellPropOnEnemies() {}
  string getName()  const override {return "Terrify Enemies";}
  SpellId getId()   const override {return SpellId::terrifyEnemies;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::severe;
  }
private:
  PropId getPropId() const override {return propTerrified;}
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(3);
  }
};

class SpellParalyzeEnemies: public SpellPropOnEnemies {
public:
  SpellParalyzeEnemies() : SpellPropOnEnemies() {}
  string getName()  const override {return "Paralyze Enemies";}
  SpellId getId()   const override {return SpellId::paralyzeEnemies;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  PropId getPropId() const override {return propParalyzed;}
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(6);
  }
};

class SpellDisease: public Spell {
public:
  SpellDisease() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Disease";}
  SpellId getId()               const override {return SpellId::disease;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

class SpellSummonRandom: public Spell {
public:
  SpellSummonRandom() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Summon monster";}
  SpellId getId()               const override {return SpellId::summonRandom;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

class SpellHealSelf: public Spell {
public:
  SpellHealSelf() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Healing";}
  SpellId getId()               const override {return SpellId::healSelf;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

class SpellMiGoHypnosis: public Spell {
public:
  SpellMiGoHypnosis() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "MiGo Hypnosis";}
  SpellId getId()               const override {return SpellId::miGoHypnosis;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

class SpellImmolation: public Spell {
public:
  SpellImmolation() : Spell() {}
  bool isGoodForMonsterToCastNow(Monster* const monster) override;
  bool isAvailForAllMonsters()  const override {return true;}
  bool isAvailForPlayer()       const override {return false;}
  string getName()              const override {return "Immolation";}
  SpellId getId()               const override {return SpellId::immolation;}
  IntrSpellShock getShockTypeIntrCast() const override {
    return IntrSpellShock::disturbing;
  }
private:
  SpellCastRetData cast_(Actor* const caster) const override;
  int getMaxSpiCost_() const override {
    return PlayerBon::getSpiOccultistCanCastAtLvl(2);
  }
};

#endif

#ifndef SPELLS_H
#define SPELLS_H

#include <vector>
#include <string>

#include "Item.h"
#include "CmnTypes.h"
#include "Properties.h"
#include "PlayerBon.h"

class Actor;
class Mon;

const std::string summonWarningStr =
    "There is a small risk that whatever is summoned will be hostile to the caster. "
    "Do not call up that which you cannot put down.";

enum class SpellId
{
    //Available for player and all monsters
    darkbolt,
    azaWrath,
    slowMon,
    terrifyMon,
    paralyzeMon,
    teleport,
    summonMon,
    pest,

    //Player only
    mayhem, //NOTE: This can be cast by the final boss
    detItems,
    detTraps,
    detMon,
    opening,
    sacrLife,
    sacrSpi,
    elemRes,
    light,
    bless,

    cloudMinds,

    //Monsters only
    disease,
    healSelf,
    knockBack,
    miGoHypno,
    burn,

    //Spells from special sources
    pharaohStaff, //From the Staff of the Pharaohs artifact

    END
};

enum class IntrSpellShock {mild, disturbing, severe};

class Spell;

namespace SpellHandling
{

Spell* getRandomSpellForMon();
Spell* mkSpellFromId(const SpellId spellId);

} //SpellHandling

enum class SpellEffectNoticed {no, yes};

class Spell
{
public:
    Spell() {}
    virtual ~Spell() {}
    SpellEffectNoticed cast(Actor* const caster, const bool IS_INTRINSIC) const;

    virtual bool allowMonCastNow(Mon& mon) const
    {
        (void)mon;
        return false;
    }
    virtual bool isAvailForAllMon()       const = 0;
    virtual bool isAvailForPlayer()       const = 0;
    virtual std::string getName()         const = 0;
    virtual SpellId getId()               const = 0;

    virtual std::vector<std::string> getDescr() const = 0;

    Range getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster = nullptr) const;

    int getShockLvlIntrCast() const
    {
        const IntrSpellShock shockType = getShockTypeIntrCast();

        switch (shockType)
        {
        case IntrSpellShock::mild:        return 2;
        case IntrSpellShock::disturbing:  return 8;
        case IntrSpellShock::severe:      return 16;
        }
        return -1;
    }

    virtual IntrSpellShock getShockTypeIntrCast() const = 0;
protected:
    virtual SpellEffectNoticed cast_(Actor* const caster) const = 0;

    virtual int getMaxSpiCost_() const = 0;
};

class SpellDarkbolt: public Spell
{
public:
    SpellDarkbolt() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return true;}
    std::string getName()           const override {return "Darkbolt";}
    SpellId getId()                 const override {return SpellId::darkbolt;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::mild;
    }
    std::vector<std::string> getDescr() const override
    {
        return
        {
            "Siphons power from some hellish mystic source, which is focused into a "
            "bolt cast towards a target with great force."
        };
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(1) - (SPI_PER_LVL * 3);
    }
};

class SpellAzaWrath: public Spell
{
public:
    SpellAzaWrath() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return true;}
    std::string getName()           const override {return "Azathoths Wrath";}
    SpellId getId()                 const override {return SpellId::azaWrath;}

    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> getDescr() const override
    {
        return
        {
            "Channels the destructive force of Azathoth unto all visible enemies."

            /*
            "This spell will generally inflict damage, but due to the chaotic nature of "
            "its source, it can occasionally have other effects, and may affect the "
            "caster as well [TODO]."*/
        };
    }

private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellMayhem: public Spell
{
public:
    SpellMayhem() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return false;}
    bool isAvailForPlayer()         const override {return true;}
    std::string getName()           const override {return "Mayhem";}
    SpellId getId()                 const override {return SpellId::mayhem;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::severe;
    }
    std::vector<std::string> getDescr() const override
    {
        return
        {
            "Engulfs all visible enemies in flames, and causes terrible destruction on "
            "the surrounding area."
        };
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(11);
    }
};

class SpellPest: public Spell
{
public:
    SpellPest() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return true;}
    std::string getName()           const override {return "Pestilence";}
    SpellId getId()                 const override {return SpellId::pest;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return
        {
            "Summons spiders or rats.",
            summonWarningStr
        };
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellPharaohStaff : public Spell
{
public:
    SpellPharaohStaff() {}
    ~SpellPharaohStaff() {}

    bool allowMonCastNow(Mon& mon)    const override;

    virtual bool isAvailForAllMon()   const override {return false;}
    virtual bool isAvailForPlayer()   const override {return false;}
    virtual std::string getName()     const override {return "Summon Mummy servant";}
    virtual SpellId getId()           const override
    {
        return SpellId::pharaohStaff;
    }

    virtual std::vector<std::string> getDescr() const override
    {
        return
        {
            "Summons a loyal Mummy servant which will fight for the caster.",

            "If an allied Mummy is already present, this spell will instead heal it.",

            summonWarningStr
        };
    }

    virtual IntrSpellShock getShockTypeIntrCast() const
    {
        return IntrSpellShock::disturbing;
    }
protected:
    virtual SpellEffectNoticed cast_(Actor* const caster) const override;

    virtual int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellDetItems: public Spell
{
public:
    SpellDetItems() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Detect Items";}
    SpellId getId()               const override {return SpellId::detItems;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"Reveals the presence of all items in the surrounding area."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(6);
    }
};

class SpellDetTraps: public Spell
{
public:
    SpellDetTraps() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Detect Traps";}
    SpellId getId()               const override {return SpellId::detTraps;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"Reveals the presence of all traps in the surrounding area."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(4);
    }
};

class SpellDetMon: public Spell
{
public:
    SpellDetMon() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Detect Creatures";}
    SpellId getId()               const override {return SpellId::detMon;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"Reveals the presence of all creatures in the surrounding area."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(6);
    }
};

class SpellOpening: public Spell
{
public:
    SpellOpening() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Opening";}
    SpellId getId()               const override {return SpellId::opening;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"Opens all locks, lids and doors in the surrounding area."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(5);
    }
};

class SpellSacrLife: public Spell
{
public:
    SpellSacrLife() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Sacrifice Life";}
    SpellId getId()               const override {return SpellId::sacrLife;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return
        {
            "Brings the caster to the brink of death in order to restore the spirit. "
            "The amount restored is proportional to the life sacrificed."
        };
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellSacrSpi: public Spell
{
public:
    SpellSacrSpi() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Sacrifice Spirit";}
    SpellId getId()               const override {return SpellId::sacrSpi;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return
        {
            "Brings the caster to the brink of spiritual death in order to restore "
            "health. The amount restored is proportional to the spirit sacrificed."
        };
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(1) - (SPI_PER_LVL * 3);
    }
};

class SpellCloudMinds: public Spell
{
public:
    SpellCloudMinds() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Cloud Minds";}
    SpellId getId()               const override {return SpellId::cloudMinds;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::mild;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"All enemies forget your presence."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(1) - (SPI_PER_LVL * 2);
    }
};

class SpellBless: public Spell
{
public:
    SpellBless() : Spell() {}
    bool isAvailForAllMon()       const override {return false;}
    bool isAvailForPlayer()       const override {return true;}
    std::string getName()         const override {return "Bless";}
    SpellId getId()               const override {return SpellId::bless;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"Bends the universe in favor of the caster."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(11);
    }
};

class SpellLight: public Spell
{
public:
    SpellLight() : Spell() {}

    bool isAvailForAllMon() const override {return false;}
    bool isAvailForPlayer() const override {return true;}

    std::string getName()   const override {return "Light";}
    SpellId     getId()     const override {return SpellId::light;}

    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::mild;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"Illuminates the area around the caster."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(1);
    }
};

class SpellKnockBack: public Spell
{
public:
    SpellKnockBack() : Spell() {}

    bool allowMonCastNow(Mon& mon) const override;

    bool isAvailForAllMon() const override {return true;}
    bool isAvailForPlayer() const override {return false;}

    std::string getName()   const override {return "Knockback";}
    SpellId     getId()     const override {return SpellId::knockBack;}

    IntrSpellShock  getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override {return PLAYER_START_SPI + 2;}
};

class SpellTeleport: public Spell
{
public:
    SpellTeleport() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return true;}
    std::string getName()           const override {return "Teleport";}
    SpellId getId()                 const override {return SpellId::teleport;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {"Instantly moves the caster to a different position."};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellElemRes: public Spell
{
public:
    SpellElemRes() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return true;}
    std::string getName()           const override {return "Elemental Resistance";}
    SpellId getId()                 const override {return SpellId::elemRes;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return
        {
            "For a brief time, the caster is completely shielded from fire, cold and "
            "electricity."
        };
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(4);
    }
};

class SpellPropOnMon: public Spell
{
public:
    SpellPropOnMon() : Spell() {}
    virtual bool            allowMonCastNow(Mon& mon)   const override;
    bool                    isAvailForAllMon()          const override {return true;}
    bool                    isAvailForPlayer()          const override {return true;}
    virtual std::string     getName()                   const override = 0;
    virtual SpellId         getId()                     const override = 0;
    virtual IntrSpellShock  getShockTypeIntrCast()      const override = 0;
protected:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    virtual PropId getPropId()    const           = 0;
    virtual int getMaxSpiCost_()  const override  = 0;
};

class SpellSlowMon: public SpellPropOnMon
{
public:
    SpellSlowMon() : SpellPropOnMon() {}

    std::string getName()   const override {return "Slow Enemies";}
    SpellId     getId()     const override {return SpellId::slowMon;}

    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> getDescr() const override
    {
        return {"Causes all visible enemies to move slower."};
    }
private:
    PropId getPropId() const override {return PropId::slowed;}

    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(1);
    }
};

class SpellTerrifyMon: public SpellPropOnMon
{
public:
    SpellTerrifyMon() : SpellPropOnMon() {}

    std::string getName()   const override {return "Terrify Enemies";}
    SpellId     getId()     const override {return SpellId::terrifyMon;}

    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::severe;
    }

    std::vector<std::string> getDescr() const override
    {
        return {"Causes terror in the minds of all visible enemies."};
    }
private:
    PropId getPropId() const override {return PropId::terrified;}

    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(3);
    }
};

class SpellParalyzeMon: public SpellPropOnMon
{
public:
    SpellParalyzeMon() : SpellPropOnMon() {}

    std::string getName()   const override {return "Paralyze Enemies";}
    SpellId     getId()     const override {return SpellId::paralyzeMon;}

    virtual bool allowMonCastNow(Mon& mon) const override;

    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> getDescr() const override
    {
        return {"All visible enemies are paralyzed for a brief moment."};
    }
private:
    PropId getPropId() const override {return PropId::paralyzed;}
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(6);
    }
};

class SpellDisease: public Spell
{
public:
    SpellDisease() : Spell() {}
    bool allowMonCastNow(Mon& mon) const override;

    bool isAvailForAllMon() const override {return true;}
    bool isAvailForPlayer() const override {return false;}

    std::string getName() const override {return "Disease";}

    SpellId getId() const override {return SpellId::disease;}

    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> getDescr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;

    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellSummonMon: public Spell
{
public:
    SpellSummonMon() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return true;}
    std::string getName()           const override {return "Summon Creature";}
    SpellId getId()                 const override {return SpellId::summonMon;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return
        {
            "Summons a creature to do the caster's bidding. A more powerful sorcerer "
            "(higher character level) can summon beings of greater might and rarity.",

            summonWarningStr
        };
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellHealSelf: public Spell
{
public:
    SpellHealSelf() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return false;}
    std::string getName()           const override {return "Healing";}
    SpellId getId()                 const override {return SpellId::healSelf;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellMiGoHypno: public Spell
{
public:
    SpellMiGoHypno() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return false;}
    std::string getName()           const override {return "MiGo Hypnosis";}
    SpellId getId()                 const override {return SpellId::miGoHypno;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

class SpellBurn: public Spell
{
public:
    SpellBurn() : Spell() {}
    bool allowMonCastNow(Mon& mon)  const override;
    bool isAvailForAllMon()         const override {return true;}
    bool isAvailForPlayer()         const override {return false;}
    std::string getName()           const override {return "Immolation";}
    SpellId getId()                 const override {return SpellId::burn;}
    IntrSpellShock getShockTypeIntrCast() const override
    {
        return IntrSpellShock::disturbing;
    }
    std::vector<std::string> getDescr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_(Actor* const caster) const override;
    int getMaxSpiCost_() const override
    {
        return PlayerBon::getSpiOccultistCanCastAtLvl(2);
    }
};

#endif

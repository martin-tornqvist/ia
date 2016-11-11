#ifndef SPELLS_HPP
#define SPELLS_HPP

#include <vector>
#include <string>

#include "item.hpp"
#include "properties.hpp"
#include "player_bon.hpp"

class Actor;
class Mon;

const std::string summon_warning_str =
    "There is a small risk that whatever is summoned will be hostile to the "
    "caster. Do not call up that which you cannot put down.";

enum class SpellId
{
    //Available for player and all monsters
    darkbolt,
    aza_wrath,
    enfeeble_mon,
    teleport,
    summon,
    pest,

    //Player only
    mayhem,
    det_items,
    det_traps,
    det_mon,
    opening,
    sacr_life,
    sacr_spi,
    res,
    light,
    bless,
    anim_wpns,
    cloud_minds,

    //Ghoul background
    frenzy,

    //Monsters only
    disease,
    heal_self,
    knock_back,
    mi_go_hypno,
    burn,

    //Spells from special sources
    pharaoh_staff, //From the Staff of the Pharaohs artifact

    END
};

enum class IntrSpellShock
{
    mild,
    disturbing,
    severe
};

class Spell;

namespace spell_handling
{

Spell* random_spell_for_mon();
Spell* mk_spell_from_id(const SpellId spell_id);

} //spell_handling

enum class SpellEffectNoticed
{
    no,
    yes
};

class Spell
{
public:
    Spell() {}

    virtual ~Spell() {}

    SpellEffectNoticed cast(Actor* const caster,
                            const bool is_intrinsic,
                            const bool is_base_cost_only) const;

    virtual bool allow_mon_cast_now(Mon& mon) const
    {
        (void)mon;
        return false;
    }

    virtual bool mon_can_learn() const = 0;
    virtual bool player_can_learn() const = 0;
    virtual std::string name() const = 0;
    virtual SpellId id() const = 0;

    virtual std::vector<std::string> descr() const = 0;

    Range spi_cost(const bool is_base_cost_only, Actor* const caster = nullptr) const;

    int shock_lvl_intr_cast() const
    {
        const IntrSpellShock shock_type = shock_type_intr_cast();

        switch (shock_type)
        {
        case IntrSpellShock::mild:
            return 2;

        case IntrSpellShock::disturbing:
            return 8;

        case IntrSpellShock::severe:
            return 16;
        }

        return -1;
    }

    virtual IntrSpellShock shock_type_intr_cast() const = 0;

protected:
    virtual SpellEffectNoticed cast_impl(Actor* const caster) const = 0;

    virtual int max_spi_cost() const = 0;

    void on_resist(Actor& target) const;
};

class SpellDarkbolt: public Spell
{
public:
    SpellDarkbolt() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Darkbolt";
    }

    SpellId id() const override
    {
        return SpellId::darkbolt;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::mild;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Siphons power from some hellish mystic source, which is focused into a "
            "bolt cast towards a target with great force."
        };
    }

private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 4;
    }
};

class SpellAzaWrath: public Spell
{
public:
    SpellAzaWrath() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Azathoths Wrath";
    }

    SpellId id() const override
    {
        return SpellId::aza_wrath;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
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
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 9;
    }
};

class SpellMayhem: public Spell
{
public:
    SpellMayhem() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Mayhem";
    }

    SpellId id() const override
    {
        return SpellId::mayhem;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::severe;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Engulfs all visible enemies in flames, and causes terrible destruction on "
            "the surrounding area."
        };
    }

private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 16;
    }
};

class SpellPest: public Spell
{
public:
    SpellPest() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Pestilence";
    }

    SpellId id() const override
    {
        return SpellId::pest;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Summons spiders or rats.",
            summon_warning_str
        };
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellAnimWpns: public Spell
{
public:
    SpellAnimWpns() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Animate Weapons";
    }

    SpellId id() const override
    {
        return SpellId::anim_wpns;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::mild;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Infuses lifeless weapons with a spirit of their own, causing them to rise up into the "
            "air and protect their master (for a while). It is only possible to animate basic "
            "melee weapons however - \"modern\" mechanisms such as pistols or machine guns are far "
            "too complex."
        };
    }

private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellPharaohStaff : public Spell
{
public:
    SpellPharaohStaff() {}
    ~SpellPharaohStaff() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    virtual bool mon_can_learn() const override
    {
        return false;
    }

    virtual bool player_can_learn() const override
    {
        return false;
    }

    virtual std::string name() const override
    {
        return "Summon Mummy servant";
    }

    virtual SpellId id() const override
    {
        return SpellId::pharaoh_staff;
    }

    virtual std::vector<std::string> descr() const override
    {
        return
        {
            "Summons a loyal Mummy servant which will fight for the caster.",

            "If an allied Mummy is already present, this spell will instead heal it.",

            summon_warning_str
        };
    }

    virtual IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }
protected:
    virtual SpellEffectNoticed cast_impl(Actor* const caster) const override;

    virtual int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellDetItems: public Spell
{
public:
    SpellDetItems() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Detect Items";
    }

    SpellId id() const override
    {
        return SpellId::det_items;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {"Reveals the presence of all items in the surrounding area."};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 11;
    }
};

class SpellDetTraps: public Spell
{
public:
    SpellDetTraps() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Detect Traps";
    }

    SpellId id() const override
    {
        return SpellId::det_traps;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {"Reveals the presence of all traps in the surrounding area."};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 9;
    }
};

class SpellDetMon: public Spell
{
public:
    SpellDetMon() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Detect Creatures";
    }

    SpellId id() const override
    {
        return SpellId::det_mon;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::mild;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Reveals the presence of all creatures in the surrounding area."
        };
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 4;
    }
};

class SpellOpening: public Spell
{
public:
    SpellOpening() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Opening";
    }

    SpellId id() const override
    {
        return SpellId::opening;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Opens all locks, lids and doors in the surrounding area."
        };
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 10;
    }
};

class SpellSacrLife: public Spell
{
public:
    SpellSacrLife() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Sacrifice Life";
    }

    SpellId id() const override
    {
        return SpellId::sacr_life;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Brings the caster to the brink of death in order to restore the spirit. "
            "The amount restored is proportional to the life sacrificed."
        };
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellSacrSpi: public Spell
{
public:
    SpellSacrSpi() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Sacrifice Spirit";
    }

    SpellId id() const override
    {
        return SpellId::sacr_spi;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Brings the caster to the brink of spiritual death in order to restore "
            "health. The amount restored is proportional to the spirit sacrificed."
        };
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 3;
    }
};

class SpellCloudMinds: public Spell
{
public:
    SpellCloudMinds() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Cloud Minds";
    }

    SpellId id() const override
    {
        return SpellId::cloud_minds;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::mild;
    }

    std::vector<std::string> descr() const override
    {
        return {"All enemies forget your presence."};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 5;
    }
};

class SpellFrenzy: public Spell
{
public:
    SpellFrenzy() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Incite Frenzy";
    }

    SpellId id() const override
    {
        return SpellId::frenzy;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::mild;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Incites a great rage in the caster, which will charge their enemies with a "
            "terrible, uncontrollable fury."
        };
    }

private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 3;
    }
};

class SpellBless: public Spell
{
public:
    SpellBless() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Bless";
    }

    SpellId id() const override
    {
        return SpellId::bless;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {"Bends the universe in favor of the caster."};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 16;
    }
};

class SpellLight: public Spell
{
public:
    SpellLight() : Spell() {}

    bool mon_can_learn() const override
    {
        return false;
    }
    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Light";
    }

    SpellId id() const override
    {
        return SpellId::light;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::mild;
    }

    std::vector<std::string> descr() const override
    {
        return {"Illuminates the area around the caster."};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 4;
    }
};

class SpellKnockBack: public Spell
{
public:
    SpellKnockBack() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }
    bool player_can_learn() const override
    {
        return false;
    }

    std::string name() const override
    {
        return "Knockback";
    }

    SpellId id() const override
    {
        return SpellId::knock_back;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 8;
    }
};

class SpellTeleport: public Spell
{
public:
    SpellTeleport() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Teleport";
    }

    SpellId id() const override
    {
        return SpellId::teleport;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {"Instantly moves the caster to a different position."};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellRes: public Spell
{
public:
    SpellRes() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Resistance";
    }

    SpellId id() const override
    {
        return SpellId::res;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "For a brief time, the caster is completely shielded from fire and electricity."
        };
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 9;
    }
};

class SpellEnfeebleMon: public Spell
{
public:
    SpellEnfeebleMon() : Spell() {}

    virtual bool allow_mon_cast_now(Mon& mon) const override;

    std::string name() const override
    {
        return "Enfeeble Enemies";
    }

    SpellId id() const override
    {
        return SpellId::enfeeble_mon;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {"Attempts to enfeeble all visible enemies by exhausting their stamina, or by "
                "terrorizing their minds."};
    }

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

protected:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellDisease: public Spell
{
public:
    SpellDisease() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return false;
    }

    std::string name() const override
    {
        return "Disease";
    }

    SpellId id() const override
    {
        return SpellId::disease;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellSummonMon: public Spell
{
public:
    SpellSummonMon() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::string name() const override
    {
        return "Summon Creature";
    }

    SpellId id() const override
    {
        return SpellId::summon;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return
        {
            "Summons a creature to do the caster's bidding. A more powerful sorcerer "
            "(higher character level) can summon beings of greater might and rarity.",

            summon_warning_str
        };
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellHealSelf: public Spell
{
public:
    SpellHealSelf() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return false;
    }

    std::string name() const override
    {
        return "Healing";
    }

    SpellId id() const override
    {
        return SpellId::heal_self;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellMiGoHypno: public Spell
{
public:
    SpellMiGoHypno() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return false;
    }

    std::string name() const override
    {
        return "MiGo Hypnosis";
    }

    SpellId id() const override
    {
        return SpellId::mi_go_hypno;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellBurn: public Spell
{
public:
    SpellBurn() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return false;
    }

    std::string name() const override
    {
        return "Immolation";
    }

    SpellId id() const override
    {
        return SpellId::burn;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr() const override
    {
        return {""};
    }
private:
    SpellEffectNoticed cast_impl(Actor* const caster) const override;

    int max_spi_cost() const override
    {
        return 7;
    }
};

#endif

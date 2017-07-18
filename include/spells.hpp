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
    // Available for player and all monsters
    darkbolt,
    aza_wrath,
    enfeeble_mon,
    teleport,
    summon,
    pest,

    // Player only
    mayhem,
    searching,
    opening,
    sacr_life,
    sacr_spi,
    res,
    light,
    bless,
    anim_wpns,

    // Ghoul background
    frenzy,

    // Monsters only
    disease,
    heal_self,
    knockback,
    mi_go_hypno,
    burn,
    deafen,

    // Spells from special sources
    pharaoh_staff, // From the Staff of the Pharaohs artifact

    END
};

enum class SpellSrc
{
    learned,
    manuscript,
    item
};

enum class IntrSpellShock
{
    mild,
    disturbing,
    severe
};

enum class IsIntrinsic
{
    no,
    yes
};

class Spell;

namespace spell_handling
{

Spell* random_spell_for_mon();
Spell* mk_spell_from_id(const SpellId spell_id);

} // spell_handling

class Spell
{
public:
    Spell() {}

    virtual ~Spell() {}

    void cast(Actor* const caster,
              const int skill,
              const IsIntrinsic intrinsic) const;

    virtual bool allow_mon_cast_now(Mon& mon) const
    {
        (void)mon;
        return false;
    }

    virtual int mon_cooldown() const
    {
        return 3;
    }

    virtual bool mon_can_learn() const = 0;

    virtual bool player_can_learn() const = 0;

    virtual std::string name() const = 0;

    virtual SpellId id() const = 0;

    virtual bool can_be_improved_with_skill() const
    {
        return true;
    }

    std::vector<std::string> descr(const int skill) const;

    Range spi_cost(Actor* const caster = nullptr) const;

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

    virtual void run_effect(Actor* const caster, const int skill) const = 0;

protected:
    virtual int max_spi_cost() const = 0;

    virtual std::vector<std::string> descr_specific(const int skill) const = 0;

    void on_resist(Actor& target) const;
};

class SpellDarkbolt: public Spell
{
public:
    SpellDarkbolt() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 5;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 5;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 7;
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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 9;
    }
};

class SpellPest: public Spell
{
public:
    SpellPest() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 8;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 8;
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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    bool can_be_improved_with_skill() const override
    {
        return false;
    }

    std::vector<std::string> descr_specific(const int skill) const override;

    virtual IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    virtual void run_effect(Actor* const caster, const int skill) const override;

protected:
    virtual int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellSearching: public Spell
{
public:
    SpellSearching() : Spell() {}

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
        return "Searching";
    }

    SpellId id() const override
    {
        return SpellId::searching;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 7;
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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 3;
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

    bool can_be_improved_with_skill() const override
    {
        return false;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::mild;
    }

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 5;
    }

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
        return SpellId::knockback;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(const int skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 30;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 11;
    }
};

class SpellRes: public Spell
{
public:
    SpellRes() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 20;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 5;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override;

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    void run_effect(Actor* const caster, const int skill) const override;

protected:
    int max_spi_cost() const override
    {
        return 6;
    }
};

class SpellDisease: public Spell
{
public:
    SpellDisease() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 10;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 8;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override;

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 5;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 5;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster, const int skill) const override;

private:
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

    int mon_cooldown() const override
    {
        return 7;
    }

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

    std::vector<std::string> descr_specific(const int skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 7;
    }
};

class SpellDeafen: public Spell
{
public:
    SpellDeafen() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 5;
    }

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
        return "Deafen";
    }

    SpellId id() const override
    {
        return SpellId::deafen;
    }

    IntrSpellShock shock_type_intr_cast() const override
    {
        return IntrSpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(const int skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster, const int skill) const override;

private:
    int max_spi_cost() const override
    {
        return 4;
    }
};

#endif // SPELLS_HPP

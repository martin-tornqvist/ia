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
    // Available for player and monsters
    darkbolt,
    enfeeble,
    teleport,
    summon,
    pest,
    see_invis,
    heal,
    spell_shield,

    // Player only
    aza_wrath,
    mayhem,
    searching,
    opening,
    res,
    light,
    bless,
    anim_wpns,
    insight,
    transmut,

    // Ghoul background
    frenzy,

    // Monsters only
    disease,
    knockback,
    mi_go_hypno,
    burn,
    deafen,

    // Spells from special sources
    pharaoh_staff, // From the Staff of the Pharaohs artifact
    subdue_wpns, // Learned at the same time as Animate Weapons

    END
};

enum class SpellSkill
{
    basic,
    expert,
    master
};

enum class SpellSrc
{
    learned,
    manuscript,
    item
};

enum class SpellShock
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
              const SpellSkill skill,
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

    std::vector<std::string> descr(const SpellSkill skill) const;

    Range spi_cost(const SpellSkill skill, Actor* const caster = nullptr) const;

    int shock_value() const;

    virtual SpellShock shock_type() const = 0;

    virtual void run_effect(Actor* const caster,
                            const SpellSkill skill) const = 0;

protected:
    virtual int max_spi_cost(const SpellSkill skill) const = 0;

    virtual std::vector<std::string> descr_specific(
        const SpellSkill skill) const = 0;

    void on_resist(Actor& target) const;
};

class SpellDarkbolt: public Spell
{
public:
    SpellDarkbolt() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 6;
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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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
        return 6;
    }

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
        return "Azathoth's Wrath";
    }

    SpellId id() const override
    {
        return SpellId::aza_wrath;
    }

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 6;
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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 6;
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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 6;
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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 6;
    }
};

class SpellSubdueWpns : public Spell
{
public:
    SpellSubdueWpns() {}
    ~SpellSubdueWpns() {}

    virtual bool mon_can_learn() const override
    {
        return false;
    }

    virtual bool player_can_learn() const override
    {
        return true;
    }

    virtual std::string name() const override
    {
        return "Subdue Weapons";
    }

    virtual SpellId id() const override
    {
        return SpellId::subdue_wpns;
    }

    bool can_be_improved_with_skill() const override
    {
        return false;
    }

    virtual SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

protected:
    virtual int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 2;
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

    virtual SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

protected:
    virtual int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 5;
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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 4;
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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 8;
    }
};

class SpellInsight: public Spell
{
public:
    SpellInsight() : Spell() {}

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
        return "Insight";
    }

    SpellId id() const override
    {
        return SpellId::insight;
    }

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 8;
    }
};

class SpellTransmut: public Spell
{
public:
    SpellTransmut() : Spell() {}

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
        return "Transmutation";
    }

    SpellId id() const override
    {
        return SpellId::transmut;
    }

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 8;
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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 10;
    }
};

class SpellSeeInvis: public Spell
{
public:
    SpellSeeInvis() : Spell() {}

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
        return "See Invisible";
    }

    SpellId id() const override
    {
        return SpellId::see_invis;
    }

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 8;
    }
};

class SpellSpellShield: public Spell
{
public:
    SpellSpellShield() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 3;
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
        return "Spell Shield";
    }

    SpellId id() const override
    {
        return SpellId::spell_shield;
    }

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override;
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

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 6;
    }
};

class SpellEnfeeble: public Spell
{
public:
    SpellEnfeeble() : Spell() {}

    virtual bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 5;
    }

    std::string name() const override
    {
        return "Enfeeble";
    }

    SpellId id() const override
    {
        return SpellId::enfeeble;
    }

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    bool mon_can_learn() const override
    {
        return true;
    }

    bool player_can_learn() const override
    {
        return true;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

protected:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 7;
    }
};

class SpellHeal: public Spell
{
public:
    SpellHeal() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    int mon_cooldown() const override
    {
        return 6;
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
        return "Healing";
    }

    SpellId id() const override
    {
        return SpellId::heal;
    }

    SpellShock shock_type() const override
    {
        return SpellShock::mild;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override;

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 6;
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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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
        return 9;
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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

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

    SpellShock shock_type() const override
    {
        return SpellShock::disturbing;
    }

    std::vector<std::string> descr_specific(
        const SpellSkill skill) const override
    {
        (void)skill;

        return {""};
    }

    void run_effect(Actor* const caster,
                    const SpellSkill skill) const override;

private:
    int max_spi_cost(const SpellSkill skill) const override
    {
        (void)skill;

        return 4;
    }
};

#endif // SPELLS_HPP

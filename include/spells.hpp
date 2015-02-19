#ifndef SPELLS_H
#define SPELLS_H

#include <vector>
#include <string>

#include "item.hpp"
#include "cmn_types.hpp"
#include "properties.hpp"
#include "player_bon.hpp"

class Actor;
class Mon;

const std::string summon_warning_str =
    "There is a small risk that whatever is summoned will be hostile to the caster. "
    "Do not call up that which you cannot put down.";

enum class Spell_id
{
    //Available for player and all monsters
    darkbolt,
    aza_wrath,
    slow_mon,
    terrify_mon,
    paralyze_mon,
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
    elem_res,
    light,
    bless,

    cloud_minds,

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

enum class Intr_spell_shock {mild, disturbing, severe};

class Spell;

namespace Spell_handling
{

Spell* get_random_spell_for_mon();
Spell* mk_spell_from_id(const Spell_id spell_id);

} //Spell_handling

enum class Spell_effect_noticed {no, yes};

class Spell
{
public:
    Spell() {}
    virtual ~Spell() {}
    Spell_effect_noticed cast(Actor* const caster, const bool IS_INTRINSIC) const;

    virtual bool allow_mon_cast_now(Mon& mon) const
    {
        (void)mon;
        return false;
    }
    virtual bool is_avail_for_all_mon()       const = 0;
    virtual bool is_avail_for_player()       const = 0;
    virtual std::string get_name()         const = 0;
    virtual Spell_id get_id()               const = 0;

    virtual std::vector<std::string> get_descr() const = 0;

    Range get_spi_cost(const bool IS_BASE_COST_ONLY, Actor* const caster = nullptr) const;

    int get_shock_lvl_intr_cast() const
    {
        const Intr_spell_shock shock_type = get_shock_type_intr_cast();

        switch (shock_type)
        {
        case Intr_spell_shock::mild:        return 2;
        case Intr_spell_shock::disturbing:  return 8;
        case Intr_spell_shock::severe:      return 16;
        }
        return -1;
    }

    virtual Intr_spell_shock get_shock_type_intr_cast() const = 0;
protected:
    virtual Spell_effect_noticed cast_(Actor* const caster) const = 0;

    virtual int get_max_spi_cost_() const = 0;
};

class Spell_darkbolt: public Spell
{
public:
    Spell_darkbolt() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return true;}
    std::string get_name()           const override {return "Darkbolt";}
    Spell_id get_id()                 const override {return Spell_id::darkbolt;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::mild;
    }
    std::vector<std::string> get_descr() const override
    {
        return
        {
            "Siphons power from some hellish mystic source, which is focused into a "
            "bolt cast towards a target with great force."
        };
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(1) - (SPI_PER_LVL * 3);
    }
};

class Spell_aza_wrath: public Spell
{
public:
    Spell_aza_wrath() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return true;}
    std::string get_name()           const override {return "Azathoths Wrath";}
    Spell_id get_id()                 const override {return Spell_id::aza_wrath;}

    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }

    std::vector<std::string> get_descr() const override
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
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_mayhem: public Spell
{
public:
    Spell_mayhem() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return false;}
    bool is_avail_for_player()         const override {return true;}
    std::string get_name()           const override {return "Mayhem";}
    Spell_id get_id()                 const override {return Spell_id::mayhem;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::severe;
    }
    std::vector<std::string> get_descr() const override
    {
        return
        {
            "Engulfs all visible enemies in flames, and causes terrible destruction on "
            "the surrounding area."
        };
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(11);
    }
};

class Spell_pest: public Spell
{
public:
    Spell_pest() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return true;}
    std::string get_name()           const override {return "Pestilence";}
    Spell_id get_id()                 const override {return Spell_id::pest;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return
        {
            "Summons spiders or rats.",
            summon_warning_str
        };
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_pharaoh_staff : public Spell
{
public:
    Spell_pharaoh_staff() {}
    ~Spell_pharaoh_staff() {}

    bool allow_mon_cast_now(Mon& mon)    const override;

    virtual bool is_avail_for_all_mon()   const override {return false;}
    virtual bool is_avail_for_player()   const override {return false;}
    virtual std::string get_name()     const override {return "Summon Mummy servant";}
    virtual Spell_id get_id()           const override
    {
        return Spell_id::pharaoh_staff;
    }

    virtual std::vector<std::string> get_descr() const override
    {
        return
        {
            "Summons a loyal Mummy servant which will fight for the caster.",

            "If an allied Mummy is already present, this spell will instead heal it.",

            summon_warning_str
        };
    }

    virtual Intr_spell_shock get_shock_type_intr_cast() const
    {
        return Intr_spell_shock::disturbing;
    }
protected:
    virtual Spell_effect_noticed cast_(Actor* const caster) const override;

    virtual int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_det_items: public Spell
{
public:
    Spell_det_items() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Detect Items";}
    Spell_id get_id()               const override {return Spell_id::det_items;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"Reveals the presence of all items in the surrounding area."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(6);
    }
};

class Spell_det_traps: public Spell
{
public:
    Spell_det_traps() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Detect Traps";}
    Spell_id get_id()               const override {return Spell_id::det_traps;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"Reveals the presence of all traps in the surrounding area."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(4);
    }
};

class Spell_det_mon: public Spell
{
public:
    Spell_det_mon() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Detect Creatures";}
    Spell_id get_id()               const override {return Spell_id::det_mon;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"Reveals the presence of all creatures in the surrounding area."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(6);
    }
};

class Spell_opening: public Spell
{
public:
    Spell_opening() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Opening";}
    Spell_id get_id()               const override {return Spell_id::opening;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"Opens all locks, lids and doors in the surrounding area."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(5);
    }
};

class Spell_sacr_life: public Spell
{
public:
    Spell_sacr_life() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Sacrifice Life";}
    Spell_id get_id()               const override {return Spell_id::sacr_life;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return
        {
            "Brings the caster to the brink of death in order to restore the spirit. "
            "The amount restored is proportional to the life sacrificed."
        };
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_sacr_spi: public Spell
{
public:
    Spell_sacr_spi() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Sacrifice Spirit";}
    Spell_id get_id()               const override {return Spell_id::sacr_spi;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return
        {
            "Brings the caster to the brink of spiritual death in order to restore "
            "health. The amount restored is proportional to the spirit sacrificed."
        };
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(1) - (SPI_PER_LVL * 3);
    }
};

class Spell_cloud_minds: public Spell
{
public:
    Spell_cloud_minds() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Cloud Minds";}
    Spell_id get_id()               const override {return Spell_id::cloud_minds;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::mild;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"All enemies forget your presence."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(1) - (SPI_PER_LVL * 2);
    }
};

class Spell_bless: public Spell
{
public:
    Spell_bless() : Spell() {}
    bool is_avail_for_all_mon()       const override {return false;}
    bool is_avail_for_player()       const override {return true;}
    std::string get_name()         const override {return "Bless";}
    Spell_id get_id()               const override {return Spell_id::bless;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"Bends the universe in favor of the caster."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(11);
    }
};

class Spell_light: public Spell
{
public:
    Spell_light() : Spell() {}

    bool is_avail_for_all_mon() const override {return false;}
    bool is_avail_for_player() const override {return true;}

    std::string get_name()   const override {return "Light";}
    Spell_id     get_id()     const override {return Spell_id::light;}

    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::mild;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"Illuminates the area around the caster."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(1);
    }
};

class Spell_knock_back: public Spell
{
public:
    Spell_knock_back() : Spell() {}

    bool allow_mon_cast_now(Mon& mon) const override;

    bool is_avail_for_all_mon() const override {return true;}
    bool is_avail_for_player() const override {return false;}

    std::string get_name()   const override {return "Knockback";}
    Spell_id     get_id()     const override {return Spell_id::knock_back;}

    Intr_spell_shock  get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {""};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override {return PLAYER_START_SPI + 2;}
};

class Spell_teleport: public Spell
{
public:
    Spell_teleport() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return true;}
    std::string get_name()           const override {return "Teleport";}
    Spell_id get_id()                 const override {return Spell_id::teleport;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {"Instantly moves the caster to a different position."};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_elem_res: public Spell
{
public:
    Spell_elem_res() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return true;}
    std::string get_name()           const override {return "Elemental Resistance";}
    Spell_id get_id()                 const override {return Spell_id::elem_res;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return
        {
            "For a brief time, the caster is completely shielded from fire, cold and "
            "electricity."
        };
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(4);
    }
};

class Spell_prop_on_mon: public Spell
{
public:
    Spell_prop_on_mon() : Spell() {}
    virtual bool            allow_mon_cast_now(Mon& mon)   const override;
    bool                    is_avail_for_all_mon()          const override {return true;}
    bool                    is_avail_for_player()          const override {return true;}
    virtual std::string     get_name()                   const override = 0;
    virtual Spell_id         get_id()                     const override = 0;
    virtual Intr_spell_shock  get_shock_type_intr_cast()      const override = 0;
protected:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    virtual Prop_id get_prop_id()    const           = 0;
    virtual int get_max_spi_cost_()  const override  = 0;
};

class Spell_slow_mon: public Spell_prop_on_mon
{
public:
    Spell_slow_mon() : Spell_prop_on_mon() {}

    std::string get_name()   const override {return "Slow Enemies";}
    Spell_id     get_id()     const override {return Spell_id::slow_mon;}

    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }

    std::vector<std::string> get_descr() const override
    {
        return {"Causes all visible enemies to move slower."};
    }
private:
    Prop_id get_prop_id() const override {return Prop_id::slowed;}

    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(1);
    }
};

class Spell_terrify_mon: public Spell_prop_on_mon
{
public:
    Spell_terrify_mon() : Spell_prop_on_mon() {}

    std::string get_name()   const override {return "Terrify Enemies";}
    Spell_id     get_id()     const override {return Spell_id::terrify_mon;}

    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::severe;
    }

    std::vector<std::string> get_descr() const override
    {
        return {"Causes terror in the minds of all visible enemies."};
    }
private:
    Prop_id get_prop_id() const override {return Prop_id::terrified;}

    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(3);
    }
};

class Spell_paralyze_mon: public Spell_prop_on_mon
{
public:
    Spell_paralyze_mon() : Spell_prop_on_mon() {}

    std::string get_name()   const override {return "Paralyze Enemies";}
    Spell_id     get_id()     const override {return Spell_id::paralyze_mon;}

    virtual bool allow_mon_cast_now(Mon& mon) const override;

    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }

    std::vector<std::string> get_descr() const override
    {
        return {"All visible enemies are paralyzed for a brief moment."};
    }
private:
    Prop_id get_prop_id() const override {return Prop_id::paralyzed;}
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(6);
    }
};

class Spell_disease: public Spell
{
public:
    Spell_disease() : Spell() {}
    bool allow_mon_cast_now(Mon& mon) const override;

    bool is_avail_for_all_mon() const override {return true;}
    bool is_avail_for_player() const override {return false;}

    std::string get_name() const override {return "Disease";}

    Spell_id get_id() const override {return Spell_id::disease;}

    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }

    std::vector<std::string> get_descr() const override
    {
        return {""};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;

    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_summon_mon: public Spell
{
public:
    Spell_summon_mon() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return true;}
    std::string get_name()           const override {return "Summon Creature";}
    Spell_id get_id()                 const override {return Spell_id::summon;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return
        {
            "Summons a creature to do the caster's bidding. A more powerful sorcerer "
            "(higher character level) can summon beings of greater might and rarity.",

            summon_warning_str
        };
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_heal_self: public Spell
{
public:
    Spell_heal_self() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return false;}
    std::string get_name()           const override {return "Healing";}
    Spell_id get_id()                 const override {return Spell_id::heal_self;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {""};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_mi_go_hypno: public Spell
{
public:
    Spell_mi_go_hypno() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return false;}
    std::string get_name()           const override {return "Mi_go Hypnosis";}
    Spell_id get_id()                 const override {return Spell_id::mi_go_hypno;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {""};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

class Spell_burn: public Spell
{
public:
    Spell_burn() : Spell() {}
    bool allow_mon_cast_now(Mon& mon)  const override;
    bool is_avail_for_all_mon()         const override {return true;}
    bool is_avail_for_player()         const override {return false;}
    std::string get_name()           const override {return "Immolation";}
    Spell_id get_id()                 const override {return Spell_id::burn;}
    Intr_spell_shock get_shock_type_intr_cast() const override
    {
        return Intr_spell_shock::disturbing;
    }
    std::vector<std::string> get_descr() const override
    {
        return {""};
    }
private:
    Spell_effect_noticed cast_(Actor* const caster) const override;
    int get_max_spi_cost_() const override
    {
        return Player_bon::get_spi_occultist_can_cast_at_lvl(2);
    }
};

#endif

#ifndef ITEM_AMULET_HPP
#define ITEM_AMULET_HPP

#include "item.hpp"
#include "actor_data.hpp"

#include <vector>

class Prop;
class Amulet;

enum class Amulet_effect_id
{
    //Primary effects (i.e. one of these must be on the amulet)
    rFire,
    rElec,
    rPoison,
    rDisease,
    tele_ctrl,
    light,
    haste,
    spell_reflect,
    hp_bon,
    spi_bon,
    hp_regen_bon,
    conflict,

    //Secondary
    START_OF_SECONDARY_EFFECTS,
    hp_pen,
    spi_pen,
    hp_regen_pen,
    burden,
    shriek,
    random_tele,
    summon,
    fire,

    END
};

class Amulet_effect
{
public:
    Amulet_effect(Amulet* const amulet) :
        amulet_(amulet) {}

    virtual ~Amulet_effect() {}

    virtual Amulet_effect_id id() const = 0;

    virtual void on_equip(const Verbosity verbosity)
    {
        (void)verbosity;
    }

    virtual Unequip_allowed on_unequip()
    {
        return Unequip_allowed::yes;
    }

    virtual void on_std_turn_equipped() {}
    virtual void on_actor_turn_equipped() {}

    virtual void change_item_weight(int& weight_ref)
    {
        (void)weight_ref;
    }

    virtual int hp_regen_change() const
    {
        return 0;
    }

    virtual std::string descr() const = 0;

    void save();
    void load();

protected:
    Amulet* const amulet_;
};

//Base class for effects which just apply a property
class Amulet_property_effect : public Amulet_effect
{
public:
    Amulet_property_effect(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    virtual ~Amulet_property_effect() {}

    void on_equip(const Verbosity verbosity) override;

    Unequip_allowed on_unequip() override final;

protected:
    virtual Prop* mk_prop() const = 0;
};

//--------------------------------------------------------- EFFECTS
class Amulet_effect_rFire : public Amulet_property_effect
{
public:
    Amulet_effect_rFire(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_rFire() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::rFire;
    }

    std::string descr() const override
    {
        return "It shields the wearer against fire.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_rElec : public Amulet_property_effect
{
public:
    Amulet_effect_rElec(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_rElec() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::rElec;
    }

    std::string descr() const override
    {
        return "It shields the wearer against electricity.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_rPoison : public Amulet_property_effect
{
public:
    Amulet_effect_rPoison(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_rPoison() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::rPoison;
    }

    std::string descr() const override
    {
        return "It shields the wearer against poison.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_rDisease : public Amulet_property_effect
{
public:
    Amulet_effect_rDisease(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_rDisease() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::rDisease;
    }

    std::string descr() const override
    {
        return "It shields the wearer against disease.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_tele_control : public Amulet_property_effect
{
public:
    Amulet_effect_tele_control(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_tele_control() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::tele_ctrl;
    }

    std::string descr() const override
    {
        return "It grants the wearer power to control teleportation.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_light : public Amulet_property_effect
{
public:
    Amulet_effect_light(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_light() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::light;
    }

    std::string descr() const override
    {
        return "It illuminates the area around the wearer.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_spell_reflect : public Amulet_property_effect
{
public:
    Amulet_effect_spell_reflect(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_spell_reflect() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::spell_reflect;
    }

    std::string descr() const override
    {
        return "It shields the wearer against spells, and reflects them back on the caster.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_haste : public Amulet_property_effect
{
public:
    Amulet_effect_haste(Amulet* const amulet) :
        Amulet_property_effect(amulet) {}

    ~Amulet_effect_haste() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::haste;
    }

    std::string descr() const override
    {
        return "It speeds up the wearer.";
    }

protected:
    Prop* mk_prop() const override;
};

class Amulet_effect_hp_bon : public Amulet_effect
{
public:
    Amulet_effect_hp_bon(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_hp_bon() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::hp_bon;
    }

    std::string descr() const override
    {
        return "It grants stronger vitality.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Amulet_effect_hp_pen : public Amulet_effect
{
public:
    Amulet_effect_hp_pen(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_hp_pen() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::hp_pen;
    }

    std::string descr() const override
    {
        return "It makes the wearer frailer.";
    }

    void on_equip(const Verbosity verbosity) override;

    Unequip_allowed on_unequip() override;
};

class Amulet_effect_spi_bon : public Amulet_effect
{
public:
    Amulet_effect_spi_bon(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_spi_bon() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::spi_bon;
    }

    std::string descr() const override
    {
        return "It strengthens the spirit of the wearer.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Amulet_effect_spi_pen : public Amulet_effect
{
public:
    Amulet_effect_spi_pen(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_spi_pen() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::spi_pen;
    }

    std::string descr() const override
    {
        return "It weakens the spirit of the wearer.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Amulet_effect_random_tele : public Amulet_effect
{
public:
    Amulet_effect_random_tele(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_random_tele() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::random_tele;
    }

    std::string descr() const override
    {
        return "It occasionally teleports the wearer.";
    }

    void on_std_turn_equipped() override;
};

class Amulet_effect_summon_mon : public Amulet_effect
{
public:
    Amulet_effect_summon_mon(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_summon_mon() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::summon;
    }

    std::string descr() const override
    {
        return "It calls deadly interdimensional beings into the existence of the "
               "wearer.";
    }

    void on_std_turn_equipped() override;
};

class Amulet_effect_fire : public Amulet_effect
{
public:
    Amulet_effect_fire(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_fire() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::fire;
    }

    std::string descr() const override
    {
        return "It spontaneously sets objects around the caster on fire.";
    }

    void on_std_turn_equipped() override;
};

class Amulet_effect_shriek : public Amulet_effect
{
public:
    Amulet_effect_shriek(Amulet* const amulet);

    ~Amulet_effect_shriek() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::shriek;
    }

    std::string descr() const override
    {
        return "It occasionally emits a disembodied voice in a horrible shrieking tone.";
    }

    void on_std_turn_equipped() override;

private:
    std::vector<std::string> words_;
};

class Amulet_effect_conflict : public Amulet_effect
{
public:
    Amulet_effect_conflict(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_conflict() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::conflict;
    }

    std::string descr() const override
    {
        return "It sows hatred in the minds of nearby creatures, and turns allies "
               "against each other.";
    }

    void on_std_turn_equipped() override;
};

class Amulet_effect_burden : public Amulet_effect
{
public:
    Amulet_effect_burden(Amulet* const amulet);

    ~Amulet_effect_burden() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::burden;
    }

    std::string descr() const override
    {
        return "It is inexplicably heavy for its size.";
    }

    void on_equip(const Verbosity verbosity) override;
    void change_item_weight(int& weight_ref) override;
};

class Amulet_effect_hp_regen_bon : public Amulet_effect
{
public:
    Amulet_effect_hp_regen_bon(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_hp_regen_bon() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::hp_regen_bon;
    }

    int hp_regen_change() const override
    {
        return -2;
    }

    std::string descr() const override
    {
        return "The wounds of the wearer heal faster.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Amulet_effect_hp_regen_pen : public Amulet_effect
{
public:
    Amulet_effect_hp_regen_pen(Amulet* const amulet) :
        Amulet_effect(amulet) {}

    ~Amulet_effect_hp_regen_pen() {}

    Amulet_effect_id id() const override
    {
        return Amulet_effect_id::hp_regen_pen;
    }

    int hp_regen_change() const override
    {
        return 2;
    }

    std::string descr() const override
    {
        return "The wounds of the wearer heal slower.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Amulet : public Item
{
public:
    Amulet(Item_data_t* const item_data);

    ~Amulet();

    std::vector<std::string> descr() const override final;

    void on_equip_hook(const Verbosity verbosity) override final;

    Unequip_allowed on_unequip_hook() override final;

    void on_std_turn_in_inv(const Inv_type inv_type) override final;

    void on_actor_turn_in_inv(const Inv_type inv_type) override final;

    int hp_regen_change(const Inv_type inv_type) const override;

    Clr interface_clr() const override
    {
        return clr_orange;
    }

    void identify(const Verbosity verbosity) override final;

    int weight() const override;

    //Called from the effects
    void effect_noticed(const Amulet_effect_id effect_id);

private:
    virtual std::string name_inf() const override;

    std::vector<Amulet_effect*> effects_;
};

namespace amulet_handling
{

void init();

void save();
void load();

} //amulet_handling

#endif // ITEM_AMULET_HPP

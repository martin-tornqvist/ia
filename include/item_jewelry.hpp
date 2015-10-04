#ifndef ITEM_JEWELRY_H
#define ITEM_JEWELRY_H

#include "item.hpp"
#include "actor_data.hpp"

#include <vector>

class Prop;
class Jewelry;

enum class Jewelry_effect_id
{
    //Primary effects (i.e. one of these must be on the jewelry)
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

class Jewelry_effect
{
public:
    Jewelry_effect(Jewelry* const jewelry) :
        jewelry_(jewelry) {}

    virtual ~Jewelry_effect() {}

    virtual Jewelry_effect_id id() const = 0;

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
    Jewelry* const jewelry_;
};

//Base class for effects which just apply a property
class Jewelry_property_effect : public Jewelry_effect
{
public:
    Jewelry_property_effect(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    virtual ~Jewelry_property_effect() {}

    void on_equip(const Verbosity verbosity) override;

    Unequip_allowed on_unequip() override final;

protected:
    virtual Prop* mk_prop() const = 0;
};

//--------------------------------------------------------- EFFECTS
class Jewelry_effect_rFire : public Jewelry_property_effect
{
public:
    Jewelry_effect_rFire(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_rFire() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::rFire;
    }

    std::string descr() const override
    {
        return "It shields the wearer against fire.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_rElec : public Jewelry_property_effect
{
public:
    Jewelry_effect_rElec(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_rElec() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::rElec;
    }

    std::string descr() const override
    {
        return "It shields the wearer against electricity.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_rPoison : public Jewelry_property_effect
{
public:
    Jewelry_effect_rPoison(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_rPoison() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::rPoison;
    }

    std::string descr() const override
    {
        return "It shields the wearer against poison.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_rDisease : public Jewelry_property_effect
{
public:
    Jewelry_effect_rDisease(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_rDisease() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::rDisease;
    }

    std::string descr() const override
    {
        return "It shields the wearer against disease.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_tele_control : public Jewelry_property_effect
{
public:
    Jewelry_effect_tele_control(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_tele_control() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::tele_ctrl;
    }

    std::string descr() const override
    {
        return "It grants the wearer power to control teleportation.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_light : public Jewelry_property_effect
{
public:
    Jewelry_effect_light(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_light() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::light;
    }

    std::string descr() const override
    {
        return "It illuminates the area around the wearer.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_spell_reflect : public Jewelry_property_effect
{
public:
    Jewelry_effect_spell_reflect(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_spell_reflect() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::spell_reflect;
    }

    std::string descr() const override
    {
        return "It shields the wearer against spells, and reflects them back on the caster.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_haste : public Jewelry_property_effect
{
public:
    Jewelry_effect_haste(Jewelry* const jewelry) :
        Jewelry_property_effect(jewelry) {}

    ~Jewelry_effect_haste() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::haste;
    }

    std::string descr() const override
    {
        return "It speeds up the wearer.";
    }

protected:
    Prop* mk_prop() const override;
};

class Jewelry_effect_hp_bon : public Jewelry_effect
{
public:
    Jewelry_effect_hp_bon(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_hp_bon() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::hp_bon;
    }

    std::string descr() const override
    {
        return "It grants stronger vitality.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Jewelry_effect_hp_pen : public Jewelry_effect
{
public:
    Jewelry_effect_hp_pen(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_hp_pen() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::hp_pen;
    }

    std::string descr() const override
    {
        return "It makes the wearer frailer.";
    }

    void on_equip(const Verbosity verbosity) override;

    Unequip_allowed on_unequip() override;
};

class Jewelry_effect_spi_bon : public Jewelry_effect
{
public:
    Jewelry_effect_spi_bon(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_spi_bon() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::spi_bon;
    }

    std::string descr() const override
    {
        return "It strengthens the spirit of the wearer.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Jewelry_effect_spi_pen : public Jewelry_effect
{
public:
    Jewelry_effect_spi_pen(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_spi_pen() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::spi_pen;
    }

    std::string descr() const override
    {
        return "It weakens the spirit of the wearer.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Jewelry_effect_random_tele : public Jewelry_effect
{
public:
    Jewelry_effect_random_tele(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_random_tele() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::random_tele;
    }

    std::string descr() const override
    {
        return "It occasionally teleports the wearer.";
    }

    void on_std_turn_equipped() override;
};

class Jewelry_effect_summon_mon : public Jewelry_effect
{
public:
    Jewelry_effect_summon_mon(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_summon_mon() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::summon;
    }

    std::string descr() const override
    {
        return "It calls deadly interdimensional beings into the existence of the "
               "wearer.";
    }

    void on_std_turn_equipped() override;
};

class Jewelry_effect_fire : public Jewelry_effect
{
public:
    Jewelry_effect_fire(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_fire() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::fire;
    }

    std::string descr() const override
    {
        return "It spontaneously sets objects around the caster on fire.";
    }

    void on_std_turn_equipped() override;
};

class Jewelry_effect_shriek : public Jewelry_effect
{
public:
    Jewelry_effect_shriek(Jewelry* const jewelry);

    ~Jewelry_effect_shriek() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::shriek;
    }

    std::string descr() const override
    {
        return "It occasionally emits a disembodied voice in a horrible shrieking tone.";
    }

    void on_std_turn_equipped() override;

private:
    std::vector<std::string> words_;
};

class Jewelry_effect_conflict : public Jewelry_effect
{
public:
    Jewelry_effect_conflict(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_conflict() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::conflict;
    }

    std::string descr() const override
    {
        return "It sows hatred in the minds of nearby creatures, and turns allies "
               "against each other.";
    }

    void on_std_turn_equipped() override;
};

class Jewelry_effect_burden : public Jewelry_effect
{
public:
    Jewelry_effect_burden(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_burden() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::burden;
    }

    std::string descr() const override
    {
        return "It burdens the wearer, as if there was an invisible weight to carry.";
    }

    void on_equip(const Verbosity verbosity) override;
    void change_item_weight(int& weight_ref) override;
};

class Jewelry_effect_hp_regen_bon : public Jewelry_effect
{
public:
    Jewelry_effect_hp_regen_bon(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_hp_regen_bon() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::hp_regen_bon;
    }

    int hp_regen_change() const override
    {
        return -12;
    }

    std::string descr() const override
    {
        return "The wounds of the wearer heal faster.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Jewelry_effect_hp_regen_pen : public Jewelry_effect
{
public:
    Jewelry_effect_hp_regen_pen(Jewelry* const jewelry) :
        Jewelry_effect(jewelry) {}

    ~Jewelry_effect_hp_regen_pen() {}

    Jewelry_effect_id id() const override
    {
        return Jewelry_effect_id::hp_regen_pen;
    }

    int hp_regen_change() const override
    {
        return 12;
    }

    std::string descr() const override
    {
        return "The wounds of the wearer heal slower.";
    }

    void on_equip(const Verbosity verbosity) override;
    Unequip_allowed on_unequip() override;
};

class Jewelry : public Item
{
public:
    Jewelry(Item_data_t* const item_data);

    ~Jewelry();

    std::vector<std::string> descr() const override final;

    void on_equip_hook(const Verbosity verbosity) override final;

    Unequip_allowed on_unequip_hook() override final;

    void on_std_turn_in_inv(const Inv_type inv_type) override final;

    void on_actor_turn_in_inv(const Inv_type inv_type) override final;

    int hp_regen_change(const Inv_type inv_type) const;

    Clr interface_clr() const override
    {
        return clr_orange;
    }

    void identify(const Verbosity verbosity) override final;

    int weight() const override;

    //Called from the effects
    void effect_noticed(const Jewelry_effect_id effect_id);

private:
    virtual std::string name_inf() const;

    std::vector<Jewelry_effect*> effects_;
};

namespace jewelry_handling
{

void init();

void save();
void load();

} //Jewelry_handling

#endif // ITEM_JEWELRY_H

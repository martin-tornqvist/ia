#ifndef ITEM_AMULET_HPP
#define ITEM_AMULET_HPP

#include "item.hpp"
#include "actor_data.hpp"

#include <vector>

class Prop;
class Amulet;

enum class AmuletEffectId
{
    //Primary effects (i.e. one of these must be on the amulet)
    r_fire,
    r_elec,
    r_poison,
    r_disease,
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

class AmuletEffect
{
public:
    AmuletEffect(Amulet* const amulet) :
        amulet_(amulet) {}

    virtual ~AmuletEffect() {}

    virtual AmuletEffectId id() const = 0;

    virtual void on_equip(const Verbosity verbosity)
    {
        (void)verbosity;
    }

    virtual UnequipAllowed on_unequip()
    {
        return UnequipAllowed::yes;
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
class AmuletPropertyEffect : public AmuletEffect
{
public:
    AmuletPropertyEffect(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    virtual ~AmuletPropertyEffect() {}

    void on_equip(const Verbosity verbosity) override;

    UnequipAllowed on_unequip() override final;

protected:
    virtual Prop* mk_prop() const = 0;
};

// -----------------------------------------------------------------------------
// Effects
// -----------------------------------------------------------------------------
class AmuletEffectRFire : public AmuletPropertyEffect
{
public:
    AmuletEffectRFire(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectRFire() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::r_fire;
    }

    std::string descr() const override
    {
        return "It shields the wearer against fire.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectRElec : public AmuletPropertyEffect
{
public:
    AmuletEffectRElec(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectRElec() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::r_elec;
    }

    std::string descr() const override
    {
        return "It shields the wearer against electricity.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectRPoison : public AmuletPropertyEffect
{
public:
    AmuletEffectRPoison(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectRPoison() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::r_poison;
    }

    std::string descr() const override
    {
        return "It shields the wearer against poison.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectRDisease : public AmuletPropertyEffect
{
public:
    AmuletEffectRDisease(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectRDisease() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::r_disease;
    }

    std::string descr() const override
    {
        return "It shields the wearer against disease.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectTeleControl : public AmuletPropertyEffect
{
public:
    AmuletEffectTeleControl(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectTeleControl() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::tele_ctrl;
    }

    std::string descr() const override
    {
        return "It grants the wearer power to control teleportation.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectLight : public AmuletPropertyEffect
{
public:
    AmuletEffectLight(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectLight() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::light;
    }

    std::string descr() const override
    {
        return "It illuminates the area around the wearer.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectSpellReflect : public AmuletPropertyEffect
{
public:
    AmuletEffectSpellReflect(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectSpellReflect() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::spell_reflect;
    }

    std::string descr() const override
    {
        return "It shields the wearer against spells, and reflects them back on the caster.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectHaste : public AmuletPropertyEffect
{
public:
    AmuletEffectHaste(Amulet* const amulet) :
        AmuletPropertyEffect(amulet) {}

    ~AmuletEffectHaste() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::haste;
    }

    std::string descr() const override
    {
        return "It speeds up the wearer.";
    }

protected:
    Prop* mk_prop() const override;
};

class AmuletEffectHpBon : public AmuletEffect
{
public:
    AmuletEffectHpBon(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectHpBon() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::hp_bon;
    }

    std::string descr() const override
    {
        return "It grants stronger vitality.";
    }

    void on_equip(const Verbosity verbosity) override;
    UnequipAllowed on_unequip() override;
};

class AmuletEffectHpPen : public AmuletEffect
{
public:
    AmuletEffectHpPen(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectHpPen() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::hp_pen;
    }

    std::string descr() const override
    {
        return "It makes the wearer frailer.";
    }

    void on_equip(const Verbosity verbosity) override;

    UnequipAllowed on_unequip() override;
};

class AmuletEffectSpiBon : public AmuletEffect
{
public:
    AmuletEffectSpiBon(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectSpiBon() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::spi_bon;
    }

    std::string descr() const override
    {
        return "It strengthens the spirit of the wearer.";
    }

    void on_equip(const Verbosity verbosity) override;
    UnequipAllowed on_unequip() override;
};

class AmuletEffectSpiPen : public AmuletEffect
{
public:
    AmuletEffectSpiPen(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectSpiPen() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::spi_pen;
    }

    std::string descr() const override
    {
        return "It weakens the spirit of the wearer.";
    }

    void on_equip(const Verbosity verbosity) override;
    UnequipAllowed on_unequip() override;
};

class AmuletEffectRandomTele : public AmuletEffect
{
public:
    AmuletEffectRandomTele(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectRandomTele() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::random_tele;
    }

    std::string descr() const override
    {
        return "It occasionally teleports the wearer.";
    }

    void on_std_turn_equipped() override;
};

class AmuletEffectSummonMon : public AmuletEffect
{
public:
    AmuletEffectSummonMon(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectSummonMon() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::summon;
    }

    std::string descr() const override
    {
        return "It calls deadly interdimensional beings into the existence of the "
               "wearer.";
    }

    void on_std_turn_equipped() override;
};

class AmuletEffectFire : public AmuletEffect
{
public:
    AmuletEffectFire(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectFire() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::fire;
    }

    std::string descr() const override
    {
        return "It spontaneously sets objects around the caster on fire.";
    }

    void on_std_turn_equipped() override;
};

class AmuletEffectShriek : public AmuletEffect
{
public:
    AmuletEffectShriek(Amulet* const amulet);

    ~AmuletEffectShriek() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::shriek;
    }

    std::string descr() const override
    {
        return "It occasionally emits a disembodied voice in a horrible shrieking tone.";
    }

    void on_std_turn_equipped() override;

private:
    std::vector<std::string> words_;
};

class AmuletEffectConflict : public AmuletEffect
{
public:
    AmuletEffectConflict(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectConflict() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::conflict;
    }

    std::string descr() const override
    {
        return "It sows hatred in the minds of nearby creatures, and turns allies "
               "against each other.";
    }

    void on_std_turn_equipped() override;
};

class AmuletEffectBurden : public AmuletEffect
{
public:
    AmuletEffectBurden(Amulet* const amulet);

    ~AmuletEffectBurden() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::burden;
    }

    std::string descr() const override
    {
        return "It is inexplicably heavy for its size.";
    }

    void on_equip(const Verbosity verbosity) override;
    void change_item_weight(int& weight_ref) override;
};

class AmuletEffectHpRegenBon : public AmuletEffect
{
public:
    AmuletEffectHpRegenBon(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectHpRegenBon() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::hp_regen_bon;
    }

    int hp_regen_change() const override
    {
        return -2;
    }

    std::string descr() const override
    {
        return "The wearer recovers faster.";
    }

    void on_equip(const Verbosity verbosity) override;
    UnequipAllowed on_unequip() override;
};

class AmuletEffectHpRegenPen : public AmuletEffect
{
public:
    AmuletEffectHpRegenPen(Amulet* const amulet) :
        AmuletEffect(amulet) {}

    ~AmuletEffectHpRegenPen() {}

    AmuletEffectId id() const override
    {
        return AmuletEffectId::hp_regen_pen;
    }

    int hp_regen_change() const override
    {
        return 2;
    }

    std::string descr() const override
    {
        return "The wearer recovers slower.";
    }

    void on_equip(const Verbosity verbosity) override;
    UnequipAllowed on_unequip() override;
};

class Amulet : public Item
{
public:
    Amulet(ItemDataT* const item_data);

    ~Amulet();

    std::vector<std::string> descr() const override final;

    void on_equip_hook(const Verbosity verbosity) override final;

    UnequipAllowed on_unequip_hook() override final;

    void on_std_turn_in_inv(const InvType inv_type) override final;

    void on_actor_turn_in_inv(const InvType inv_type) override final;

    int hp_regen_change(const InvType inv_type) const override;

    Clr interface_clr() const override
    {
        return clr_orange;
    }

    void identify(const Verbosity verbosity) override final;

    int weight() const override;

    //Called from the effects
    void effect_noticed(const AmuletEffectId effect_id);

private:
    virtual std::string name_inf() const override;

    std::vector<AmuletEffect*> effects_;
};

namespace amulet_handling
{

void init();

void save();
void load();

} //amulet_handling

#endif // ITEM_AMULET_HPP

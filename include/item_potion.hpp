#ifndef ITEM_POTION_HPP
#define ITEM_POTION_HPP

#include <vector>
#include <string>

#include "item.hpp"

enum class PotionAlignment
{
    good,
    bad
};

class Potion: public Item
{
public:
    Potion(ItemDataT* const item_data);

    virtual ~Potion() {}

    void save();

    void load();

    ConsumeItem activate(Actor* const actor) override final;

    Clr interface_clr() const override final
    {
        return clr_blue_lgt;
    }

    std::vector<std::string> descr() const override final;

    void on_actor_turn_in_inv(const InvType inv_type) override;

    void on_collide(const P& pos, Actor* actor);

    void identify(const Verbosity verbosity) override final;

    virtual const std::string real_name() const = 0;

protected:
    virtual std::string descr_identified() const = 0;

    virtual PotionAlignment alignment() const = 0;

    virtual void collide_hook(const P& pos, Actor* const actor) = 0;

    virtual void quaff_impl(Actor& actor) = 0;

    std::string name_inf() const override final;

private:
    std::string alignment_str() const;

    int alignment_feeling_countdown_;
};

class PotionVitality: public Potion
{
public:
    PotionVitality(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionVitality() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Vitality";
    }

private:
    std::string descr_identified() const override
    {
        return
            "This elixir heals all wounds and cures blindness, poisoning, "
            "infections, disease, and weakening. It can even temporarily raise "
            "the consumers condition past normal levels.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionSpirit: public Potion
{
public:
    PotionSpirit(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionSpirit() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Spirit";
    }

private:
    std::string descr_identified() const override
    {
        return "Restores the spirit.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionBlindness: public Potion
{
public:
    PotionBlindness(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionBlindness() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Blindness";
    }

private:
    std::string descr_identified() const override
    {
        return "Causes temporary loss of vision.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::bad;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionParal: public Potion
{
public:
    PotionParal(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionParal() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Paralyzation";
    }

private:
    std::string descr_identified() const override
    {
        return "Causes paralysis.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::bad;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionDisease: public Potion
{
public:
    PotionDisease(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionDisease() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Disease";
    }

private:
    std::string descr_identified() const override
    {
        return "This foul liquid causes a horrible disease.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::bad;
    }

    void collide_hook(const P& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};

class PotionConf: public Potion
{
public:
    PotionConf(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionConf() {}
    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Confusion";
    }

private:
    std::string descr_identified() const override
    {
        return "Causes confusion.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::bad;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionFortitude: public Potion
{
public:
    PotionFortitude(ItemDataT* const item_data) :
        Potion(item_data) {}

    ~PotionFortitude() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Fortitude";
    }

private:
    std::string descr_identified() const override
    {
        return "Gives the consumer complete peace of mind.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionPoison: public Potion
{
public:
    PotionPoison(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionPoison() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Poison";
    }

private:
    std::string descr_identified() const override
    {
        return "A deadly brew.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::bad;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionInsight: public Potion
{
public:
    PotionInsight(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionInsight() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Insight";
    }

private:
    std::string descr_identified() const override
    {
        return "This strange concoction causes a sudden flash of intuition.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};


class PotionRFire: public Potion
{
public:
    PotionRFire(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionRFire() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Fire Resistance";
    }

private:
    std::string descr_identified() const override
    {
        return "Protects the consumer from fire.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionCuring: public Potion
{
public:
    PotionCuring(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionCuring() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Curing";
    }

private:
    std::string descr_identified() const override
    {
        return
            "Cures blindness, poisoning, infections, disease, and weakening, "
            "and restores the consumers health by a small amount.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionRElec: public Potion
{
public:
    PotionRElec(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionRElec() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Insulation";
    }

private:
    std::string descr_identified() const override
    {
        return "Protects the consumer from electricity.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

class PotionDescent: public Potion
{
public:
    PotionDescent(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionDescent() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Descent";
    }

private:
    std::string descr_identified() const override
    {
        return "A bizarre liquid that causes the consumer to dematerialize and sink "
               "through the ground.";
    }

    // TODO: Not sure about the alignment for this one...
    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};

class PotionInvis: public Potion
{
public:
    PotionInvis(ItemDataT* const item_data) :
        Potion(item_data) {}
    ~PotionInvis() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Invisibility";
    }

private:
    std::string descr_identified() const override
    {
        return
            "Makes the consumer invisible to normal vision for a brief time. "
            "Attacking or casting spells immediately reveals the consumer.";
    }

    PotionAlignment alignment() const override
    {
        return PotionAlignment::good;
    }

    void collide_hook(const P& pos, Actor* const actor) override;
};

namespace potion_handling
{

struct PotionLook
{
    std::string name_plain;
    std::string name_a;
    Clr clr;
};

void init();

void save();
void load();

} //PotionHandling

#endif

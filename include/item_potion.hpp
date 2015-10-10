#ifndef ITEM_POTION_H
#define ITEM_POTION_H

#include <vector>
#include <string>

#include "item.hpp"
#include "cmn_data.hpp"

class Potion: public Item
{
public:
    Potion(Item_data_t* const item_data) :
        Item(item_data) {}

    virtual ~Potion() {}

    Consume_item activate(Actor* const actor) override final;

    Clr interface_clr() const override final
    {
        return clr_blue_lgt;
    }

    std::vector<std::string> descr() const override final;

    void on_collide(const Pos& pos, Actor* actor);

    void identify(const Verbosity verbosity) override final;

    virtual const std::string real_name() const = 0;

protected:
    virtual std::string descr_identified() const = 0;

    //TODO: *All* potions do nothing but run the quaff implementation when this
    //is called. That should probably be moved up to the base collide function
    virtual void collide_hook(const Pos& pos, Actor* const actor) = 0;

    virtual void quaff_impl(Actor& actor) = 0;

    std::string name_inf() const override final;
};

class Potion_vitality: public Potion
{
public:
    Potion_vitality(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_vitality() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Vitality";
    }

private:
    std::string descr_identified() const override
    {
        return "This elixir heals all wounds and cures blindness, poisoning, infections, disease, "
               "and weakening. It can even temporarily raise the consumers condition past normal "
               "levels.";
    }

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_spirit: public Potion
{
public:
    Potion_spirit(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_spirit() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_blindness: public Potion
{
public:
    Potion_blindness(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_blindness() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_paral: public Potion
{
public:
    Potion_paral(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_paral() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_disease: public Potion
{
public:
    Potion_disease(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_disease() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};

class Potion_conf: public Potion
{
public:
    Potion_conf(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_conf() {}
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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_fortitude: public Potion
{
public:
    Potion_fortitude(Item_data_t* const item_data) :
        Potion(item_data) {}

    ~Potion_fortitude() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_poison: public Potion
{
public:
    Potion_poison(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_poison() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_insight: public Potion
{
public:
    Potion_insight(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_insight() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};

class Potion_clairv: public Potion
{
public:
    Potion_clairv(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_clairv() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Clairvoyance";
    }

private:
    std::string descr_identified() const override
    {
        return "Bestows visions of the surrounding area.";
    }

    void collide_hook(const Pos& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};

class Potion_rFire: public Potion
{
public:
    Potion_rFire(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_rFire() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_curing: public Potion
{
public:
    Potion_curing(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_curing() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Curing";
    }

private:
    std::string descr_identified() const override
    {
        return "Cures blindness, poisoning, infections, disease, and weakening, and restores the "
               "consumers health by a small amount.";
    }

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_rElec: public Potion
{
public:
    Potion_rElec(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_rElec() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_rAcid: public Potion
{
public:
    Potion_rAcid(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_rAcid() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Acid Resistance";
    }

private:
    std::string descr_identified() const override
    {
        return "Protects the consumer from acid.";
    }

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_descent: public Potion
{
public:
    Potion_descent(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_descent() {}

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

    void collide_hook(const Pos& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};

class Potion_invis: public Potion
{
public:
    Potion_invis(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_invis() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "Invisibility";
    }

private:
    std::string descr_identified() const override
    {
        return "Makes the consumer invisible to normal vision.";
    }

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

class Potion_see_invis: public Potion
{
public:
    Potion_see_invis(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_see_invis() {}

    void quaff_impl(Actor& actor) override;

    const std::string real_name() const override
    {
        return "See Invisible";
    }

private:
    std::string descr_identified() const override
    {
        return "Grants the consumer extraordinary vision, including the ability to see that "
               "which is normally invisible.";
    }

    void collide_hook(const Pos& pos, Actor* const actor) override;
};

namespace potion_handling
{

struct Potion_look
{
    std::string name_plain;
    std::string name_a;
    Clr clr;
};

void init();

void save();
void load();

} //Potion_handling

#endif

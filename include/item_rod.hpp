#ifndef ITEM_ROD_HPP
#define ITEM_ROD_HPP

#include "item.hpp"
#include "spells.hpp"

class Rod: public Item
{
public:
    Rod(ItemData* const item_data) :
        Item                    (item_data),
        nr_charge_turns_left_   (0) {}

    virtual ~Rod() {}

    void save() override final;

    void load() override final;

    ConsumeItem activate(Actor* const actor) override final;

    Color interface_color() const override final
    {
        return colors::violet();
    }

    void on_std_turn_in_inv(const InvType inv_type) override final;

    std::vector<std::string> descr() const override final;

    void identify(const Verbosity verbosity) override final;

    virtual const std::string real_name() const = 0;

protected:
    virtual std::string descr_identified() const = 0;

    virtual void run_effect() = 0;

    virtual int nr_turns_to_recharge() const
    {
        return 250;
    }

    std::string name_inf() const override final;

    void set_max_charge_turns_left();

private:
    int nr_charge_turns_left_;
};

class RodCuring : public Rod
{
public:
    RodCuring(ItemData* const item_data) :
        Rod(item_data) {}

    ~RodCuring() {}

    const std::string real_name() const override
    {
        return "Curing";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device cures blindness, poisoning, "
            "infections, disease, weakening, and life sapping, and restores "
            "the user's health by a small amount.";
    }

    void run_effect() override;
};

class RodOpening : public Rod
{
public:
    RodOpening(ItemData* const item_data) :
        Rod(item_data) {}

    ~RodOpening() {}

    const std::string real_name() const override
    {
        return "Opening";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device opens all locks, lids and doors in "
            "the surrounding area (except heavy doors operated externally by "
            "a switch).";
    }

    void run_effect() override;
};

class RodBless : public Rod
{
public:
    RodBless(ItemData* const item_data) :
        Rod(item_data) {}

    ~RodBless() {}

    const std::string real_name() const override
    {
        return "Blessing";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device bends reality in favor of the "
            "user for a while.";
    }

    void run_effect() override;
};

class RodCloudMinds : public Rod
{
public:
    RodCloudMinds(ItemData* const item_data) :
        Rod(item_data) {}

    ~RodCloudMinds() {}

    const std::string real_name() const override
    {
        return "Cloud Minds";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device clouds the memories of all "
            "creatures in the area, causing them to forget the presence of "
            "the user.";
    }

    virtual int nr_turns_to_recharge() const override
    {
        return 90;
    }

    void run_effect() override;
};

class RodShockwave : public Rod
{
public:
    RodShockwave(ItemData* const item_data) :
        Rod(item_data) {}

    ~RodShockwave() {}

    const std::string real_name() const override
    {
        return "Shockwave";
    }

protected:
    std::string descr_identified() const override
    {
        return
            "When activated, this device generates a shock wave which "
            "violently pushes away any adjacent creatures and destroys "
            "structures.";
    }

    void run_effect() override;
};

namespace rod_handling
{

struct RodLook
{
    std::string name_plain;
    std::string name_a;
    Color color;
};

void init();

void save();
void load();

} // rod_handling

#endif // ITEM_ROD_HPP

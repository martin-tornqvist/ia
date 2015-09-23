#ifndef ITEM_ROD
#define ITEM_ROD

#include "item.hpp"
#include "spells.hpp"

class Rod: public Item
{
public:
    Rod(Item_data_t* const item_data) :
        Item                    (item_data),
        nr_charge_turns_left_   (0) {}

    virtual ~Rod() {}

    void save() override final;

    void load() override final;

    Consume_item activate(Actor* const actor) override final;

    Clr interface_clr() const override final
    {
        return clr_violet;
    }

    void on_std_turn_in_inv(const Inv_type inv_type) override final;

    std::vector<std::string> descr() const override final;

    void identify(const Verbosity verbosity) override final;

    virtual const std::string real_name() const = 0;

protected:
    virtual std::string descr_identified() const = 0;

    virtual void activate_impl() = 0;

    std::string name_inf() const override final;

private:
    int nr_charge_turns_left_;
};

class Rod_purge_invis : public Rod
{
public:
    Rod_purge_invis(Item_data_t* const item_data) :
        Rod(item_data) {}

    ~Rod_purge_invis() {}

    const std::string real_name() const override
    {
        return "Purge Invisible";
    }

protected:
    std::string descr_identified() const override
    {
        return "When activated, this device reveals any hidden or invisible creatures in the area "
               "around the user.";
    }

    void activate_impl() override;
};

class Rod_curing : public Rod
{
public:
    Rod_curing(Item_data_t* const item_data) :
        Rod(item_data) {}

    ~Rod_curing() {}

    const std::string real_name() const override
    {
        return "Curing";
    }

protected:
    std::string descr_identified() const override
    {
        return "When activated, this device cures any physical illness, and heals the users "
               "wounds by a small amount.";
    }

    void activate_impl() override;
};

class Rod_opening : public Rod
{
public:
    Rod_opening(Item_data_t* const item_data) :
        Rod(item_data) {}

    ~Rod_opening() {}

    const std::string real_name() const override
    {
        return "Opening";
    }

protected:
    std::string descr_identified() const override
    {
        return "When activated, this device opens all locks, lids and doors in the surrounding "
               "area.";
    }

    void activate_impl() override;
};

class Rod_bless : public Rod
{
public:
    Rod_bless(Item_data_t* const item_data) :
        Rod(item_data) {}

    ~Rod_bless() {}

    const std::string real_name() const override
    {
        return "Blessing";
    }

protected:
    std::string descr_identified() const override
    {
        return "When activated, this device bends the universe in favor of the caster for a while.";
    }

    void activate_impl() override;
};

namespace rod_handling
{

struct Rod_look
{
    std::string name_plain;
    std::string name_a;
    Clr clr;
};

void init();

void save();
void load();

} //rod_handling

#endif // ITEM_ROD

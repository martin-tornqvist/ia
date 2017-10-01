#ifndef ITEM_DEVICE_HPP
#define ITEM_DEVICE_HPP

#include <vector>
#include <string>

#include "item.hpp"

class Device: public Item
{
public:
    Device(ItemDataT* const item_data);

    virtual ~Device() {}

    virtual ConsumeItem activate(Actor* const actor) override = 0;

    Clr interface_clr() const override final
    {
        return clr_cyan;
    }

    virtual void on_std_turn_in_inv(const InvType inv_type) override
    {
        (void)inv_type;
    }

    void identify(const Verbosity verbosity) override;
};

class StrangeDevice : public Device
{
public:
    StrangeDevice(ItemDataT* const item_data);

    virtual std::vector<std::string> descr() const override final;

    ConsumeItem activate(Actor* const actor) override;

    virtual std::string name_inf() const override;

    virtual void save() override;
    virtual void load() override;

    Condition condition_;

private:
    virtual std::string descr_identified() const = 0;

    virtual ConsumeItem run_effect() = 0;
};

class DeviceBlaster : public StrangeDevice
{
public:
    DeviceBlaster(ItemDataT* const item_data) :
        StrangeDevice(item_data) {}

    ~DeviceBlaster() override {}

private:
    std::string descr_identified() const override
    {
        return
            "When activated, this device blasts all visible enemies with "
            "infernal power.";
    }

    ConsumeItem run_effect() override;
};

class DeviceShockwave : public StrangeDevice
{
public:
    DeviceShockwave(ItemDataT* const item_data) :
        StrangeDevice(item_data) {}

    ~DeviceShockwave() override {}

private:
    std::string descr_identified() const override
    {
        return
            "When activated, this device generates a shock wave which "
            "violently pushes away any adjacent creatures and destroys "
            "structures.";
    }

    ConsumeItem run_effect() override;
};

class DeviceRejuvenator : public StrangeDevice
{
public:
    DeviceRejuvenator(ItemDataT* const item_data) :
        StrangeDevice(item_data) {}

    ~DeviceRejuvenator() override {}

private:
    std::string descr_identified() const override
    {
        return
            "When activated, this device heals all wounds and physical "
            "maladies. The procedure is very painful and invasive "
            "however, and causes great shock to the user.";
    }

    ConsumeItem run_effect() override;
};

class DeviceTranslocator : public StrangeDevice
{
public:
    DeviceTranslocator(ItemDataT* const item_data) :
        StrangeDevice(item_data) {}

    ~DeviceTranslocator() override {}

private:
    std::string descr_identified() const override
    {
        return
            "When activated, this device teleports all visible enemies to "
            "different locations.";
    }

    ConsumeItem run_effect() override;
};

class DevicePurgeInvis : public StrangeDevice
{
public:
    DevicePurgeInvis(ItemDataT* const item_data) :
        StrangeDevice(item_data) {}

    ~DevicePurgeInvis() {}

private:
    std::string descr_identified() const override
    {
        return
            "When activated, this device reveals any hidden or invisible "
            "creatures in the area around the user.";
    }

    ConsumeItem run_effect() override;
};

class DeviceSentryDrone : public StrangeDevice
{
public:
    DeviceSentryDrone(ItemDataT* const item_data) :
        StrangeDevice(item_data) {}

    ~DeviceSentryDrone() override {}

private:
    std::string descr_identified() const override
    {
        return
            "When activated, this device will \"come alive\" and guard the "
            "user.";
    }

    ConsumeItem run_effect() override;
};

enum class LanternWorkingState
{
    working,
    flicker
};

class DeviceLantern : public Device
{
public:
    DeviceLantern(ItemDataT* const item_data);

    ~DeviceLantern() override {}

    ConsumeItem activate(Actor* const actor) override;
    void on_std_turn_in_inv(const InvType inv_type) override;
    void on_pickup_hook() override;

    LgtSize lgt_size() const override;

    void save() override;
    void load() override;

    int nr_turns_left_;
    int nr_flicker_turns_left_;
    LanternWorkingState working_state_;
    bool is_activated_;

private:
    void toggle();

    int random_nr_turns_to_next_bad_effect() const;

    std::string name_inf() const override;
};

#endif // ITEM_DEVICE_HPP

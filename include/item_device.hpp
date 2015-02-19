#ifndef ITEM_DEVICE_H
#define ITEM_DEVICE_H

#include <vector>
#include <string>

#include "Item.h"

class Device: public Item
{
public:
    Device(Item_data_t* const item_data);

    virtual ~Device() {}

    virtual Consume_item activate(Actor* const actor) override = 0;

    Clr get_interface_clr() const override final {return clr_cyan;}

    virtual void on_std_turn_in_inv(const Inv_type inv_type) override {(void)inv_type;}

    void identify(const bool IS_SILENT_IDENTIFY) override;
};

class Strange_device : public Device
{
public:
    Strange_device(Item_data_t* const item_data);

    virtual std::vector<std::string> get_descr() const override final;

    Consume_item activate(Actor* const actor) override;

    virtual std::string get_name_inf() const override;

    virtual void store_to_save_lines(std::vector<std::string>& lines)    override;
    virtual void setup_from_save_lines(std::vector<std::string>& lines)  override;

    Condition condition_;

private:
    virtual std::vector<std::string> get_descr_identified() const = 0;

    virtual Consume_item trigger_effect() = 0;
};

class Device_blaster : public Strange_device
{
public:
    Device_blaster(Item_data_t* const item_data) :
        Strange_device(item_data) {}

    ~Device_blaster() override {}

private:
    std::vector<std::string> get_descr_identified() const
    {
        return {"When activated, this strange device blasts all visible enemies with "
                "infernal power."
               };
    }

    Consume_item trigger_effect() override;
};

class Device_shockwave : public Strange_device
{
public:
    Device_shockwave(Item_data_t* const item_data) :
        Strange_device(item_data) {}

    ~Device_shockwave() override {}

private:
    std::vector<std::string> get_descr_identified() const
    {
        return {"When activated, this strange device generates a shock wave which violently "
                "pushes away any adjacent creatures and destroys structures."
               };
    }

    Consume_item trigger_effect() override;
};

class Device_rejuvenator : public Strange_device
{
public:
    Device_rejuvenator(Item_data_t* const item_data) :
        Strange_device(item_data) {}

    ~Device_rejuvenator() override {}

private:
    std::vector<std::string> get_descr_identified() const
    {
        return {"When activated, this strange device heals all wounds and physical "
                "maladies."
               };
    }

    Consume_item trigger_effect() override;
};

class Device_translocator : public Strange_device
{
public:
    Device_translocator(Item_data_t* const item_data) :
        Strange_device(item_data) {}

    ~Device_translocator() override {}

private:
    std::vector<std::string> get_descr_identified() const
    {
        return {"When activated, this strange device teleports all visible enemies to "
                "different locations."
               };
    }

    Consume_item trigger_effect() override;
};

class Device_sentry_drone : public Strange_device
{
public:
    Device_sentry_drone(Item_data_t* const item_data) :
        Strange_device(item_data) {}

    ~Device_sentry_drone() override {}

private:
    std::vector<std::string> get_descr_identified() const
    {
        return {"When activated, this strange device will \"come alive\" and guard the "
                "user."
               };
    }

    Consume_item trigger_effect() override;
};

enum class Lantern_working_state {working, flicker};

class Device_lantern : public Device
{
public:
    Device_lantern(Item_data_t* const item_data);

    ~Device_lantern() override {}

    Consume_item activate(Actor* const actor)            override;
    void          on_std_turn_in_inv(const Inv_type inv_type) override;
    void          on_pickup_to_backpack(Inventory& inv)    override;

    Lgt_size get_lgt_size() const override;

    void store_to_save_lines  (std::vector<std::string>& lines) override;
    void setup_from_save_lines(std::vector<std::string>& lines) override;

    int                 nr_turns_left_;
    int                 nr_flicker_turns_left_;
    Lantern_working_state working_state_;
    bool                is_activated_;

private:
    void toggle();

    int get_random_nr_turns_to_next_bad_effect() const;

    std::string get_name_inf() const override;
};

#endif

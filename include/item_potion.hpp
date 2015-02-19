#ifndef ITEM_POTION_H
#define ITEM_POTION_H

#include <vector>
#include <string>

#include "item.h"
#include "cmn_data.h"

class Potion: public Item
{
public:
    Potion(Item_data_t* const item_data) :
        Item(item_data) {}

    virtual ~Potion() {}

    Consume_item activate(Actor* const actor) override final;

    Clr get_interface_clr() const override final {return clr_blue_lgt;}

    std::vector<std::string> get_descr() const override final;

    void quaff(Actor& actor);

    void collide(const Pos& pos, Actor* actor);

    void identify(const bool IS_SILENT_IDENTIFY) override final;

    virtual const std::string get_real_name() const = 0;

protected:
    virtual std::vector<std::string> get_descr_identified() const = 0;

    virtual void collide_(const Pos& pos, Actor* const actor) = 0;

    virtual void quaff_(Actor& actor) = 0;

    std::string get_name_inf() const override final;
};

class Potion_vitality: public Potion
{
public:
    Potion_vitality(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_vitality() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Vitality";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"This elixir heals all wounds and physical maladies."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_spirit: public Potion
{
public:
    Potion_spirit(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_spirit() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Spirit";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Restores the spirit."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_blindness: public Potion
{
public:
    Potion_blindness(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_blindness() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Blindness";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Causes temporary loss of vision."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_paral: public Potion
{
public:
    Potion_paral(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_paral() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Paralyzation";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Causes paralysis."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_disease: public Potion
{
public:
    Potion_disease(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_disease() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Disease";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"This foul liquid causes a horrible disease."};
    }

    void collide_(const Pos& pos, Actor* const actor) override
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
    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Confusion";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Causes confusion."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_frenzy: public Potion
{
public:
    Potion_frenzy(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_frenzy() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Frenzy";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Causes uncontrollable fury."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_fortitude: public Potion
{
public:
    Potion_fortitude(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_fortitude() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Fortitude";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Gives the consumer complete peace of mind."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_poison: public Potion
{
public:
    Potion_poison(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_poison() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Poison";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"A deadly brew."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_insight: public Potion
{
public:
    Potion_insight(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_insight() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Insight";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"This strange concoction causes a sudden flash of intuition."};
    }

    void collide_(const Pos& pos, Actor* const actor) override
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

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Clairvoyance";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Bestows visions of the surrounding area."};
    }

    void collide_(const Pos& pos, Actor* const actor) override
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

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Fire Resistance";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Protects the consumer from fire."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_antidote: public Potion
{
public:
    Potion_antidote(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_antidote() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Antidote";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Cures poisoning."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_rElec: public Potion
{
public:
    Potion_rElec(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_rElec() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Insulation";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Protects the consumer from electricity."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_rAcid: public Potion
{
public:
    Potion_rAcid(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_rAcid() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Acid Resistance";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"Protects the consumer from acid."};
    }

    void collide_(const Pos& pos, Actor* const actor) override;
};

class Potion_descent: public Potion
{
public:
    Potion_descent(Item_data_t* const item_data) :
        Potion(item_data) {}
    ~Potion_descent() {}

    void quaff_(Actor& actor) override;

    const std::string get_real_name() const override {return "Descent";}

private:
    std::vector<std::string> get_descr_identified() const override
    {
        return {"A bizarre liquid that causes the consumer to dematerialize and sink "
                "through the ground."
               };
    }

    void collide_(const Pos& pos, Actor* const actor) override
    {
        (void)pos;
        (void)actor;
    }
};

struct Potion_look
{
    std::string name_plain;
    std::string name_a;
    Clr clr;
};

namespace Potion_handling
{

void init();

void store_to_save_lines  (std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

} //Potion_handling

#endif

#ifndef ITEM_H
#define ITEM_H

#include "art.h"
#include "inventory_handling.h"
#include "converters.h"
#include "cmn_data.h"

class Item_data_t;
class Prop;
class Actor;
class Spell;

enum class Item_activate_ret_type {keep, destroyed};

class Item
{
public:
    Item(Item_data_t* item_data);

    Item& operator=(Item& other) = delete;

    virtual ~Item();

    Item_id            get_id()       const;
    const Item_data_t&  get_data()     const;
    virtual Clr       get_clr()      const;
    char              get_glyph()    const;
    Tile_id            get_tile()     const;

    virtual Lgt_size   get_lgt_size()  const {return Lgt_size::none;}

    std::string get_name(const Item_ref_type   ref_type,
                        const Item_ref_inf    inf     = Item_ref_inf::yes,
                        const Item_ref_att_inf att_inf  = Item_ref_att_inf::none) const;

    virtual std::vector<std::string> get_descr() const;

    virtual void identify(const bool IS_SILENT_IDENTIFY) {(void)IS_SILENT_IDENTIFY;}

    virtual void store_to_save_lines  (std::vector<std::string>& lines)  {(void)lines;}
    virtual void setup_from_save_lines(std::vector<std::string>& lines)  {(void)lines;}

    virtual int get_weight() const;

    std::string get_weight_str() const;

    virtual Consume_item activate(Actor* const actor);

    virtual Clr get_interface_clr() const {return clr_brown;}

    virtual void            on_std_turn_in_inv    (const Inv_type inv_type) {(void)inv_type;}
    virtual void            on_actor_turn_in_inv  (const Inv_type inv_type) {(void)inv_type;}
    virtual void            on_pickup_to_backpack(Inventory& inv)    {(void)inv;}

    virtual void on_equip(const bool IS_SILENT)
    {
        (void)IS_SILENT;
    }

    virtual Unequip_allowed  on_unequip() {return Unequip_allowed::yes;}

    virtual int get_hp_regen_change(const Inv_type inv_type) const
    {
        (void)inv_type;
        return 0;
    }

    //Used when attempting to fire or throw an item
    bool is_in_effective_range_lmt(const Pos& p0, const Pos& p1) const;

    int nr_items_;

    //Properties to apply when wearing something like a ring of fire resistance
    std::vector<Prop*>  carrier_props_;

    //Spells granted to the carrier
    std::vector<Spell*> carrier_spells_;

    int melee_dmg_plus_;

protected:
    //E.g. "{Off}" for Lanterns, "{60}" for Medical Bags, or "4/7" for Pistols
    virtual std::string get_name_inf() const {return "";}

    Item_data_t* data_;
};

class Armor: public Item
{
public:
    Armor(Item_data_t* const item_data);

    ~Armor() {}

    void store_to_save_lines  (std::vector<std::string>& lines) override;
    void setup_from_save_lines(std::vector<std::string>& lines) override;

    Clr                     get_interface_clr() const override {return clr_gray;}
    virtual void            on_equip(const bool IS_SILENT)   override;
    virtual Unequip_allowed  on_unequip()                     override;

    int   get_durability()     const {return dur_;}
    void  set_max_durability()        {dur_ = 100;}
    bool  is_destroyed()       const {return get_armor_points() <= 0;}

    std::string get_armor_data_line(const bool WITH_BRACKETS) const;

    int take_dur_hit_and_get_reduced_dmg(const int DMG_BEFORE);

protected:
    int get_armor_points() const;

    virtual void on_equip_(const bool IS_SILENT)
    {
        (void)IS_SILENT;
    }
    virtual Unequip_allowed  on_unequip_() {return Unequip_allowed::yes;}

    std::string get_name_inf() const override {return get_armor_data_line(true);}

    int dur_;
};

class Armor_asb_suit: public Armor
{
public:
    Armor_asb_suit(Item_data_t* const item_data) : Armor(item_data) {}
    ~Armor_asb_suit() {}

private:
    void            on_equip_(const bool IS_SILENT)  override;
    Unequip_allowed  on_unequip_()                    override;
};

class Armor_heavy_coat: public Armor
{
public:
    Armor_heavy_coat(Item_data_t* const item_data) : Armor(item_data) {}
    ~Armor_heavy_coat() {}

private:
    void            on_equip_(const bool IS_SILENT)  override;
    Unequip_allowed  on_unequip_()                    override;
};

class Armor_mi_go: public Armor
{
public:
    Armor_mi_go(Item_data_t* const item_data) : Armor(item_data) {}
    ~Armor_mi_go() {}

    void on_std_turn_in_inv(const Inv_type inv_type) override;

private:
    void            on_equip_(const bool IS_SILENT)  override;
    Unequip_allowed  on_unequip_()                    override;
};

class Wpn: public Item
{
public:
    Wpn(Item_data_t* const item_data, Item_data_t* const ammo_data, int ammo_cap = 0,
        bool is_using_clip = false);

    virtual ~Wpn() {}

    Wpn& operator=(const Wpn& other) = delete;

    void set_random_melee_plus();

    void store_to_save_lines  (std::vector<std::string>& lines) override;
    void setup_from_save_lines(std::vector<std::string>& lines) override;

    Clr get_clr()          const override;
    Clr get_interface_clr() const override {return clr_gray;}

    const Item_data_t& get_ammo_data() {return *ammo_data_;}

    virtual void on_projectile_blocked(const Pos& pos, Actor* actor)
    {
        (void)pos;
        (void)actor;
    }

    const int   AMMO_CAP;
    const bool  IS_USING_CLIP;

    int nr_ammo_loaded;

protected:
    std::string get_name_inf() const override;

    Item_data_t* const ammo_data_;
};

class Pharaoh_staff: public Wpn
{
public:
    Pharaoh_staff(Item_data_t* const item_data);
};

class Sawed_off: public Wpn
{
public:
    Sawed_off(Item_data_t* const item_data, Item_data_t* const ammo_data) :
        Wpn(item_data, ammo_data, 2, false) {}
    ~Sawed_off() {}
};

class Pump_shotgun: public Wpn
{
public:
    Pump_shotgun(Item_data_t* const item_data, Item_data_t* const ammo_data) :
        Wpn(item_data, ammo_data, 8, false) {}
    ~Pump_shotgun() {}
};

class Pistol: public Wpn
{
public:
    Pistol(Item_data_t* const item_data, Item_data_t* const ammo_data) :
        Wpn(item_data, ammo_data, 7, true) {}
    ~Pistol() {}
};

class Flare_gun: public Wpn
{
public:
    Flare_gun(Item_data_t* const item_data, Item_data_t* const ammo_data) :
        Wpn(item_data, ammo_data, 1, false) {}
    ~Flare_gun() {}
};

class Machine_gun: public Wpn
{
public:
    Machine_gun(Item_data_t* const item_data, Item_data_t* const ammo_data);
    ~Machine_gun() {}
};

class Incinerator: public Wpn
{
public:
    Incinerator(Item_data_t* const item_data, Item_data_t* const ammo_data);
    void on_projectile_blocked(const Pos& pos, Actor* actor_hit);
    ~Incinerator() {}
};

class Mi_go_gun: public Wpn
{
public:
    Mi_go_gun(Item_data_t* const item_data, Item_data_t* const ammo_data);
    ~Mi_go_gun() {}
};

class Spike_gun: public Wpn
{
public:
    Spike_gun(Item_data_t* const item_data, Item_data_t* const ammo_data);
    ~Spike_gun() {}
};

class Ammo: public Item
{
public:
    Ammo(Item_data_t* const item_data) : Item(item_data) {}
    virtual ~Ammo() {}
    Clr get_interface_clr() const override {return clr_white;}
};

class Ammo_clip: public Ammo
{
public:
    Ammo_clip(Item_data_t* const item_data);

    ~Ammo_clip() {}

    int ammo_;

    void set_full_ammo();

    void store_to_save_lines(std::vector<std::string>& lines) override
    {
        lines.push_back(to_str(ammo_));
    }

    void setup_from_save_lines(std::vector<std::string>& lines)
    {
        ammo_ = to_int(lines.front());
        lines.erase(begin(lines));
    }

protected:
    std::string get_name_inf() const override {return "{" + to_str(ammo_) + "}";}
};

enum class Med_bag_action
{
    sanitize_infection,
//  take_morphine,
    treat_wounds,
    END
};

class Medical_bag: public Item
{
public:
    Medical_bag(Item_data_t* const item_data) :
        Item                    (item_data),
        nr_supplies_             (40),
        nr_turns_until_heal_wounds_ (-1),
        nr_turns_left_sanitize_    (-1),
        cur_action_              (Med_bag_action::END) {}

    ~Medical_bag() {}

    void on_pickup_to_backpack(Inventory& inv) override;

    Consume_item activate(Actor* const actor) override;

    void continue_action();
    void interrupted();
    void finish_cur_action();

    Clr get_interface_clr() const override {return clr_green;}

    void store_to_save_lines(std::vector<std::string>& lines) override
    {
        lines.push_back(to_str(nr_supplies_));
    }
    void setup_from_save_lines(std::vector<std::string>& lines) override
    {
        nr_supplies_ = to_int(lines.front());
        lines.erase(begin(lines));
    }

    int get_nr_supplies() const {return nr_supplies_;}

protected:
    Med_bag_action choose_action() const;

    int get_tot_turns_for_sanitize() const;
    int get_tot_suppl_for_sanitize() const;

    std::string get_name_inf() const override {return "{" + to_str(nr_supplies_) + "}";}

    int nr_supplies_;

    int nr_turns_until_heal_wounds_;
    int nr_turns_left_sanitize_;

    Med_bag_action cur_action_;
};

class Headwear: public Item
{
public:
    Headwear(Item_data_t* item_data) : Item(item_data) {}

    Clr get_interface_clr() const override {return clr_brown;}
};

//class Hideous_mask: public Headwear
//{
//public:
//    Hideous_mask(Item_data_t* item_data);
//
//    void on_std_turn_in_inv(const Inv_type inv_type) override;
//};

class Gas_mask: public Headwear
{
public:
    Gas_mask(Item_data_t* item_data) : Headwear(item_data), nr_turns_left_(60) {}

    void            on_equip(const bool IS_SILENT)   override;
    Unequip_allowed  on_unequip()                     override;

    void            decr_turns_left(Inventory& carrier_inv);

protected:
    std::string get_name_inf() const override {return "{" + to_str(nr_turns_left_) + "}";}

    int nr_turns_left_;
};

class Explosive : public Item
{
public:
    virtual ~Explosive() {}

    Explosive() = delete;

    Consume_item activate(Actor* const actor) override final;
    Clr get_interface_clr() const override final {return clr_red_lgt;}

    virtual void        on_std_turn_player_hold_ignited()          = 0;
    virtual void        on_thrown_ignited_landing(const Pos& p)  = 0;
    virtual void        on_player_paralyzed()                   = 0;
    virtual Clr         get_ignited_projectile_clr() const       = 0;
    virtual std::string get_str_on_player_throw()     const       = 0;

protected:
    Explosive(Item_data_t* const item_data) : Item(item_data), fuse_turns_(-1) {}

    virtual int   get_std_fuse_turns() const = 0;
    virtual void  on_player_ignite()  const = 0;

    int fuse_turns_;
};

class Dynamite: public Explosive
{
public:
    Dynamite(Item_data_t* const item_data) : Explosive(item_data) {}

    void        on_thrown_ignited_landing(const Pos& p)  override;
    void        on_std_turn_player_hold_ignited()          override;
    void        on_player_paralyzed()                   override;
    Clr         get_ignited_projectile_clr()       const override {return clr_red_lgt;}
    std::string get_str_on_player_throw()           const override
    {
        return "I throw a lit dynamite stick.";
    }

protected:
    int get_std_fuse_turns() const override {return 6;}
    void on_player_ignite() const override;
};

class Molotov: public Explosive
{
public:
    Molotov(Item_data_t* const item_data) : Explosive(item_data) {}

    void        on_thrown_ignited_landing(const Pos& p)  override;
    void        on_std_turn_player_hold_ignited()          override;
    void        on_player_paralyzed()                   override;
    Clr         get_ignited_projectile_clr()       const override {return clr_yellow;}
    std::string get_str_on_player_throw()           const override
    {
        return "I throw a lit Molotov Cocktail.";
    }

protected:
    int get_std_fuse_turns() const override {return 12;}
    void on_player_ignite() const override;
};

class Flare: public Explosive
{
public:
    Flare(Item_data_t* const item_data) : Explosive(item_data) {}

    void        on_thrown_ignited_landing(const Pos& p)  override;
    void        on_std_turn_player_hold_ignited()          override;
    void        on_player_paralyzed()                   override;
    Clr get_ignited_projectile_clr()               const override {return clr_yellow;}
    std::string get_str_on_player_throw()           const override
    {
        return "I throw a lit flare.";
    }

protected:
    int get_std_fuse_turns() const override {return 200;}
    void on_player_ignite() const override;
};

class Smoke_grenade: public Explosive
{
public:
    Smoke_grenade(Item_data_t* const item_data) : Explosive(item_data) {}

    void        on_thrown_ignited_landing(const Pos& p)  override;
    void        on_std_turn_player_hold_ignited()          override;
    void        on_player_paralyzed()                   override;
    Clr         get_ignited_projectile_clr()       const override;
    std::string get_str_on_player_throw()           const override
    {
        return "I throw a smoke grenade.";
    }

protected:
    int get_std_fuse_turns() const override {return 12;}
    void on_player_ignite() const override;
};

#endif

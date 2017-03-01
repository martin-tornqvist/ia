#ifndef ITEM_HPP
#define ITEM_HPP

#include "art.hpp"
#include "inventory_handling.hpp"
#include "rl_utils.hpp"

class ItemDataT;
class Prop;
class Actor;
class Spell;

enum class ItemActivateRetType
{
    keep,
    destroyed
};

class Item
{
public:
    Item(ItemDataT* item_data);

    Item(Item& other) = delete;

    Item& operator=(const Item& other);

    virtual ~Item();

    ItemId id() const;

    const ItemDataT& data() const;

    virtual Clr clr() const;

    char glyph() const;

    TileId tile() const;

    virtual LgtSize lgt_size() const
    {
        return LgtSize::none;
    }

    std::string name(const ItemRefType ref_type,
                     const ItemRefInf inf = ItemRefInf::yes,
                     const ItemRefAttInf att_inf = ItemRefAttInf::none) const;

    virtual std::vector<std::string> descr() const;

    virtual void identify(const Verbosity verbosity)
    {
        (void)verbosity;
    }

    virtual void save() {}

    virtual void load() {}

    DiceParam dmg(const AttMode att_mode, const Actor* const actor) const;

    virtual int weight() const;

    std::string weight_str() const;

    virtual ConsumeItem activate(Actor* const actor);

    virtual Clr interface_clr() const
    {
        return clr_yellow_drk;
    }

    virtual void on_std_turn_in_inv(const InvType inv_type)
    {
        (void)inv_type;
    }

    virtual void on_actor_turn_in_inv(const InvType inv_type)
    {
        (void)inv_type;
    }

    void on_pickup(Actor& actor);

    // "on_pickup()" should be called before this
    void on_equip(const Verbosity verbosity);

    UnequipAllowed on_unequip();

    // This is the opposite of "on_pickup()". If this is a wielded item,
    // "on_unequip()" should be called first.
    void on_removed_from_inv();

    // Called when:
    // * Player walks into the same cell as the item,
    // * The item is dropped into the same cell as the player,
    // * The item is picked up,
    // * The item is found in an item container, but not picked up
    void on_found();

    virtual void on_projectile_blocked(const P& pos, Actor* actor)
    {
        (void)pos;
        (void)actor;
    }

    virtual void on_melee_hit(Actor& actor_hit, const int dmg)
    {
        (void)actor_hit;
        (void)dmg;
    }

    virtual void on_melee_kill(Actor& actor_killed)
    {
        (void)actor_killed;
    }

    virtual void on_ranged_hit(Actor& actor_hit)
    {
        (void)actor_hit;
    }

    void add_carrier_prop(Prop* const prop, const Verbosity verbosity);

    void clear_carrier_props();

    virtual int hp_regen_change(const InvType inv_type) const
    {
        (void)inv_type;
        return 0;
    }

    // Used when attempting to fire or throw an item
    bool is_in_effective_range_lmt(const P& p0, const P& p1) const;

    Actor* actor_carrying()
    {
        return actor_carrying_;
    }

    void clear_actor_carrying()
    {
        actor_carrying_ = nullptr;
    }

    const std::vector<Prop*>& carrier_props() const
    {
        return carrier_props_;
    }

    void add_carrier_spell(Spell* const spell);

    void clear_carrier_spells();

    const std::vector<Spell*>& carrier_spells() const
    {
        return carrier_spells_;
    }

    void give_xp_for_identify(const Verbosity verbosity = Verbosity::verbose);

    int nr_items_;

    int melee_dmg_plus_;

protected:
    // E.g. "{Off}" for Lanterns, "{60}" for Medical Bags, or "4/7" for Pistols
    virtual std::string name_inf() const
    {
        return "";
    }

    virtual void on_pickup_hook() {}

    virtual void on_equip_hook(const Verbosity verbosity)
    {
        (void)verbosity;
    }

    virtual UnequipAllowed on_unequip_hook()
    {
        return UnequipAllowed::yes;
    }

    virtual void on_removed_from_inv_hook() {}

    virtual void specific_dmg_mod(DiceParam& dice,
                                  const Actor* const actor) const
    {
        (void)dice;
        (void)actor;
    }

    ItemDataT* data_;

    Actor* actor_carrying_;

private:
    // Properties to apply when wearing something like a ring of fire resistance
    std::vector<Prop*> carrier_props_;

    // Spells granted to the carrier
    std::vector<Spell*> carrier_spells_;
};

class Armor: public Item
{
public:
    Armor(ItemDataT* const item_data);

    ~Armor() {}

    void save() override;
    void load() override;

    Clr interface_clr() const override
    {
        return clr_gray;
    }

    int durability() const
    {
        return dur_;
    }

    void set_max_durability()
    {
        dur_ = 100;
    }

    bool is_destroyed() const
    {
        return armor_points() <= 0;
    }

    std::string armor_points_str(const bool with_brackets) const;

    int take_dur_hit_and_get_reduced_dmg(const int dmg_before);

protected:
    int armor_points() const;

    virtual UnequipAllowed on_unequip_hook() override
    {
        return UnequipAllowed::yes;
    }

    std::string name_inf() const override
    {
        return armor_points_str(true);
    }

    int dur_;
};

class ArmorAsbSuit: public Armor
{
public:
    ArmorAsbSuit(ItemDataT* const item_data) :
        Armor(item_data) {}

    ~ArmorAsbSuit() {}

    void on_equip_hook(const Verbosity verbosity) override;

private:
    UnequipAllowed on_unequip_hook() override;
};

class ArmorMiGo: public Armor
{
public:
    ArmorMiGo(ItemDataT* const item_data) :
        Armor(item_data) {}

    ~ArmorMiGo() {}

    void on_equip_hook(const Verbosity verbosity) override;

private:

    UnequipAllowed on_unequip_hook() override;
};

class Wpn: public Item
{
public:
    Wpn(ItemDataT* const item_data);

    virtual ~Wpn() {}

    Wpn& operator=(const Wpn& other) = delete;

    void save() override;
    void load() override;

    Clr clr() const override;

    Clr interface_clr() const override
    {
        return clr_gray;
    }

    void set_random_melee_plus();

    const ItemDataT& ammo_data()
    {
        return *ammo_data_;
    }

    int nr_ammo_loaded_;

protected:
    std::string name_inf() const override;

    ItemDataT* ammo_data_;
};

class PlayerGhoulClaw : public Wpn
{
public:
    PlayerGhoulClaw(ItemDataT* const item_data) :
        Wpn(item_data) {}

private:
    void on_melee_hit(Actor& actor_hit, const int dmg) override;

    void on_melee_kill(Actor& actor_killed) override;
};

class ZombieDust : public Wpn
{
public:
    ZombieDust(ItemDataT* const item_data) :
        Wpn(item_data) {}

    void on_ranged_hit(Actor& actor_hit);
};

class SawedOff: public Wpn
{
public:
    SawedOff(ItemDataT* const item_data);

    ~SawedOff() {}
};

class PumpShotgun: public Wpn
{
public:
    PumpShotgun(ItemDataT* const item_data);
    ~PumpShotgun() {}
};

class Pistol: public Wpn
{
public:
    Pistol(ItemDataT* const item_data);
    ~Pistol() {}
};

class FlareGun: public Wpn
{
public:
    FlareGun(ItemDataT* const item_data);
    ~FlareGun() {}
};

class MachineGun: public Wpn
{
public:
    MachineGun(ItemDataT* const item_data);
    ~MachineGun() {}
};

class Incinerator: public Wpn
{
public:
    Incinerator(ItemDataT* const item_data);
    void on_projectile_blocked(const P& pos, Actor* actor_hit);
    ~Incinerator() {}
};

class MiGoGun: public Wpn
{
public:
    MiGoGun(ItemDataT* const item_data);
    ~MiGoGun() {}

protected:
    void specific_dmg_mod(DiceParam& dice,
                          const Actor* const actor) const override;
};

class SpikeGun: public Wpn
{
public:
    SpikeGun(ItemDataT* const item_data);
    ~SpikeGun() {}
};

class RavenPeck : public Wpn
{
public:
    RavenPeck(ItemDataT* const item_data) :
        Wpn(item_data) {}

    void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class VampireBatBite : public Wpn
{
public:
    VampireBatBite(ItemDataT* const item_data) :
        Wpn(item_data) {}

    void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class DustVortexEngulf : public Wpn
{
public:
    DustVortexEngulf(ItemDataT* const item_data) :
        Wpn(item_data) {}

    void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class SpittingCobraSpit : public Wpn
{
public:
    SpittingCobraSpit(ItemDataT* const item_data) :
        Wpn(item_data) {}

    void on_ranged_hit(Actor& actor_hit) override;
};

class Ammo: public Item
{
public:
    Ammo(ItemDataT* const item_data) : Item(item_data) {}
    virtual ~Ammo() {}

    Clr interface_clr() const override
    {
        return clr_white;
    }
};

class AmmoMag: public Ammo
{
public:
    AmmoMag(ItemDataT* const item_data);

    ~AmmoMag() {}

    int ammo_;

    void set_full_ammo();

    void save() override;

    void load() override;

protected:
    std::string name_inf() const override
    {
        return "{" + std::to_string(ammo_) + "}";
    }
};

enum class MedBagAction
{
    treat_wound,
    sanitize_infection,
    END
};

class MedicalBag: public Item
{
public:
    MedicalBag(ItemDataT* const item_data);

    ~MedicalBag() {}

    void save() override;

    void load() override;

    void on_pickup_hook() override;

    ConsumeItem activate(Actor* const actor) override;

    void continue_action();

    void interrupted();

    void finish_current_action();

    Clr interface_clr() const override
    {
        return clr_green;
    }

    int nr_supplies_;

protected:
    MedBagAction choose_action() const;

    int tot_suppl_for_action(const MedBagAction action) const;

    int tot_turns_for_action(const MedBagAction action) const;

    std::string name_inf() const override
    {
        return "{" + std::to_string(nr_supplies_) + "}";
    }

    int nr_turns_left_action_;

    MedBagAction current_action_;
};

class Headwear: public Item
{
public:
    Headwear(ItemDataT* item_data) :
        Item(item_data) {}

    Clr interface_clr() const override
    {
        return clr_brown;
    }
};

// class HideousMask: public Headwear
// {
// public:
// HideousMask(ItemDataT* item_data);
//
// void on_std_turn_in_inv(const InvType inv_type) override;
// };

class GasMask: public Headwear
{
public:
    GasMask(ItemDataT* item_data) :
        Headwear        (item_data),
        nr_turns_left_  (60) {}

    void on_equip_hook(const Verbosity verbosity) override;

    UnequipAllowed on_unequip_hook() override;

    void decr_turns_left(Inventory& carrier_inv);

protected:
    std::string name_inf() const override
    {
        return "{" + std::to_string(nr_turns_left_) + "}";
    }

    int nr_turns_left_;
};

class Explosive : public Item
{
public:
    virtual ~Explosive() {}

    Explosive() = delete;

    ConsumeItem activate(Actor* const actor) override final;
    Clr interface_clr() const override final
    {
        return clr_red_lgt;
    }

    virtual void on_std_turn_player_hold_ignited() = 0;
    virtual void on_thrown_ignited_landing(const P& p) = 0;
    virtual void on_player_paralyzed() = 0;
    virtual Clr ignited_projectile_clr() const = 0;
    virtual std::string str_on_player_throw() const = 0;

protected:
    Explosive(ItemDataT* const item_data) :
        Item(item_data),
        fuse_turns_(-1) {}

    virtual int std_fuse_turns() const = 0;
    virtual void on_player_ignite() const = 0;

    int fuse_turns_;
};

class Dynamite: public Explosive
{
public:
    Dynamite(ItemDataT* const item_data) :
        Explosive(item_data) {}

    void on_thrown_ignited_landing(const P& p) override;
    void on_std_turn_player_hold_ignited() override;
    void on_player_paralyzed() override;

    Clr ignited_projectile_clr() const override
    {
        return clr_red_lgt;
    }

    std::string str_on_player_throw() const override
    {
        return "I throw a lit dynamite stick.";
    }

protected:
    int std_fuse_turns() const override
    {
        return 6;
    }

    void on_player_ignite() const override;
};

class Molotov: public Explosive
{
public:
    Molotov(ItemDataT* const item_data) : Explosive(item_data) {}

    void on_thrown_ignited_landing(const P& p) override;
    void on_std_turn_player_hold_ignited() override;
    void on_player_paralyzed() override;

    Clr ignited_projectile_clr() const override
    {
        return clr_yellow;
    }

    std::string str_on_player_throw() const override
    {
        return "I throw a lit Molotov Cocktail.";
    }

protected:
    int std_fuse_turns() const override
    {
        return 12;
    }
    void on_player_ignite() const override;
};

class Flare: public Explosive
{
public:
    Flare(ItemDataT* const item_data) : Explosive(item_data) {}

    void on_thrown_ignited_landing(const P& p) override;
    void on_std_turn_player_hold_ignited() override;
    void on_player_paralyzed() override;

    Clr ignited_projectile_clr() const override
    {
        return clr_yellow;
    }

    std::string str_on_player_throw() const override
    {
        return "I throw a lit flare.";
    }

protected:
    int std_fuse_turns() const override
    {
        return 200;
    }
    void on_player_ignite() const override;
};

class SmokeGrenade: public Explosive
{
public:
    SmokeGrenade(ItemDataT* const item_data) : Explosive(item_data) {}

    void on_thrown_ignited_landing(const P& p) override;
    void on_std_turn_player_hold_ignited() override;
    void on_player_paralyzed() override;
    Clr ignited_projectile_clr() const override;

    std::string str_on_player_throw() const override
    {
        return "I throw a smoke grenade.";
    }

protected:
    int std_fuse_turns() const override
    {
        return 12;
    }

    void on_player_ignite() const override;
};

#endif // ITEM_HPP

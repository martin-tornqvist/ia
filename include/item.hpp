#ifndef ITEM_HPP
#define ITEM_HPP

#include "gfx.hpp"
#include "inventory_handling.hpp"
#include "rl_utils.hpp"
#include "item_att_property.hpp"

class ItemData;
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
        Item(ItemData* item_data);

        Item(Item& other) = delete;

        Item& operator=(const Item& other);

        virtual ~Item();

        ItemId id() const;

        ItemData& data() const;

        virtual Color color() const;

        char character() const;

        TileId tile() const;

        virtual LgtSize lgt_size() const
        {
                return LgtSize::none;
        }

        std::string name(
                const ItemRefType ref_type,
                const ItemRefInf inf = ItemRefInf::yes,
                const ItemRefAttInf att_inf = ItemRefAttInf::none) const;

        virtual std::vector<std::string> descr() const;

        std::string dmg_str(const ItemRefAttInf att_inf,
                            const ItemRefDmg dmg_value) const;

        virtual void identify(const Verbosity verbosity)
        {
                (void)verbosity;
        }

        virtual void save() {}

        virtual void load() {}

        virtual int weight() const;

        std::string weight_str() const;

        virtual ConsumeItem activate(Actor* const actor);

        virtual Color interface_color() const
        {
                return colors::dark_yellow();
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

        void on_unequip();

        // This is the opposite of "on_pickup()". If this is a wielded item,
        // "on_unequip()" should be called first.
        void on_removed_from_inv();

        // Called when:
        // * Player walks into the same cell as the item,
        // * The item is dropped into the same cell as the player,
        // * The item is picked up,
        // * The item is found in an item container, but not picked up
        void on_player_found();

        virtual void on_player_reached_new_dlvl() {}

        virtual void on_projectile_blocked(const P prev_pos,
                                           const P current_pos)
        {
                (void)prev_pos;
                (void)current_pos;
        }

        virtual void on_melee_hit(Actor& actor_hit, const int dmg)
        {
                (void)actor_hit;
                (void)dmg;
        }

        Dice melee_dmg(const Actor* const attacker) const;
        Dice ranged_dmg(const Actor* const attacker) const;
        Dice thrown_dmg(const Actor* const attacker) const;

        ItemAttProp prop_applied_on_melee(const Actor* const attacker) const;
        ItemAttProp prop_applied_on_ranged(const Actor* const attacker) const;

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

        int nr_items_;

        // Base damage (not including actor properties, player traits, etc)
        Dice melee_base_dmg_;
        Dice ranged_base_dmg_;

protected:
        // E.g. "{Off}" for Lanterns, or "4/7" for Pistols
        virtual std::string name_inf() const
        {
                return "";
        }

        virtual void on_pickup_hook() {}

        virtual void on_equip_hook(const Verbosity verbosity)
        {
                (void)verbosity;
        }

        virtual void on_unequip_hook() {}

        virtual void on_removed_from_inv_hook() {}

        virtual void specific_dmg_mod(Dice& dice,
                                      const Actor* const actor) const
        {
                (void)dice;
                (void)actor;
        }

        ItemAttProp prop_applied_intr_attack(const Actor* const attacker) const;

        ItemData* data_;

        Actor* actor_carrying_;

private:
        // Properties to apply on owning actor (when e.g. wearing the item, or
        // just keeping it in the inventory)
        std::vector<Prop*> carrier_props_;

        // Spells granted to the carrier
        std::vector<Spell*> carrier_spells_;
};

class Armor: public Item
{
public:
        Armor(ItemData* const item_data);

        ~Armor() {}

        void save() override;
        void load() override;

        Color interface_color() const override
        {
                return colors::gray();
        }

        int armor_points() const;

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

        void hit(const int dmg);

protected:
        std::string name_inf() const override;

        int dur_;
};

class ArmorAsbSuit: public Armor
{
public:
        ArmorAsbSuit(ItemData* const item_data) :
                Armor(item_data) {}

        ~ArmorAsbSuit() {}

        void on_equip_hook(const Verbosity verbosity) override;

private:
        void on_unequip_hook() override;
};

class ArmorMiGo: public Armor
{
public:
        ArmorMiGo(ItemData* const item_data) :
                Armor(item_data) {}

        ~ArmorMiGo() {}

        void on_equip_hook(const Verbosity verbosity) override;
};

class Wpn: public Item
{
public:
        Wpn(ItemData* const item_data);

        virtual ~Wpn() {}

        Wpn& operator=(const Wpn& other) = delete;

        void save() override;
        void load() override;

        Color color() const override;

        Color interface_color() const override
        {
                return colors::gray();
        }

        void set_random_melee_plus();

        const ItemData& ammo_data()
        {
                return *ammo_data_;
        }

        int ammo_loaded_;

protected:
        std::string name_inf() const override;

        ItemData* ammo_data_;
};

class SpikedMace : public Wpn
{
public:
        SpikedMace(ItemData* const item_data) :
                Wpn(item_data) {}

private:
        void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class PlayerGhoulClaw : public Wpn
{
public:
        PlayerGhoulClaw(ItemData* const item_data) :
                Wpn(item_data) {}

private:
        void on_melee_hit(Actor& actor_hit, const int dmg) override;

        void on_melee_kill(Actor& actor_killed) override;
};

class ZombieDust : public Wpn
{
public:
        ZombieDust(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_ranged_hit(Actor& actor_hit);
};

class Incinerator: public Wpn
{
public:
        Incinerator(ItemData* const item_data);
        ~Incinerator() {}

        void on_projectile_blocked(const P prev_pos,
                                   const P current_pos);
};

class MiGoGun: public Wpn
{
public:
        MiGoGun(ItemData* const item_data);
        ~MiGoGun() {}

protected:
        void specific_dmg_mod(Dice& dice,
                              const Actor* const actor) const override;
};

class RavenPeck : public Wpn
{
public:
        RavenPeck(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class VampiricBite : public Wpn
{
public:
        VampiricBite(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class MindLeechSting : public Wpn
{
public:
        MindLeechSting(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class SpiritLeechSting : public Wpn
{
public:
        SpiritLeechSting(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class LifeLeechSting : public Wpn
{
public:
        LifeLeechSting(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class DustEngulf : public Wpn
{
public:
        DustEngulf(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_melee_hit(Actor& actor_hit, const int dmg) override;
};

class SnakeVenomSpit : public Wpn
{
public:
        SnakeVenomSpit(ItemData* const item_data) :
                Wpn(item_data) {}

        void on_ranged_hit(Actor& actor_hit) override;
};

class Ammo: public Item
{
public:
        Ammo(ItemData* const item_data) : Item(item_data) {}
        virtual ~Ammo() {}

        Color interface_color() const override
        {
                return colors::white();
        }
};

class AmmoMag: public Ammo
{
public:
        AmmoMag(ItemData* const item_data);

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
        MedicalBag(ItemData* const item_data);

        ~MedicalBag() {}

        void save() override;

        void load() override;

        void on_pickup_hook() override;

        ConsumeItem activate(Actor* const actor) override;

        void continue_action();

        void interrupted();

        void finish_current_action();

        Color interface_color() const override
        {
                return colors::green();
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
        Headwear(ItemData* item_data) :
                Item(item_data) {}

        Color interface_color() const override
        {
                return colors::brown();
        }
};

class GasMask: public Headwear
{
public:
        GasMask(ItemData* item_data) :
                Headwear        (item_data),
                nr_turns_left_  (60) {}

        void on_equip_hook(const Verbosity verbosity) override;

        void on_unequip_hook() override;

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

        Color interface_color() const override final
        {
                return colors::light_red();
        }

        virtual void on_std_turn_player_hold_ignited() = 0;
        virtual void on_thrown_ignited_landing(const P& p) = 0;
        virtual void on_player_paralyzed() = 0;
        virtual Color ignited_projectile_color() const = 0;
        virtual std::string str_on_player_throw() const = 0;

protected:
        Explosive(ItemData* const item_data) :
                Item(item_data),
                fuse_turns_(-1) {}

        virtual int std_fuse_turns() const = 0;
        virtual void on_player_ignite() const = 0;

        int fuse_turns_;
};

class Dynamite: public Explosive
{
public:
        Dynamite(ItemData* const item_data) :
                Explosive(item_data) {}

        void on_thrown_ignited_landing(const P& p) override;
        void on_std_turn_player_hold_ignited() override;
        void on_player_paralyzed() override;

        Color ignited_projectile_color() const override
        {
                return colors::light_red();
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
        Molotov(ItemData* const item_data) : Explosive(item_data) {}

        void on_thrown_ignited_landing(const P& p) override;
        void on_std_turn_player_hold_ignited() override;
        void on_player_paralyzed() override;

        Color ignited_projectile_color() const override
        {
                return colors::yellow();
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
        Flare(ItemData* const item_data) : Explosive(item_data) {}

        void on_thrown_ignited_landing(const P& p) override;
        void on_std_turn_player_hold_ignited() override;
        void on_player_paralyzed() override;

        Color ignited_projectile_color() const override
        {
                return colors::yellow();
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
        SmokeGrenade(ItemData* const item_data) : Explosive(item_data) {}

        void on_thrown_ignited_landing(const P& p) override;
        void on_std_turn_player_hold_ignited() override;
        void on_player_paralyzed() override;

        Color ignited_projectile_color() const override;

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

#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include <vector>
#include <string>

#include "global.hpp"

class Item;
class Actor;

enum class ItemId;

enum class SlotId
{
    wpn,
    wpn_alt,
    thrown,
    body,
    head,
    END
};

struct InvSlot
{
    InvSlot(SlotId id_, std::string name_) :
        id      (id_),
        name    (name_),
        item    (nullptr) {}

    InvSlot() :
        id      (SlotId::wpn),
        name    (""),
        item    (nullptr) {}

    SlotId id;
    std::string name;
    Item* item;
};

class Inventory
{
public:
    Inventory(Actor* const owning_actor);

    ~Inventory();

    void save() const;
    void load();

    // Equip item from backpack
    void equip_backpack_item(const size_t backpack_idx,
                             const SlotId slot_id);

    // Attempt to unequip item from slot
    UnequipAllowed try_unequip_slot(const SlotId id);

    //
    // NOTE: The "put_in_*" functions should NEVER be called on items already in
    //       the inventory.
    void put_in_slot(const SlotId id,
                     Item* item,
                     Verbosity verbosity);

    void put_in_backpack(Item* item);

    void put_in_intrinsics(Item* item);

    void drop_all_non_intrinsic(const P& pos);

    void swap_wielded_and_prepared();

    bool has_item_in_slot(SlotId id) const;

    bool has_ammo_for_firearm_in_inventory();

    Item* item_in_backpack(const ItemId id);

    int backpack_idx(const ItemId item_id) const;

    Item* item_in_slot(const SlotId id) const;

    void remove_without_destroying(const InvType inv_type,
                                   const size_t idx);

    void decr_item_in_slot(SlotId slot_id);

    void decr_item_in_backpack(const size_t idx);

    void decr_item(Item* const item);

    void decr_item_type_in_backpack(const ItemId item_id);

    Item* remove_item_in_slot(const SlotId slot_id,
                              const bool delete_item);

    Item* remove_item_in_backpack_with_idx(const size_t idx,
                                           const bool delete_item);

    Item* remove_item_in_backpack_with_ptr(Item* const item,
                                           const bool delete_item);

    int intrinsics_size() const
    {
        return intrinsics_.size();
    }

    Item* intrinsic_in_element(const int idx) const;

    Item* last_item_in_backpack();

    bool has_item_in_backpack(const ItemId id) const;

    int item_stack_size_in_backpack(const ItemId id) const;

    void sort_backpack();

    int total_item_weight() const;

    InvSlot slots_[(size_t)SlotId::END];
    std::vector<Item*> backpack_;
    std::vector<Item*> intrinsics_;

private:
    // Puts the item in the slot, and prints messages
    void equip(const SlotId id,
               Item* const item,
               Verbosity verbosity);

    void equip_from_backpack(const SlotId id,
                             const size_t backpack_idx);

    UnequipAllowed try_move_from_slot_to_backpack(const SlotId id);

    // Checks if the item is stackable, and if so attempts to stack it with
    // another item of the same type in the backpack. The item pointer is still
    // valid if a stack occurs (it is the other item that gets destroyed)
    bool try_stack_in_backpack(Item* item);

    Actor* const owning_actor_;
};

#endif // INVENTORY_HPP

#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <string>

#include "cmn_types.hpp"

class Item;
class Actor;

enum class Item_id;

enum class Slot_id
{
    wpn,
    wpn_alt,
    thrown,
    body,
    head,
    neck,
    END
};

struct Inv_slot
{
    Inv_slot(Slot_id id_, std::string name_) :
        id      (id_),
        name    (name_),
        item    (nullptr) {}

    Inv_slot() :
        id      (Slot_id::wpn),
        name    (""),
        item    (nullptr) {}

    Slot_id id;
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

    //Equip item from backpack
    void equip_backpack_item(const size_t BACKPACK_IDX, const Slot_id slot_id);

    //Attempt to unequip item from slot
    Unequip_allowed try_unequip_slot(const Slot_id id);

    //NOTE: All "put_in_*" functions should NEVER be called on items already in the inventory.

    //NOTE: Item will be put in backpack if the slot is occupied.
    void put_in_slot(const Slot_id id, Item* item);

    Item* remove_from_slot(const Slot_id id);

    void put_in_backpack(Item* item);

    void put_in_intrinsics(Item* item);

    void drop_all_non_intrinsic(const P& pos);

    void swap_wielded_and_prepared(const Pass_time pass_time);

    bool has_item_in_slot(Slot_id id) const;

    bool has_ammo_for_firearm_in_inventory();

    Item* item_in_backpack(const Item_id id);

    int backpack_idx(const Item_id item_id) const;

    Item* item_in_slot(const Slot_id id) const;

    void remove_without_destroying(const Inv_type inv_type, const size_t IDX);

    void decr_item_in_slot(Slot_id slot_id);

    void decr_item_in_backpack(const size_t IDX);

    void decr_item(Item* const item);

    void decr_item_type_in_backpack(const Item_id item_id);

    Item* remove_item_in_backpack_with_idx(const size_t IDX, const bool DELETE_ITEM);
    Item* remove_item_in_backpack_with_ptr(Item* const item, const bool DELETE_ITEM);

    int intrinsics_size() const
    {
        return intrinsics_.size();
    }

    Item* intrinsic_in_element(const int IDX) const;

    Item* last_item_in_backpack();

    bool has_item_in_backpack(const Item_id id) const;

    int item_stack_size_in_backpack(const Item_id id) const;

    void sort_backpack();

    int total_item_weight() const;

    Inv_slot slots_[int(Slot_id::END)];
    std::vector<Item*> backpack_;
    std::vector<Item*> intrinsics_;

private:
    void move_from_backpack_to_slot(const Slot_id id, const size_t BACKPACK_IDX);

    Unequip_allowed try_move_from_slot_to_backpack(const Slot_id id);

    //This checks if the item is stackable, and if so attempts to stack it with another item
    //of the same type in the backpack. The item pointer is still valid if a stack occurs
    //(it is the other item that gets destroyed)
    bool try_stack_in_backpack(Item* item);

    Actor* const owning_actor_;
};

#endif

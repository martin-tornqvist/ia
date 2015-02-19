#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include <string>

#include "cmn_types.h"

class Item;

enum class Item_id;

enum class Slot_id
{
    wielded,
    wielded_alt,
    thrown,
    body,
    head,
    neck,
    ring1,
    ring2,
    END
};

struct Inv_slot
{
    Inv_slot(Slot_id id_, std::string name_) :
        id    (id_),
        name  (name_),
        item  (nullptr) {}

    Inv_slot() :
        id    (Slot_id::wielded),
        name  (""),
        item  (nullptr) {}

    Slot_id      id;
    std::string name;
    Item*       item;
};

class Inventory
{
public:
    Inventory();

    ~Inventory();

    void drop_all_non_intrinsic(const Pos& pos);

    bool has_item_in_slot(Slot_id id) const;

    //NOTE: This will put the item in general if the slot is occupied
    void put_in_slot(const Slot_id id, Item* item);

    void put_in_general(Item* item);

    void put_in_intrinsics(Item* item);

    bool move_to_general(const Slot_id id);

    void move_from_general_to_intrinsics(const size_t GEN_IDX);

    void move_item_to_slot(Inv_slot& slot, const size_t GEN_IDX);

    void equip_general_item(const size_t GEN_IDX, const Slot_id slot_id);

    void swap_wielded_and_prepared(const bool IS_FREE_TURN);

    bool has_ammo_for_firearm_in_inventory();

    Item* get_first_item_in_backpack_with_id(const Item_id id);

    int get_backpack_idx_with_item_id(const Item_id item_id) const;

    Item* get_item_in_slot(const Slot_id id) const;

    void remove_without_destroying(const Inv_type inv_type, const size_t IDX);

    void decr_item_in_slot(Slot_id slot_id);

    void decr_item_in_general(const size_t IDX);

    void decr_item(Item* const item);

    void decr_item_type_in_general(const Item_id item_id);

    void remove_item_in_backpack_with_idx(const size_t IDX, const bool DELETE_ITEM);
    void remove_item_in_backpack_with_ptr(Item* const item, const bool DELETE_ITEM);

    int get_intrinsics_size() const {return intrinsics_.size();}

    Item* get_intrinsic_in_element(const int IDX) const;

    Item* get_last_item_in_general();

    bool has_item_in_backpack(const Item_id id) const;

    int get_item_stack_size_in_general(const Item_id id) const;

    void decr_dynamite_in_general();

    void sort_general_inventory();

    int get_total_item_weight() const;

    void store_to_save_lines  (std::vector<std::string>& lines) const;
    void setup_from_save_lines(std::vector<std::string>& lines);

    Inv_slot               slots_[int(Slot_id::END)];
    std::vector<Item*>    general_;
    std::vector<Item*>    intrinsics_;
};

#endif

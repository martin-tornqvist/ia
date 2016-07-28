#include "inventory.hpp"

#include <algorithm>
#include <vector>

#include "init.hpp"
#include "item.hpp"
#include "drop.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "game_time.hpp"
#include "render.hpp"
#include "item_factory.hpp"
#include "create_character.hpp"
#include "dungeon_master.hpp"
#include "player_bon.hpp"
#include "map.hpp"
#include "save_handling.hpp"

Inventory::Inventory(Actor* const owning_actor) :
    owning_actor_(owning_actor)
{
    auto set_slot = [&](const SlotId id, const std::string & name)
    {
        slots_[int(id)] = {id, name};
    };

    set_slot(SlotId::wpn,      "Wielded");
    set_slot(SlotId::wpn_alt,  "Prepared");
    set_slot(SlotId::thrown,   "Thrown");
    set_slot(SlotId::body,     "Body");
    set_slot(SlotId::head,     "Head");
    set_slot(SlotId::neck,     "Neck");
}

Inventory::~Inventory()
{
    for (size_t i = 0; i < int(SlotId::END); ++i)
    {
        auto& slot = slots_[i];

        if (slot.item)
        {
            delete slot.item;
        }
    }

    for (Item* item : backpack_)
    {
        delete item;
    }

    for (Item* item : intrinsics_)
    {
        delete item;
    }
}

void Inventory::save() const
{
    for (const InvSlot& slot : slots_)
    {
        Item* const item = slot.item;

        if (item)
        {
            save_handling::put_int(int(item->id()));
            save_handling::put_int(item->nr_items_);

            item->save();
        }
        else //No item in this slot
        {
            save_handling::put_int(int(ItemId::END));
        }
    }

    save_handling::put_int(backpack_.size());

    for (Item* item : backpack_)
    {
        save_handling::put_int(int(item->id()));
        save_handling::put_int(item->nr_items_);

        item->save();
    }
}

void Inventory::load()
{
    for (InvSlot& slot : slots_)
    {
        //Any previous item is destroyed
        Item* item = slot.item;

        delete item;
        slot.item = nullptr;

        const ItemId id = ItemId(save_handling::get_int());

        if (id != ItemId::END)
        {
            item = item_factory::mk(id);

            item->nr_items_ = save_handling::get_int();

            item->load();

            slot.item = item;

            //When loading the game, "wear" the item to apply properties from wearing and set
            //owning actor
            ASSERT(owning_actor_);
            item->on_pickup(*owning_actor_);
            item->on_equip(Verbosity::silent);
        }
    }

    while (backpack_.size() != 0)
    {
        remove_item_in_backpack_with_idx(0, true);
    }

    const int backpack_size = save_handling::get_int();

    for (int i = 0; i < backpack_size; ++i)
    {
        const ItemId id = ItemId(save_handling::get_int());

        Item* item = item_factory::mk(id);

        item->nr_items_ = save_handling::get_int();

        item->load();

        backpack_.push_back(item);

        //When loading the game, "pick up" the item again to set owning actor
        ASSERT(owning_actor_);
        item->on_pickup(*owning_actor_);
    }
}

bool Inventory::has_item_in_backpack(const ItemId id) const
{
    auto it = std::find_if(begin(backpack_), end(backpack_), [id](Item * item)
    {
        return item->id() == id;
    });

    return it != end(backpack_);
}

int Inventory::item_stack_size_in_backpack(const ItemId id) const
{
    for (size_t i = 0; i < backpack_.size(); ++i)
    {
        if (backpack_[i]->data().id == id)
        {
            if (backpack_[i]->data().is_stackable)
            {
                return backpack_[i]->nr_items_;
            }
            else //Not stackable
            {
                return 1;
            }
        }
    }

    return 0;
}

bool Inventory::try_stack_in_backpack(Item* item)
{
    //If item stacks, see if there are other items of same type
    if (item->data().is_stackable)
    {
        for (size_t i = 0; i < backpack_.size(); ++i)
        {
            Item* const other = backpack_[i];

            if (other->id() == item->id())
            {
                //Keeping picked up item and destroying the one in the inventory
                //(then the parameter pointer is still valid).
                item->nr_items_ += other->nr_items_;
                delete other;
                backpack_[i] = item;

                return true;
            }
        }
    }

    return false;
}

void Inventory::put_in_backpack(Item* item)
{
    ASSERT(!item->actor_carrying());

    bool is_stacked = try_stack_in_backpack(item);

    if (!is_stacked)
    {
        backpack_.push_back(item);
        sort_backpack();
    }

    //NOTE: This may destroy the item (e.g. a Medical bag combining with another)
    item->on_pickup(*owning_actor_);
}

void Inventory::drop_all_non_intrinsic(const P& pos)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    Item* item = nullptr;

    //Drop from slots
    for (InvSlot& slot : slots_)
    {
        item = slot.item;

        if (item)
        {
            item->on_removed_from_inv();

            item_drop::drop_item_on_map(pos, *item);

            slot.item = nullptr;
        }
    }

    //Drop from backpack
    for (Item* item : backpack_)
    {
        item->on_removed_from_inv();

        item_drop::drop_item_on_map(pos, *item);
    }

    backpack_.clear();

    TRACE_FUNC_END_VERBOSE;
}

bool Inventory::has_ammo_for_firearm_in_inventory()
{
    Wpn* weapon = static_cast<Wpn*>(item_in_slot(SlotId::wpn));

    //If weapon found
    if (weapon)
    {
        ASSERT(!weapon->data().ranged.has_infinite_ammo); //Should not happen

        //If weapon is a firearm
        if (weapon->data().ranged.is_ranged_wpn)
        {
            //Get weapon ammo type
            const ItemId ammo_id = weapon->data().ranged.ammo_item_id;

            //Look for that ammo type in inventory
            for (size_t i = 0; i < backpack_.size(); ++i)
            {
                if (backpack_[i]->data().id == ammo_id)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void Inventory::decr_item_in_slot(SlotId slot_id)
{
    Item* item = item_in_slot(slot_id);
    bool stack = item->data().is_stackable;
    bool delete_item = true;

    if (stack)
    {
        item->nr_items_ -= 1;

        if (item->nr_items_ > 0)
        {
            delete_item = false;
        }
    }

    if (delete_item)
    {
        slots_[int(slot_id)].item = nullptr;
        delete item;
    }
}

Item* Inventory::remove_item_in_backpack_with_idx(const size_t idx, const bool delete_item)
{
    ASSERT(idx < backpack_.size());

    Item* item = backpack_[idx];

    backpack_.erase(begin(backpack_) + idx);

    item->on_removed_from_inv();

    if (delete_item)
    {
        delete item;
        item = nullptr;
    }

    return item;
}

Item* Inventory::remove_item_in_backpack_with_ptr(Item* const item, const bool delete_item)
{
    for (size_t i = 0; i < backpack_.size(); ++i)
    {
        const Item* const current_item = backpack_[i];

        if (current_item == item)
        {
            return remove_item_in_backpack_with_idx(i, delete_item);
        }
    }

    TRACE << "Parameter item not in backpack" << std::endl;
    ASSERT(false);

    return nullptr;
}

void Inventory::decr_item_in_backpack(const size_t idx)
{
    Item* item              = backpack_[idx];
    bool  is_stackable       = item->data().is_stackable;
    bool  should_delete_item  = true;

    if (is_stackable)
    {
        --item->nr_items_;
        should_delete_item = item->nr_items_ <= 0;
    }

    if (should_delete_item)
    {
        backpack_.erase(begin(backpack_) + idx);
        delete item;
    }
}

void Inventory::decr_item_type_in_backpack(const ItemId id)
{
    for (size_t i = 0; i < backpack_.size(); ++i)
    {
        if (backpack_[i]->data().id == id)
        {
            decr_item_in_backpack(i);
            return;
        }
    }
}

void Inventory::decr_item(Item* const item)
{
    for (InvSlot& slot : slots_)
    {
        if (slot.item == item)
        {
            decr_item_in_slot(slot.id);
            return;
        }
    }

    for (size_t i = 0; i < backpack_.size(); ++i)
    {
        if (backpack_[i] == item)
        {
            decr_item_in_backpack(i);
            return;
        }
    }
}

void Inventory::move_from_backpack_to_slot(const SlotId id, const size_t backpack_idx)
{
    ASSERT(id != SlotId::END);

    auto& slot = slots_[size_t(id)];

    const bool backpack_slot_exists = backpack_idx < backpack_.size();

    ASSERT(backpack_slot_exists);

    if (backpack_slot_exists)
    {
        Item* item              = backpack_[backpack_idx];
        Item* slot_item_before  = slot.item;

        backpack_.erase(begin(backpack_) + backpack_idx);

        //If there is an item in this slot already, move it to backpack
        auto unequip_allowed_result = UnequipAllowed::yes;

        if (slot_item_before)
        {
            unequip_allowed_result = try_move_from_slot_to_backpack(id);
        }

        if (unequip_allowed_result == UnequipAllowed::yes)
        {
            slot.item = item;

            item->on_equip(Verbosity::verbose);
        }
    }

    sort_backpack();
}

UnequipAllowed Inventory::try_move_from_slot_to_backpack(const SlotId id)
{
    ASSERT(id != SlotId::END);

    auto& slot = slots_[size_t(id)];

    Item* const item = slot.item;

    auto unequip_allowed_result = UnequipAllowed::no;

    if (item)
    {
        unequip_allowed_result = item->on_unequip();

        if (unequip_allowed_result == UnequipAllowed::yes)
        {
            slot.item = nullptr;

            bool is_stacked = try_stack_in_backpack(item);

            if (!is_stacked)
            {
                backpack_.push_back(item);
                sort_backpack();
            }
        }
    }

    return unequip_allowed_result;
}

void Inventory::equip_backpack_item(const size_t backpack_idx, const SlotId slot_id)
{
    ASSERT(slot_id != SlotId::END);
    ASSERT(owning_actor_);

    render::draw_map_state();

    move_from_backpack_to_slot(slot_id, backpack_idx);

    if (owning_actor_->is_player())
    {
        Item* const         item_after  = item_in_slot(slot_id);
        const std::string   name        = item_after->name(ItemRefType::plural);
        std::string         msg         = "";

        switch (slot_id)
        {
        case SlotId::wpn:
            msg = "I am now wielding " + name + ".";
            break;

        case SlotId::wpn_alt:
            msg = "I am now using " + name + " as a prepared weapon.";
            break;

        case SlotId::thrown:
            msg = "I now have " + name + " at hand for throwing.";
            break;

        case SlotId::body:
            msg = "I am now wearing " + name + ".";
            break;

        case SlotId::head:
            msg = "I am now wearing " + name + ".";
            break;

        case SlotId::neck:
            msg = "I am now wearing " + name + ".";
            break;

        case SlotId::END: {}
            break;
        }

        msg_log::add(msg,
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);
    }
}

UnequipAllowed Inventory::try_unequip_slot(const SlotId id)
{
    auto& slot = slots_[size_t(id)];

    Item* item = slot.item;

    ASSERT(item);

    render::draw_map_state();

    auto unequip_allowed_result = try_move_from_slot_to_backpack(slot.id);

    if (owning_actor_->is_player() && unequip_allowed_result == UnequipAllowed::yes)
    {
        //The message should be of the form "... my [item]", so we never want the item name to be
        //"A [item]". Then we use plural form for stacks of items, and plain form for single items.
        const auto item_ref_type = item->nr_items_ > 1 ?
                                   ItemRefType::plural :
                                   ItemRefType::plain;

        const std::string name = item->name(item_ref_type);

        std::string msg = "";

        switch (id)
        {
        case SlotId::wpn:
            msg = "I put away my " + name + ".";
            break;

        case SlotId::wpn_alt:
            msg = "I put away my " + name + ".";
            break;

        case SlotId::thrown:
            msg = "I put away my " + name + ".";
            break;

        case SlotId::body:
            msg = "I have taken off my " + name + ".";
            break;

        case SlotId::head:
            msg = "I have taken off my " + name + ".";
            break;

        case SlotId::neck:
            msg = "I have taken off my " + name + ".";
            break;

        case SlotId::END:
            break;
        }

        msg_log::add(msg,
                     clr_text,
                     false,
                     MorePromptOnMsg::yes);
    }

    return unequip_allowed_result;
}

void Inventory::swap_wielded_and_prepared(const PassTime pass_time)
{
    auto& slot1 = slots_[int(SlotId::wpn)];
    auto& slot2 = slots_[int(SlotId::wpn_alt)];
    Item* item1 = slot1.item;
    Item* item2 = slot2.item;
    slot1.item  = item2;
    slot2.item  = item1;

    render::draw_map_state();

    game_time::tick(pass_time);
}

bool Inventory::has_item_in_slot(SlotId id) const
{
    ASSERT(id != SlotId::END && "Illegal slot id");
    return slots_[int(id)].item;
}

void Inventory::remove_without_destroying(const InvType inv_type, const size_t idx)
{
    if (inv_type == InvType::slots)
    {
        ASSERT(idx != int(SlotId::END));
        slots_[idx].item = nullptr;
    }
    else //Backpack
    {
        ASSERT(idx < backpack_.size());
        backpack_.erase(begin(backpack_) + idx);
    }
}

Item* Inventory::item_in_backpack(const ItemId id)
{
    auto it = std::find_if(begin(backpack_), end(backpack_), [id](Item * item)
    {
        return item->id() == id;
    });

    if (it == end(backpack_))
    {
        return nullptr;
    }

    return *it;
}

int Inventory::backpack_idx(const ItemId id) const
{
    for (size_t i = 0; i < backpack_.size(); ++i)
    {
        if (backpack_[i]->id() == id)
        {
            return i;
        }
    }

    return -1;
}

Item* Inventory::item_in_slot(SlotId id) const
{
    ASSERT(id != SlotId::END);

    return slots_[size_t(id)].item;
}

Item* Inventory::intrinsic_in_element(int idx) const
{
    if (intrinsics_size() > idx)
    {
        return intrinsics_[idx];
    }

    return nullptr;
}

void Inventory::put_in_intrinsics(Item* item)
{
    ASSERT(item->data().type == ItemType::melee_wpn_intr ||
           item->data().type == ItemType::ranged_wpn_intr);

    intrinsics_.push_back(item);

    item->on_pickup(*owning_actor_);
}

Item* Inventory::last_item_in_backpack()
{
    if (!backpack_.empty())
    {
        return backpack_[backpack_.size() - 1];
    }

    return nullptr;
}

void Inventory::put_in_slot(const SlotId id, Item* item)
{
    ASSERT(!item->actor_carrying());

    ASSERT(id != SlotId::END);

    for (InvSlot& slot : slots_)
    {
        if (slot.id == id)
        {
            if (slot.item)
            {
                put_in_backpack(item);
            }
            else //Slot not occupied
            {
                slot.item = item;

                item->on_pickup(*owning_actor_);
                item->on_equip(Verbosity::silent);
            }
            return;
        }
    }

    //Should never happen
    ASSERT(false);
}

Item* Inventory::remove_from_slot(const SlotId id)
{
    ASSERT(id != SlotId::END);

    for (InvSlot& slot : slots_)
    {
        if (slot.id == id)
        {
            Item* item = slot.item;

            if (item)
            {
                ASSERT(item->actor_carrying());

                slot.item = nullptr;

                item->on_unequip();
                item->on_removed_from_inv();

                return item;
            }
        }
    }

    return nullptr;
}

int Inventory::total_item_weight() const
{
    int weight = 0;

    for (size_t i = 0; i < size_t(SlotId::END); ++i)
    {
        if (slots_[i].item)
        {
            weight += slots_[i].item->weight();
        }
    }

    for (size_t i = 0; i < backpack_.size(); ++i)
    {
        weight += backpack_[i]->weight();
    }

    return weight;
}

// Function for lexicographically comparing two items
struct LexicograhicalCompareItems
{
    bool operator()(const Item* const item1, const Item* const item2)
    {
        const std::string& item_name1 = item1->name(ItemRefType::plain);
        const std::string& item_name2 = item2->name(ItemRefType::plain);

        return lexicographical_compare(item_name1.begin(), item_name1.end(),
                                       item_name2.begin(), item_name2.end());
    }
};

void Inventory::sort_backpack()
{
    LexicograhicalCompareItems lex_cmp;

    //First, take out prioritized items
    std::vector<Item*> prio_items;

    for (auto it = begin(backpack_); it != end(backpack_); /* No increment */)
    {
        Item* const item = *it;

        if (item->data().is_prio_in_backpack_list)
        {
            prio_items.push_back(item);

            it = backpack_.erase(it);
        }
        else //Item not prioritized
        {
            ++it;
        }
    }

    //Sort the prioritized items lexicographically
    if (!prio_items.empty())
    {
        std::sort(begin(prio_items), end(prio_items), lex_cmp);
    }

    //Categorize the remaining items
    std::vector< std::vector<Item*> > sort_buffer;

    //Sort according to item interface color first
    for (Item* item : backpack_)
    {
        bool is_added_to_buffer = false;

        //Check if item should be added to any existing color group
        for (auto& group : sort_buffer)
        {
            const Clr clr_current_group = group[0]->interface_clr();

            if (is_clr_equal(item->interface_clr(), clr_current_group))
            {
                group.push_back(item);
                is_added_to_buffer = true;
                break;
            }
        }

        if (is_added_to_buffer)
        {
            continue;
        }

        //Item is a new color, create a new color group
        std::vector<Item*> new_group;
        new_group.push_back(item);
        sort_buffer.push_back(new_group);
    }

    //Sort lexicographically secondarily
    for (auto& group : sort_buffer)
    {
        std::sort(begin(group), end(group), lex_cmp);
    }

    //Add the sorted items to the backpack
    backpack_ = prio_items; //NOTE: prio_items may be empty

    for (size_t i = 0; i < sort_buffer.size(); ++i)
    {
        for (size_t ii = 0; ii < sort_buffer[i].size(); ii++)
        {
            backpack_.push_back(sort_buffer[i][ii]);
        }
    }
}

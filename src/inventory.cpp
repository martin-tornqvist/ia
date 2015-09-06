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
#include "utils.hpp"

Inventory::Inventory(Actor* const owning_actor) :
    owning_actor_(owning_actor)
{
    auto set_slot = [&](const Slot_id id, const std::string & name)
    {
        slots_[int(id)] = {id, name};
    };

    set_slot(Slot_id::wpn,      "Wielded");
    set_slot(Slot_id::wpn_alt,  "Prepared");
    set_slot(Slot_id::thrown,   "Thrown");
    set_slot(Slot_id::body,     "Body");
    set_slot(Slot_id::head,     "Head");
    set_slot(Slot_id::neck,     "Neck");
    set_slot(Slot_id::ring1,    "Ring");
    set_slot(Slot_id::ring2,    "Ring");
}

Inventory::~Inventory()
{
    for (size_t i = 0; i < int(Slot_id::END); ++i)
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

void Inventory::store_to_save_lines(std::vector<std::string>& lines) const
{
    for (const Inv_slot& slot : slots_)
    {
        Item* const item = slot.item;

        if (item)
        {
            lines.push_back(to_str(int(item->id())));
            lines.push_back(to_str(item->nr_items_));
            item->store_to_save_lines(lines);
        }
        else //No item in this slot
        {
            lines.push_back(to_str(int(Item_id::END)));
        }
    }

    lines.push_back(to_str(backpack_.size()));

    for (Item* item : backpack_)
    {
        lines.push_back(to_str(int(item->id())));
        lines.push_back(to_str(item->nr_items_));
        item->store_to_save_lines(lines);
    }
}

void Inventory::setup_from_save_lines(std::vector<std::string>& lines)
{
    for (Inv_slot& slot : slots_)
    {
        //Previous item is destroyed
        Item* item = slot.item;

        if (item)
        {
            delete item;
            slot.item = nullptr;
        }

        const Item_id id = Item_id(to_int(lines.front()));
        lines.erase(begin(lines));

        if (id != Item_id::END)
        {
            item = item_factory::mk(id);
            item->nr_items_ = to_int(lines.front());
            lines.erase(begin(lines));
            item->setup_from_save_lines(lines);
            slot.item = item;

            //When loading the game, wear the item to apply properties from wearing
            assert(owning_actor_);
            item->on_pickup(*owning_actor_);
            item->on_equip(Verbosity::silent);
        }
    }

    while (backpack_.size() != 0)
    {
        remove_item_in_backpack_with_idx(0, true);
    }

    const int BACKPACK_SIZE = to_int(lines.front());
    lines.erase(begin(lines));

    for (int i = 0; i < BACKPACK_SIZE; ++i)
    {
        const Item_id id = Item_id(to_int(lines.front()));
        lines.erase(begin(lines));
        Item* item = item_factory::mk(id);
        item->nr_items_ = to_int(lines.front());
        lines.erase(begin(lines));
        item->setup_from_save_lines(lines);
        backpack_.push_back(item);
    }
}

bool Inventory::has_item_in_backpack(const Item_id id) const
{
    auto it = std::find_if(begin(backpack_), end(backpack_), [id](Item * item)
    {
        return item->id() == id;
    });

    return it != end(backpack_);
}

int Inventory::item_stack_size_in_backpack(const Item_id id) const
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
    assert(!item->actor_carrying());

    bool is_stacked = try_stack_in_backpack(item);

    if (!is_stacked)
    {
        backpack_.push_back(item);
        sort_backpack();
    }

    //NOTE: This may destroy the item (e.g. a Medical bag combining with another)
    item->on_pickup(*owning_actor_);
}

void Inventory::drop_all_non_intrinsic(const Pos& pos)
{
    Item* item;

    //Drop from slots
    for (Inv_slot& slot : slots_)
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

    backpack_.resize(0);
}

bool Inventory::has_ammo_for_firearm_in_inventory()
{
    Wpn* weapon = static_cast<Wpn*>(item_in_slot(Slot_id::wpn));

    //If weapon found
    if (weapon)
    {
        assert(!weapon->data().ranged.has_infinite_ammo); //Should not happen

        //If weapon is a firearm
        if (weapon->data().ranged.is_ranged_wpn)
        {
            //Get weapon ammo type
            const Item_id ammo_id = weapon->data().ranged.ammo_item_id;

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

void Inventory::decr_item_in_slot(Slot_id slot_id)
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

Item* Inventory::remove_item_in_backpack_with_idx(const size_t IDX, const bool DELETE_ITEM)
{
    assert(IDX < backpack_.size());

    Item* item = backpack_[IDX];

    backpack_.erase(begin(backpack_) + IDX);

    item->on_removed_from_inv();

    if (DELETE_ITEM)
    {
        delete item;
        item = nullptr;
    }

    return item;
}

Item* Inventory::remove_item_in_backpack_with_ptr(Item* const item, const bool DELETE_ITEM)
{
    for (size_t i = 0; i < backpack_.size(); ++i)
    {
        const Item* const cur_item = backpack_[i];

        if (cur_item == item)
        {
            return remove_item_in_backpack_with_idx(i, DELETE_ITEM);
        }
    }

    TRACE << "Parameter item not in backpack" << std::endl;
    assert(false);

    return nullptr;
}

void Inventory::decr_item_in_backpack(const size_t IDX)
{
    Item* item              = backpack_[IDX];
    bool  is_stackable       = item->data().is_stackable;
    bool  should_delete_item  = true;

    if (is_stackable)
    {
        --item->nr_items_;
        should_delete_item = item->nr_items_ <= 0;
    }

    if (should_delete_item)
    {
        backpack_.erase(begin(backpack_) + IDX);
        delete item;
    }
}

void Inventory::decr_item_type_in_backpack(const Item_id id)
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
    for (Inv_slot& slot : slots_)
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

void Inventory::move_from_backpack_to_slot(const Slot_id id, const size_t BACKPACK_IDX)
{
    assert(id != Slot_id::END);

    auto& slot = slots_[size_t(id)];

    const bool BACKPACK_SLOT_EXISTS = BACKPACK_IDX < backpack_.size();

    assert(BACKPACK_SLOT_EXISTS);

    if (BACKPACK_SLOT_EXISTS)
    {
        Item* item              = backpack_[BACKPACK_IDX];
        Item* slot_item_before  = slot.item;

        backpack_.erase(begin(backpack_) + BACKPACK_IDX);

        //If there is an item in this slot already, move it to backpack
        auto unequip_allowed_result = Unequip_allowed::yes;

        if (slot_item_before)
        {
            unequip_allowed_result = try_move_from_slot_to_backpack(id);
        }

        if (unequip_allowed_result == Unequip_allowed::yes)
        {
            slot.item = item;

            item->on_equip(Verbosity::verbose);
        }
    }

    sort_backpack();
}

Unequip_allowed Inventory::try_move_from_slot_to_backpack(const Slot_id id)
{
    assert(id != Slot_id::END);

    auto& slot = slots_[size_t(id)];

    Item* const item = slot.item;

    auto unequip_allowed_result = Unequip_allowed::no;

    if (item)
    {
        unequip_allowed_result = item->on_unequip();

        if (unequip_allowed_result == Unequip_allowed::yes)
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

void Inventory::equip_backpack_item(const size_t BACKPACK_IDX, const Slot_id slot_id)
{
    assert(slot_id != Slot_id::END);
    assert(owning_actor_);

    render::draw_map_and_interface();

    move_from_backpack_to_slot(slot_id, BACKPACK_IDX);

    if (owning_actor_->is_player())
    {
        Item* const         item_after  = item_in_slot(slot_id);
        const std::string   name        = item_after->name(Item_ref_type::plural);
        std::string         msg         = "";

        switch (slot_id)
        {
        case Slot_id::wpn:
            msg = "I am now wielding " + name + ".";
            break;

        case Slot_id::wpn_alt:
            msg = "I am now using " + name + " as a prepared weapon.";
            break;

        case Slot_id::thrown:
            msg = "I now have " + name + " at hand for throwing.";
            break;

        case Slot_id::body:
            msg = "I am now wearing " + name + ".";
            break;

        case Slot_id::head:
            msg = "I am now wearing " + name + ".";
            break;

        case Slot_id::neck:
            msg = "I am now wearing " + name + ".";
            break;

        case Slot_id::ring1:
            msg = "I am now wearing " + name + ".";
            break;

        case Slot_id::ring2:
            msg = "I am now wearing " + name + ".";
            break;

        case Slot_id::END: {}
            break;
        }

        msg_log::add(msg, clr_white, false, More_prompt_on_msg::yes);
    }
}

Unequip_allowed Inventory::try_unequip_slot(const Slot_id id)
{
    auto& slot = slots_[size_t(id)];

    Item* item = slot.item;

    assert(item);

    render::draw_map_and_interface();

    auto unequip_allowed_result = try_move_from_slot_to_backpack(slot.id);

    if (owning_actor_->is_player() && unequip_allowed_result == Unequip_allowed::yes)
    {
        //The message should be of the form "... my [item]", so we never want the item name to be
        //"A [item]". Then we use plural form for stacks of items, and plain form for single items.
        const auto item_ref_type = item->nr_items_ > 1 ?
                                   Item_ref_type::plural :
                                   Item_ref_type::plain;

        const std::string name = item->name(item_ref_type);

        std::string msg = "";

        switch (id)
        {
        case Slot_id::wpn:
            msg = "I put away my " + name + ".";
            break;

        case Slot_id::wpn_alt:
            msg = "I put away my " + name + ".";
            break;

        case Slot_id::thrown:
            msg = "I put away my " + name + ".";
            break;

        case Slot_id::body:
            msg = "I have taken off my " + name + ".";
            break;

        case Slot_id::head:
            msg = "I have taken off my " + name + ".";
            break;

        case Slot_id::neck:
            msg = "I have taken off my " + name + ".";
            break;

        case Slot_id::ring1:
            msg = "I have taken off my " + name + ".";
            break;

        case Slot_id::ring2:
            msg = "I have taken off my " + name + ".";
            break;

        case Slot_id::END:
            break;
        }

        msg_log::add(msg, clr_white, false, More_prompt_on_msg::yes);
    }

    return unequip_allowed_result;
}

void Inventory::swap_wielded_and_prepared(const bool IS_FREE_TURN)
{
    auto& slot1 = slots_[int(Slot_id::wpn)];
    auto& slot2 = slots_[int(Slot_id::wpn_alt)];
    Item* item1 = slot1.item;
    Item* item2 = slot2.item;
    slot1.item  = item2;
    slot2.item  = item1;

    render::draw_map_and_interface();

    game_time::tick(IS_FREE_TURN);
}

bool Inventory::has_item_in_slot(Slot_id id) const
{
    assert(id != Slot_id::END && "Illegal slot id");
    return slots_[int(id)].item;
}

void Inventory::remove_without_destroying(const Inv_type inv_type, const size_t IDX)
{
    if (inv_type == Inv_type::slots)
    {
        assert(IDX != int(Slot_id::END));
        slots_[IDX].item = nullptr;
    }
    else
    {
        assert(IDX < backpack_.size());
        backpack_.erase(begin(backpack_) + IDX);
    }
}

Item* Inventory::item_in_backpack(const Item_id id)
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

int Inventory::backpack_idx(const Item_id id) const
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

Item* Inventory::item_in_slot(Slot_id id) const
{
    assert(id != Slot_id::END && "Illegal slot id");
    return slots_[int(id)].item;
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
    assert(item->data().type == Item_type::melee_wpn_intr ||
           item->data().type == Item_type::ranged_wpn_intr);

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

void Inventory::put_in_slot(const Slot_id id, Item* item)
{
    assert(!item->actor_carrying());

    assert(id != Slot_id::END);

    for (Inv_slot& slot : slots_)
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
    assert(false);
}

Item* Inventory::remove_from_slot(const Slot_id id)
{
    assert(id != Slot_id::END);

    for (Inv_slot& slot : slots_)
    {
        if (slot.id == id)
        {
            Item* item = slot.item;

            if (item)
            {
                assert(item->actor_carrying());

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

    for (size_t i = 0; i < size_t(Slot_id::END); ++i)
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
struct Lexicograhical_compare_items
{
    bool operator()(const Item* const item1, const Item* const item2)
    {
        const std::string& item_name1 = item1->name(Item_ref_type::plain);
        const std::string& item_name2 = item2->name(Item_ref_type::plain);

        return lexicographical_compare(item_name1.begin(), item_name1.end(),
                                       item_name2.begin(), item_name2.end());
    }
};

void Inventory::sort_backpack()
{
    Lexicograhical_compare_items lex_cmp;

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
            const Clr clr_cur_group = group[0]->interface_clr();

            if (utils::is_clr_eq(item->interface_clr(), clr_cur_group))
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

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

using namespace std;

Inventory::Inventory()
{
    auto set_slot = [&](const Slot_id id, const string & name)
    {
        slots_[int(id)] = {id, name};
    };

    set_slot(Slot_id::wielded,    "Wielding");
    set_slot(Slot_id::wielded_alt, "Prepared");
    set_slot(Slot_id::thrown,     "Thrown");
    set_slot(Slot_id::body,       "Body");
    set_slot(Slot_id::head,       "Head");
    set_slot(Slot_id::neck,       "Neck");
    set_slot(Slot_id::ring1,      "Ring");
    set_slot(Slot_id::ring2,      "Ring");
}

Inventory::~Inventory()
{
    for (size_t i = 0; i < int(Slot_id::END); ++i)
    {
        auto& slot = slots_[i];

        if (slot.item) {delete slot.item;}
    }

    for (Item* item : general_)    {delete item;}

    for (Item* item : intrinsics_) {delete item;}
}

void Inventory::store_to_save_lines(vector<string>& lines) const
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
        else
        {
            lines.push_back(to_str(int(Item_id::END)));
        }
    }

    lines.push_back(to_str(general_.size()));

    for (Item* item : general_)
    {
        lines.push_back(to_str(int(item->id())));
        lines.push_back(to_str(item->nr_items_));
        item->store_to_save_lines(lines);
    }
}

void Inventory::setup_from_save_lines(vector<string>& lines)
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
            item->on_equip(true);
        }
    }

    while (general_.size() != 0)
    {
        remove_item_in_backpack_with_idx(0, true);
    }

    const int NR_OF_GENERAL = to_int(lines.front());
    lines.erase(begin(lines));

    for (int i = 0; i < NR_OF_GENERAL; ++i)
    {
        const Item_id id = Item_id(to_int(lines.front()));
        lines.erase(begin(lines));
        Item* item = item_factory::mk(id);
        item->nr_items_ = to_int(lines.front());
        lines.erase(begin(lines));
        item->setup_from_save_lines(lines);
        general_.push_back(item);
    }
}

bool Inventory::has_item_in_backpack(const Item_id id) const
{
    auto it = std::find_if(begin(general_), end(general_), [id](Item * item)
    {
        return item->id() == id;
    });

    return it != end(general_);
}

int Inventory::item_stack_size_in_general(const Item_id id) const
{
    for (size_t i = 0; i < general_.size(); ++i)
    {
        if (general_[i]->data().id == id)
        {
            if (general_[i]->data().is_stackable)
            {
                return general_[i]->nr_items_;
            }
            else
            {
                return 1;
            }
        }
    }

    return 0;
}

void Inventory::decr_dynamite_in_general()
{
    for (size_t i = 0; i < general_.size(); ++i)
    {
        if (general_[i]->data().id == Item_id::dynamite)
        {
            decr_item_in_general(i);
            break;
        }
    }
}

void Inventory::put_in_general(Item* item)
{
    bool is_stacked = false;

    //If item stacks, see if there is other items of same type
    if (item->data().is_stackable)
    {
        for (size_t i = 0; i < general_.size(); ++i)
        {
            Item* const other = general_[i];

            if (other->id() == item->id())
            {
                //Keeping picked up item and destroying the one in the inventory
                //(then the parameter pointer is still valid).
                item->nr_items_ += other->nr_items_;
                delete other;
                general_[i] = item;
                is_stacked   = true;
                break;
            }
        }
    }

    if (!is_stacked)
    {
        general_.push_back(item);
    }

    //NOTE: May destroy the item (e.g. a Medical Bag combining with another)
    item->on_pickup_to_backpack(*this);
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
            item_drop::drop_item_on_map(pos, *item);

            slot.item = nullptr;
        }
    }

    //Drop from general
    size_t i = 0;

    while (i < general_.size())
    {
        item = general_[i];

        if (item)
        {
            item_drop::drop_item_on_map(pos, *item);

            general_.erase(begin(general_) + i);
        }

        i++;
    }
}

bool Inventory::has_ammo_forFirearm_in_inventory()
{
    Wpn* weapon = static_cast<Wpn*>(item_in_slot(Slot_id::wielded));

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
            for (size_t i = 0; i < general_.size(); ++i)
            {
                if (general_[i]->data().id == ammo_id)
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

void Inventory::remove_item_in_backpack_with_idx(const size_t IDX, const bool DELETE_ITEM)
{
    if (general_.size() > IDX)
    {
        if (DELETE_ITEM)
        {
            delete general_[IDX];
        }

        general_.erase(begin(general_) + IDX);
    }
}

void Inventory::remove_item_in_backpack_with_ptr(Item* const item, const bool DELETE_ITEM)
{
    for (auto it = begin(general_); it < end(general_); ++it)
    {
        if (*it == item)
        {
            if (DELETE_ITEM)
            {
                delete *it;
            }

            general_.erase(it);
            return;
        }
    }

    TRACE << "Parameter item not in backpack" << endl;
    assert(false);
}

void Inventory::decr_item_in_general(const size_t IDX)
{
    Item* item              = general_[IDX];
    bool  is_stackable       = item->data().is_stackable;
    bool  should_delete_item  = true;

    if (is_stackable)
    {
        --item->nr_items_;
        should_delete_item = item->nr_items_ <= 0;
    }

    if (should_delete_item)
    {
        general_.erase(begin(general_) + IDX);
        delete item;
    }
}

void Inventory::decr_item_type_in_general(const Item_id id)
{
    for (size_t i = 0; i < general_.size(); ++i)
    {
        if (general_[i]->data().id == id)
        {
            decr_item_in_general(i);
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

    for (size_t i = 0; i < general_.size(); ++i)
    {
        if (general_[i] == item)
        {
            decr_item_in_general(i);
            return;
        }
    }
}

void Inventory::move_item_to_slot(Inv_slot& slot, const size_t GEN_IDX)
{
    bool general_slot_exists  = GEN_IDX < general_.size();
    Item* item              = nullptr;
    Item* slot_item          = slot.item;

    if (general_slot_exists) {item = general_[GEN_IDX];}

    if (general_slot_exists && item)
    {
        if (slot_item)
        {
            general_.erase(begin(general_) + GEN_IDX);
            general_.push_back(slot_item);
            slot.item = item;
        }
        else
        {
            slot.item = item;
            general_.erase(begin(general_) + GEN_IDX);
        }
    }
}

void Inventory::equip_general_item(const size_t GEN_IDX, const Slot_id slot_id)
{
    assert(slot_id != Slot_id::END);

    move_item_to_slot(slots_[int(slot_id)], GEN_IDX);

    const bool IS_PLAYER  = this == &map::player->inv();

    if (IS_PLAYER)
    {
        Item* const     item_after  = item_in_slot(slot_id);
        const string    name        = item_after->name(Item_ref_type::plural);
        string          msg         = "";

        switch (slot_id)
        {
        case Slot_id::wielded:
            msg = "I am now wielding " + name + ".";
            break;

        case Slot_id::wielded_alt:
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

        msg_log::add(msg, clr_white, false, true);
    }
}

void Inventory::swap_wielded_and_prepared(
    const bool IS_FREE_TURN)
{
    auto& slot1 = slots_[int(Slot_id::wielded)];
    auto& slot2 = slots_[int(Slot_id::wielded_alt)];
    Item* item1 = slot1.item;
    Item* item2 = slot2.item;
    slot1.item  = item2;
    slot2.item  = item1;

    render::draw_map_and_interface();

    game_time::tick(IS_FREE_TURN);
}

void Inventory::move_from_general_to_intrinsics(const size_t GEN_IDX)
{
    bool general_slot_exists = GEN_IDX < general_.size();

    if (general_slot_exists)
    {
        Item* item = general_[GEN_IDX];
        bool item_exists_in_general_slot = item;

        if (item_exists_in_general_slot)
        {
            intrinsics_.push_back(item);
            general_.erase(begin(general_) + GEN_IDX);
        }
    }
}

bool Inventory::move_to_general(const Slot_id id)
{
    assert(id != Slot_id::END);

    auto& slot = slots_[size_t(id)];

    Item* const item = slot.item;

    if (item)
    {
        slot.item = nullptr;
        put_in_general(item);
        return true;
    }

    return false;
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
        assert(IDX < general_.size());
        general_.erase(begin(general_) + IDX);
    }
}

Item* Inventory::first_item_in_backpack_with_id(const Item_id id)
{
    auto it = std::find_if(begin(general_), end(general_), [id](Item * item)
    {
        return item->id() == id;
    });

    if (it == end(general_))
    {
        return nullptr;
    }

    return *it;
}

int Inventory::backpack_idx_with_item_id(const Item_id id) const
{
    for (size_t i = 0; i < general_.size(); ++i)
    {
        if (general_[i]->id() == id)
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
}

Item* Inventory::last_item_in_general()
{
    if (!general_.empty()) {return general_[general_.size() - 1];}

    return nullptr;
}

void Inventory::put_in_slot(const Slot_id id, Item* item)
{
    for (Inv_slot& slot : slots_)
    {
        if (slot.id == id)
        {
            if (slot.item)
            {
                general_.push_back(item);
            }
            else
            {
                slot.item = item;
            }

            return;
        }
    }

    general_.push_back(item);
    assert(false && "Bad slot id");
}

int Inventory::total_item_weight() const
{
    int weight = 0;

    for (size_t i = 0; i < size_t(Slot_id::END); ++i)
    {
        if (slots_[i].item) {weight += slots_[i].item->weight();}
    }

    for (size_t i = 0; i < general_.size(); ++i)
    {
        weight += general_[i]->weight();
    }

    return weight;
}

// Function for lexicographically comparing two items
struct Lexicograhical_compare_items
{
    bool operator()(const Item* const item1, const Item* const item2)
    {
        const string& item_name1 = item1->name(Item_ref_type::plain);
        const string& item_name2 = item2->name(Item_ref_type::plain);
        return lexicographical_compare(item_name1.begin(), item_name1.end(),
                                       item_name2.begin(), item_name2.end());
    }
};

void Inventory::sort_general_inventory()
{
    vector< vector<Item*> > sort_buffer;

    //Sort according to item interface color first
    for (Item* item : general_)
    {

        bool is_added_to_buffer = false;

        //Check if item should be added to any existing color group
        for (vector<Item*>& group : sort_buffer)
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
        else
        {
            //Item is a new color, create a new color group
            vector<Item*> new_group;
            new_group.push_back(item);
            sort_buffer.push_back(new_group);
        }
    }

    //Sort lexicographically secondarily
    Lexicograhical_compare_items cmp;

    for (vector<Item*>& group : sort_buffer)
    {
        std::sort(group.begin(), group.end(), cmp);
    }

    //Set the inventory from the sorting buffer
    general_.clear();

    for (size_t i = 0; i < sort_buffer.size(); ++i)
    {
        for (size_t ii = 0; ii < sort_buffer[i].size(); ii++)
        {
            general_.push_back(sort_buffer[i][ii]);
        }
    }
}

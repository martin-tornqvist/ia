#include "pickup.hpp"

#include <string>

#include "init.hpp"
#include "item.hpp"
#include "map.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "drop.hpp"
#include "item_factory.hpp"
#include "game_time.hpp"
#include "audio.hpp"

namespace item_pickup
{

void try_pick()
{
    msg_log::clear();

    const P& pos = map::player->pos;

    Item* const item = map::cells[pos.x][pos.y].item;

    if (!item)
    {
        msg_log::add("I see nothing to pick up here.");

        return;
    }

    Inventory& inv = map::player->inv();

    const std::string item_name = item->name(ItemRefType::plural);

    audio::play(SfxId::pickup);

    msg_log::add("I pick up " + item_name + ".");

    //
    // NOTE: This calls the items pickup hook, which may destroy the item
    //       (e.g. combine with others)
    //
    inv.put_in_backpack(item);

    map::cells[pos.x][pos.y].item = nullptr;

    game_time::tick();
}

Ammo* unload_ranged_wpn(Wpn& wpn)
{
    ASSERT(!wpn.data().ranged.has_infinite_ammo);

    const int nr_ammo_loaded = wpn.nr_ammo_loaded_;

    if (nr_ammo_loaded == 0)
    {
        return nullptr;
    }

    const ItemId ammo_id = wpn.data().ranged.ammo_item_id;

    ItemDataT& ammo_data = item_data::data[(size_t)ammo_id];

    Item* spawned_ammo = item_factory::mk(ammo_id);

    if (ammo_data.type == ItemType::ammo_mag)
    {
        // Unload a mag
        static_cast<AmmoMag*>(spawned_ammo)->ammo_ = nr_ammo_loaded;
    }
    else
    {
        // Unload loose ammo
        spawned_ammo->nr_items_ = nr_ammo_loaded;
    }

    wpn.nr_ammo_loaded_ = 0;

    return static_cast<Ammo*>(spawned_ammo);
}

void try_unload_wpn_or_pickup_ammo()
{
    Item* item = map::cells[map::player->pos.x][map::player->pos.y].item;

    if (item)
    {
        if (item->data().ranged.is_ranged_wpn)
        {
            Wpn* const wpn = static_cast<Wpn*>(item);

            const std::string wpn_name =
                wpn->name(ItemRefType::a, ItemRefInf::yes);

            if (!wpn->data().ranged.has_infinite_ammo)
            {
                Ammo* const spawned_ammo = unload_ranged_wpn(*wpn);

                if (spawned_ammo)
                {
                    audio::play(SfxId::pickup);

                    msg_log::add("I unload " + wpn_name + ".");

                    // Calls items pickup hook
                    // NOTE: This may destroy the item (e.g. combining)
                    map::player->inv().put_in_backpack(spawned_ammo);

                    game_time::tick();
                    return;
                }
            }
        }
        else // Not a ranged weapon
        {
            if (item->data().type == ItemType::ammo ||
                item->data().type == ItemType::ammo_mag)
            {
                try_pick();
                return;
            }
        }
    }

    msg_log::add("I see no ammo to unload or pick up here.");
}

} // item_pickup

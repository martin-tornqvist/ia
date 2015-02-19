#include "Pickup.h"

#include <string>

#include "Init.h"
#include "Item.h"
#include "Map.h"
#include "Actor_player.h"
#include "Log.h"
#include "Query.h"
#include "Drop.h"
#include "Item_factory.h"
#include "Game_time.h"
#include "Audio.h"

using namespace std;

namespace Item_pickup
{

namespace
{

void pickup_effects(Actor* actor, Item* item)
{
    (void)actor;
    (void)item;
}

} //namespace

//Can always be called, to check if something is there to be picked up.
void try_pick()
{
    const Pos&  pos   = Map::player->pos;
    Item* const item  = Map::cells[pos.x][pos.y].item;

    if (item)
    {
        Inventory& player_inv = Map::player->get_inv();

        const string ITEM_NAME = item->get_name(Item_ref_type::plural);

        //If picked up item is missile weapon, try to add it to carried stack.
        if (item->get_data().ranged.is_throwing_wpn)
        {
            Item* const carried_missile = player_inv.get_item_in_slot(Slot_id::thrown);
            if (carried_missile)
            {
                if (item->get_id() == carried_missile->get_data().id)
                {
                    Audio::play(Sfx_id::pickup);

                    Log::add_msg("I add " + ITEM_NAME + " to my missile stack.");
                    carried_missile->nr_items_ += item->nr_items_;
                    delete item;
                    Map::cells[pos.x][pos.y].item = nullptr;
                    Game_time::tick();
                    return;
                }
            }
        }

        Audio::play(Sfx_id::pickup);

        Log::clear_log();
        Log::add_msg("I pick up " + ITEM_NAME + ".");

        player_inv.put_in_general(item);

        Map::cells[pos.x][pos.y].item = nullptr;

        Game_time::tick();
    }
    else //No item in this cell
    {
        Log::clear_log();
        Log::add_msg("I see nothing to pick up here.");
    }
}

Ammo* unload_ranged_wpn(Wpn& wpn)
{
    assert(!wpn.get_data().ranged.has_infinite_ammo);

    const int NR_AMMO_LOADED = wpn.nr_ammo_loaded;

    if (NR_AMMO_LOADED == 0)
    {
        return nullptr;
    }

    const Item_id      ammo_id      = wpn.get_data().ranged.ammo_item_id;
    Item_data_t* const  ammo_data    = Item_data::data[int(ammo_id)];
    Item*             spawned_ammo = Item_factory::mk(ammo_id);

    if (ammo_data->type == Item_type::ammo_clip)
    {
        //Unload a clip
        static_cast<Ammo_clip*>(spawned_ammo)->ammo_ = NR_AMMO_LOADED;
    }
    else
    {
        //Unload loose ammo
        spawned_ammo->nr_items_ = NR_AMMO_LOADED;
    }

    wpn.nr_ammo_loaded = 0;

    return static_cast<Ammo*>(spawned_ammo);
}

void try_unload_wpn_or_pickup_ammo()
{
    Item* item = Map::cells[Map::player->pos.x][Map::player->pos.y].item;

    if (item)
    {
        if (item->get_data().ranged.is_ranged_wpn)
        {
            Wpn* const wpn = static_cast<Wpn*>(item);

            const string wpn_name = wpn->get_name(Item_ref_type::a, Item_ref_inf::yes);

            if (!wpn->get_data().ranged.has_infinite_ammo)
            {
                Ammo* const spawned_ammo = unload_ranged_wpn(*wpn);

                if (spawned_ammo)
                {
                    Audio::play(Sfx_id::pickup);

                    Log::add_msg("I unload " + wpn_name + ".");

                    Map::player->get_inv().put_in_general(spawned_ammo);

                    Game_time::tick();
                    return;
                }
            }
        }
        else //Not a ranged weapon
        {
            if (
                item->get_data().type == Item_type::ammo ||
                item->get_data().type == Item_type::ammo_clip)
            {
                try_pick();
                return;
            }
        }
    }

    Log::add_msg("I see no ammo to unload or pick up here.");
}

} //Item_pickup

#include "reload.hpp"

#include <string>
#include <climits>

#include "init.hpp"
#include "item.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "inventory.hpp"
#include "item_factory.hpp"
#include "game.hpp"
#include "player_bon.hpp"
#include "game_time.hpp"
#include "audio.hpp"
#include "io.hpp"

namespace reload
{

namespace
{

void msg_reload_fumble(const Actor& actor, const Item& ammo)
{
    const std::string ammo_name = ammo.name(ItemRefType::a);

    if (actor.is_player())
    {
        msg_log::add("I fumble with " + ammo_name + ".");
    }
    else //Is monster
    {
        if (map::player->can_see_actor(actor))
        {
            msg_log::add(actor.name_the() + " fumbles with " + ammo_name + ".");
        }
    }
}

void msg_reloaded(const Actor& actor,
                  const Wpn& wpn,
                  const Item& ammo)
{
    if (actor.is_player())
    {
        audio::play(wpn.data().ranged.reload_sfx);

        if (ammo.data().type == ItemType::ammo_mag)
        {
            const std::string wpn_name =
                wpn.name(ItemRefType::plain, ItemRefInf::none);

            msg_log::add("I reload my " + wpn_name +
                         " (" + std::to_string(wpn.nr_ammo_loaded_) +
                         "/" +
                         std::to_string(wpn.data().ranged.max_ammo) + ").");
        }
        else //Not a mag
        {
            const std::string ammo_name = ammo.name(ItemRefType::a);

            msg_log::add("I load " + ammo_name +
                         " (" + std::to_string(wpn.nr_ammo_loaded_) +
                         "/" +
                         std::to_string(wpn.data().ranged.max_ammo) + ").");
        }
    }
    else //Is monster
    {
        if (map::player->can_see_actor(actor))
        {
            msg_log::add(actor.name_the() + " reloads.");
        }
    }
}

} //namespace

void try_reload(Actor& actor, Item* const item_to_reload)
{
    if (!item_to_reload)
    {
        msg_log::add("I am not wielding a weapon.");

        return;
    }

    ASSERT(item_to_reload->data().type == ItemType::melee_wpn ||
           item_to_reload->data().type == ItemType::ranged_wpn);

    Wpn* const wpn = static_cast<Wpn*>(item_to_reload);

    const int wpn_max_ammo = wpn->data().ranged.max_ammo;

    if (wpn_max_ammo == 0)
    {
        msg_log::add("This weapon does not use ammo.");
        return;
    }

    const int ammo_loaded_before = wpn->nr_ammo_loaded_;

    if (ammo_loaded_before >= wpn_max_ammo)
    {
        const std::string item_name =
            wpn->name(ItemRefType::plain, ItemRefInf::none);

        msg_log::add("My " + item_name + " is already loaded.");
        return;
    }

    const ItemId ammo_item_id = wpn->data().ranged.ammo_item_id;

    const ItemDataT& ammo_data = item_data::data[(size_t)ammo_item_id];

    const bool is_using_mag = ammo_data.type == ItemType::ammo_mag;

    Item* ammo_item = nullptr;

    size_t ammo_backpack_idx = 0;

    int max_mag_ammo = 0;

    Inventory& inv = actor.inv();

    // Find ammo in backpack to reload from
    for (size_t i = 0; i < inv.backpack_.size(); ++i)
    {
        Item* const item = inv.backpack_[i];

        if (item->id() == ammo_item_id)
        {
            if (is_using_mag)
            {
                // Find mag with most ammo in it

                const AmmoMag* const mag = static_cast<const AmmoMag*>(item);

                const int nr_ammo = mag->ammo_;

                if (nr_ammo > max_mag_ammo)
                {
                    max_mag_ammo = nr_ammo;

                    if (nr_ammo > ammo_loaded_before)
                    {
                        // Magazine is a candidate for reloading
                        ammo_item = item;

                        ammo_backpack_idx = i;
                    }
                }
            }
            else // Not using mag
            {
                // Just use first item with matching ammo id
                ammo_item = item;

                ammo_backpack_idx = i;

                break;
            }
        }
    }

    if (!ammo_item)
    {
        // Loaded mag has more ammo than any mag in backpack
        if (is_using_mag &&
            max_mag_ammo > 0 &&
            max_mag_ammo <= ammo_loaded_before)
        {
            const std::string mag_name =
                ammo_data.base_name.names[(size_t)ItemRefType::plain];

            msg_log::add("I carry no " +
                         mag_name +
                         " with more ammo than already loaded.");
        }
        else
        {
            msg_log::add("I carry no ammunition for this weapon.");
        }

        return;
    }

    // Being blinded or terrified makes it harder to reload
    const bool is_blind = !actor.prop_handler().allow_see();

    const bool is_terrified = actor.has_prop(PropId::terrified);

    const int k = 48;

    const int fumble_pct = (k * is_blind) + (k * is_terrified);

    int speed_pct_diff = 0;

    if (rnd::percent(fumble_pct))
    {
        msg_reload_fumble(actor, *ammo_item);
    }
    else // Not fumbling
    {
        if (actor.is_player())
        {
            if (player_bon::traits[size_t(Trait::adept_marksman)])
            {
                speed_pct_diff = 50;
            }

            if (player_bon::traits[size_t(Trait::expert_marksman)])
            {
                speed_pct_diff = 50;
            }
        }

        bool is_mag = ammo_item->data().type == ItemType::ammo_mag;

        if (is_mag)
        {
            AmmoMag* mag_item = static_cast<AmmoMag*>(ammo_item);

            wpn->nr_ammo_loaded_ = mag_item->ammo_;

            msg_reloaded(actor, *wpn, *ammo_item);

            // Destroy loaded mag
            inv.remove_item_in_backpack_with_idx(ammo_backpack_idx, true);

            // If weapon previously contained ammo, create a new mag item
            if (ammo_loaded_before > 0)
            {
                ammo_item = item_factory::mk(ammo_item_id);

                mag_item = static_cast<AmmoMag*>(ammo_item);

                mag_item->ammo_ = ammo_loaded_before;

                inv.put_in_backpack(mag_item);
            }
        }
        else // Not using mag
        {
            ++wpn->nr_ammo_loaded_;

            msg_reloaded(actor, *wpn, *ammo_item);

            inv.decr_item_in_backpack(ammo_backpack_idx);
        }
    }

    game_time::tick(speed_pct_diff);
}

void player_arrange_pistol_mags()
{
    Player&     player  = *map::player;
    Inventory&  inv     = player.inv();

    Wpn* wielded_pistol = nullptr;

    Item* const wielded_item = inv.item_in_slot(SlotId::wpn);

    if (wielded_item && wielded_item->id() == ItemId::pistol)
    {
        wielded_pistol = static_cast<Wpn*>(wielded_item);
    }

    // Find the two non-full magazines in the backpack with the least/most ammo.
    // NOTE: The min and max magazines may be the same item.
    int min_mag_ammo = INT_MAX;

    int max_mag_ammo = 0;

    AmmoMag* min_mag = nullptr; // Most empty magazine

    AmmoMag* max_mag = nullptr; // Most full magazine

    size_t min_mag_backpack_idx = 0;

    const int pistol_max_ammo =
        item_data::data[(size_t)ItemId::pistol].ranged.max_ammo;

    for (size_t i = 0; i < inv.backpack_.size(); ++i)
    {
        Item* const item = inv.backpack_[i];

        if (item->id() != ItemId::pistol_mag)
        {
            continue;
        }

        AmmoMag* const mag = static_cast<AmmoMag*>(item);

        if (mag->ammo_ == pistol_max_ammo)
        {
            continue;
        }

        // NOTE: For min mag, we check for lesser OR EQUAL rounds loaded - this
        //       way, when several "least full mags" are found, the last one
        //       will be picked as THE max mag to use. The purpose of this is
        //       that we should try avoid picking the same mag as min and max
        //       (e.g. if we have two mags with 6 bullets each, then we want to
        //       move a bullet).
        if (mag->ammo_ <= min_mag_ammo)
        {
            min_mag_ammo            = mag->ammo_;
            min_mag                 = mag;
            min_mag_backpack_idx    = i;
        }

        // Use the first "most full mag" that we find, as the max mag
        if (mag->ammo_ > max_mag_ammo)
        {
            max_mag_ammo    = mag->ammo_;
            max_mag         = mag;
        }
    }

    if (!min_mag)
    {
        //No least full mag exists, do nothing
        return;
    }

    //If wielded pistol is not fully loaded, move round from least full mag
    if (
        wielded_pistol &&
        wielded_pistol->nr_ammo_loaded_ < pistol_max_ammo)
    {
        --min_mag->ammo_;
        ++wielded_pistol->nr_ammo_loaded_;

        const std::string name =
            wielded_pistol->name(ItemRefType::plain,
                                 ItemRefInf::yes);

        msg_log::add("I move a round from a magazine to my " + name + ".");
    }
    //Otherwise, if two non-full mags exists, move from least to most full
    else if (
        max_mag &&
        min_mag != max_mag)
    {
        --min_mag->ammo_;
        ++max_mag->ammo_;

        msg_log::add("I move a round from one magazine to another.");
    }

    if (min_mag->ammo_ == 0)
    {
        inv.remove_item_in_backpack_with_idx(min_mag_backpack_idx, true);
    }
}

} //reload

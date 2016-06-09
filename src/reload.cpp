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
#include "dungeon_master.hpp"
#include "player_bon.hpp"
#include "game_time.hpp"
#include "audio.hpp"
#include "render.hpp"

namespace reload
{

namespace
{

void msg_reload_fumble(const Actor& actor, const Item& ammo)
{
    const std::string ammo_name = ammo.name(Item_ref_type::a);

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
                  const Item& ammo,
                  const Pass_time pass_time)
{
    const std::string swift_str = pass_time == Pass_time::no ? " swiftly" : "";

    if (actor.is_player())
    {
        audio::play(wpn.data().ranged.reload_sfx);

        if (ammo.data().type == Item_type::ammo_mag)
        {
            const std::string wpn_name = wpn.name(Item_ref_type::plain, Item_ref_inf::none);

            msg_log::add("I" + swift_str + " reload my " + wpn_name +
                         " (" + to_str(wpn.nr_ammo_loaded_) + "/" +
                         to_str(wpn.data().ranged.max_ammo) + ").");
        }
        else //Not a mag
        {
            const std::string ammo_name = ammo.name(Item_ref_type::a);

            msg_log::add("I" + swift_str + " load " + ammo_name +
                         " (" + to_str(wpn.nr_ammo_loaded_) + "/" +
                         to_str(wpn.data().ranged.max_ammo) + ").");
        }

        render::draw_map_state();
    }
    else //Is monster
    {
        if (map::player->can_see_actor(actor))
        {
            msg_log::add(actor.name_the() + swift_str + " reloads.");
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

    ASSERT(item_to_reload->data().type == Item_type::melee_wpn ||
           item_to_reload->data().type == Item_type::ranged_wpn);

    Wpn* const wpn = static_cast<Wpn*>(item_to_reload);

    const int WPN_MAX_AMMO = wpn->data().ranged.max_ammo;

    if (WPN_MAX_AMMO == 0)
    {
        msg_log::add("This weapon does not use ammo.");
        return;
    }

    const int AMMO_LOADED_BEFORE = wpn->nr_ammo_loaded_;

    if (AMMO_LOADED_BEFORE >= WPN_MAX_AMMO)
    {
        const std::string item_name = wpn->name(Item_ref_type::plain, Item_ref_inf::none);

        msg_log::add("My " + item_name + " is already loaded.");
        return;
    }

    const Item_id       ammo_item_id    = wpn->data().ranged.ammo_item_id;
    const Item_data_t&  ammo_data       = item_data::data[(size_t)ammo_item_id];
    const bool          IS_USING_MAG    = ammo_data.type == Item_type::ammo_mag;

    Item*   ammo_item           = nullptr;
    size_t  ammo_backpack_idx   = 0;
    int     max_mag_ammo        = 0;

    Inventory& inv = actor.inv();

    //Find ammo in backpack to reload from
    for (size_t i = 0; i < inv.backpack_.size(); ++i)
    {
        Item* const item = inv.backpack_[i];

        if (item->id() == ammo_item_id)
        {
            if (IS_USING_MAG)
            {
                //Find mag with most ammo in it

                const Ammo_mag* const mag = static_cast<const Ammo_mag*>(item);

                const int nr_ammo = mag->ammo_;

                if (nr_ammo > max_mag_ammo)
                {
                    max_mag_ammo = nr_ammo;

                    if (nr_ammo > AMMO_LOADED_BEFORE)
                    {
                        //Magazine is a candidate for reloading
                        ammo_item           = item;
                        ammo_backpack_idx   = i;
                    }
                }
            }
            else //Not using mag
            {
                //Just use first item with matching ammo id
                ammo_item           = item;
                ammo_backpack_idx   = i;

                break;
            }
        }
    }

    if (!ammo_item)
    {
        //Loaded mag has more ammo than any mag in backpack
        if (
            IS_USING_MAG        &&
            max_mag_ammo > 0    &&
            max_mag_ammo <= AMMO_LOADED_BEFORE)
        {
            const std::string mag_name = ammo_data.base_name.names[(size_t)Item_ref_type::plain];

            msg_log::add("I carry no " + mag_name + " with more ammo than already loaded.");
        }
        else
        {
            msg_log::add("I carry no ammunition for this weapon.");
        }

        return;
    }

    //Being blinded or terrified makes it harder to reload
    const bool  IS_BLIND        = !actor.prop_handler().allow_see();
    const bool  IS_TERRIFIED    = actor.has_prop(Prop_id::terrified);
    const int   K               = 48;

    const int FUMBLE_PCT = (K * IS_BLIND) + (K * IS_TERRIFIED);

    Pass_time pass_time = Pass_time::yes;

    if (rnd::percent(FUMBLE_PCT))
    {
        msg_reload_fumble(actor, *ammo_item);
    }
    else //Not fumbling
    {
        if (actor.is_player())
        {
            const int SWIFT_RELOAD_N_IN_10 =
                player_bon::traits[size_t(Trait::master_marksman)]  ? 7 :
                player_bon::traits[size_t(Trait::expert_marksman)]  ? 5 :
                player_bon::traits[size_t(Trait::adept_marksman)]   ? 3 : 0;

            pass_time = rnd::fraction(SWIFT_RELOAD_N_IN_10, 10) ?
                        Pass_time::no : Pass_time::yes;
        }

        bool is_mag = ammo_item->data().type == Item_type::ammo_mag;

        if (is_mag)
        {
            Ammo_mag* mag_item = static_cast<Ammo_mag*>(ammo_item);

            wpn->nr_ammo_loaded_ = mag_item->ammo_;

            msg_reloaded(actor, *wpn, *ammo_item, pass_time);

            //Destroy loaded mag
            inv.remove_item_in_backpack_with_idx(ammo_backpack_idx, true);

            //If weapon previously contained ammo, create a new mag item
            if (AMMO_LOADED_BEFORE > 0)
            {
                ammo_item       = item_factory::mk(ammo_item_id);
                mag_item        = static_cast<Ammo_mag*>(ammo_item);
                mag_item->ammo_ = AMMO_LOADED_BEFORE;

                inv.put_in_backpack(mag_item);
            }
        }
        else //Not using mag
        {
            ++wpn->nr_ammo_loaded_;

            msg_reloaded(actor, *wpn, *ammo_item, pass_time);

            inv.decr_item_in_backpack(ammo_backpack_idx);
        }
    }

    game_time::tick(pass_time);
}

void player_arrange_pistol_mags()
{
    Player&     player  = *map::player;
    Inventory&  inv     = player.inv();

    Wpn* wielded_pistol = nullptr;

    Item* const wielded_item = inv.item_in_slot(Slot_id::wpn);

    if (wielded_item && wielded_item->id() == Item_id::pistol)
    {
        wielded_pistol = static_cast<Wpn*>(wielded_item);
    }

    //Find the two non-full magazines in the backpack with the least/most ammo.
    //NOTE: The min and max magazines may be the same item.
    int         min_mag_ammo            = INT_MAX;
    int         max_mag_ammo            = 0;
    Ammo_mag*   min_mag                 = nullptr;  //Most empty magazine
    Ammo_mag*   max_mag                 = nullptr;  //Most full magazine
    size_t      min_mag_backpack_idx    = 0;

    const int PISTOL_MAX_AMMO =
        item_data::data[(size_t)Item_id::pistol].ranged.max_ammo;

    for (size_t i = 0; i < inv.backpack_.size(); ++i)
    {
        Item* const item = inv.backpack_[i];

        if (item->id() != Item_id::pistol_mag)
        {
            continue;
        }

        Ammo_mag* const mag = static_cast<Ammo_mag*>(item);

        if (mag->ammo_ == PISTOL_MAX_AMMO)
        {
            continue;
        }

        //NOTE: For min mag, we check for lesser OR EQUAL rounds loaded - this
        //way, when several "least full mags" are found, the LAST one will be
        //picked as THE max mag to use. The purpose of this is that we should
        //try avoid picking the same mag as min and max (e.g. if we have two
        //mags with 6 bullets each, then we want to move a bullet).
        if (mag->ammo_ <= min_mag_ammo)
        {
            min_mag_ammo            = mag->ammo_;
            min_mag                 = mag;
            min_mag_backpack_idx    = i;
        }

        //Use the first "most full mag" that we find, as the max mag
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
        wielded_pistol->nr_ammo_loaded_ < PISTOL_MAX_AMMO)
    {
        --min_mag->ammo_;
        ++wielded_pistol->nr_ammo_loaded_;

        const std::string name =
            wielded_pistol->name(Item_ref_type::plain,
                                 Item_ref_inf::yes);

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

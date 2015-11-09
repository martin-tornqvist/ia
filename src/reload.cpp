#include "reload.hpp"

#include <string>

#include "converters.hpp"
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
#include "utils.hpp"

namespace reload
{

namespace
{

void output_reload_fumble(const Actor& actor, const Item& ammo)
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

void output_reloaded(const Actor& actor,
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

        render::draw_map_and_interface();
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

    assert(item_to_reload->data().type == Item_type::melee_wpn ||
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
    const Item_data_t&  ammo_data       = item_data::data[size_t(ammo_item_id)];
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
            const std::string mag_name = ammo_data.base_name.names[size_t(Item_ref_type::plain)];

            msg_log::add("I carry no " + mag_name + " with more ammo than already loaded.");
        }
        else
        {
            msg_log::add("I carry no ammunition for this weapon.");
        }

        return;
    }

    //Being blinded or terrified makes it harder to reload (if both, it's extremely difficult)
    const bool  IS_BLIND        = !actor.prop_handler().allow_see();
    const bool  IS_TERRIFIED    = actor.has_prop(Prop_id::terrified);
    const int   K               = 48;

    const int FUMBLE_PCT = (K * IS_BLIND) + (K * IS_TERRIFIED);

    Pass_time pass_time = Pass_time::yes;

    if (rnd::percent(FUMBLE_PCT))
    {
        output_reload_fumble(actor, *ammo_item);
    }
    else //Not fumbling
    {
        if (actor.is_player())
        {
            const bool IS_EXP_MARK = player_bon::traits[size_t(Trait::expert_marksman)];

            pass_time = (IS_EXP_MARK && rnd::coin_toss()) ?
                          Pass_time::no : Pass_time::yes;
        }

        bool is_mag = ammo_item->data().type == Item_type::ammo_mag;

        if (is_mag)
        {
            Ammo_mag* mag_item = static_cast<Ammo_mag*>(ammo_item);

            wpn->nr_ammo_loaded_ = mag_item->ammo_;

            output_reloaded(actor, *wpn, *ammo_item, pass_time);

            //Destroy loaded mag
            inv.remove_item_in_backpack_with_idx(ammo_backpack_idx, true);

            //If weapon previously contained ammo, create a new mag item
            if (AMMO_LOADED_BEFORE > 0)
            {
                ammo_item           = item_factory::mk(ammo_item_id);
                mag_item           = static_cast<Ammo_mag*>(ammo_item);
                mag_item->ammo_    = AMMO_LOADED_BEFORE;

                inv.put_in_backpack(mag_item);
            }
        }
        else //Not using mag
        {
            ++wpn->nr_ammo_loaded_;

            output_reloaded(actor, *wpn, *ammo_item, pass_time);

            inv.decr_item_in_backpack(ammo_backpack_idx);
        }
    }

    game_time::tick(pass_time);
}

void player_arrange_pistol_mags()
{
    Player&     player  = *map::player;
    Inventory&  inv     = player.inv();

    Item* const wielded_item    = inv.item_in_slot(Slot_id::wpn);
    Item* const ready_item      = inv.item_in_slot(Slot_id::wpn_alt);

    Wpn* wielded_pistol   = nullptr;
    Wpn* ready_pistol     = nullptr;

    if (wielded_item && wielded_item->id() == Item_id::pistol)
    {
        wielded_pistol = static_cast<Wpn*>(wielded_item);
    }

    if (ready_item && ready_item->id() == Item_id::pistol)
    {
        ready_pistol = static_cast<Wpn*>(ready_item);
    }

    //Find the two magazines in the backpack with the least and most ammo, respectively
    //NOTE: The min and max magazines can be the same item
    int         min_mag_ammo            = INT_MAX;
    int         max_mag_ammo            = 0;
    Ammo_mag*   min_mag                 = nullptr;
    Ammo_mag*   max_mag                 = nullptr;
    size_t      min_mag_backpack_idx    = 0;

    for (size_t i = 0; i < inv.backpack_.size(); ++i)
    {
        Item* const item = inv.backpack_[i];

        if (item->id() == Item_id::pistol_mag)
        {
            Ammo_mag* const mag = static_cast<Ammo_mag*>(item);

            if (mag->ammo_ < min_mag_ammo)
            {
                min_mag_ammo            = mag->ammo_;
                min_mag                 = mag;
                min_mag_backpack_idx    = i;
            }

            if (mag->ammo_ > max_mag_ammo)
            {
                max_mag_ammo    = mag->ammo_;
                max_mag         = mag;
            }
        }
    }

    const int PISTOL_MAX_AMMO = item_data::data[size_t(Item_id::pistol)].ranged.max_ammo;

    auto try_move_to_pistol = [&](Wpn * const wpn, Ammo_mag * const mag)
    {
        if (
            wpn &&
            mag &&
            wpn->nr_ammo_loaded_ < PISTOL_MAX_AMMO)
        {
            --min_mag->ammo_;
            ++wielded_pistol->nr_ammo_loaded_;

            if (min_mag->ammo_ == 0)
            {
                inv.remove_item_in_backpack_with_idx(min_mag_backpack_idx, true);
            }

            const std::string name = wpn->name(Item_ref_type::plain, Item_ref_inf::yes);

            msg_log::add("I move a round from a magazine to my " + name + ".");

            return true;
        }

        return false;
    };

    //-----------------------------------------------------------------------------
    // If a mag exists with more ammo than wielded pistol, reload.
    //-----------------------------------------------------------------------------
    if (
        wielded_pistol  &&
        max_mag         &&
        wielded_pistol->nr_ammo_loaded_ < max_mag_ammo)
    {
        //NOTE: The reload function will choose the mag with most ammo
        try_reload(player, wielded_pistol);
        return;
    }

    //-----------------------------------------------------------------------------
    //If wielded pistol is not fully loaded and a mag exists, move from min mag
    //-----------------------------------------------------------------------------
    if (try_move_to_pistol(wielded_pistol, min_mag))
    {
        game_time::tick();
        return;
    }

    //-----------------------------------------------------------------------------
    //If a mag exists with more ammo than readied pistol, reload.
    //-----------------------------------------------------------------------------
    if (
        ready_pistol    &&
        max_mag         &&
        ready_pistol->nr_ammo_loaded_ < max_mag_ammo)
    {
        //NOTE: The reload function will choose the mag with most ammo
        try_reload(player, ready_pistol);
        return;
    }

    //-----------------------------------------------------------------------------
    //If readied pistol is not fully loaded and a mag exists, move from min mag
    //-----------------------------------------------------------------------------
    if (try_move_to_pistol(ready_pistol, min_mag))
    {
        game_time::tick();
        return;
    }

    //-----------------------------------------------------------------------------
    //If two non-full mags exists, move from lowest mag to highest
    //-----------------------------------------------------------------------------
    int         mag_2_ammo  = 0;
    Ammo_mag*   mag_2       = nullptr;

    for (size_t i = 0; i < inv.backpack_.size(); ++i)
    {
        Item* const item = inv.backpack_[i];

        if (item->id() == Item_id::pistol_mag && item != min_mag)
        {
            Ammo_mag* const mag = static_cast<Ammo_mag*>(item);

            if (mag->ammo_ > mag_2_ammo && mag->ammo_ < PISTOL_MAX_AMMO)
            {
                mag_2_ammo  = mag->ammo_;
                mag_2       = mag;
            }
        }
    }

    if (min_mag && mag_2)
    {
        --min_mag->ammo_;
        ++mag_2->ammo_;

        if (min_mag->ammo_ == 0)
        {
            inv.remove_item_in_backpack_with_idx(min_mag_backpack_idx, true);
        }

        msg_log::add("I move a round from one magazine to another.");

        game_time::tick();

        return;
    }

    //-----------------------------------------------------------------------------
    //No action needed
    //-----------------------------------------------------------------------------
    if (min_mag || max_mag)
    {
        msg_log::add("My pistol magazines are fully organized.");
    }
    else //No magazines carried
    {
        msg_log::add("I carry no extra pistol magazines.");
    }
}

} //reload

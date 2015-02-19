#include "reload.hpp"

#include <string>

#include "converters.hpp"
#include "item.hpp"
#include "actor_player.hpp"
#include "log.hpp"
#include "map.hpp"
#include "inventory.hpp"
#include "item_factory.hpp"
#include "dungeon_master.hpp"
#include "player_bon.hpp"
#include "game_time.hpp"
#include "audio.hpp"
#include "render.hpp"
#include "utils.hpp"

using namespace std;

namespace reload
{

namespace
{

void print_msg_and_play_sfx(Actor& actor_reloading, Wpn* const wpn,
                        Item* const ammo, const Reload_result result,
                        const bool IS_SWIFT_RELOAD)
{
    string ammo_name = "";
    bool is_clip = false;

    if (ammo)
    {
        ammo_name  = ammo->get_name(Item_ref_type::a);
        is_clip    = ammo->get_data().type == Item_type::ammo_clip;
    }

    const bool IS_PLAYER    = actor_reloading.is_player();
    const string actor_name  = actor_reloading.get_name_the();

    switch (result)
    {
    case Reload_result::not_carrying_wpn:
        if (IS_PLAYER)
        {
            Log::add_msg("I am not wielding a weapon.");
        }
        break;

    case Reload_result::wpn_not_using_ammo:
        if (IS_PLAYER)
        {
            Log::add_msg("This weapon does not use ammo.");
        }
        break;

    case Reload_result::already_full:
        if (IS_PLAYER)
        {
            Log::add_msg("Weapon already loaded.");
        }
        break;

    case Reload_result::no_ammo:
        if (IS_PLAYER)
        {
            Log::add_msg("I carry no ammunition for this weapon.");
        }
        break;

    case Reload_result::success:
    {
        const string swift_str = IS_SWIFT_RELOAD ? " swiftly" : "";
        if (IS_PLAYER)
        {
            Audio::play(wpn->get_data().ranged.reload_sfx);

            if (is_clip)
            {
                const string wpn_name = wpn->get_name(Item_ref_type::plain, Item_ref_inf::none);
                Log::add_msg(
                    "I" + swift_str + " reload the " + wpn_name +
                    " (" + to_str(wpn->nr_ammo_loaded) + "/" + to_str(wpn->AMMO_CAP) + ").");
            }
            else
            {
                Log::add_msg(
                    "I" + swift_str + " load " + ammo_name + " (" + to_str(wpn->nr_ammo_loaded) +
                    "/" + to_str(wpn->AMMO_CAP) + ").");
            }
            Render::draw_map_and_interface();
        }
        else
        {
            if (Map::player->can_see_actor(actor_reloading, nullptr))
            {
                Log::add_msg(actor_name + swift_str + " reloads.");
            }
        }
    } break;

    case Reload_result::fumble:
        if (IS_PLAYER)
        {
            Log::add_msg("I fumble with " + ammo_name + ".");
        }
        else
        {
            if (Map::player->can_see_actor(actor_reloading, nullptr))
            {
                Log::add_msg(actor_name + " fumbles with " + ammo_name + ".");
            }
        }
        break;
    }
}

} //namespace

bool reload_wielded_wpn(Actor& actor_reloading)
{
    bool did_act = false;

    Inventory& inv      = actor_reloading.get_inv();
    Item* const wpn_item = inv.get_item_in_slot(Slot_id::wielded);

    if (!wpn_item)
    {
        print_msg_and_play_sfx(actor_reloading, nullptr, nullptr,
                           Reload_result::not_carrying_wpn, false);
        return did_act;
    }

    Wpn* const    wpn           = static_cast<Wpn*>(wpn_item);
    Reload_result  result        = Reload_result::no_ammo;
    bool          is_swift_reload = false;

    if (actor_reloading.is_player())
    {
        is_swift_reload = Player_bon::traits[int(Trait::expert_marksman)] &&
                        Rnd::coin_toss();
    }

    const int wpn_ammo_capacity = wpn->AMMO_CAP;

    if (wpn_ammo_capacity == 0)
    {
        print_msg_and_play_sfx(actor_reloading, wpn, nullptr,
                           Reload_result::wpn_not_using_ammo, false);
    }
    else
    {
        const Item_id ammo_type = wpn->get_data().ranged.ammo_item_id;
        Item* item            = nullptr;

        if (wpn->nr_ammo_loaded < wpn_ammo_capacity)
        {
            for (size_t i = 0; i < inv.general_.size(); ++i)
            {
                item = inv.general_[i];

                if (item->get_id() == ammo_type)
                {
                    Prop_handler& prop_hlr = actor_reloading.get_prop_handler();

                    bool props[size_t(Prop_id::END)];
                    prop_hlr.get_prop_ids(props);

                    const bool IS_RELOADER_BLIND      = !actor_reloading.get_prop_handler().allow_see();
                    const bool IS_REALOADER_TERRIFIED = props[int(Prop_id::terrified)];

                    const int CHANCE_TO_FUMBLE = (IS_RELOADER_BLIND      ? 48 : 0) +
                                                 (IS_REALOADER_TERRIFIED ? 48 : 0);

                    if (Rnd::percent() < CHANCE_TO_FUMBLE)
                    {
                        is_swift_reload = false;
                        result        = Reload_result::fumble;

                        print_msg_and_play_sfx(actor_reloading, nullptr, item, Reload_result::fumble,
                                           false);
                    }
                    else //Not fumbling
                    {
                        result      = Reload_result::success;
                        bool is_clip = item->get_data().type == Item_type::ammo_clip;

                        if (is_clip)
                        {
                            const int previous_ammo_count = wpn->nr_ammo_loaded;
                            Ammo_clip* clip_item          = static_cast<Ammo_clip*>(item);
                            wpn->nr_ammo_loaded           = clip_item->ammo_;

                            print_msg_and_play_sfx(actor_reloading, wpn, item, result, is_swift_reload);

                            //Destroy loaded clip
                            inv.remove_item_in_backpack_with_idx(i, true);

                            //If weapon previously contained ammo, create a new clip item
                            if (previous_ammo_count > 0)
                            {
                                item = Item_factory::mk(ammo_type);
                                clip_item = static_cast<Ammo_clip*>(item);
                                clip_item->ammo_ = previous_ammo_count;
                                inv.put_in_general(clip_item);
                            }
                        }
                        else //Ammo is stackable (e.g. shotgun shells)
                        {
                            wpn->nr_ammo_loaded += 1;

                            print_msg_and_play_sfx(actor_reloading, wpn, item, result, is_swift_reload);

                            //Decrease ammo item number
                            inv.decr_item_in_general(i);
                        }
                    }
                    break;
                }
            }
            if (result == Reload_result::no_ammo)
            {
                print_msg_and_play_sfx(actor_reloading, wpn, item, result, is_swift_reload);
            }
        }
        else //Weapon is full
        {
            result = Reload_result::already_full;
            print_msg_and_play_sfx(actor_reloading, wpn, item, result, is_swift_reload);
        }
    }

    if (result == Reload_result::success || result == Reload_result::fumble)
    {
        did_act = true;
        Game_time::tick(is_swift_reload);
    }

    return did_act;
}

} //Reload

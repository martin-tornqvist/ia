#include "init.hpp"

#include <memory>

#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map_travel.hpp"
#include "input.hpp"
#include "reload.hpp"
#include "kick.hpp"
#include "render.hpp"
#include "close.hpp"
#include "jam_with_spike.hpp"
#include "pickup.hpp"
#include "inventory_handling.hpp"
#include "marker.hpp"
#include "map.hpp"
#include "dungeon_master.hpp"
#include "player_spells_handling.hpp"
#include "manual.hpp"
#include "character_descr.hpp"
#include "query.hpp"
#include "save_handling.hpp"
#include "item_factory.hpp"
#include "actor_factory.hpp"
#include "actor_mon.hpp"
#include "player_bon.hpp"
#include "create_character.hpp"
#include "disarm.hpp"
#include "sdl_wrapper.hpp"
#include "popup.hpp"
#include "look.hpp"
#include "attack.hpp"
#include "throwing.hpp"
#include "utils.hpp"

using namespace std;

namespace input
{

namespace
{

SDL_Event sdl_event_;
bool is_inited_ = false;

void query_quit()
{
    const vector<string> quit_choices = vector<string> {"yes", "no"};
    const int QUIT_CHOICE = popup::show_menu_msg (
                                "Save and highscore are not kept.",
                                false, quit_choices, "Quit the current game?");

    if (QUIT_CHOICE == 0)
    {
        init::quit_to_main_menu = true;
        render::clear_screen();
        render::update_screen();
    }
}

} //Namespace

void init()
{
    is_inited_ = true;
}

void cleanup()
{
    is_inited_ = false;
}

void handle_map_mode_key_press(const Key_data& d)
{
    //----------------------------------- MOVEMENT
    if (d.sdl_key == SDLK_RIGHT || d.key == '6' || d.key == 'l')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();

            if (d.is_shift_held)
            {
                map::player->move_dir (Dir::up_right);
            }
            else if (d.is_ctrl_held)
            {
                map::player->move_dir (Dir::down_right);
            }
            else
            {
                map::player->move_dir (Dir::right);
            }
        }

        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_DOWN || d.key == '2' || d.key == 'j')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();
            map::player->move_dir (Dir::down);
        }

        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_LEFT || d.key == '4' || d.key == 'h')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();

            if (d.is_shift_held)
            {
                map::player->move_dir (Dir::up_left);
            }
            else if (d.is_ctrl_held)
            {
                map::player->move_dir (Dir::down_left);
            }
            else
            {
                map::player->move_dir (Dir::left);
            }
        }

        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_UP || d.key == '8' || d.key == 'k')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();
            map::player->move_dir (Dir::up);
        }

        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_PAGEUP || d.key == '9' || d.key == 'u')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();
            map::player->move_dir (Dir::up_right);
        }

        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_PAGEDOWN || d.key == '3' || d.key == 'n')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();
            map::player->move_dir (Dir::down_right);
        }

        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_END || d.key == '1' || d.key == 'b')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();
            map::player->move_dir (Dir::down_left);
        }

        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_HOME || d.key == '7' || d.key == 'y')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();
            map::player->move_dir (Dir::up_left);
        }

        clear_events();
        return;
    }
    else if (d.key == '5' || d.key == '.')
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();

            if (player_bon::traits[int (Trait::steady_aimer)])
            {
                Prop_handler& prop_hlr = map::player->get_prop_handler();

                int nr_turns_aiming_old = 0;

                if (player_bon::traits[int (Trait::sharp_shooter)])
                {
                    Prop* const prop_aiming_old =
                        prop_hlr.get_prop (Prop_id::aiming, Prop_src::applied);

                    if (prop_aiming_old)
                    {
                        nr_turns_aiming_old =
                            static_cast<Prop_aiming*> (prop_aiming_old)->nr_turns_aiming;
                    }
                }

                Prop_aiming* const aiming = new Prop_aiming (Prop_turns::specific, 1);
                aiming->nr_turns_aiming += nr_turns_aiming_old;
                prop_hlr.try_apply_prop (aiming);
            }

            map::player->move_dir (Dir::center);
        }

        clear_events();
        return;
    }

    //----------------------------------- MANUAL
    else if (d.key == '?')
    {
        msg_log::clear();
        manual::run();
        clear_events();
        return;
    }

    //----------------------------------- OPTIONS
    else if (d.key == '=')
    {
        msg_log::clear();
        config::run_options_menu();
        clear_events();
        return;
    }

    //----------------------------------- RELOAD
    else if (d.key == 'r')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            reload::reload_wielded_wpn (* (map::player) );
        }

        clear_events();
        return;
    }

    //----------------------------------- KICK
    else if ( (d.key == 'q') )
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            kick::player_kick();
            render::draw_map_and_interface();
        }

        clear_events();
        return;
    }

    //----------------------------------- CLOSE
    else if (d.key == 'c')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            close::player_close();
        }

        clear_events();
        return;
    }

    //----------------------------------- JAM
    else if (d.key == 'D')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            jam_with_spike::player_jam();
        }

        clear_events();
        return;
    }

    //----------------------------------- DISARM
    else if (d.key == 'd')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            disarm::player_disarm();
        }

        clear_events();
        return;
    }

    //----------------------------------- UNLOAD AMMO FROM GROUND
    else if (d.key == 'G')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            item_pickup::try_unload_wpn_or_pickup_ammo();
        }

        clear_events();
        return;
    }

    //----------------------------------- AIM/FIRE FIREARM
    else if (d.key == 'f')
    {
        msg_log::clear();

        if (
            map::player->is_alive() &&
            map::player->get_prop_handler().allow_attack_ranged (true) )
        {
            auto* const item = map::player->get_inv().get_item_in_slot (Slot_id::wielded);

            if (item)
            {
                const Item_data_t& item_data = item->get_data();

                if (item_data.ranged.is_ranged_wpn)
                {
                    auto* wpn = static_cast<Wpn*> (item);

                    //TODO: Quick hack for the Mi-go gun, shouldn't be here - refactor
                    if (
                        wpn->get_data().id == Item_id::mi_go_gun  &&
                        wpn->nr_ammo_loaded == 0                &&
                        map::player->get_hp() > 1)
                    {
                        const string wpn_name = wpn->get_name (Item_ref_type::plain,
                                                               Item_ref_inf::none);

                        msg_log::add ("The " + wpn_name + " draws power from my essence!",
                                      clr_msg_bad);

                        render::draw_map_and_interface();
                        ++wpn->nr_ammo_loaded;
                        map::player->hit (1, Dmg_type::pure);
                        return;
                    }

                    if (wpn->nr_ammo_loaded >= 1 || item_data.ranged.has_infinite_ammo)
                    {
                        auto on_marker_at_pos = [&] (const Pos & p)
                        {
                            msg_log::clear();
                            look::print_location_info_msgs (p);

                            auto* const actor = utils::get_actor_at_pos (p);

                            if (
                                actor               &&
                                !actor->is_player()  &&
                                map::player->can_see_actor (*actor, nullptr) )
                            {
                                bool tgt_props[size_t (Prop_id::END)];
                                actor->get_prop_handler().get_prop_ids (tgt_props);

                                const bool GETS_UNDEAD_BANE_BON =
                                    player_bon::gets_undead_bane_bon (*map::player,
                                                                      actor->get_data() );

                                if (
                                    !tgt_props[int (Prop_id::ethereal)] ||
                                    GETS_UNDEAD_BANE_BON)
                                {
                                    Ranged_att_data data (*map::player, *wpn, actor->pos,
                                                          actor->pos);
                                    msg_log::add (to_str (data.hit_chance_tot) +
                                                  "% hit chance.");
                                }
                            }

                            msg_log::add ("[f] to fire" + cancel_info_str);
                        };

                        auto on_key_press = [&] (const Pos & p, const Key_data & d_)
                        {
                            if (d_.key == 'f')
                            {
                                if (p != map::player->pos)
                                {
                                    msg_log::clear();
                                    render::draw_map_and_interface();

                                    Actor* const actor = utils::get_actor_at_pos (p);

                                    if (
                                        actor &&
                                        map::player->can_see_actor (*actor, nullptr) )
                                    {
                                        map::player->tgt_ = actor;
                                    }

                                    attack::ranged (*map::player, *wpn, p);

                                    return Marker_done::yes;
                                }
                            }
                            else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                            {
                                msg_log::clear();
                                return Marker_done::yes;
                            }

                            return Marker_done::no;
                        };

                        marker::run (Marker_draw_tail::yes, Marker_use_player_tgt::yes,
                                     on_marker_at_pos, on_key_press,
                                     wpn->get_data().ranged.effective_range);
                    }
                    else /* Not enough ammo loaded */ if (config::is_ranged_wpn_auto_reload() )
                    {
                        reload::reload_wielded_wpn (* (map::player) );
                    }
                    else // Not enough ammo loaded, and auto reloading is disabled
                    {
                        msg_log::add ("There is no ammo loaded.");
                    }
                }
                else //Wielded item is not a ranged weapon
                {
                    msg_log::add ("I am not wielding a firearm.");
                }
            }
            else //Not wielding any item
            {
                msg_log::add ("I am not wielding a weapon.");
            }
        }

        clear_events();
        return;
    }

    //----------------------------------- GET
    else if (d.key == 'g')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            const Pos& p = map::player->pos;
            Item* const item_at_player = map::cells[p.x][p.y].item;

            if (item_at_player)
            {
                if (item_at_player->get_data().id == Item_id::trapezohedron)
                {
                    dungeon_master::win_game();
                    init::quit_to_main_menu = true;
                }
            }

            if (!init::quit_to_main_menu) {item_pickup::try_pick();}
        }

        clear_events();
        return;
    }

    //----------------------------------- INVENTORY SCREEN
    else if (d.key == 'w')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            inv_handling::run_inv_screen();
        }

        clear_events();
        return;
    }

    //----------------------------------- SWAP TO PREPARED ITEM
    else if (d.key == 'z')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {

            const bool IS_FREE_TURN = player_bon::get_bg() == Bg::war_vet;

            const string swift_str = IS_FREE_TURN ? " swiftly" : "";

            Inventory& inv = map::player->get_inv();

            Item* const wielded   = inv.get_item_in_slot (Slot_id::wielded);
            Item* const alt       = inv.get_item_in_slot (Slot_id::wielded_alt);
            const string ALT_NAME = alt ? alt->get_name (Item_ref_type::a) : "";

            if (wielded || alt)
            {
                if (wielded)
                {
                    if (alt)
                    {
                        msg_log::add ("I" + swift_str + " swap to my prepared weapon (" +
                                      ALT_NAME + ").");
                    }
                    else
                    {
                        const string NAME = wielded->get_name (Item_ref_type::a);
                        msg_log::add ("I" + swift_str + " put away my weapon (" +
                                      NAME + ").");
                    }
                }
                else
                {
                    msg_log::add ("I" + swift_str + " wield my prepared weapon (" +
                                  ALT_NAME + ").");
                }

                inv.swap_wielded_and_prepared (IS_FREE_TURN);
            }
            else
            {
                msg_log::add ("I have neither a wielded nor a prepared weapon.");
            }
        }

        clear_events();
        return;
    }

    //----------------------------------- WAIT/SEARCH
    else if (d.key == 's')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            vector<Actor*> seen_mon;
            map::player->get_seen_foes (seen_mon);

            if (seen_mon.empty() )
            {
                const int TURNS_TO_APPLY = 5;
                msg_log::add ("I pause for a while...");
                map::player->wait_turns_left = TURNS_TO_APPLY - 1;
                game_time::tick();
            }
            else
            {
                msg_log::add (msg_mon_prevent_cmd);
                render::draw_map_and_interface();
            }
        }

        clear_events();
        return;
    }

    else if (d.key == 'e')
    {
        //----------------------------------- QUICK WALK
        msg_log::clear();

        if (map::player->is_alive() )
        {
            vector<Actor*> seen_mon;
            map::player->get_seen_foes (seen_mon);

            if (!seen_mon.empty() )
            {
                //Monster is seen, prevent quick move
                msg_log::add (msg_mon_prevent_cmd);
                render::draw_map_and_interface();
            }
            else if (!map::player->get_prop_handler().allow_see() )
            {
                //Player is blinded
                msg_log::add ("Not while blind.");
                render::draw_map_and_interface();
            }
            else
            {
                bool props[size_t (Prop_id::END)];
                map::player->get_prop_handler().get_prop_ids (props);

                if (props[int (Prop_id::poisoned)])
                {
                    //Player is poisoned
                    msg_log::add ("Not while poisoned.");
                    render::draw_map_and_interface();
                }
                else if (props[int (Prop_id::confused)])
                {
                    //Player is confused
                    msg_log::add ("Not while confused.");
                    render::draw_map_and_interface();
                }
                else
                {
                    msg_log::add ("Which direction?" + cancel_info_str);
                    render::draw_map_and_interface();
                    const Dir dir = query::dir();
                    msg_log::clear();

                    if (dir == Dir::center)
                    {
                        render::update_screen();
                    }
                    else
                    {
                        map::player->set_quick_move (dir);
                    }

                }
            }
        }

        clear_events();
        return;
    }

    //----------------------------------- THROW ITEM
    else if (d.key == 't')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            if (map::player->active_explosive)
            {
                auto on_marker_at_pos = [] (const Pos & p)
                {
                    msg_log::clear();
                    look::print_location_info_msgs (p);
                    msg_log::add ("[t] to throw." + cancel_info_str);
                };

                auto on_key_press = [] (const Pos & p, const Key_data & d_)
                {
                    if (d_.sdl_key == SDLK_RETURN || d_.key == 't')
                    {
                        msg_log::clear();
                        render::draw_map_and_interface();
                        throwing::player_throw_lit_explosive (p);
                        return Marker_done::yes;
                    }
                    else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                    {
                        msg_log::clear();
                        return Marker_done::yes;
                    }

                    return Marker_done::no;
                };

                marker::run (Marker_draw_tail::yes, Marker_use_player_tgt::no, on_marker_at_pos,
                             on_key_press);
            }
            else //Not holding explosive
            {
                if (map::player->get_prop_handler().allow_attack_ranged (true) )
                {
                    Inventory& player_inv  = map::player->get_inv();
                    Item* item_stack       = player_inv.get_item_in_slot (Slot_id::thrown);

                    if (item_stack)
                    {
                        Item* item_to_throw     = item_factory::copy_item (item_stack);
                        item_to_throw->nr_items_ = 1;

                        auto on_marker_at_pos = [&] (const Pos & p)
                        {
                            msg_log::clear();
                            look::print_location_info_msgs (p);

                            auto* const actor = utils::get_actor_at_pos (p);

                            if (
                                actor               &&
                                !actor->is_player()  &&
                                map::player->can_see_actor (*actor, nullptr) )
                            {
                                bool tgt_props[size_t (Prop_id::END)];
                                actor->get_prop_handler().get_prop_ids (tgt_props);

                                const bool GETS_UNDEAD_BANE_BON =
                                    player_bon::gets_undead_bane_bon (*map::player,
                                                                      actor->get_data() );

                                if (
                                    !tgt_props[int (Prop_id::ethereal)] ||
                                    GETS_UNDEAD_BANE_BON)
                                {
                                    Throw_att_data data (*map::player, *item_to_throw,
                                                         actor->pos, actor->pos);

                                    msg_log::add (to_str (data.hit_chance_tot) +
                                                  "% hit chance.");
                                }
                            }

                            msg_log::add ("[t] to throw");
                        };

                        auto on_key_press = [&] (const Pos & p, const Key_data & d_)
                        {
                            if (d_.sdl_key == SDLK_RETURN || d_.key == 't')
                            {
                                if (p == map::player->pos)
                                {
                                    msg_log::add (
                                        "I think I can persevere a little longer.");
                                }
                                else
                                {
                                    msg_log::clear();
                                    render::draw_map_and_interface();

                                    Actor* const actor = utils::get_actor_at_pos (p);

                                    if (actor) {map::player->tgt_ = actor;}

                                    throwing::throw_item (*map::player, p, *item_to_throw);
                                    player_inv.decr_item_in_slot (Slot_id::thrown);
                                }

                                return Marker_done::yes;
                            }
                            else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                            {
                                delete item_to_throw;
                                item_to_throw = nullptr;
                                msg_log::clear();
                                return Marker_done::yes;
                            }

                            return Marker_done::no;
                        };

                        marker::run (Marker_draw_tail::yes, Marker_use_player_tgt::yes,
                                     on_marker_at_pos, on_key_press,
                                     item_to_throw->get_data().ranged.effective_range);
                    }
                    else //No item equipped
                    {
                        msg_log::add (
                            "I have no missiles chosen for throwing (press 'w').");
                    }
                }
            }
        }

        clear_events();
        return;
    }

    //-----------------------------------  VIEW DESCRIPTIONS
    else if (d.key == 'v')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            if (map::player->get_prop_handler().allow_see() )
            {
                auto on_marker_at_pos = [&] (const Pos & p)
                {
                    msg_log::clear();
                    look::print_location_info_msgs (p);

                    const auto* const actor = utils::get_actor_at_pos (p);

                    if (
                        actor                   &&
                        actor != map::player    &&
                        map::player->can_see_actor (*actor, nullptr) )
                    {
                        msg_log::add ("[v] for description");
                    }

                    msg_log::add (cancel_info_str_no_space);
                };

                auto on_key_press = [&] (const Pos & p, const Key_data & d_)
                {
                    if (d_.key == 'v')
                    {
                        const auto* const actor = utils::get_actor_at_pos (p);

                        if (
                            actor                   &&
                            actor != map::player    &&
                            map::player->can_see_actor (*actor, nullptr) )
                        {
                            msg_log::clear();

                            look::print_detailed_actor_descr (*actor);

                            render::draw_map_and_interface();

                            on_marker_at_pos (p);
                        }
                    }
                    else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                    {
                        msg_log::clear();
                        return Marker_done::yes;
                    }

                    return Marker_done::no;
                };

                marker::run (Marker_draw_tail::yes, Marker_use_player_tgt::yes, on_marker_at_pos,
                             on_key_press);
            }
            else
            {
                msg_log::add ("I cannot see.");
            }
        }

        clear_events();
        return;
    }

    //----------------------------------- AUTO MELEE
    else if (d.sdl_key == SDLK_TAB)
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            map::player->auto_melee();
        }

        clear_events();
        return;
    }

    //----------------------------------- RE-CAST PREVIOUS MEMORIZED SPELL
    else if (d.key == 'x')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            player_spells_handling::try_cast_prev_spell();
        }

        clear_events();
        return;
    }

    //----------------------------------- MEMORIZED SPELLS
    else if (d.key == 'X')
    {
        msg_log::clear();

        if (map::player->is_alive() )
        {
            player_spells_handling::player_select_spell_to_cast();
        }

        clear_events();
        return;
    }

    //----------------------------------- CHARACTER INFO
    else if (d.key == '@')
    {
        character_descr::run();
        clear_events();
        return;
    }
    //----------------------------------- LOG HISTORY
    else if (d.key == 'm')
    {
        msg_log::display_history();
        clear_events();
        return;
    }

    //----------------------------------- ITEM SHORTCUTS
    else if (d.key == 'a')
    {
        Inventory&  inv         = map::player->get_inv();
        Item*       medical_bag  = inv.get_first_item_in_backpack_with_id (Item_id::medical_bag);
        Item*       lantern     = inv.get_first_item_in_backpack_with_id (Item_id::electric_lantern);

        msg_log::clear();

        if (!medical_bag && !lantern)
        {
            msg_log::add ("No item with shortcut carried.");
            render::draw_map_and_interface();
            clear_events();
            return;
        }

        msg_log::add ("Use which item?",          clr_white_high);

        if (medical_bag)
        {
            msg_log::add ("[a] Medical Bag",        clr_white_high);
        }

        if (lantern)
        {
            msg_log::add ("[e] Electric Lantern",   clr_white_high);
        }

        msg_log::add (cancel_info_str_no_space,       clr_white_high);

        render::draw_map_and_interface();

        while (true)
        {
            const Key_data shortcut_key_data = query::letter (true);

            if (
                shortcut_key_data.sdl_key == SDLK_ESCAPE ||
                shortcut_key_data.sdl_key == SDLK_SPACE)
            {
                msg_log::clear();
                render::draw_map_and_interface();
                break;
            }
            else if (medical_bag && shortcut_key_data.key == 'a')
            {
                msg_log::clear();

                for (Item* const item : map::player->get_inv().general_)
                {
                    if (item->get_id() == Item_id::medical_bag)
                    {
                        item->activate (map::player);
                        break;
                    }
                }

                break;
            }
            else if (lantern && shortcut_key_data.key == 'e')
            {
                msg_log::clear();

                for (Item* const item : map::player->get_inv().general_)
                {
                    if (item->get_id() == Item_id::electric_lantern)
                    {
                        item->activate (map::player);
                        break;
                    }
                }

                break;
            }
        }

        clear_events();
        return;
    }

    //----------------------------------- MENU
    else if (d.sdl_key == SDLK_ESCAPE)
    {
        if (map::player->is_alive() )
        {
            msg_log::clear();

            const vector<string> choices {"Options", "Tome of Wisdom", "Quit", "Cancel"};
            const int CHOICE = popup::show_menu_msg ("", true, choices);

            if (CHOICE == 0)
            {
                //---------------------------- Options
                config::run_options_menu();
                render::draw_map_and_interface();
            }
            else if (CHOICE == 1)
            {
                //---------------------------- Manual
                manual::run();
                render::draw_map_and_interface();
            }
            else if (CHOICE == 2)
            {
                //---------------------------- Quit
                query_quit();
            }
        }
        else //Player not alive
        {
            init::quit_to_main_menu = true;
        }

        clear_events();
        return;
    }

    //----------------------------------- QUIT
    else if (d.key == 'Q')
    {
        query_quit();
        clear_events();
        return;
    }

    //----------------------------------- DESCEND CHEAT
    else if (d.sdl_key == SDLK_F2)
    {
        if (IS_DEBUG_MODE)
        {
            map_travel::go_to_nxt();
            clear_events();
        }

        return;
    }

    //----------------------------------- XP CHEAT
    else if (d.sdl_key == SDLK_F3)
    {
        if (IS_DEBUG_MODE)
        {
            dungeon_master::player_gain_xp (100);
            clear_events();
        }

        return;
    }

    //----------------------------------- VISION CHEAT
    else if (d.sdl_key == SDLK_F4)
    {
        if (IS_DEBUG_MODE)
        {
            if (init::is_cheat_vision_enabled)
            {
                for (int x = 0; x < MAP_W; ++x)
                {
                    for (int y = 0; y < MAP_H; ++y)
                    {
                        map::cells[x][y].is_seen_by_player = false;
                        map::cells[x][y].is_explored     = false;
                    }
                }

                init::is_cheat_vision_enabled = false;
            }
            else
            {
                init::is_cheat_vision_enabled = true;
            }

            map::player->update_fov();
            render::draw_map_and_interface();
        }

        clear_events();
    }

    //----------------------------------- INSANITY CHEAT
    else if (d.sdl_key == SDLK_F5)
    {
        if (IS_DEBUG_MODE)
        {
            map::player->incr_shock (50, Shock_src::misc);
            clear_events();
        }

        return;
    }

    //----------------------------------- DROP ITEMS AROUND PLAYER
    else if (d.sdl_key == SDLK_F6)
    {
        if (IS_DEBUG_MODE)
        {
            item_factory::mk_item_on_floor (Item_id::gas_mask, map::player->pos);

            for (int i = 0; i < int (Item_id::END); ++i)
            {
                const auto* const item_data = item_data::data[i];

                if (item_data->value != Item_value::normal && item_data->allow_spawn)
                {
                    item_factory::mk_item_on_floor (Item_id (i), map::player->pos);
                }
            }

            clear_events();
        }

        return;
    }

    //----------------------------------- TELEPORT
    else if (d.sdl_key == SDLK_F7)
    {
        if (IS_DEBUG_MODE)
        {
            msg_log::clear();
            map::player->teleport();
            clear_events();
        }

        return;
    }

    //----------------------------------- INFECTED
    else if (d.sdl_key == SDLK_F8)
    {
        if (IS_DEBUG_MODE)
        {
            map::player->get_prop_handler().try_apply_prop (new Prop_infected (Prop_turns::std) );
            clear_events();
        }

        return;
    }

    //----------------------------------- UNDEFINED COMMANDS
    else if (d.key != -1)
    {
        string cmd_tried = " ";
        cmd_tried[0] = d.key;
        msg_log::clear();
        msg_log::add ("Unknown command '" + cmd_tried + "'.");
        clear_events();
        return;
    }
}

void map_mode_input()
{
    if (is_inited_)
    {
        const Key_data& d = get_input();

        if (!init::quit_to_main_menu)
        {
            handle_map_mode_key_press (d);
        }
    }
}

void clear_events()
{
    if (is_inited_)
    {
        while (SDL_PollEvent (&sdl_event_) ) {}
    }
}

Key_data get_input(const bool IS_O_RETURN)
{
    Key_data ret = Key_data();

    if (!is_inited_)
    {
        return ret;
    }

    SDL_StartTextInput();

    bool is_done = false;

    while (!is_done)
    {
        sdl_wrapper::sleep (1);

        const bool DID_POLL_EVENT = SDL_PollEvent(&sdl_event_);

        if (!DID_POLL_EVENT)
        {
            continue;
        }

        switch (sdl_event_.type)
        {

        case SDL_WINDOWEVENT:
        {
            switch (sdl_event_.window.event)
            {
	    case SDL_WINDOWEVENT_FOCUS_GAINED:
            case SDL_WINDOWEVENT_RESTORED:
            {
                render::update_screen();
            }
            break;

            default:
                break;
            }
        }
        break;

        case SDL_QUIT:
            ret = Key_data(SDLK_ESCAPE);
            is_done = true;
            break;

        case SDL_KEYDOWN:
        {
            const SDL_Keycode sdl_key = sdl_event_.key.keysym.sym;

            //Do not return shift, control or alt as separate key events
            if (
                sdl_key == SDLK_LSHIFT ||
                sdl_key == SDLK_RSHIFT ||
                sdl_key == SDLK_LCTRL  ||
                sdl_key == SDLK_RCTRL  ||
                sdl_key == SDLK_LALT   ||
                sdl_key == SDLK_RALT)
            {
                continue;
            }

            Uint16 mod = SDL_GetModState();

            const bool  IS_SHIFT_HELD = mod & KMOD_SHIFT;
            const bool  IS_CTRL_HELD  = mod & KMOD_CTRL;
            const bool  IS_ALT_HELD   = mod & KMOD_ALT;

            ret = Key_data(-1, sdl_key, IS_SHIFT_HELD, IS_CTRL_HELD);

            if (sdl_key >= SDLK_F1 && sdl_key <= SDLK_F9)
            {
                //F keys
                is_done = true;
            }
            else //Not an F key
            {
                switch (sdl_key)
                {
                case SDLK_RETURN:
                case SDLK_RETURN2:
                case SDLK_KP_ENTER:
                    if (IS_ALT_HELD)
                    {
                        config::toggle_fullscreen();
                        clear_events();
                        continue;
                    }
                    else //Alt is not held
                    {
                        ret.sdl_key = SDLK_RETURN;
                        is_done = true;
                    }
                    break;

                case SDLK_SPACE:
                case SDLK_BACKSPACE:
                case SDLK_TAB:
                case SDLK_PAGEUP:
                case SDLK_PAGEDOWN:
                case SDLK_END:
                case SDLK_HOME:
                case SDLK_INSERT:
                case SDLK_DELETE:
                case SDLK_LEFT:
                case SDLK_RIGHT:
                case SDLK_UP:
                case SDLK_DOWN:
                case SDLK_ESCAPE:
                    is_done = true;
                    break;

                case SDLK_MENU:
                case SDLK_PAUSE:
                default:
                    break;
                }
            }
        }
        break;

        case SDL_TEXTINPUT:
        {
            const char c = sdl_event_.text.text[0];

            if ((c == 'o' || c == 'O') && IS_O_RETURN)
            {
                const bool IS_SHIFT_HELD = c == 'O';
                ret = Key_data(-1, SDLK_RETURN, IS_SHIFT_HELD, false);
                is_done = true;
            }
            else if (c >= 33 && c < 126)
            {
                //ASCII char entered
                //(Decimal unicode '!' = 33, '~' = 126)

                clear_events();
                ret = Key_data(c);
                is_done = true;
            }
            else
            {
                continue;
            }
        }
        break;

        default:
            break;

        } //End of event type switch
    } //End of while loop

    SDL_StopTextInput();

    return ret;
}

} //Input


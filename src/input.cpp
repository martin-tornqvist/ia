#include "init.hpp"

#include <memory>

#include "actor_player.hpp"
#include "log.hpp"
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

namespace Input
{

namespace
{

SDL_Event* sdl_event_ = nullptr;

void query_quit()
{
    const vector<string> quit_choices = vector<string> {"yes", "no"};
    const int QUIT_CHOICE = Popup::show_menu_msg(
                                "Save and highscore are not kept.",
                                false, quit_choices, "Quit the current game?");
    if (QUIT_CHOICE == 0)
    {
        Init::quit_to_main_menu = true;
        Render::clear_screen();
        Render::update_screen();
    }
}

} //Namespace

void init()
{
    if (!sdl_event_) {sdl_event_ = new SDL_Event;}
    set_key_repeat_delays();
}

void cleanup()
{
    if (sdl_event_)
    {
        delete sdl_event_;
        sdl_event_ = nullptr;
    }
}

void handle_map_mode_key_press(const Key_data& d)
{
    //----------------------------------- MOVEMENT
    if (d.sdl_key == SDLK_RIGHT            || d.key == '6' || d.key == 'l')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            if (d.is_shift_held)
            {
                Map::player->move_dir(Dir::up_right);
            }
            else if (d.is_ctrl_held)
            {
                Map::player->move_dir(Dir::down_right);
            }
            else
            {
                Map::player->move_dir(Dir::right);
            }
        }
        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_DOWN      || d.key == '2' || d.key == 'j')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            Map::player->move_dir(Dir::down);
        }
        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_LEFT      || d.key == '4' || d.key == 'h')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            if (d.is_shift_held)
            {
                Map::player->move_dir(Dir::up_left);
            }
            else if (d.is_ctrl_held)
            {
                Map::player->move_dir(Dir::down_left);
            }
            else
            {
                Map::player->move_dir(Dir::left);
            }
        }
        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_UP        || d.key == '8' || d.key == 'k')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            Map::player->move_dir(Dir::up);
        }
        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_PAGEUP    || d.key == '9' || d.key == 'u')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            Map::player->move_dir(Dir::up_right);
        }
        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_PAGEDOWN  || d.key == '3' || d.key == 'n')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            Map::player->move_dir(Dir::down_right);
        }
        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_END       || d.key == '1' || d.key == 'b')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            Map::player->move_dir(Dir::down_left);
        }
        clear_events();
        return;
    }
    else if (d.sdl_key == SDLK_HOME      || d.key == '7' || d.key == 'y')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();
            Map::player->move_dir(Dir::up_left);
        }
        clear_events();
        return;
    }
    else if (d.key == '5' || d.key == '.')
    {
        if (Map::player->is_alive())
        {
            Log::clear_log();

            if (Player_bon::traits[int(Trait::steady_aimer)])
            {
                Prop_handler& prop_hlr = Map::player->get_prop_handler();

                int nr_turns_aiming_old = 0;

                if (Player_bon::traits[int(Trait::sharp_shooter)])
                {
                    Prop* const prop_aiming_old =
                        prop_hlr.get_prop(Prop_id::aiming, Prop_src::applied);

                    if (prop_aiming_old)
                    {
                        nr_turns_aiming_old =
                            static_cast<Prop_aiming*>(prop_aiming_old)->nr_turns_aiming;
                    }
                }

                Prop_aiming* const aiming = new Prop_aiming(Prop_turns::specific, 1);
                aiming->nr_turns_aiming += nr_turns_aiming_old;
                prop_hlr.try_apply_prop(aiming);
            }
            Map::player->move_dir(Dir::center);
        }
        clear_events();
        return;
    }

    //----------------------------------- MANUAL
    else if (d.key == '?')
    {
        Log::clear_log();
        Manual::run();
        clear_events();
        return;
    }

    //----------------------------------- OPTIONS
    else if (d.key == '=')
    {
        Log::clear_log();
        Config::run_options_menu();
        clear_events();
        return;
    }

    //----------------------------------- RELOAD
    else if (d.key == 'r')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Reload::reload_wielded_wpn(*(Map::player));
        }
        clear_events();
        return;
    }

    //----------------------------------- KICK
    else if ((d.key == 'q'))
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Kick::player_kick();
            Render::draw_map_and_interface();
        }
        clear_events();
        return;
    }

    //----------------------------------- CLOSE
    else if (d.key == 'c')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Close::player_close();
        }
        clear_events();
        return;
    }

    //----------------------------------- JAM
    else if (d.key == 'D')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Jam_with_spike::player_jam();
        }
        clear_events();
        return;
    }

    //----------------------------------- DISARM
    else if (d.key == 'd')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Disarm::player_disarm();
        }
        clear_events();
        return;
    }

    //----------------------------------- UNLOAD AMMO FROM GROUND
    else if (d.key == 'G')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Item_pickup::try_unload_wpn_or_pickup_ammo();
        }
        clear_events();
        return;
    }

    //----------------------------------- AIM/FIRE FIREARM
    else if (d.key == 'f')
    {
        Log::clear_log();

        if (
            Map::player->is_alive() &&
            Map::player->get_prop_handler().allow_attack_ranged(true))
        {
            auto* const item = Map::player->get_inv().get_item_in_slot(Slot_id::wielded);

            if (item)
            {
                const Item_data_t& item_data = item->get_data();

                if (item_data.ranged.is_ranged_wpn)
                {
                    auto* wpn = static_cast<Wpn*>(item);

                    //TODO: Quick hack for the Mi-go gun, shouldn't be here - refactor
                    if (
                        wpn->get_data().id == Item_id::mi_go_gun  &&
                        wpn->nr_ammo_loaded == 0                &&
                        Map::player->get_hp() > 1)
                    {
                        const string wpn_name = wpn->get_name(Item_ref_type::plain,
                                                            Item_ref_inf::none);

                        Log::add_msg("The " + wpn_name + " draws power from my essence!",
                                    clr_msg_bad);

                        Render::draw_map_and_interface();
                        ++wpn->nr_ammo_loaded;
                        Map::player->hit(1, Dmg_type::pure);
                        return;
                    }

                    if (wpn->nr_ammo_loaded >= 1 || item_data.ranged.has_infinite_ammo)
                    {
                        auto on_marker_at_pos = [&](const Pos & p)
                        {
                            Log::clear_log();
                            Look::print_location_info_msgs(p);

                            auto* const actor = Utils::get_actor_at_pos(p);

                            if (
                                actor               &&
                                !actor->is_player()  &&
                                Map::player->can_see_actor(*actor, nullptr))
                            {
                                bool tgt_props[size_t(Prop_id::END)];
                                actor->get_prop_handler().get_prop_ids(tgt_props);

                                const bool GETS_UNDEAD_BANE_BON =
                                    Player_bon::gets_undead_bane_bon(*Map::player,
                                                                 actor->get_data());

                                if (
                                    !tgt_props[int(Prop_id::ethereal)] ||
                                    GETS_UNDEAD_BANE_BON)
                                {
                                    Ranged_att_data data(*Map::player, *wpn, actor->pos,
                                                       actor->pos);
                                    Log::add_msg(to_str(data.hit_chance_tot) +
                                                "% hit chance.");
                                }
                            }

                            Log::add_msg("[f] to fire" + cancel_info_str);
                        };

                        auto on_key_press = [&](const Pos & p, const Key_data & d_)
                        {
                            if (d_.key == 'f')
                            {
                                if (p != Map::player->pos)
                                {
                                    Log::clear_log();
                                    Render::draw_map_and_interface();

                                    Actor* const actor = Utils::get_actor_at_pos(p);

                                    if (
                                        actor &&
                                        Map::player->can_see_actor(*actor, nullptr))
                                    {
                                        Map::player->tgt_ = actor;
                                    }

                                    Attack::ranged(*Map::player, *wpn, p);

                                    return Marker_done::yes;
                                }
                            }
                            else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                            {
                                Log::clear_log();
                                return Marker_done::yes;
                            }
                            return Marker_done::no;
                        };

                        Marker::run(Marker_draw_tail::yes, Marker_use_player_tgt::yes,
                                    on_marker_at_pos, on_key_press,
                                    wpn->get_data().ranged.effective_range);
                    }
                    else /* Not enough ammo loaded */ if (Config::is_ranged_wpn_auto_reload())
                    {
                        Reload::reload_wielded_wpn(*(Map::player));
                    }
                    else // Not enough ammo loaded, and auto reloading is disabled
                    {
                        Log::add_msg("There is no ammo loaded.");
                    }
                }
                else //Wielded item is not a ranged weapon
                {
                    Log::add_msg("I am not wielding a firearm.");
                }
            }
            else //Not wielding any item
            {
                Log::add_msg("I am not wielding a weapon.");
            }
        }
        clear_events();
        return;
    }

    //----------------------------------- GET
    else if (d.key == 'g')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            const Pos& p = Map::player->pos;
            Item* const item_at_player = Map::cells[p.x][p.y].item;
            if (item_at_player)
            {
                if (item_at_player->get_data().id == Item_id::trapezohedron)
                {
                    Dungeon_master::win_game();
                    Init::quit_to_main_menu = true;
                }
            }
            if (!Init::quit_to_main_menu) {Item_pickup::try_pick();}
        }
        clear_events();
        return;
    }

    //----------------------------------- INVENTORY SCREEN
    else if (d.key == 'w')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Inv_handling::run_inv_screen();
        }
        clear_events();
        return;
    }

    //----------------------------------- SWAP TO PREPARED ITEM
    else if (d.key == 'z')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {

            const bool IS_FREE_TURN = Player_bon::get_bg() == Bg::war_vet;

            const string swift_str = IS_FREE_TURN ? " swiftly" : "";

            Inventory& inv = Map::player->get_inv();

            Item* const wielded   = inv.get_item_in_slot(Slot_id::wielded);
            Item* const alt       = inv.get_item_in_slot(Slot_id::wielded_alt);
            const string ALT_NAME = alt ? alt->get_name(Item_ref_type::a) : "";
            if (wielded || alt)
            {
                if (wielded)
                {
                    if (alt)
                    {
                        Log::add_msg("I" + swift_str + " swap to my prepared weapon (" +
                                    ALT_NAME + ").");
                    }
                    else
                    {
                        const string NAME = wielded->get_name(Item_ref_type::a);
                        Log::add_msg("I" + swift_str + " put away my weapon (" +
                                    NAME + ").");
                    }
                }
                else
                {
                    Log::add_msg("I" + swift_str + " wield my prepared weapon (" +
                                ALT_NAME + ").");
                }
                inv.swap_wielded_and_prepared(IS_FREE_TURN);
            }
            else
            {
                Log::add_msg("I have neither a wielded nor a prepared weapon.");
            }
        }
        clear_events();
        return;
    }

    //----------------------------------- WAIT/SEARCH
    else if (d.key == 's')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            vector<Actor*> seen_mon;
            Map::player->get_seen_foes(seen_mon);
            if (seen_mon.empty())
            {
                const int TURNS_TO_APPLY = 5;
                Log::add_msg("I pause for a while...");
                Map::player->wait_turns_left = TURNS_TO_APPLY - 1;
                Game_time::tick();
            }
            else
            {
                Log::add_msg(msg_mon_prevent_cmd);
                Render::draw_map_and_interface();
            }
        }
        clear_events();
        return;
    }

    else if (d.key == 'e')
    {
        //----------------------------------- QUICK WALK
        Log::clear_log();
        if (Map::player->is_alive())
        {
            vector<Actor*> seen_mon;
            Map::player->get_seen_foes(seen_mon);

            if (!seen_mon.empty())
            {
                //Monster is seen, prevent quick move
                Log::add_msg(msg_mon_prevent_cmd);
                Render::draw_map_and_interface();
            }
            else if (!Map::player->get_prop_handler().allow_see())
            {
                //Player is blinded
                Log::add_msg("Not while blind.");
                Render::draw_map_and_interface();
            }
            else
            {
                bool props[size_t(Prop_id::END)];
                Map::player->get_prop_handler().get_prop_ids(props);
                if (props[int(Prop_id::poisoned)])
                {
                    //Player is poisoned
                    Log::add_msg("Not while poisoned.");
                    Render::draw_map_and_interface();
                }
                else if (props[int(Prop_id::confused)])
                {
                    //Player is confused
                    Log::add_msg("Not while confused.");
                    Render::draw_map_and_interface();
                }
                else
                {
                    Log::add_msg("Which direction?" + cancel_info_str);
                    Render::draw_map_and_interface();
                    const Dir dir = Query::dir();
                    Log::clear_log();
                    if (dir == Dir::center)
                    {
                        Render::update_screen();
                    }
                    else
                    {
                        Map::player->set_quick_move(dir);
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
        Log::clear_log();
        if (Map::player->is_alive())
        {
            if (Map::player->active_explosive)
            {
                auto on_marker_at_pos = [](const Pos & p)
                {
                    Log::clear_log();
                    Look::print_location_info_msgs(p);
                    Log::add_msg("[t] to throw." + cancel_info_str);
                };

                auto on_key_press = [](const Pos & p, const Key_data & d_)
                {
                    if (d_.sdl_key == SDLK_RETURN || d_.key == 't')
                    {
                        Log::clear_log();
                        Render::draw_map_and_interface();
                        Throwing::player_throw_lit_explosive(p);
                        return Marker_done::yes;
                    }
                    else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                    {
                        Log::clear_log();
                        return Marker_done::yes;
                    }
                    return Marker_done::no;
                };

                Marker::run(Marker_draw_tail::yes, Marker_use_player_tgt::no, on_marker_at_pos,
                            on_key_press);
            }
            else //Not holding explosive
            {
                if (Map::player->get_prop_handler().allow_attack_ranged(true))
                {
                    Inventory& player_inv  = Map::player->get_inv();
                    Item* item_stack       = player_inv.get_item_in_slot(Slot_id::thrown);

                    if (item_stack)
                    {
                        Item* item_to_throw     = Item_factory::copy_item(item_stack);
                        item_to_throw->nr_items_ = 1;

                        auto on_marker_at_pos = [&](const Pos & p)
                        {
                            Log::clear_log();
                            Look::print_location_info_msgs(p);

                            auto* const actor = Utils::get_actor_at_pos(p);

                            if (
                                actor               &&
                                !actor->is_player()  &&
                                Map::player->can_see_actor(*actor, nullptr))
                            {
                                bool tgt_props[size_t(Prop_id::END)];
                                actor->get_prop_handler().get_prop_ids(tgt_props);

                                const bool GETS_UNDEAD_BANE_BON =
                                    Player_bon::gets_undead_bane_bon(*Map::player,
                                                                 actor->get_data());

                                if (
                                    !tgt_props[int(Prop_id::ethereal)] ||
                                    GETS_UNDEAD_BANE_BON)
                                {
                                    Throw_att_data data(*Map::player, *item_to_throw,
                                                      actor->pos, actor->pos);

                                    Log::add_msg(to_str(data.hit_chance_tot) +
                                                "% hit chance.");
                                }
                            }

                            Log::add_msg("[t] to throw");
                        };

                        auto on_key_press = [&](const Pos & p, const Key_data & d_)
                        {
                            if (d_.sdl_key == SDLK_RETURN || d_.key == 't')
                            {
                                if (p == Map::player->pos)
                                {
                                    Log::add_msg(
                                        "I think I can persevere a little longer.");
                                }
                                else
                                {
                                    Log::clear_log();
                                    Render::draw_map_and_interface();

                                    Actor* const actor = Utils::get_actor_at_pos(p);
                                    if (actor) {Map::player->tgt_ = actor;}

                                    Throwing::throw_item(*Map::player, p, *item_to_throw);
                                    player_inv.decr_item_in_slot(Slot_id::thrown);
                                }
                                return Marker_done::yes;
                            }
                            else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                            {
                                delete item_to_throw;
                                item_to_throw = nullptr;
                                Log::clear_log();
                                return Marker_done::yes;
                            }
                            return Marker_done::no;
                        };

                        Marker::run(Marker_draw_tail::yes, Marker_use_player_tgt::yes,
                                    on_marker_at_pos, on_key_press,
                                    item_to_throw->get_data().ranged.effective_range);
                    }
                    else //No item equipped
                    {
                        Log::add_msg(
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
        Log::clear_log();
        if (Map::player->is_alive())
        {
            if (Map::player->get_prop_handler().allow_see())
            {
                auto on_marker_at_pos = [&](const Pos & p)
                {
                    Log::clear_log();
                    Look::print_location_info_msgs(p);

                    const auto* const actor = Utils::get_actor_at_pos(p);

                    if (
                        actor                   &&
                        actor != Map::player    &&
                        Map::player->can_see_actor(*actor, nullptr))
                    {
                        Log::add_msg("[v] for description");
                    }

                    Log::add_msg(cancel_info_str_no_space);
                };

                auto on_key_press = [&](const Pos & p, const Key_data & d_)
                {
                    if (d_.key == 'v')
                    {
                        const auto* const actor = Utils::get_actor_at_pos(p);

                        if (
                            actor                   &&
                            actor != Map::player    &&
                            Map::player->can_see_actor(*actor, nullptr))
                        {
                            Log::clear_log();

                            Look::print_detailed_actor_descr(*actor);

                            Render::draw_map_and_interface();

                            on_marker_at_pos(p);
                        }
                    }
                    else if (d_.sdl_key == SDLK_SPACE || d_.sdl_key == SDLK_ESCAPE)
                    {
                        Log::clear_log();
                        return Marker_done::yes;
                    }
                    return Marker_done::no;
                };

                Marker::run(Marker_draw_tail::yes, Marker_use_player_tgt::yes, on_marker_at_pos,
                            on_key_press);
            }
            else
            {
                Log::add_msg("I cannot see.");
            }
        }
        clear_events();
        return;
    }

    //----------------------------------- AUTO MELEE
    else if (d.sdl_key == SDLK_TAB)
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Map::player->auto_melee();
        }
        clear_events();
        return;
    }

    //----------------------------------- RE-CAST PREVIOUS MEMORIZED SPELL
    else if (d.key == 'x')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Player_spells_handling::try_cast_prev_spell();
        }
        clear_events();
        return;
    }

    //----------------------------------- MEMORIZED SPELLS
    else if (d.key == 'X')
    {
        Log::clear_log();
        if (Map::player->is_alive())
        {
            Player_spells_handling::player_select_spell_to_cast();
        }
        clear_events();
        return;
    }

    //----------------------------------- CHARACTER INFO
    else if (d.key == '@')
    {
        Character_descr::run();
        clear_events();
        return;
    }
    //----------------------------------- LOG HISTORY
    else if (d.key == 'm')
    {
        Log::display_history();
        clear_events();
        return;
    }

    //----------------------------------- ITEM SHORTCUTS
    else if (d.key == 'a')
    {
        Inventory&  inv         = Map::player->get_inv();
        Item*       medical_bag  = inv.get_first_item_in_backpack_with_id(Item_id::medical_bag);
        Item*       lantern     = inv.get_first_item_in_backpack_with_id(Item_id::electric_lantern);

        Log::clear_log();

        if (!medical_bag && !lantern)
        {
            Log::add_msg("No item with shortcut carried.");
            Render::draw_map_and_interface();
            clear_events();
            return;
        }

        Log::add_msg("Use which item?",          clr_white_high);

        if (medical_bag)
        {
            Log::add_msg("[a] Medical Bag",        clr_white_high);
        }
        if (lantern)
        {
            Log::add_msg("[e] Electric Lantern",   clr_white_high);
        }

        Log::add_msg(cancel_info_str_no_space,       clr_white_high);

        Render::draw_map_and_interface();

        while (true)
        {
            const Key_data shortcut_key_data = Query::letter(true);

            if (
                shortcut_key_data.sdl_key == SDLK_ESCAPE ||
                shortcut_key_data.sdl_key == SDLK_SPACE)
            {
                Log::clear_log();
                Render::draw_map_and_interface();
                break;
            }
            else if (medical_bag && shortcut_key_data.key == 'a')
            {
                Log::clear_log();

                for (Item* const item : Map::player->get_inv().general_)
                {
                    if (item->get_id() == Item_id::medical_bag)
                    {
                        item->activate(Map::player);
                        break;
                    }
                }
                break;
            }
            else if (lantern && shortcut_key_data.key == 'e')
            {
                Log::clear_log();

                for (Item* const item : Map::player->get_inv().general_)
                {
                    if (item->get_id() == Item_id::electric_lantern)
                    {
                        item->activate(Map::player);
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
        if (Map::player->is_alive())
        {
            Log::clear_log();

            const vector<string> choices {"Options", "Tome of Wisdom", "Quit", "Cancel"};
            const int CHOICE = Popup::show_menu_msg("", true, choices);

            if (CHOICE == 0)
            {
                //---------------------------- Options
                Config::run_options_menu();
                Render::draw_map_and_interface();
            }
            else if (CHOICE == 1)
            {
                //---------------------------- Manual
                Manual::run();
                Render::draw_map_and_interface();
            }
            else if (CHOICE == 2)
            {
                //---------------------------- Quit
                query_quit();
            }
        }
        else //Player not alive
        {
            Init::quit_to_main_menu = true;
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
            Map_travel::go_to_nxt();
            clear_events();
        }
        return;
    }

    //----------------------------------- XP CHEAT
    else if (d.sdl_key == SDLK_F3)
    {
        if (IS_DEBUG_MODE)
        {
            Dungeon_master::player_gain_xp(100);
            clear_events();
        }
        return;
    }

    //----------------------------------- VISION CHEAT
    else if (d.sdl_key == SDLK_F4)
    {
        if (IS_DEBUG_MODE)
        {
            if (Init::is_cheat_vision_enabled)
            {
                for (int x = 0; x < MAP_W; ++x)
                {
                    for (int y = 0; y < MAP_H; ++y)
                    {
                        Map::cells[x][y].is_seen_by_player = false;
                        Map::cells[x][y].is_explored     = false;
                    }
                }
                Init::is_cheat_vision_enabled = false;
            }
            else
            {
                Init::is_cheat_vision_enabled = true;
            }
            Map::player->update_fov();
            Render::draw_map_and_interface();
        }
        clear_events();
    }

    //----------------------------------- INSANITY CHEAT
    else if (d.sdl_key == SDLK_F5)
    {
        if (IS_DEBUG_MODE)
        {
            Map::player->incr_shock(50, Shock_src::misc);
            clear_events();
        }
        return;
    }

    //----------------------------------- DROP ITEMS AROUND PLAYER
    else if (d.sdl_key == SDLK_F6)
    {
        if (IS_DEBUG_MODE)
        {
            Item_factory::mk_item_on_floor(Item_id::gas_mask, Map::player->pos);

            for (int i = 0; i < int(Item_id::END); ++i)
            {
                const auto* const item_data = Item_data::data[i];

                if (item_data->value != Item_value::normal && item_data->allow_spawn)
                {
                    Item_factory::mk_item_on_floor(Item_id(i), Map::player->pos);
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
            Log::clear_log();
            Map::player->teleport();
            clear_events();
        }
        return;
    }

    //----------------------------------- INFECTED
    else if (d.sdl_key == SDLK_F8)
    {
        if (IS_DEBUG_MODE)
        {
            Map::player->get_prop_handler().try_apply_prop(new Prop_infected(Prop_turns::std));
            clear_events();
        }
        return;
    }

    //----------------------------------- UNDEFINED COMMANDS
    else if (d.key != -1)
    {
        string cmd_tried = " ";
        cmd_tried[0] = d.key;
        Log::clear_log();
        Log::add_msg("Unknown command '" + cmd_tried + "'.");
        clear_events();
        return;
    }
}

void set_key_repeat_delays()
{
    TRACE_FUNC_BEGIN;
    const int DELAY     = Config::get_key_repeat_delay();
    const int INTERVAL  = Config::get_key_repeat_interval();
    SDL_Enable_key_repeat(DELAY, INTERVAL);
    TRACE_FUNC_END;
}

void map_mode_input()
{
    if (sdl_event_)
    {
        const Key_data& d = get_input();

        if (!Init::quit_to_main_menu)
        {
            handle_map_mode_key_press(d);
        }
    }
}

void clear_events()
{
    if (sdl_event_) {while (SDL_Poll_event(sdl_event_)) {}}
}

Key_data get_input(const bool IS_O_RETURN)
{
    if (!sdl_event_)
    {
        return Key_data();
    }

    while (true)
    {
        Sdl_wrapper::sleep(1);

        while (SDL_Poll_event(sdl_event_))
        {
            if (sdl_event_->type == SDL_QUIT)
            {
                return Key_data(SDLK_ESCAPE);
            }
            else if (sdl_event_->type == SDL_KEYDOWN)
            {
                // ASCII char entered?
                // Decimal unicode:
                // '!' = 33
                // '~' = 126
                Uint16 unicode = sdl_event_->key.keysym.unicode;

                if ((unicode == 'o' || unicode == 'O') && IS_O_RETURN)
                {
                    const bool IS_SHIFT_HELD = unicode == 'O';
                    return Key_data(-1, SDLK_RETURN, IS_SHIFT_HELD, false);
                }
                else if (unicode >= 33 && unicode < 126)
                {
                    clear_events();
                    return Key_data(char(unicode));
                }
                else
                {
                    //Other key pressed? (escape, return, space, etc)
                    const SDLKey sdl_key = sdl_event_->key.keysym.sym;

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

                    SDLMod      mod           = SDL_Get_mod_state();
                    const bool  IS_SHIFT_HELD = mod & KMOD_SHIFT;
                    const bool  IS_CTRL_HELD  = mod & KMOD_CTRL;
                    const bool  IS_ALT_HELD   = mod & KMOD_ALT;

                    Key_data ret(-1, sdl_key, IS_SHIFT_HELD, IS_CTRL_HELD);

                    if (sdl_key >= SDLK_F1 && sdl_key <= SDLK_F15)
                    {
                        //F-keys
                        return ret;
                    }
                    else
                    {
                        switch (sdl_key)
                        {
                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                            if (IS_ALT_HELD)
                            {
                                Config::toggle_fullscreen();
                                clear_events();
                                continue;
                            }
                            else
                            {
                                ret.sdl_key = SDLK_RETURN;
                                return ret;
                            }
                            break;
                        case SDLK_MENU:         continue;   break;
                        case SDLK_PAUSE:        continue;   break;
                        case SDLK_SPACE:        return ret; break;
                        case SDLK_BACKSPACE:    return ret; break;
                        case SDLK_TAB:          return ret; break;
                        case SDLK_PAGEUP:       return ret; break;
                        case SDLK_PAGEDOWN:     return ret; break;
                        case SDLK_END:          return ret; break;
                        case SDLK_HOME:         return ret; break;
                        case SDLK_INSERT:       return ret; break;
                        case SDLK_DELETE:       return ret; break;
                        case SDLK_LEFT:         return ret; break;
                        case SDLK_RIGHT:        return ret; break;
                        case SDLK_UP:           return ret; break;
                        case SDLK_DOWN:         return ret; break;
                        case SDLK_ESCAPE:       return ret; break;
                        default:                continue;   break;
                        }
                    }
                }
            }
        }
    }
}

bool is_key_held(const SDLKey key)
{
    Uint8* keystate = SDL_Get_key_state(nullptr);
    return keystate[key];
}

} //Input


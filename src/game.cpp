#include "game.hpp"

#include <vector>
#include <memory>

#include "init.hpp"
#include "draw_map.hpp"
#include "popup.hpp"
#include "io.hpp"
#include "highscore.hpp"
#include "text_format.hpp"
#include "query.hpp"
#include "actor_items.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "sdl_base.hpp"
#include "map.hpp"
#include "create_character.hpp"
#include "actor_mon.hpp"
#include "saving.hpp"
#include "status_lines.hpp"
#include "feature_rigid.hpp"
#include "feature_mob.hpp"
#include "feature_door.hpp"
#include "reload.hpp"
#include "wham.hpp"
#include "close.hpp"
#include "disarm.hpp"
#include "pickup.hpp"
#include "look.hpp"
#include "attack.hpp"
#include "throwing.hpp"
#include "marker.hpp"
#include "explosion.hpp"
#include "item_factory.hpp"
#include "actor_factory.hpp"
#include "player_spells.hpp"
#include "map_travel.hpp"
#include "manual.hpp"
#include "postmortem.hpp"
#include "character_descr.hpp"
#include "mapgen.hpp"
#include "actor_data.hpp"
#include "colors.hpp"
#include "property.hpp"
#include "property_data.hpp"
#include "property_handler.hpp"

namespace game
{

namespace
{

int clvl_ = 0;
int xp_pct_ = 0;
int xp_accum_ = 0;
TimeData start_time_;

std::vector<HistoryEvent> history_events_;

void query_quit()
{
    const auto quit_choices = std::vector<std::string>
    {
        "Yes",
        "No"
    };

    const int quit_choice =
        popup::show_menu_msg(
            "Save and highscore are not kept.",
            quit_choices,
            "Quit the current game?");

    if (quit_choice == 0)
    {
        states::pop();

        init::cleanup_session();
    }
}

} // namespace

void init()
{
    clvl_ = 1;
    xp_pct_ = 0;
    xp_accum_ = 0;

    history_events_.clear();
}

void save()
{
    saving::put_int(clvl_);
    saving::put_int(xp_pct_);
    saving::put_int(xp_accum_);
    saving::put_int(start_time_.year_);
    saving::put_int(start_time_.month_);
    saving::put_int(start_time_.day_);
    saving::put_int(start_time_.hour_);
    saving::put_int(start_time_.minute_);
    saving::put_int(start_time_.second_);

    saving::put_int(history_events_.size());

    for (const HistoryEvent& event : history_events_)
    {
        saving::put_str(event.msg);
        saving::put_int(event.turn);
    }
}

void load()
{
    clvl_ = saving::get_int();
    xp_pct_ = saving::get_int();
    xp_accum_ = saving::get_int();
    start_time_.year_ = saving::get_int();
    start_time_.month_ = saving::get_int();
    start_time_.day_ = saving::get_int();
    start_time_.hour_ = saving::get_int();
    start_time_.minute_ = saving::get_int();
    start_time_.second_ = saving::get_int();

    const int nr_events = saving::get_int();

    for (int i = 0; i < nr_events; ++i)
    {
        const std::string msg = saving::get_str();
        const int turn = saving::get_int();

        history_events_.push_back({msg, turn});
    }
}

void handle_player_input(const InputData& input)
{
    msg_log::clear();

    switch (input.key)
    {

    // Movement
    case SDLK_RIGHT:
    case '6':
    case 'l':
    {
        if (input.is_shift_held)
        {
            map::player->move(Dir::up_right);
        }
        else if (input.is_ctrl_held)
        {
            map::player->move(Dir::down_right);
        }
        else
        {
            map::player->move(Dir::right);
        }
    }
    break;

    case SDLK_DOWN:
    case '2':
    case 'j':
    {
        map::player->move(Dir::down);
        return;
    }
    break;

    case SDLK_LEFT:
    case '4':
    case 'h':
    {
        if (input.is_shift_held)
        {
            map::player->move(Dir::up_left);
        }
        else if (input.is_ctrl_held)
        {
            map::player->move(Dir::down_left);
        }
        else
        {
            map::player->move(Dir::left);
        }
    }
    break;

    case SDLK_UP:
    case '8':
    case 'k':
    {
        map::player->move(Dir::up);
    }
    break;

    case SDLK_PAGEUP:
    case '9':
    case 'u':
        {
            map::player->move(Dir::up_right);
        }
        break;

    case SDLK_PAGEDOWN:
    case '3':
    case 'n':
    {
        map::player->move(Dir::down_right);
    }
    break;

    case SDLK_END:
    case '1':
    case 'b':
    {
        map::player->move(Dir::down_left);
    }
    break;

    case SDLK_HOME:
    case '7':
    case 'y':
    {
        map::player->move(Dir::up_left);
    }
    break;

    case '5':
    case '.':
    {
        if (player_bon::traits[(size_t)Trait::steady_aimer])
        {
            Prop* aiming = new PropAiming();

            aiming->set_duration(1);

            map::player->properties().apply(aiming);
        }

        map::player->move(Dir::center);
    }
    break;

    // Wait
    case 's':
    {
        if (map::player->is_seeing_burning_feature())
        {
            msg_log::add(msg_fire_prevent_cmd);
        }
        else if (!map::player->seen_foes().empty())
        {
            msg_log::add(msg_mon_prevent_cmd);
        }
        else // We are allowed to wait
        {
            // NOTE: We should not print any "wait" message here, since it will
            // look weird in some cases - e.g. when the waiting is immediately
            // interrupted by a message from rearranging pistol magazines.

            // NOTE: Waiting with 's' merely performs "move" into the center
            //  position a number of turns (i.e. the same as pressing '5')
            const int turns_to_apply = 5;

            map::player->wait_turns_left = turns_to_apply - 1;

            game_time::tick();
        }
    }
    break;

    // Manual
    case '?':
    case SDLK_F1:
    {
        std::unique_ptr<State> browse_manual_state(new BrowseManual);

        states::push(std::move(browse_manual_state));
    }
    break;

    // Options
    case '=':
    {
        std::unique_ptr<State> config_state(new ConfigState);

        states::push(std::move(config_state));
    }
    break;

    // Reload
    case 'r':
    {
        Item* const wpn = map::player->inv().item_in_slot(SlotId::wpn);

        reload::try_reload(*map::player, wpn);
    }
    break;

    // Kick
    case 'w':
    {
        wham::run();
    }
    break;

    // Close
    case 'c':
    {
        close_door::player_try_close_or_jam();
    }
    break;

    // Disarm
    case 'd':
    {
        disarm::player_disarm();
    }
    break;

    // Unload
    case 'G':
    {
        item_pickup::try_unload_wpn_or_pickup_ammo();
    }
    break;

    // Aim/fire
    case 'f':
    {
        const bool is_allowed =
            map::player->properties().
            allow_attack_ranged(Verbosity::verbose);

        if (is_allowed)
        {
            auto* const item = map::player->inv().item_in_slot(SlotId::wpn);

            if (item)
            {
                const ItemData& item_data = item->data();

                if (item_data.ranged.is_ranged_wpn)
                {
                    auto* wpn = static_cast<Wpn*>(item);

                    if ((wpn->ammo_loaded_ >= 1) ||
                        item_data.ranged.has_infinite_ammo)
                    {
                        // Not enough health for Mi-go gun?

                        // TODO: This doesn't belong here - refactor
                        if (wpn->data().id == ItemId::mi_go_gun)
                        {
                            if (map::player->hp() <= mi_go_gun_hp_drained)
                            {
                                msg_log::add(
                                    "I don't have enough health to fire it.");

                                return;
                            }
                        }

                        std::unique_ptr<State> aim_state(
                            new Aiming(map::player->pos, *wpn));

                        states::push(std::move(aim_state));
                    }
                    // Not enough ammo loaded - auto reload?
                    else if (config::is_ranged_wpn_auto_reload())
                    {
                        reload::try_reload(*map::player, item);
                    }
                    else // Not enough ammo loaded, and auto reloading disabled
                    {
                        msg_log::add("There is no ammo loaded.");
                    }
                }
                else // Wielded item is not a ranged weapon
                {
                    msg_log::add("I am not wielding a firearm.");
                }
            }
            else // Not wielding any item
            {
                msg_log::add("I am not wielding a weapon.");
            }
        }
    }
    break;

    // Get
    case 'g':
    {
        const P& p = map::player->pos;

        Item* const item_at_player = map::cells[p.x][p.y].item;

        if (item_at_player &&
            item_at_player->data().id == ItemId::trapez)
        {
            game::add_history_event("Beheld The Shining Trapezohedron!");

            states::pop();

            // Show victory text
            game::win_game();

            // Go to postmortem state
            std::unique_ptr<State> postmortem_state(
                new PostmortemMenu(IsWin::yes));

            states::push(std::move(postmortem_state));

            return;
        }

        item_pickup::try_pick();
    }
    break;

    // Inventory screen
    case 'i':
    {
        std::unique_ptr<State> browse_inv(new BrowseInv);

        states::push(std::move(browse_inv));
    }
    break;

    // Apply item
    case 'a':
    {
        std::unique_ptr<State> apply_state(new Apply);

        states::push(std::move(apply_state));
    }
    break;

    // Swap to prepared weapon
    case 'z':
    {
        Inventory& inv = map::player->inv();

        Item* const wielded = inv.item_in_slot(SlotId::wpn);

        Item* const alt = inv.item_in_slot(SlotId::wpn_alt);

        if (wielded || alt)
        {
            const std::string alt_name_a =
                alt ?
                alt->name(ItemRefType::a) :
                "";

            // War veteran swaps instantly
            const bool is_instant = player_bon::bg() == Bg::war_vet;

            const std::string swift_str =
                is_instant ?
                "swiftly " :
                "";

            if (wielded)
            {
                if (alt)
                {
                    msg_log::add("I " +
                                 swift_str +
                                 "swap to " +
                                 alt_name_a +
                                 ".");
                }
                else // No current alt weapon
                {
                    const std::string name = wielded->name(ItemRefType::plain);

                    msg_log::add("I " +
                                 swift_str +
                                 "put away my " +
                                 name +
                                 ".");
                }
            }
            else // No current wielded item
            {
                msg_log::add("I " +
                             swift_str +
                             "wield " +
                             alt_name_a +
                             ".");
            }

            inv.swap_wielded_and_prepared();

            if (!is_instant)
            {
                game_time::tick();
            }
        }
        else // No wielded weapon and no alt weapon
        {
            msg_log::add("I have neither a wielded nor a prepared weapon.");
        }
    }
    break;

    // Auto-walk
    case 'e':
    {
        if (map::player->is_seeing_burning_feature())
        {
            msg_log::add(msg_fire_prevent_cmd);
        }
        else if (!map::player->seen_foes().empty())
        {
            msg_log::add(msg_mon_prevent_cmd);
        }
        else if (!map::player->properties().allow_see())
        {
            msg_log::add("Not while blind.");
        }
        else if (map::player->has_prop(PropId::poisoned))
        {
            msg_log::add("Not while poisoned.");
        }
        else if (map::player->has_prop(PropId::confused))
        {
            msg_log::add("Not while confused.");
        }
        else // We are allowed to use auto-walk
        {
            msg_log::add("Which direction?" + cancel_info_str,
                         colors::light_white());

            const Dir input_dir = query::dir(AllowCenter::no);

            msg_log::clear();

            if (input_dir == Dir::END || input_dir == Dir::center)
            {
                // Invalid direction
                io::update_screen();
            }
            else // Valid direction
            {
                map::player->set_quick_move(input_dir);
            }

        }
    }
    break;

    // Throw
    case 't':
    {
        const Item* explosive = map::player->active_explosive_;

        if (explosive)
        {
            std::unique_ptr<State> throwing_explosive(
                new ThrowingExplosive(map::player->pos, *explosive));

            states::push(std::move(throwing_explosive));
        }
        else // Not holding explosive - run throwing attack instead
        {
                if (!map::player->thrown_item_)
            {
                msg_log::add("No item selected for throwing (press [T]).");

                return;
            }

            const bool is_allowed =
                map::player->properties().
                allow_attack_ranged(Verbosity::verbose);

            if (is_allowed)
            {
                std::unique_ptr<State> throwing(
                    new Throwing(map::player->pos,
                                 *map::player->thrown_item_));

                states::push(std::move(throwing));
            }
        }
    }
    break;

    // Select item for throwing
    case 'T':
    {
        std::unique_ptr<State> select_throw(new SelectThrow);

        states::push(std::move(select_throw));
    }
    break;

    // View
    case 'v':
    {
        if (map::player->properties().allow_see())
        {
            std::unique_ptr<State> view_state(
                new Viewing(map::player->pos));

            states::push(std::move(view_state));
        }
        else // Cannot see
        {
            msg_log::add("I cannot see.");
        }
    }
    break;

    // Auto melee
    case SDLK_TAB:
    {
        map::player->auto_melee();
    }
    break;

    // Cast spell / use power
    case 'x':
    {
        std::unique_ptr<State> browse_spell_state(new BrowseSpell);

        states::push(std::move(browse_spell_state));
    }
    break;

    // Character info
    case '@':
    {
        std::unique_ptr<State> char_descr_state(new CharacterDescr);

        states::push(std::move(char_descr_state));
    }
    break;

    case 'm':
    {
        std::unique_ptr<State> msg_history_state(new MsgHistoryState);

        states::push(std::move(msg_history_state));
    }
    break;

    // Make noise
    case 'N':
    {
        if (player_bon::bg() == Bg::ghoul)
        {
            msg_log::add("I let out a chilling howl.");
        }
        else // Not ghoul
        {
            msg_log::add("I make some noise.");
        }


        Snd snd("",
                SfxId::END,
                IgnoreMsgIfOriginSeen::yes,
                map::player->pos,
                map::player,
                SndVol::low,
                AlertsMon::yes);

        snd_emit::run(snd);

        game_time::tick();
    }
    break;

    // Menu
    case SDLK_ESCAPE:
    {
        const std::vector<std::string> choices
        {
            "Options",
            "Tome of Wisdom",
            "Quit",
            "Cancel"
        };

        const int choice =
            popup::show_menu_msg("", choices);

        if (choice == 0)
        {
            // Options
            std::unique_ptr<State> config_state(new ConfigState);

            states::push(std::move(config_state));
        }
        else if (choice == 1)
        {
            // Manual
            std::unique_ptr<State> browse_manual_state(new BrowseManual);

            states::push(std::move(browse_manual_state));
        }
        else if (choice == 2)
        {
            // Quit
            query_quit();
        }
    }
    break;

    // Quit
    case 'Q':
    {
        query_quit();
    }
    break;

    // Some cheat commands enabled in debug builds
#ifndef NDEBUG
    case SDLK_F2:
    {
        map_travel::go_to_nxt();
    }
    break;

    case SDLK_F3:
    {
        game::incr_player_xp(100, Verbosity::silent);
    }
    break;

    case SDLK_F4:
    {
        if (init::is_cheat_vision_enabled)
        {
            for (int x = 0; x < map_w; ++x)
            {
                for (int y = 0; y < map_h; ++y)
                {
                    map::cells[x][y].is_seen_by_player = false;
                    map::cells[x][y].is_explored = false;
                }
            }

            init::is_cheat_vision_enabled = false;
        }
        else // Cheat vision was not enabled
        {
            init::is_cheat_vision_enabled = true;
        }

        map::player->update_fov();
    }
    break;

    case SDLK_F5:
    {
        map::player->incr_shock(50, ShockSrc::misc);
    }
    break;

    case SDLK_F6:
    {
        item_factory::make_item_on_floor(ItemId::gas_mask, map::player->pos);

        for (size_t i = 0; i < (size_t)ItemId::END; ++i)
        {
            const auto& item_data = item_data::data[i];

            if (item_data.value != ItemValue::normal && item_data.allow_spawn)
            {
                item_factory::make_item_on_floor(ItemId(i), map::player->pos);
            }
        }
    }
    break;

    case SDLK_F7:
    {
        map::player->teleport();
    }
    break;

    case SDLK_F8:
    {
        map::player->apply_prop(
            new PropInfected());
    }
    break;

    case SDLK_F9:
    {
        const std::string query_str = "Summon monster id:";

        io::draw_text(query_str, Panel::screen, P(0, 0), colors::yellow());

        const int idx =
            query::number(P(query_str.size(), 0),
                          colors::light_white(),
                          0,
                          (int)ActorId::END,
                          0,
                          false);

        const ActorId mon_id = ActorId(idx);

        actor_factory::spawn(map::player->pos, {mon_id});
    }
    break;
#endif // NDEBUG

    // Undefined commands
    default:
    {
        if (input.key != -1 && input.key != SDLK_RETURN)
        {
            std::string cmd_tried = " ";

            cmd_tried[0] = input.key;

            if (cmd_tried != " ")
            {
                msg_log::add("Unknown command '" + cmd_tried + "'.");
            }
        }
    }

    } // key switch
}

int clvl()
{
    return clvl_;
}

int xp_pct()
{
    return xp_pct_;
}

int xp_accumulated()
{
    return xp_accum_;
}

TimeData start_time()
{
    return start_time_;
}

void incr_player_xp(const int xp_gained,
                    const Verbosity verbosity)
{
    if (!map::player->is_alive())
    {
        return;
    }

    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("(+" + std::to_string(xp_gained) + "% XP)");
    }

    xp_pct_ += xp_gained;

    xp_accum_ += xp_gained;

    while (xp_pct_ >= 100)
    {
        if (clvl_ < player_max_clvl)
        {
            ++clvl_;

            msg_log::add("Welcome to level " + std::to_string(clvl_) + "!",
                         colors::green(),
                         false,
                         MorePromptOnMsg::yes);

            msg_log::more_prompt();

            {
                const int hp_gained = 1;

                map::player->change_max_hp(hp_gained,
                                           Verbosity::silent);

                map::player->restore_hp(hp_gained,
                                        false,
                                        Verbosity::silent);
            }

            {
                const int spi_gained = 1;

                map::player->change_max_spi(spi_gained,
                                            Verbosity::silent);

                map::player->restore_spi(spi_gained,
                                         false,
                                         Verbosity::silent);
            }

            if ((clvl_ % 2) == 0)
            {
                std::unique_ptr<State> trait_state(new PickTraitState);

                states::push(std::move(trait_state));
            }
        }

        xp_pct_ -= 100;
    }
}

void incr_clvl()
{
    ++clvl_;
}

void win_game()
{
    io::cover_panel(Panel::screen);
    io::update_screen();

    const std::vector<std::string> win_msg =
    {
        "As I approach the crystal, an eerie glow illuminates the area. "
        "I notice a figure observing me from the edge of the light. "
        "There is no doubt concerning the nature of this entity; "
        "it is the Faceless God who dwells in the depths of the earth - "
        "Nyarlathotep!",

        "I panic. Why is it I find myself here, stumbling around in darkness? "
        "Is this all part of a plan? "
        "The being beckons me to gaze into the stone.",

        "In the radiance I see visions beyond eternity, visions of unreal "
        "reality, visions of the brightest light of day and the darkest night "
        "of madness. There is only onward now, I have to see, I have to KNOW.",

        "So I make a pact with the Fiend.",

        "I now harness the shadows that stride from world to world to sow "
        "death and madness. "
        "The destinies of all things on earth, living and dead, are mine."
    };

    const int padding = 9;

    const int x0 = padding;

    const int max_w = map_w - (padding * 2);

    const int line_delay = 50;

    int y = 2;

    for (const std::string& section_msg : win_msg)
    {
        const auto section_lines = text_format::split(section_msg, max_w);

        for (const std::string& line : section_lines)
        {
            io::draw_text(line,
                          Panel::screen,
                          P(x0, y),
                          colors::white(),
                          colors::black());

            io::update_screen();

            sdl_base::sleep(line_delay);

            ++y;
        }
        ++y;
    }

    ++y;

    const std::string cmd_label = "[space/esc/enter] to continue";

    io::draw_text_center(
        "[space/esc/enter] to continue",
        Panel::screen,
        P(map_w_half, screen_h - 2),
        colors::menu_dark(),
        colors::black(),
        false);

    io::update_screen();

    query::wait_for_confirm();
}

void on_mon_seen(Actor& actor)
{
    auto& d = actor.data();

    if (!d.has_player_seen)
    {
        d.has_player_seen = true;

        // Give XP based on monster shock rating
        int xp_gained = 0;

        switch (d.mon_shock_lvl)
        {
        case ShockLvl::unsettling:
            xp_gained = 3;
            break;

        case ShockLvl::frightening:
            xp_gained = 5;
            break;

        case ShockLvl::terrifying:
            xp_gained = 10;
            break;

        case ShockLvl::mind_shattering:
            xp_gained = 20;
            break;

        case ShockLvl::none:
        case ShockLvl::END:
            break;
        }

        if (xp_gained > 0)
        {
            const std::string name = actor.name_a();

            msg_log::add("I have discovered " + name + "!");

            incr_player_xp(xp_gained);

            msg_log::more_prompt();

            add_history_event("Discovered " + name + ".");

            // We also cause some shock the first time
            double shock_value =
                map::player->shock_lvl_to_value(d.mon_shock_lvl);

            // Dampen the progression a bit
            shock_value = pow(shock_value, 0.9);

            map::player->incr_shock(shock_value, ShockSrc::see_mon);
        }
    }
}

void on_mon_killed(Actor& actor)
{
    ActorData& d = actor.data();

    d.nr_kills += 1;

    const int min_hp_for_sadism_bon = 4;

    if (d.hp >= min_hp_for_sadism_bon &&
        insanity::has_sympt(InsSymptId::sadism))
    {
        map::player->shock_ = std::max(0.0, map::player->shock_ - 3.0);
    }

    if (d.is_unique)
    {
        const std::string name = actor.name_the();

        add_history_event("Defeated " + name + ".");
    }
}

void add_history_event(const std::string msg)
{
    const int turn_nr = game_time::turn_nr();

    history_events_.push_back({msg, turn_nr});
}

const std::vector<HistoryEvent>& history()
{
    return history_events_;
}

} // game

// -----------------------------------------------------------------------------
// Game state
// -----------------------------------------------------------------------------
StateId GameState::id()
{
    return StateId::game;
}

void GameState::on_start()
{
    if (entry_mode_ == GameEntryMode::new_game)
    {
        // Character creation may have affected maximum hp and spi (either
        // positively or negatively), so here we need to (re)set the current hp
        // and spi to the maximum values
        map::player->set_hp_and_spi_to_max();

        map::player->data().ability_values.reset();

        actor_items::make_for_actor(*map::player);

        game::add_history_event("Started journey");

        if (!config::is_intro_lvl_skipped())
        {
            io::clear_screen();

            const std::string msg =
                "I stand on a cobbled forest path, ahead lies a shunned and "
                "decrepit old church building. "
                "This is the access point to the abhorred \"Cult of Starry "
                "Wisdom\". "
                "I am determined to enter these sprawling catacombs and rob "
                "them of treasures and knowledge. "
                "At the depths of the abyss lies my true destiny, an artifact "
                "of non-human origin called \"The shining Trapezohedron\" - a "
                "window to all the secrets of the universe!";

            popup::show_msg(msg,
                            "The story so far...",
                            SfxId::END,
                            5);
        }
    }

    if (config::is_intro_lvl_skipped() ||
        (entry_mode_ == GameEntryMode::load_game))
    {
        // Build first/next dungeon level
        map_travel::go_to_nxt();
    }
    else
    {
        // Build forest.
        mapgen::make_intro_lvl();
    }

    map::player->update_fov();

    game::start_time_ = current_time();
}

void GameState::draw()
{
    draw_map::run();

    status_lines::draw();

    msg_log::draw();
}

void GameState::update()
{
    // To avoid redrawing the map for *each* actor, we instead run acting inside
    // a loop here. We exit the loop if the next actor to act is the player.
    // Then another state cycle will be executed, and rendering performed.
    while (true)
    {
        // Let the current actor act
        Actor* actor = game_time::current_actor();

        const bool allow_act = actor->properties().allow_act();

        const bool is_gibbed = actor->state() == ActorState::destroyed;

        if (allow_act && !is_gibbed)
        {
            // Tell actor to "do something". If this is the player, input is
            // read from either the player or the bot. If it's a monster, the
            // AI handles it.
            actor->act();
        }
        else // Actor cannot act
        {
            if (actor->is_player())
            {
                sdl_base::sleep(ms_delay_player_unable_act);
            }

            game_time::tick();
        }

        // NOTE: This state may have been popped at this point

        // We have quit the current game, or the player is dead?
        if (!map::player ||
            !states::contains_state(StateId::game) ||
            (map::player->state() != ActorState::alive))
        {
            break;
        }

        // Exit the loop if the next actor is the player (to trigger rendering).
        const Actor* next_actor = game_time::current_actor();

        if (next_actor->is_player())
        {
            break;
        }
    }

    // Player is dead?
    if (map::player && (map::player->state() != ActorState::alive))
    {
        TRACE << "Player died" << std::endl;

        static_cast<Player*>(map::player)->wait_turns_left = -1;

        audio::play(SfxId::death);

        msg_log::add("-I AM DEAD!-",
                     colors::msg_bad(),
                     false,
                     MorePromptOnMsg::yes);

        // Go to postmortem menu
        states::pop();

        std::unique_ptr<State> postmortem_state(
            new PostmortemMenu(IsWin::no));

        states::push(std::move(postmortem_state));

        return;
    }
}

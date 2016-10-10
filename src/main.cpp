#include "init.hpp"

#include "rl_utils.hpp"
#include "state.hpp"
#include "io.hpp"
#include "init.hpp"
#include "main_menu.hpp"

#ifdef _WIN32
#undef main
#endif
int main(int argc, char* argv[])
{
    TRACE_FUNC_BEGIN;

    (void)argc;
    (void)argv;

    init::init_io();
    init::init_game();

    std::unique_ptr<State> main_menu_state(new MainMenuState);

    states::push(std::move(main_menu_state));

    // Loop while there is at least one state
    while (!states::is_empty())
    {
        states::start();

        if (states::is_empty())
        {
            break;
        }

        io::clear_screen();

        states::draw();

        io::update_screen();

        states::update();
    }

    init::cleanup_game();
    init::cleanup_io();

    return 0;

    // -------------------------------------------------------------------------
    // Ye olde crap way
    // -------------------------------------------------------------------------
    /*
    init::init_io();
    init::init_game();

    while (!quit_game)
    {
        init::init_session();

        int intro_mus_chan = -1;

        const GameEntryMode game_entry_type =
            main_menu::run(quit_game, intro_mus_chan);

        if (!quit_game)
        {
            init::quit_to_main_menu = false;

            if (game_entry_type == GameEntryMode::new_game)
            {
                if (config::is_bot_playing())
                {
                    player_bon::set_all_traits_to_picked();
                }

                create_character::create_character();
                map::player->mk_start_items();

                if (config::is_intro_lvl_skipped())
                {
                    //Build first dungeon level
                    map_travel::go_to_nxt();
                }
                else //Using intro level
                {
                    //Build forest.
                    io::clear_screen();
                    io::update_screen();
                    mapgen::mk_intro_lvl();
                }

                game::set_start_time_to_now();

                const TimeData& t = game::start_time();

                TRACE << "Game started on: "
                      << t.time_str(TimeType::minute, true)
                      << std::endl;
            }

            audio::fade_out_channel(intro_mus_chan);

            if (game_entry_type == GameEntryMode::new_game)
            {
                if (!config::is_intro_lvl_skipped())
                {
                    io::clear_screen();

                    const std::string msg =
                        "I stand on a cobbled forest path, ahead lies a "
                        "shunned and decrepit old church. I know of the things "
                        "that dwell below, and of the Cult of Starry Wisdom "
                        "and the monstrous sacrifices dedicated to their "
                        "rulers. But now they are weak - only deranged "
                        "fanatics grasping at false promises. I will enter "
                        "these sprawling catacombs and rob them of treasures "
                        "and knowledge! At the depths of the abyss lies my "
                        "true destiny, an artifact of non-human origin called "
                        "\"The shining Trapezohedron\" - a window to all the "
                        "secrets of the universe.";

                    popup::show_msg(msg,
                                    false,
                                    "The story so far...",
                                    SfxId::END,
                                    1);
                }

                game::add_history_event("Started journey.");
            }

            map::player->update_fov();
            io::draw_map_state();

            // ========== M A I N   L O O P ==========
            while (!init::quit_to_main_menu)
            {
                if (map::player->is_alive())
                {
                    Actor* const actor = game_time::current_actor();

                    // Properties running on the actor's turn are not
                    // immediately applied on the actor, but instead placed in a
                    // buffer. This is to ensure that e.g. a property set to
                    // last one turn actually covers one turn (and not applied
                    // after the actor acts, and ends before the actor's next
                    // turn). The contents of the buffer are moved to the
                    // applied properties here.
                    actor->prop_handler().apply_actor_turn_prop_buffer();

                    actor->update_clr();

                    const bool allow_act =
                        actor->prop_handler().allow_act();

                    const bool is_gibbed =
                        actor->state() == ActorState::destroyed;

                    if (allow_act && !is_gibbed)
                    {
                        // Tell actor to "do something". If this is the player,
                        // input is read from either the player or the bot. If
                        // it's a monster, the AI handles it.
                        actor->act();
                    }
                    else //Actor cannot act
                    {
                        if (actor->is_player())
                        {
                            sdl_base::sleep(ms_delay_player_unable_act);
                        }

                        game_time::tick();
                    }
                }
                else //Player is dead
                {
                    //Run postmortem, then return to main menu
                    static_cast<Player*>(map::player)->wait_turns_left = -1;

                    audio::play(SfxId::death);

                    msg_log::add("-I AM DEAD!-",
                                 clr_msg_bad,
                                 false,
                                 MorePromptOnMsg::yes);

                    msg_log::clear();

                    highscore::on_game_over(false);

                    postmortem::run(&quit_game);

                    init::quit_to_main_menu = true;
                }
            }
        }

        init::cleanup_session();
    }

    init::cleanup_game();
    init::cleanup_io();

    TRACE_FUNC_END;

    return 0;
    */
}

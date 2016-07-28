#include "init.hpp"

#include <SDL.h>

#include "sdl_wrapper.hpp"
#include "config.hpp"
#include "render.hpp"
#include "main_menu.hpp"
#include "player_bon.hpp"
#include "bot.hpp"
#include "create_character.hpp"
#include "actor_player.hpp"
#include "mapgen.hpp"
#include "map_travel.hpp"
#include "dungeon_master.hpp"
#include "popup.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "highscore.hpp"
#include "postmortem.hpp"
#include "map.hpp"

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

    bool quit_game = false;

    while (!quit_game)
    {
        init::init_session();

        int intro_mus_chan = -1;
        const GameEntryMode game_entry_type = main_menu::run(quit_game, intro_mus_chan);

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
                    render::clear_screen();
                    render::update_screen();
                    mapgen::mk_intro_lvl();
                }

                dungeon_master::set_time_started_to_now();
                const TimeData& t = dungeon_master::start_time();
                TRACE << "Game started on: " << t.time_str(TimeType::minute, true)
                      << std::endl;
            }

            audio::fade_out_channel(intro_mus_chan);

            if (game_entry_type == GameEntryMode::new_game)
            {
                if (!config::is_intro_lvl_skipped())
                {
                    render::clear_screen();

                    const std::string msg =
                        "I stand on a cobbled forest path, ahead lies a shunned and decrepit old "
                        "church. I know of the things that dwell below, and of the Cult of Starry "
                        "Wisdom and the monstrous sacrifices dedicated to their rulers. But now "
                        "they are weak - only deranged fanatics grasping at false promises. "
                        "I will enter these sprawling catacombs and rob them of treasures and "
                        "knowledge! At the depths of the abyss lies my true destiny, an artifact "
                        "of non-human origin called \"The shining Trapezohedron\" - a window to "
                        "all the secrets of the universe.";

                    popup::show_msg(msg, false, "The story so far...", SfxId::END, 1);
                }

                dungeon_master::add_history_event("Started journey.");
            }

            map::player->update_fov();
            render::draw_map_state();

            //========== M A I n   L O O P ==========
            while (!init::quit_to_main_menu)
            {
                if (map::player->is_alive())
                {
                    Actor* const actor = game_time::current_actor();

                    //Properties running on the actor's turn are not immediately applied
                    //on the actor, but instead placed in a buffer. This is to ensure
                    //that e.g. a property set to last one turn actually covers one turn
                    //(and not applied after the actor acts, and ends before the actor's
                    //next turn).
                    //The contents of the buffer are moved to the applied properties here.
                    actor->prop_handler().apply_actor_turn_prop_buffer();

                    actor->update_clr();

                    const bool allow_act  = actor->prop_handler().allow_act();
                    const bool is_gibbed  = actor->state() == ActorState::destroyed;

                    if (allow_act && !is_gibbed)
                    {
                        //Tell actor to "do something". If this is the player, input is read from
                        //either the human player or the bot. If it's a monster, the AI handles it.
                        actor->act();
                    }
                    else //Actor cannot act
                    {
                        if (actor->is_player())
                        {
                            sdl_wrapper::sleep(ms_delay_player_unable_act);
                        }

                        game_time::tick();
                    }
                }
                else //Player is dead
                {
                    //Run postmortem, then return to main menu
                    static_cast<Player*>(map::player)->wait_turns_left = -1;

                    audio::play(SfxId::death);

                    msg_log::add("I am dead!",
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
}


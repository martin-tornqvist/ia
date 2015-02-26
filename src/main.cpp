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
#include "map_gen.hpp"
#include "map_travel.hpp"
#include "dungeon_master.hpp"
#include "popup.hpp"
#include "msg_log.hpp"
#include "query.hpp"
#include "highscore.hpp"
#include "postmortem.hpp"
#include "map.hpp"
#include "utils.hpp"

using namespace std;

#ifdef _WIN32
#undef main
#endif
int main(int argc, char* argv[])
{
    TRACE_FUNC_BEGIN;

    (void)argc;
    (void)argv;

    init::init_iO();
    init::init_game();

    bool quit_game = false;

    while (!quit_game)
    {
        init::init_session();

        int intro_mus_chan = -1;
        const Game_entry_mode game_entry_type = main_menu::run(quit_game, intro_mus_chan);

        if (!quit_game)
        {
            init::quit_to_main_menu = false;

            if (game_entry_type == Game_entry_mode::new_game)
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
                else
                {
                    //Build forest.
                    render::clear_screen();
                    render::update_screen();
                    map_gen::mk_intro_lvl();
                }

                dungeon_master::set_time_started_to_now();
                const Time_data& t = dungeon_master::get_start_time();
                TRACE << "Game started on: " << t.get_time_str(Time_type::minute, true)
                      << endl;
            }

            audio::fade_out_channel(intro_mus_chan);

            map::player->update_fov();
            render::draw_map_and_interface();

            if (game_entry_type == Game_entry_mode::new_game && !config::is_intro_lvl_skipped())
            {
                const string msg =
                    "I stand on a cobbled forest path, ahead lies a shunned and "
                    "decrepit old church. In countless dreams this place "
                    "appeared to me - I know of the things that dwell below, and of the "
                    "Cult of Starry Wisdom and the monstrous sacrifices dedicated "
                    "to their overlords. But now they are nothing - only a few deranged "
                    "fanatics shamble about the corridors, grasping at false "
                    "promises. I will enter these sprawling catacombs and rob them of "
                    "treasures and knowledge. But at the depths of the abyss "
                    "lies my true destiny, an artifact of non-human origin called "
                    "\"The shining Trapezohedron\" - a window to all secrets of the "
                    "universe.";

                popup::show_msg(msg, true, "The story so far...", Sfx_id::END, 1);
            }

            //========== M A I N   L O O P ==========
            while (!init::quit_to_main_menu)
            {
                if (map::player->is_alive())
                {
                    Actor* const actor = game_time::get_cur_actor();

                    //Properties running on the actor's turn are not immediately applied
                    //on the actor, but instead placed in a buffer. This is to ensure
                    //that e.g. a property set to last one turn actually covers one turn
                    //(and not applied after the actor acts, and ends before the actor's
                    //next turn)
                    //The contents of the buffer are moved to the applied properties here
                    actor->get_prop_handler().apply_actor_turn_prop_buffer();

                    actor->update_clr();

                    const bool ALLOW_ACT  = actor->get_prop_handler().allow_act();
                    const bool IS_GIBBED  = actor->get_state() == Actor_state::destroyed;

                    if (ALLOW_ACT && !IS_GIBBED)
                    {
                        actor->on_actor_turn();
                    }
                    else //Actor cannot act
                    {
                        if (actor->is_player())
                        {
                            sdl_wrapper::sleep(DELAY_PLAYER_UNABLE_TO_ACT);
                        }

                        game_time::tick();
                    }
                }
                else //Player is dead
                {
                    //Run postmortem, then return to main menu
                    static_cast<Player*>(map::player)->wait_turns_left = -1;
                    audio::play(Sfx_id::death);
                    msg_log::add("I am dead...", clr_msg_bad, false, true);
                    msg_log::clear();
                    high_score::on_game_over(false);
                    postmortem::run(&quit_game);
                    init::quit_to_main_menu = true;
                }
            }
        }

        init::cleanup_session();
    }

    init::cleanup_game();
    init::cleanup_iO();

    TRACE_FUNC_END;

    return 0;
}


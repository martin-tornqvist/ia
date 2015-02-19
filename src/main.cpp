#include "Init.h"

#include <SDL.h>

#include "Sdl_wrapper.h"
#include "Config.h"
#include "Render.h"
#include "Main_menu.h"
#include "Player_bon.h"
#include "Bot.h"
#include "Create_character.h"
#include "Actor_player.h"
#include "Map_gen.h"
#include "Map_travel.h"
#include "Dungeon_master.h"
#include "Popup.h"
#include "Log.h"
#include "Query.h"
#include "Highscore.h"
#include "Postmortem.h"
#include "Map.h"
#include "Utils.h"

using namespace std;

#ifdef _WIN32
#undef main
#endif
int main(int argc, char* argv[])
{
    TRACE_FUNC_BEGIN;

    (void)argc;
    (void)argv;

    Init::init_iO();
    Init::init_game();

    bool quit_game = false;
    while (!quit_game)
    {
        Init::init_session();

        int intro_mus_chan = -1;
        const Game_entry_mode game_entry_type = Main_menu::run(quit_game, intro_mus_chan);

        if (!quit_game)
        {
            Init::quit_to_main_menu = false;

            if (game_entry_type == Game_entry_mode::new_game)
            {
                if (Config::is_bot_playing())
                {
                    Player_bon::set_all_traits_to_picked();
                }
                Create_character::create_character();
                Map::player->mk_start_items();

                if (Config::is_intro_lvl_skipped())
                {
                    //Build first dungeon level
                    Map_travel::go_to_nxt();
                }
                else
                {
                    //Build forest.
                    Render::clear_screen();
                    Render::update_screen();
                    Map_gen::mk_intro_lvl();
                }
                Dungeon_master::set_time_started_to_now();
                const Time_data& t = Dungeon_master::get_start_time();
                TRACE << "Game started on: " << t.get_time_str(Time_type::minute, true)
                      << endl;
            }

            Audio::fade_out_channel(intro_mus_chan);

            Map::player->update_fov();
            Render::draw_map_and_interface();

            if (game_entry_type == Game_entry_mode::new_game && !Config::is_intro_lvl_skipped())
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

                Popup::show_msg(msg, true, "The story so far...", Sfx_id::END, 1);
            }

            //========== M A I N   L O O P ==========
            while (!Init::quit_to_main_menu)
            {
                if (Map::player->is_alive())
                {
                    Actor* const actor = Game_time::get_cur_actor();

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
                            Sdl_wrapper::sleep(DELAY_PLAYER_UNABLE_TO_ACT);
                        }
                        Game_time::tick();
                    }
                }
                else //Player is dead
                {
                    //Run postmortem, then return to main menu
                    static_cast<Player*>(Map::player)->wait_turns_left = -1;
                    Audio::play(Sfx_id::death);
                    Log::add_msg("I am dead...", clr_msg_bad, false, true);
                    Log::clear_log();
                    High_score::on_game_over(false);
                    Postmortem::run(&quit_game);
                    Init::quit_to_main_menu = true;
                }
            }
        }
        Init::cleanup_session();
    }
    Init::cleanup_game();
    Init::cleanup_iO();

    TRACE_FUNC_END;

    return 0;
}


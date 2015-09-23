#include "init.hpp"

#include "player_bon.hpp"
#include "sdl_wrapper.hpp"
#include "config.hpp"
#include "input.hpp"
#include "render.hpp"
#include "audio.hpp"
#include "line_calc.hpp"
#include "gods.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "item_rod.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "dungeon_master.hpp"
#include "bot.hpp"
#include "manual.hpp"
#include "player_spells_handling.hpp"
#include "credits.hpp"
#include "map_templates.hpp"
#include "map_travel.hpp"
#include "query.hpp"
#include "item_jewelry.hpp"
#include "save_handling.hpp"
#include "insanity.hpp"
#include "highscore.hpp"

namespace init
{

bool is_cheat_vision_enabled    = false;
bool quit_to_main_menu          = false;

void init_io()
{
    TRACE_FUNC_BEGIN;
    sdl_wrapper::init();
    config::init();
    input::init();
    query::init();
    render::init();
    audio::init();
    TRACE_FUNC_END;
}

void cleanup_io()
{
    TRACE_FUNC_BEGIN;
    audio::cleanup();
    render::cleanup();
    input::cleanup();
    sdl_wrapper::cleanup();
    TRACE_FUNC_END;
}

void init_game()
{
    TRACE_FUNC_BEGIN;
    save_handling::init();
    line_calc::init();
    gods::init();
    manual::init();
    credits::init();
    map_templ_handling::init();
    TRACE_FUNC_END;
}

void cleanup_game()
{
    TRACE_FUNC_BEGIN;

    TRACE_FUNC_END;
}

void init_session()
{
    TRACE_FUNC_BEGIN;
    actor_data::init();
    feature_data::init();
    prop_data::init();
    item_data::init();
    scroll_handling::init();
    potion_handling::init();
    rod_handling::init();
    inv_handling::init();
    game_time::init();
    map_travel::init();
    map::init();
    player_bon::init();
    insanity::init();
    msg_log::init();
    dungeon_master::init();
    bot::init();
    player_spells_handling::init();
    jewelry_handling::init();
    highscore::init();
    TRACE_FUNC_END;
}

void cleanup_session()
{
    TRACE_FUNC_BEGIN;
    highscore::cleanup();
    player_spells_handling::cleanup();
    insanity::cleanup();
    map::cleanup();
    game_time::cleanup();
    item_data::cleanup();
    TRACE_FUNC_END;
}

} //Init

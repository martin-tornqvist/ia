#include "init.hpp"

#include <cassert>

#include "audio.hpp"
#include "bot.hpp"
#include "config.hpp"
#include "draw_map.hpp"
#include "game.hpp"
#include "highscore.hpp"
#include "insanity.hpp"
#include "io.hpp"
#include "item_artifact.hpp"
#include "item_potion.hpp"
#include "item_rod.hpp"
#include "item_scroll.hpp"
#include "line_calc.hpp"
#include "manual.hpp"
#include "map.hpp"
#include "map_templates.hpp"
#include "map_travel.hpp"
#include "msg_log.hpp"
#include "player_bon.hpp"
#include "player_spells.hpp"
#include "property_data.hpp"
#include "query.hpp"
#include "saving.hpp"
#include "sdl_base.hpp"

namespace init
{

bool is_cheat_vision_enabled = false;
bool is_demo_mapgen = false;

void init_io()
{
    TRACE_FUNC_BEGIN;

    sdl_base::init();
    config::init();
    io::init();
    query::init();
    audio::init();
    colors::init();

    TRACE_FUNC_END;
}

void cleanup_io()
{
    TRACE_FUNC_BEGIN;

    audio::cleanup();
    io::cleanup();
    sdl_base::cleanup();

    TRACE_FUNC_END;
}

void init_game()
{
    TRACE_FUNC_BEGIN;

    saving::init();
    line_calc::init();
    map_templates::init();

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
    property_data::init();
    item_data::init();
    scroll_handling::init();
    potion_handling::init();
    rod_handling::init();
    game_time::init();
    map_travel::init();
    map::init();
    player_bon::init();
    insanity::init();
    msg_log::init();
    draw_map::clear();
    game::init();
    bot::init();
    player_spells::init();
    highscore::init();

    TRACE_FUNC_END;
}

void cleanup_session()
{
    TRACE_FUNC_BEGIN;

    map_templates::clear_base_room_templates_used();

    highscore::cleanup();
    player_spells::cleanup();
    insanity::cleanup();
    map::cleanup();
    game_time::cleanup();
    item_data::cleanup();

    TRACE_FUNC_END;
}

} // init

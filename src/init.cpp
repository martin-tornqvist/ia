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
#include "map.hpp"
#include "log.hpp"
#include "dungeon_master.hpp"
#include "bot.hpp"
#include "manual.hpp"
#include "player_spells_handling.hpp"
#include "credits.hpp"
#include "map_templates.hpp"
#include "map_travel.hpp"
#include "query.hpp"
#include "item_jewelry.hpp"

using namespace std;

namespace Init
{

bool is_cheat_vision_enabled = false;
bool quit_to_main_menu       = false;

//NOTE: Initialization order matters in some cases
void init_iO()
{
    TRACE_FUNC_BEGIN;
    Sdl_wrapper::init();
    Config::init();
    Input::init();
    Query::init();
    Render::init();
    Audio::init();
    TRACE_FUNC_END;
}

void cleanup_iO()
{
    TRACE_FUNC_BEGIN;
    Audio::cleanup();
    Render::cleanup();
    Input::cleanup();
    Sdl_wrapper::cleanup();
    TRACE_FUNC_END;
}

//NOTE: Initialization order matters in some cases
void init_game()
{
    TRACE_FUNC_BEGIN;
    Line_calc::init();
    Gods::init();
    Manual::init();
    Credits::init();
    Map_templ_handling::init();
    TRACE_FUNC_END;
}

void cleanup_game()
{
    TRACE_FUNC_BEGIN;

    TRACE_FUNC_END;
}

//NOTE: Initialization order matters in some cases
void init_session()
{
    TRACE_FUNC_BEGIN;
    Actor_data::init();
    Feature_data::init();
    Prop_data::init();
    Item_data::init();
    Scroll_handling::init();
    Potion_handling::init();
    Inv_handling::init();
    Game_time::init();
    Map_travel::init();
    Map::init();
    Player_bon::init();
    Log::init();
    Dungeon_master::init();
    Bot::init();
    Player_spells_handling::init();
    Jewelry_handling::init();
    TRACE_FUNC_END;
}

void cleanup_session()
{
    TRACE_FUNC_BEGIN;
    Player_spells_handling::cleanup();
    Map::cleanup();
    Game_time::cleanup();
    Item_data::cleanup();
    TRACE_FUNC_END;
}

} //Init

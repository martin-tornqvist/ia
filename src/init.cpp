#include "init.h"

#include "player_bon.h"
#include "sdl_wrapper.h"
#include "config.h"
#include "input.h"
#include "render.h"
#include "audio.h"
#include "line_calc.h"
#include "gods.h"
#include "item_scroll.h"
#include "item_potion.h"
#include "map.h"
#include "log.h"
#include "dungeon_master.h"
#include "bot.h"
#include "manual.h"
#include "player_spells_handling.h"
#include "credits.h"
#include "map_templates.h"
#include "map_travel.h"
#include "query.h"
#include "item_jewelry.h"

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

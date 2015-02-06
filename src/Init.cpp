#include "Init.h"

#include "PlayerBon.h"
#include "SdlWrapper.h"
#include "Config.h"
#include "Input.h"
#include "Render.h"
#include "Audio.h"
#include "LineCalc.h"
#include "Gods.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "Map.h"
#include "Log.h"
#include "DungeonMaster.h"
#include "Bot.h"
#include "Manual.h"
#include "PlayerSpellsHandling.h"
#include "Credits.h"
#include "MapTemplates.h"
#include "MapTravel.h"
#include "Query.h"
#include "ItemJewelry.h"

using namespace std;

namespace Init
{

bool isCheatVisionEnabled = false;
bool quitToMainMenu       = false;

//NOTE: Initialization order matters in some cases
void initIO()
{
    TRACE_FUNC_BEGIN;
    SdlWrapper::init();
    Config::init();
    Input::init();
    Query::init();
    Render::init();
    Audio::init();
    TRACE_FUNC_END;
}

void cleanupIO()
{
    TRACE_FUNC_BEGIN;
    Audio::cleanup();
    Render::cleanup();
    Input::cleanup();
    SdlWrapper::cleanup();
    TRACE_FUNC_END;
}

//NOTE: Initialization order matters in some cases
void initGame()
{
    TRACE_FUNC_BEGIN;
    LineCalc::init();
    Gods::init();
    Manual::init();
    Credits::init();
    MapTemplHandling::init();
    TRACE_FUNC_END;
}

void cleanupGame()
{
    TRACE_FUNC_BEGIN;

    TRACE_FUNC_END;
}

//NOTE: Initialization order matters in some cases
void initSession()
{
    TRACE_FUNC_BEGIN;
    ActorData::init();
    FeatureData::init();
    PropData::init();
    ItemData::init();
    ScrollHandling::init();
    PotionHandling::init();
    InvHandling::init();
    GameTime::init();
    MapTravel::init();
    Map::init();
    PlayerBon::init();
    Log::init();
    DungeonMaster::init();
    Bot::init();
    PlayerSpellsHandling::init();
    JewelryHandling::init();
    TRACE_FUNC_END;
}

void cleanupSession()
{
    TRACE_FUNC_BEGIN;
    PlayerSpellsHandling::cleanup();
    Map::cleanup();
    GameTime::cleanup();
    ItemData::cleanup();
    TRACE_FUNC_END;
}

} //Init

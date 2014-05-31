#include "Init.h"

#include "PlayerBon.h"
#include "SdlWrapper.h"
#include "Config.h"
#include "Input.h"
#include "Renderer.h"
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

using namespace std;

namespace Init {

bool isCheatVisionEnabled = false;
bool quitToMainMenu       = false;

//Note: Initialization order matters in some cases
void initIO() {
  TRACE << "Init::initIO()..." << endl;
  SdlWrapper::init();
  Config::init();
  Input::init();
  Renderer::init();
  Audio::init();
  TRACE << "Init::initIO() [DONE]" << endl;
}

void cleanupIO() {
  TRACE << "Init::cleanupIO()..." << endl;
  Audio::cleanup();
  Renderer::cleanup();
  Input::cleanup();
  SdlWrapper::cleanup();
  TRACE << "Init::cleanupIO() [DONE]" << endl;
}

//Note: Initialization order matters in some cases
void initGame() {
  TRACE << "Init::initGame()..." << endl;
  LineCalc::init();
  Gods::init();
  Manual::init();
  Credits::init();
  MapTemplHandling::init();
  TRACE << "Init::initGame() [DONE]" << endl;
}

void cleanupGame() {
  TRACE << "Init::cleanupGame()..." << endl;

  TRACE << "Init::cleanupGame() [DONE]" << endl;
}

//Note: Initialization order matters in some cases
void initSession() {
  TRACE << "Init::initSession()..." << endl;
  ActorData::init();
  FeatureData::init();
  ScrollNameHandling::init();
  PotionNameHandling::init();
  ItemData::init();
  PropData::init();
  InvHandling::init();
  GameTime::init();
  Map::init();
  PlayerBon::init();
  Log::init();
  DungeonMaster::init();
  Bot::init();
  PlayerSpellsHandling::init();
  TRACE << "Init::initSession() [DONE]" << endl;
}

void cleanupSession() {
  TRACE << "Init::cleanupSession()..." << endl;
  PlayerSpellsHandling::cleanup();
  Map::cleanup();
  GameTime::cleanup();
  ItemData::cleanup();
  TRACE << "Init::cleanupSession() [DONE]" << endl;
}

} //Init

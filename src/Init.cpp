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
  trace << "Init::initIO()..." << endl;
  SdlWrapper::init();
  Config::init();
  Input::init();
  Renderer::init();
  Audio::init();
  trace << "Init::initIO() [DONE]" << endl;
}

void cleanupIO() {
  trace << "Init::cleanupIO()..." << endl;
  Audio::cleanup();
  Renderer::cleanup();
  Input::cleanup();
  SdlWrapper::cleanup();
  trace << "Init::cleanupIO() [DONE]" << endl;
}

//Note: Initialization order matters in some cases
void initGame() {
  trace << "Init::initGame()..." << endl;
  LineCalc::init();
  Gods::init();
  Manual::init();
  Credits::init();
  MapTemplHandling::init();
  trace << "Init::initGame() [DONE]" << endl;
}

void cleanupGame() {
  trace << "Init::cleanupGame()..." << endl;

  trace << "Init::cleanupGame() [DONE]" << endl;
}

//Note: Initialization order matters in some cases
void initSession() {
  trace << "Init::initSession()..." << endl;
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
  trace << "Init::initSession() [DONE]" << endl;
}

void cleanupSession() {
  trace << "Init::cleanupSession()..." << endl;
  PlayerSpellsHandling::cleanup();
  Map::cleanup();
  GameTime::cleanup();
  ItemData::cleanup();
  trace << "Init::cleanupSession() [DONE]" << endl;
}

} //Init

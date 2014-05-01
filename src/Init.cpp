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

using namespace std;

namespace Init {

bool isCheatVisionEnabled = false;
bool quitToMainMenu       = false;

//Note: Initialization order is important
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

//Note: Initialization order is important in some cases
void initGame() {
  trace << "Init::initGame()..." << endl;
  LineCalc::init();
  PropData::init();
  Gods::init();
  trace << "Init::initGame() [DONE]" << endl;
}

void cleanupGame() {
  trace << "Init::cleanupGame()..." << endl;

  trace << "Init::cleanupGame() [DONE]" << endl;
}

//Note: Initialization order is important in some cases
void initSession() {
  trace << "Init::initSession()..." << endl;
  ActorData::init();
  ScrollNameHandling::init();
  PotionNameHandling::init();
  ItemData::init();
  InvHandling::init();
//  playerCreateCharacter = new PlayerCreateCharacter(*this);
//  mapPatterns = new MapPatterns(*this);
//  roomThemeMaker = new RoomThemeMaker(*this);
//  player->place(Pos(PLAYER_START_X, PLAYER_START_Y),
//                actorDataHandler->dataList[actor_player]);
  GameTime::init();
//  featureDataHandler = new FeatureDataHandler();
  Map::init();
  PlayerBon::init();
//  marker = new Marker(*this);
  Log::init();
  DungeonMaster::init();
//  dungeonClimb = new DungeonClimb(*this);
//  mapTemplateHandler = new MapTemplateHandler();
//  itemPickup = new ItemPickup(*this);
//  mainMenu = new MainMenu(*this);
  Bot::init();
//  art = new Art;
//  thrower = new Thrower(*this);
//  manual = new Manual(*this);
//  populateItems = new PopulateItems(*this);
//  populateMonsters = new PopulateMonsters(*this);
//  populateTraps = new PopulateTraps(*this);
//  query = new Query(*this);
//  bash = new Bash(*this);
//  close = new Close(*this);
//  look = new Look(*this);
//  autoDescribeActor = new AutoDescribeActor();
//  postmortem = new Postmortem(*this);
//  highScore = new HighScore(*this);
//  jamWithSpike = new JamWithSpike(*this);
//  playerSpellsHandler = new PlayerSpellsHandler(*this);
//  knockBack = new KnockBack(*this);
//  examine = new Examine(*this);
//  characterDescr = new CharacterDescr(*this);
//  credits = new Credits(*this);
  trace << "Init::initSession() [DONE]" << endl;
}

void cleanupSession() {
  trace << "Init::cleanupSession()..." << endl;
  ItemData::cleanup();
  GameTime::cleanup();
  trace << "Init::cleanupSession() [DONE]" << endl;
}

} //Init

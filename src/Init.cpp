#include "Init.h"

#include "PlayerBon.h"

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
//  inventoryHandler = new InventoryHandler(*this);
//  playerCreateCharacter = new PlayerCreateCharacter(*this);
//  mapPatterns = new MapPatterns(*this);
//  roomThemeMaker = new RoomThemeMaker(*this);
//  player->place(Pos(PLAYER_START_X, PLAYER_START_Y),
//                actorDataHandler->dataList[actor_player]);
//  actorFactory = new ActorFactory(*this);
//  gameTime = new GameTime(*this);
//  featureDataHandler = new FeatureDataHandler();
//  featureFactory = new FeatureFactory(*this);
  Map::init();
  PlayerBon::init();
//  marker = new Marker(*this);
//  log = new Log(*this);
//  fov = new Fov(*this);
//  dungeonMaster = new DungeonMaster(*this);
//  dungeonClimb = new DungeonClimb(*this);
//  mapTemplateHandler = new MapTemplateHandler();
//  itemDrop = new ItemDrop(*this);
//  itemPickup = new ItemPickup(*this);
//  attack = new Attack(*this);
//  mainMenu = new MainMenu(*this);
//  bot = new Bot(*this);
//  art = new Art;
//  thrower = new Thrower(*this);
//  reload = new Reload(*this);
//  gore = new Gore(*this);
//  manual = new Manual(*this);
//  populateItems = new PopulateItems(*this);
//  populateMonsters = new PopulateMonsters(*this);
//  populateTraps = new PopulateTraps(*this);
//  query = new Query(*this);
//  bash = new Bash(*this);
//  close = new Close(*this);
//  look = new Look(*this);
//  autoDescribeActor = new AutoDescribeActor();
//  spellHandler = new SpellHandler();
//  postmortem = new Postmortem(*this);
//  highScore = new HighScore(*this);
//  popup = new Popup(*this);
//  saveHandler = new SaveHandler(*this);
//  jamWithSpike = new JamWithSpike(*this);
//  menuInputHandler = new MenuInputHandler(*this);
//  playerSpellsHandler = new PlayerSpellsHandler(*this);
//  knockBack = new KnockBack(*this);
//  examine = new Examine(*this);
//  characterDescr = new CharacterDescr(*this);
//  gods = new Gods();
//  credits = new Credits(*this);
  trace << "Init::initSession() [DONE]" << endl;
}

void cleanupSession() {
  trace << "Init::cleanupSession()..." << endl;
  ItemData::cleanup();
  trace << "Init::cleanupSession() [DONE]" << endl;
}

} //Init

#include "Engine.h"

#include <iostream>

#include "Converters.h"

#include "Config.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"

#include "AbilityValues.h"
#include "ActorData.h"
#include "ActorFactory.h"
#include "Art.h"
#include "Attack.h"
#include "Audio.h"
#include "AutoDescribeActor.h"
#include "Bash.h"
#include "Blood.h"
#include "Bot.h"
#include "CharacterDescr.h"
#include "Close.h"
#include "Credits.h"
#include "DebugModeStatPrinter.h"
#include "Map.h"
#include "DungeonClimb.h"
#include "DungeonMaster.h"
#include "Examine.h"
#include "FeatureFactory.h"
#include "Fov.h"
#include "GameTime.h"
#include "Gods.h"
#include "Hide.h"
#include "Highscore.h"
#include "Input.h"
#include "InventoryHandler.h"
#include "InventoryIndexes.h"
#include "ItemData.h"
#include "ItemDrop.h"
#include "ItemFactory.h"
#include "ItemPickup.h"
#include "ItemPotion.h"
#include "ItemScroll.h"
#include "JamWithSpike.h"
#include "FeatureData.h"
#include "FeatureTrap.h"
#include "LineCalc.h"
#include "Knockback.h"
#include "Log.h"
#include "Look.h"
#include "MainMenu.h"
#include "Manual.h"
#include "MapParsing.h"
#include "MapPatterns.h"
#include "PopulateItems.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"
#include "MapTemplates.h"
#include "Marker.h"
#include "PlayerCreateCharacter.h"
#include "PlayerBonuses.h"
#include "PlayerSpellsHandler.h"
#include "PlayerVisualMemory.h"
#include "Popup.h"
#include "Postmortem.h"
#include "Query.h"
#include "Reload.h"
#include "Renderer.h"
#include "RoomTheme.h"
#include "SaveHandler.h"
#include "Sound.h"
#include "Spells.h"
#include "Properties.h"
#include "Thrower.h"
#include "MenuInputHandler.h"
#include "SdlWrapper.h"

using namespace std;

void Engine::sleep(const Uint32 DURATION) const {
  if(Config::isBotPlaying == false) {
    sdlWrapper->sleep(DURATION);
  }
}

void Engine::initSdl() {
  sdlWrapper = new SdlWrapper;
}

void Engine::cleanupSdl() {
  delete sdlWrapper;
}

void Engine::initConfig() {
  trace << "Engine::initConfig()..." << endl;
  Config::init();
  trace << "Engine::initConfig() [DONE]" << endl;
}

void Engine::initRenderer() {
  trace << "Engine::initRenderer()..." << endl;
  renderer = new Renderer(*this);
  trace << "Engine::initRenderer() [DONE]" << endl;
}

void Engine::cleanupRenderer() {
  trace << "Engine::cleanupRenderer()..." << endl;
  delete renderer;
  trace << "Engine::cleanupRenderer() [DONE]" << endl;
}

void Engine::initAudio() {
  audio = new Audio(*this);
}

void Engine::cleanupAudio() {
  delete audio;
}

void Engine::initGame() {
  trace << "Engine::initGame()..." << endl;
  // ------- INITIALIZATIONS WHERE ORDER MAY BE IMPORTANT -------
  lineCalc = new LineCalc(*this);
  propDataHandler = new PropDataHandler(*this);
  actorDataHandler = new ActorDataHandler(*this);
  scrollNameHandler = new ScrollNameHandler(*this);
  potionNameHandler = new PotionNameHandler(*this);
  itemDataHandler = new ItemDataHandler(*this);
  abilityRoll = new AbilityRoll(*this);
  itemFactory = new ItemFactory(*this);
  inventoryHandler = new InventoryHandler(*this);
  playerBonHandler = new PlayerBonHandler(*this);
  playerCreateCharacter = new PlayerCreateCharacter(*this);
  mapPatterns = new MapPatterns(*this);
  roomThemeMaker = new RoomThemeMaker(*this);
  player = new Player(*this);
  player->place(Pos(PLAYER_START_X, PLAYER_START_Y),
                actorDataHandler->dataList[actor_player]);
  actorFactory = new ActorFactory(*this);
  gameTime = new GameTime(*this);
  featureDataHandler = new FeatureDataHandler(*this);
  featureFactory = new FeatureFactory(*this);
  map = new Map(*this);

  // ------- INITIALIZATIONS WHERE ORDER IS NOT IMPORTANT -------
  marker = new Marker(*this);
  log = new Log(*this);
  fov = new Fov(*this);
  sndEmitter = new SndEmitter(*this);
  dungeonMaster = new DungeonMaster(*this);
  input = new Input(*this);
  dungeonClimb = new DungeonClimb(*this);
  mapTemplateHandler = new MapTemplateHandler(*this);
  itemDrop = new ItemDrop(*this);
  itemPickup = new ItemPickup(*this);
  attack = new Attack(*this);
  mainMenu = new MainMenu(*this);
  bot = new Bot(*this);
  art = new Art;
  thrower = new Thrower(*this);
  reload = new Reload(*this);
  playerVisualMemory = new PlayerVisualMemory(*this);
  gore = new Gore(*this);
  manual = new Manual(*this);
  populateItems = new PopulateItems(*this);
  populateMonsters = new PopulateMonsters(*this);
  populateTraps = new PopulateTraps(*this);
  inventoryIndexes = new InventoryIndexes;
  query = new Query(*this);
  bash = new Bash(*this);
  close = new Close(*this);
  look = new Look(*this);
  autoDescribeActor = new AutoDescribeActor(*this);
  spellHandler = new SpellHandler(*this);
  postmortem = new Postmortem(*this);
  highScore = new HighScore(*this);
  popup = new Popup(*this);
  saveHandler = new SaveHandler(*this);
  jamWithSpike = new JamWithSpike(*this);
  menuInputHandler = new MenuInputHandler(*this);
  playerSpellsHandler = new PlayerSpellsHandler(*this);
  knockBack = new KnockBack(*this);
  examine = new Examine(*this);
  characterDescr = new CharacterDescr(*this);
  debugModeStatPrinter = new DebugModeStatPrinter(*this);
  gods = new Gods(*this);
  credits = new Credits(*this);
  hide = new Hide(*this);

  trace << "Engine::initGame() [DONE]" << endl;
}

void Engine::cleanupGame() {
  trace << "Engine::cleanupGame()..." << endl;

  delete map;
  delete playerCreateCharacter;
  delete playerBonHandler;
  delete playerVisualMemory;
  delete itemDataHandler;
  delete log;
  delete fov;
  delete mapPatterns;
  delete sndEmitter;
  delete featureDataHandler;
  delete dungeonMaster;
  delete input;
  delete dungeonClimb;
  delete actorFactory;
  delete mapTemplateHandler;
  delete itemDrop;
  delete itemPickup;
  delete inventoryHandler;
  delete attack;
  delete gameTime;
  delete mainMenu;
  delete itemFactory;
  delete bot;
  delete abilityRoll;
  delete featureFactory;
  delete art;
  delete thrower;
  delete reload;
  delete gore;
  delete manual;
  delete lineCalc;
  delete populateItems;
  delete populateMonsters;
  delete populateTraps;
  delete inventoryIndexes;
  delete query;
  delete actorDataHandler;
  delete scrollNameHandler;
  delete potionNameHandler;
  delete bash;
  delete close;
  delete look;
  delete autoDescribeActor;
  delete spellHandler;
  delete postmortem;
  delete highScore;
  delete popup;
  delete saveHandler;
  delete jamWithSpike;
  delete menuInputHandler;
  delete playerSpellsHandler;
  delete knockBack;
  delete examine;
  delete characterDescr;
  delete roomThemeMaker;
  delete debugModeStatPrinter;
  delete gods;
  delete credits;
  delete propDataHandler;
  delete hide;

  delete marker;

  trace << "Engine::cleanupGame() [DONE]" << endl;
}


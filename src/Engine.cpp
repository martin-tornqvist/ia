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
#include "BresenhamLine.h"
#include "CharacterDescr.h"
#include "Close.h"
#include "Credits.h"
#include "DebugModeStatPrinter.h"
#include "Disarm.h"
#include "Map.h"
#include "DungeonClimb.h"
#include "DungeonMaster.h"
#include "Explosion.h"
#include "Examine.h"
#include "FeatureFactory.h"
#include "Fov.h"
#include "GameTime.h"
#include "Gods.h"
#include "Highscore.h"
#include "CharacterLines.h"
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
#include "FovPreCalc.h"
#include "Knockback.h"
#include "Log.h"
#include "Look.h"
#include "MainMenu.h"
#include "Manual.h"
#include "MapPatterns.h"
#include "MapTests.h"
#include "MapBuild.h"
#include "MapBuildBSP.h"
#include "PopulateItems.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"
#include "MapTemplates.h"
#include "Marker.h"
#include "Pathfinding.h"
#include "PlayerCreateCharacter.h"
#include "PlayerBonuses.h"
#include "PlayerSpellsHandler.h"
#include "PlayerVisualMemory.h"
#include "Popup.h"
#include "Postmortem.h"
#include "Query.h"
#include "Reload.h"
#include "Renderer.h"
#include "RenderInventory.h"
#include "RoomTheme.h"
#include "SaveHandler.h"
#include "Sound.h"
#include "Spells.h"
#include "Properties.h"
#include "TextFormatting.h"
#include "Thrower.h"
#include "MenuInputHandler.h"
#include "SdlWrapper.h"

using namespace std;

void Engine::sleep(const Uint32 DURATION) const {
  sdlWrapper->sleep(DURATION);
}

void Engine::initSdl() {
  sdlWrapper = new SdlWrapper;
}

void Engine::cleanupSdl() {
  delete sdlWrapper;
}

void Engine::initConfig() {
  trace << "Engine::initConfig()..." << endl;
  config = new Config(this);
  trace << "Engine::initConfig() [DONE]" << endl;
}

void Engine::cleanupConfig() {
  trace << "Engine::cleanupConfig()..." << endl;
  delete config;
  trace << "Engine::cleanupConfig() [DONE]" << endl;
}

void Engine::initRenderer() {
  trace << "Engine::initRenderer()..." << endl;
  renderer = new Renderer(this);
  trace << "Engine::initRenderer() [DONE]" << endl;
}

void Engine::cleanupRenderer() {
  trace << "Engine::cleanupRenderer()..." << endl;
  delete renderer;
  trace << "Engine::cleanupRenderer() [DONE]" << endl;
}

void Engine::initAudio() {
  audio = new Audio(this);
}

void Engine::cleanupAudio() {
  delete audio;
}

void Engine::initGame() {
  trace << "Engine::initGame()..." << endl;
  // ------- INITIALIZATIONS WHERE ORDER MAY BE IMPORTANT -------
  basicUtils = new BasicUtils(this);
  propDataHandler = new PropDataHandler(this);
  actorDataHandler = new ActorDataHandler(this);
  scrollNameHandler = new ScrollNameHandler(this);
  potionNameHandler = new PotionNameHandler(this);
  itemDataHandler = new ItemDataHandler(this);
  abilityRoll = new AbilityRoll(this);
  itemFactory = new ItemFactory(this);
  inventoryHandler = new InventoryHandler(this);
  playerBonHandler = new PlayerBonHandler(this);
  playerCreateCharacter = new PlayerCreateCharacter(this);
  player = new Player;
  player->place(
    Pos(config->PLAYER_START_X, config->PLAYER_START_Y),
    &(actorDataHandler->dataList[actor_player]), this);

  // ------- INITIALIZATIONS WHERE ORDER IS NOT IMPORTANT -------
  marker = new Marker(this);
  log = new Log(this);
  pathfinder = new Pathfinder(this);
  fov = new Fov(this);
  mapBuild = new MapBuild(this);
  mapBuildBSP = new MapBuildBSP(this);
  mapPatterns = new MapPatterns(this);
  soundEmitter = new SoundEmitter(this);
  featureDataHandler = new FeatureDataHandler(this);
  dungeonMaster = new DungeonMaster(this);
  input = new Input(this, quitToMainMenu_);
  dungeonClimb = new DungeonClimb(this);
  actorFactory = new ActorFactory(this);
  mapTemplateHandler = new MapTemplateHandler(this);
  itemDrop = new ItemDrop(this);
  itemPickup = new ItemPickup(this);
  attack = new Attack(this);
  characterLines = new CharacterLines(this);
  gameTime = new GameTime(this);
  mainMenu = new MainMenu(this);
  bot = new Bot(this);
  featureFactory = new FeatureFactory(this);
  art = new Art;
  explosionMaker = new ExplosionMaker(this);
  thrower = new Thrower(this);
  reload = new Reload(this);
  map = new Map(this);
  mapTests = new MapTests(this);
  playerVisualMemory = new PlayerVisualMemory(this);
  gore = new Gore(this);
  manual = new Manual(this);
  fovPreCalc = new FovPreCalc(this);
  bresenhamLine = new BresenhamLine(this);
  populateItems = new PopulateItems(this);
  populateMonsters = new PopulateMonsters(this);
  populateTraps = new PopulateTraps(this);
  renderInventory = new RenderInventory(this);
  inventoryIndexes = new InventoryIndexes;
  query = new Query(this);
  bash = new Bash(this);
  close = new Close(this);
  look = new Look(this);
  autoDescribeActor = new AutoDescribeActor(this);
  textFormatting = new TextFormatting();
  spellHandler = new SpellHandler(this);
  postmortem = new Postmortem(this);
  highScore = new HighScore(this);
  popup = new Popup(this);
  saveHandler = new SaveHandler(this);
  jamWithSpike = new JamWithSpike(this);
  menuInputHandler = new MenuInputHandler(this);
  playerSpellsHandler = new PlayerSpellsHandler(this);
  knockBack = new KnockBack(this);
  examine = new Examine(this);
  characterDescr = new CharacterDescr(this);
  roomThemeMaker = new RoomThemeMaker(this);
  debugModeStatPrinter = new DebugModeStatPrinter(this);
  disarm = new Disarm(this);
  gods = new Gods(this);
  credits = new Credits(this);
  directionConverter = new DirectionConverter(this);

  trace << "Engine::initGame() [DONE]" << endl;
}

void Engine::cleanupGame() {
  trace << "Engine::cleanupGame()..." << endl;

  map->clearDungeon();

  delete playerBonHandler;
  delete playerVisualMemory;
  delete itemDataHandler;
  delete map;
  delete log;
  delete pathfinder;
  delete fov;
  delete mapBuild;
  delete mapBuildBSP;
  delete mapPatterns;
  delete soundEmitter;
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
  delete characterLines;
  delete gameTime;
  delete mainMenu;
  delete itemFactory;
  delete bot;
  delete abilityRoll;
  delete featureFactory;
  delete art;
  delete explosionMaker;
  delete thrower;
  delete reload;
  delete basicUtils;
  delete mapTests;
  delete gore;
  delete manual;
  delete fovPreCalc;
  delete populateItems;
  delete populateMonsters;
  delete populateTraps;
  delete renderInventory;
  delete inventoryIndexes;
  delete query;
  delete actorDataHandler;
  delete scrollNameHandler;
  delete potionNameHandler;
  delete bash;
  delete close;
  delete look;
  delete autoDescribeActor;
  delete textFormatting;
  delete spellHandler;
  delete postmortem;
  delete highScore;
  delete popup;
  delete saveHandler;
  delete bresenhamLine;
  delete jamWithSpike;
  delete menuInputHandler;
  delete playerSpellsHandler;
  delete knockBack;
  delete examine;
  delete characterDescr;
  delete roomThemeMaker;
  delete debugModeStatPrinter;
  delete disarm;
  delete gods;
  delete credits;
  delete propDataHandler;
  delete directionConverter;

  delete marker;

  trace << "Engine::cleanupGame() [DONE]" << endl;
}


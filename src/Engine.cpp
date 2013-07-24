#include "Engine.h"

#include <iostream>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

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
#include "PlayerPowersHandler.h"
#include "PlayerVisualMemory.h"
#include "Popup.h"
#include "Postmortem.h"
#include "Query.h"
#include "Reload.h"
#include "Render.h"
#include "RenderInventory.h"
#include "RoomTheme.h"
#include "SaveHandler.h"
#include "Sound.h"
#include "Spells.h"
#include "StatusEffects.h"
#include "TextFormatting.h"
#include "Thrower.h"
#include "MenuInputHandler.h"

using namespace std;

void Engine::initSdl() {
  tracer << "Engine::initSdl()..." << endl;
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_EnableUNICODE(1);
  IMG_Init(IMG_INIT_PNG);
  tracer << "Engine::initSdl() [DONE]" << endl;
}

void Engine::initConfigAndRenderer() {
  tracer << "Engine::initConfigAndRenderer()..." << endl;
  config = new Config(this);
  renderer = new Renderer(this);
  tracer << "Engine::initConfigAndRenderer() [DONE]" << endl;
}

void Engine::cleanupSdl() {
  IMG_Quit();
  SDL_Quit();
}

void Engine::cleanupConfigAndRenderer() {
  tracer << "Engine::cleanupConfigAndRenderer()..." << endl;
//  SDL_Quit();

  delete renderer;
  delete config;
  tracer << "Engine::cleanupConfigAndRenderer() [DONE]" << endl;
}

void Engine::initAudio() {
  audio = new Audio(this);
}

void Engine::cleanupAudio() {
  delete audio;
}

void Engine::initGame() {
  tracer << "Engine::initGame()..." << endl;
  // ------- INITIALIZATIONS WHERE ORDER MAY BE IMPORTANT -------
  basicUtils = new BasicUtils(this);
  actorData = new ActorData(this);
  scrollNameHandler = new ScrollNameHandler(this);
  potionNameHandler = new PotionNameHandler(this);
  itemData = new ItemData(this);
  abilityRoll = new AbilityRoll(this);
  itemFactory = new ItemFactory(this);
  inventoryHandler = new InventoryHandler(this);
  playerBonHandler = new PlayerBonHandler(this);
  playerCreateCharacter = new PlayerCreateCharacter(this);
  player = new Player;
  player->place(Pos(config->PLAYER_START_X, config->PLAYER_START_Y), &(actorData->actorDefinitions[actor_player]), this);

  // ------- INITIALIZATIONS WHERE ORDER IS NOT IMPORTANT -------
  marker = new Marker(this);
  log = new Log(this);
  pathfinder = new Pathfinder(this);
  fov = new Fov(this);
  mapBuild = new MapBuild(this);
  mapBuildBSP = new MapBuildBSP(this);
  mapPatterns = new MapPatterns(this);
  soundEmitter = new SoundEmitter(this);
  featureData = new FeatureData(this);
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
  playerPowersHandler = new PlayerPowersHandler(this);
  knockBack = new KnockBack(this);
  examine = new Examine(this);
  characterDescr = new CharacterDescr(this);
  roomThemeMaker = new RoomThemeMaker(this);
  debugModeStatPrinter = new DebugModeStatPrinter(this);
  disarm = new Disarm(this);
  gods = new Gods(this);
  credits = new Credits(this);

  tracer << "Engine::initGame() [DONE]" << endl;
}

void Engine::cleanupGame() {
  tracer << "Engine::cleanupGame()..." << endl;

  map->clearDungeon();

  delete playerBonHandler;
  delete playerVisualMemory;
  delete itemData;
  delete map;
  delete log;
  delete pathfinder;
  delete fov;
  delete mapBuild;
  delete mapBuildBSP;
  delete mapPatterns;
  delete soundEmitter;
  delete featureData;
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
  delete actorData;
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
  delete playerPowersHandler;
  delete knockBack;
  delete examine;
  delete characterDescr;
  delete roomThemeMaker;
  delete debugModeStatPrinter;
  delete disarm;
  delete gods;
  delete credits;

  delete marker;

  tracer << "Engine::cleanupGame() [DONE]" << endl;
}

void Engine::sleep(const Uint32 DURATION) const {
  SDL_Delay(DURATION);
}



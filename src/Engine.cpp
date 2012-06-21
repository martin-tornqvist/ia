#include "Engine.h"

#include <iostream>

#include "Converters.h"

#include "Config.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "SpawnCounter.h"

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
#include "CharacterInfo.h"
#include "Close.h"
#include "PlayerAllocBonus.h"
#include "Map.h"
#include "DungeonClimb.h"
#include "DungeonMaster.h"
#include "Explosion.h"
#include "FeatureFactory.h"
#include "Fov.h"
#include "GameTime.h"
#include "Highscore.h"
#include "Interface.h"
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
#include "MapBuildSpawnItems.h"
#include "MapTemplates.h"
#include "Marker.h"
#include "Pathfinding.h"
#include "Phrases.h"
#include "PlayerCreateCharacter.h"
#include "PlayerBonuses.h"
#include "PlayerPowersHandler.h"
#include "PlayerVisualMemory.h"
#include "Populate.h"
#include "Popup.h"
#include "Postmortem.h"
#include "Query.h"
#include "Reload.h"
#include "Render.h"
#include "RenderInventory.h"
#include "SaveHandler.h"
#include "Sound.h"
#include "SpecialRoom.h"
#include "Spells.h"
#include "StatusEffects.h"
#include "TextFormatting.h"
#include "Thrower.h"
#include "Timer.h"
#include "MenuInputHandler.h"
#include "TrimTravelVector.h"

using namespace std;

void Engine::initRenderer() {
	config = new Config();
	renderer = new Renderer(this);
}

void Engine::cleanupRenderer() {
	cout << "Quiting SDL" << endl;
	SDL_Quit();

	delete renderer;
	delete config;
}

void Engine::initAudio() {
	audio = new Audio(this);
}

void Engine::cleanupAudio() {
	delete audio;
}

void Engine::initGame() {
	// ------- INITIALIZATIONS WHERE ORDER MAY BE IMPORTANT -------
	basicUtils = new BasicUtils(this);
	actorData = new ActorData(this);
	scrollNameHandler = new ScrollNameHandler(this);
	potionNameHandler = new PotionNameHandler(this);
	itemData = new ItemData(this);
	abilityRoll = new AbilityRoll(this);
	itemFactory = new ItemFactory(this);
	inventoryHandler = new InventoryHandler(this);
	playerBonusHandler = new PlayerBonusHandler(this);
	playerCreateCharacter = new PlayerCreateCharacter(this);
	player = new Player;
	player->place(coord(config->PLAYER_START_X, config->PLAYER_START_Y), &(actorData->actorDefinitions[actor_player]), this);

	// ------- INITIALIZATIONS WHERE ORDER IS NOT IMPORTANT -------
	marker = new Marker(this);
	log = new MessageLog(this);
	phrases = new Phrases(this);
	pathfinder = new Pathfinder(this);
	fov = new Fov(this);
	mapBuild = new MapBuild(this);
	mapBuildBSP = new MapBuildBSP(this);
	mapPatterns = new MapPatterns(this);
	soundEmitter = new SoundEmitter(this);
	featureData = new FeatureData(this);
	playerAllocBonus = new PlayerAllocBonus(this);
	dungeonMaster = new DungeonMaster(this);
	input = new Input(this, quitToMainMenu_);
	dungeonClimb = new DungeonClimb(this);
	actorFactory = new ActorFactory(this);
	mapTemplateHandler = new MapTemplateHandler(this);
	itemDrop = new ItemDrop(this);
	itemPickup = new ItemPickup(this);
	attack = new Attack(this);
	interfaceRenderer = new Interface(this);
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
	populate = new Populate(this);
	renderInventory = new RenderInventory(this);
	inventoryIndexes = new InventoryIndexes;
	query = new Query(this);
	mapBuildSpawnItems = new MapBuildSpawnItems(this);
	bash = new Bash(this);
	close = new Close(this);
	look = new Look(this);
	autoDescribeActor = new AutoDescribeActor(this);
	textFormatting = new TextFormatting();
	//	trapFactory = new TrapFactory(this);
	spellHandler = new SpellHandler(this);
	postmortem = new Postmortem(this);
	highScore = new HighScore(this);
	popup = new Popup(this);
	saveHandler = new SaveHandler(this);
	specialRoomHandler = new SpecialRoomHandler(this);
	jamWithSpike = new JamWithSpike(this);
	trimTravelVector = new TrimTravelVector(this);
	menuInputHandler = new MenuInputHandler(this);
	playerPowersHandler = new PlayerPowersHandler(this);
	knockBack = new KnockBack(this);
	characterInfo = new CharacterInfo(this);
}

void Engine::cleanupGame() {
	map->clearDungeon();

	delete playerBonusHandler;
	delete playerVisualMemory;
	delete itemData;
	delete map;
	delete log;
	delete phrases;
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
	delete interfaceRenderer;
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
	delete populate;
	delete renderInventory;
	delete inventoryIndexes;
	delete query;
	delete mapBuildSpawnItems;
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
	delete specialRoomHandler;
	delete bresenhamLine;
	delete playerAllocBonus;
	delete jamWithSpike;
	delete trimTravelVector;
	delete menuInputHandler;
	delete playerPowersHandler;
	delete knockBack;
	delete characterInfo;

	delete marker;
}

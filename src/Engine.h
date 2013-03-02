#ifndef ENGINE_H
#define ENGINE_H

#include "DebugMode.h"

#ifdef DEBUG
#define tracer if (0) ; else cerr
#else
#define tracer if (1) ; else cerr
#endif

#include "BasicUtils.h"

class PlayerVisualMemory;
class Renderer;
class ItemData;
class Map;
class MessageLog;
class Phrases;
class Pathfinder;
class Fov;
class MapBuild;
class MapBuildBSP;
class MapPatterns;
class SoundEmitter;
class FeatureData;
class PlayerAllocBonus;
class DungeonMaster;
class Input;
class DungeonClimb;
class ActorFactory;
class MapTemplateHandler;
class ItemDrop;
class ItemPickup;
class InventoryHandler;
class Attack;
class Interface;
class GameTime;
class MainMenu;
class ItemFactory;
class Bot;
class FeatureFactory;
class Art;
class Marker;
class ExplosionMaker;
class Thrower;
class Reload;
class BasicUtils;
class MapTests;
class Gore;
class Manual;
class FovPreCalc;
class BresenhamLine;
class PopulateItems;
class PopulateMonsters;
class PopulateTraps;
class RenderInventory;
class InventoryIndexes;
class Query;
class ActorData;
class ScrollNameHandler;
class PotionNameHandler;
class Bash;
class Close;
class Look;
class AutoDescribeActor;
class TextFormatting;
class Config;
class AbilityRoll;
class PlayerBonusHandler;
class SpellHandler;
class Postmortem;
class HighScore;
class Audio;
class Popup;
class SaveHandler;
class SpecialRoomHandler;
class JamWithSpike;
class TrimTravelVector;
class MenuInputHandler;
class PlayerPowersHandler;
class KnockBack;
class CharacterInfo;
class Examine;
class PlayerCreateCharacter;
class RoomThemeMaker;

class Player;

class Engine {
public:
	Engine(bool* quitToMainMenu) : isCheatVisionEnabled(false), quitToMainMenu_(quitToMainMenu) {
	}
	~Engine() {
	}

//  void initSDL();
	void initConfigAndRenderer();
	void initGame();
	void initAudio();
	void cleanupAudio();
	void cleanupGame();
	void cleanupConfigAndRenderer();

	void sleep(const int DURATION) const;

	PlayerVisualMemory* playerVisualMemory;
	Renderer* renderer;
	ItemData* itemData;
	Map* map;
	MessageLog* log;
	Phrases* phrases;
	Pathfinder* pathfinder;
	Fov* fov;
	MapBuild* mapBuild;
	MapBuildBSP* mapBuildBSP;
	MapPatterns* mapPatterns;
	SoundEmitter* soundEmitter;
	FeatureData* featureData;
	PlayerAllocBonus* playerAllocBonus;
	DungeonMaster* dungeonMaster;
	Input* input;
	DungeonClimb* dungeonClimb;
	ActorFactory* actorFactory;
	MapTemplateHandler* mapTemplateHandler;
	ItemDrop* itemDrop;
	ItemPickup* itemPickup;
	InventoryHandler* inventoryHandler;
	Attack* attack;
	Interface* interfaceRenderer;
	GameTime* gameTime;
	MainMenu* mainMenu;
	ItemFactory* itemFactory;
	Bot* bot;
	FeatureFactory* featureFactory;
	Art* art;
	Marker* marker;
	ExplosionMaker* explosionMaker;
	Thrower* thrower;
	Reload* reload;
	BasicUtils* basicUtils;
	Dice dice;
	MapTests* mapTests;
	Gore* gore;
	Manual* manual;
	FovPreCalc* fovPreCalc;
	BresenhamLine* bresenhamLine;
	PopulateItems* populateItems;
	PopulateMonsters* populateMonsters;
	PopulateTraps* populateTraps;
	RenderInventory* renderInventory;
	InventoryIndexes* inventoryIndexes;
	Query* query;
	ActorData* actorData;
	ScrollNameHandler* scrollNameHandler;
	PotionNameHandler* potionNameHandler;
	Bash* bash;
	Close* close;
	Look* look;
	AutoDescribeActor* autoDescribeActor;
	TextFormatting* textFormatting;
	Config* config;
	AbilityRoll* abilityRoll;
	PlayerBonusHandler* playerBonusHandler;
	SpellHandler* spellHandler;
	Postmortem* postmortem;
	HighScore* highScore;
	Audio* audio;
	Popup* popup;
	SaveHandler* saveHandler;
	JamWithSpike* jamWithSpike;
	TrimTravelVector* trimTravelVector;
	MenuInputHandler* menuInputHandler;
	PlayerPowersHandler* playerPowersHandler;
	KnockBack* knockBack;
	CharacterInfo* characterInfo;
	Examine* examine;
	PlayerCreateCharacter* playerCreateCharacter;
	RoomThemeMaker* roomThemeMaker;

	Player* player;

	bool isCheatVisionEnabled;

private:
	bool* quitToMainMenu_;
};

#endif

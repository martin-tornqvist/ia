#ifndef ENGINE_H
#define ENGINE_H

#include "DebugMode.h"

//If set to 1, the trace output in debug mode will be more verbose
//(This does not affect release mode, which has no trace output)
#define VERBOSE_TRACE 0

#ifdef DEBUG
#define trace if (0) ; else cerr
#define traceHi if (VERBOSE_TRACE == 0) ; else cerr
#else
#define trace if (1) ; else cerr
#define traceHi if (1) ; else cerr
#endif

#include "BasicUtils.h"

class PlayerVisualMemory;
class Renderer;
class ItemDataHandler;
class Map;
class Log;
class Phrases;
class Pathfinder;
class Fov;
class MapBuild;
class MapBuildBSP;
class MapPatterns;
class SoundEmitter;
class FeatureDataHandler;
class DungeonMaster;
class Input;
class DungeonClimb;
class ActorFactory;
class MapTemplateHandler;
class ItemDrop;
class ItemPickup;
class InventoryHandler;
class Attack;
class CharacterLines;
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
class ActorDataHandler;
class ScrollNameHandler;
class PotionNameHandler;
class Bash;
class Close;
class Look;
class AutoDescribeActor;
class TextFormatting;
class Config;
class AbilityRoll;
class PlayerBonHandler;
class SpellHandler;
class Postmortem;
class HighScore;
class Audio;
class Popup;
class SaveHandler;
class SpecialRoomHandler;
class JamWithSpike;
class MenuInputHandler;
class PlayerSpellsHandler;
class KnockBack;
class CharacterDescr;
class Examine;
class PlayerCreateCharacter;
class RoomThemeMaker;
class DebugModeStatPrinter;
class Disarm;
class Gods;
class Credits;
class PropDataHandler;

class Player;

class Engine {
public:
  Engine(bool* quitToMainMenu) : isCheatVisionEnabled(false),
    quitToMainMenu_(quitToMainMenu) {
  }
  ~Engine() {
  }

  void initSdl();
  void initConfigAndRenderer();
  void initGame();
  void initAudio();
  void cleanupAudio();
  void cleanupGame();
  void cleanupConfigAndRenderer();
  void cleanupSdl();

  void sleep(const Uint32 DURATION) const;

  PlayerVisualMemory* playerVisualMemory;
  Renderer* renderer;
  ItemDataHandler* itemDataHandler;
  Map* map;
  Log* log;
  Phrases* phrases;
  Pathfinder* pathfinder;
  Fov* fov;
  MapBuild* mapBuild;
  MapBuildBSP* mapBuildBSP;
  MapPatterns* mapPatterns;
  SoundEmitter* soundEmitter;
  FeatureDataHandler* featureDataHandler;
  DungeonMaster* dungeonMaster;
  Input* input;
  DungeonClimb* dungeonClimb;
  ActorFactory* actorFactory;
  MapTemplateHandler* mapTemplateHandler;
  ItemDrop* itemDrop;
  ItemPickup* itemPickup;
  InventoryHandler* inventoryHandler;
  Attack* attack;
  CharacterLines* characterLines;
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
  ActorDataHandler* actorDataHandler;
  ScrollNameHandler* scrollNameHandler;
  PotionNameHandler* potionNameHandler;
  Bash* bash;
  Close* close;
  Look* look;
  AutoDescribeActor* autoDescribeActor;
  TextFormatting* textFormatting;
  Config* config;
  AbilityRoll* abilityRoll;
  PlayerBonHandler* playerBonHandler;
  SpellHandler* spellHandler;
  Postmortem* postmortem;
  HighScore* highScore;
  Audio* audio;
  Popup* popup;
  SaveHandler* saveHandler;
  JamWithSpike* jamWithSpike;
  MenuInputHandler* menuInputHandler;
  PlayerSpellsHandler* playerSpellsHandler;
  KnockBack* knockBack;
  CharacterDescr* characterDescr;
  Examine* examine;
  PlayerCreateCharacter* playerCreateCharacter;
  RoomThemeMaker* roomThemeMaker;
  DebugModeStatPrinter* debugModeStatPrinter;
  Disarm* disarm;
  Gods* gods;
  Credits* credits;
  PropDataHandler* propDataHandler;

  Player* player;

  bool isCheatVisionEnabled;

private:
  bool* quitToMainMenu_;
};

#endif

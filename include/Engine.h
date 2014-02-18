#ifndef ENGINE_H
#define ENGINE_H

#include "DebugMode.h"

#include <SDL.h>

//-------------------------------------------------------------------
// OPTIONS
//-------------------------------------------------------------------
//Level of trace output in debug mode
// 0 : Disabled
// 1 : Standard
// 2 : Verbose
#define TRACE_LVL 1

//Enable lots of rendering and delays during map gen for demo purposes
//Comment out to disable, uncomment to enable
//#define DEMO_MODE 1
//-------------------------------------------------------------------

#ifdef DEBUG
#define trace if (TRACE_LVL < 1) ; else cerr
#define traceVerbose if (TRACE_LVL < 2) ; else cerr
#else
#define trace if (1) ; else cerr
#define traceVerbose if (1) ; else cerr
#endif

class PlayerVisualMemory;
class Renderer;
class ItemDataHandler;
class Map;
class Log;
class Phrases;
class Fov;
class MapPatterns;
class SndEmitter;
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
class GameTime;
class MainMenu;
class ItemFactory;
class Bot;
class FeatureFactory;
class Art;
class Marker;
class Thrower;
class Reload;
class Gore;
class Manual;
class LineCalc;
class BresenhamLine;
class PopulateItems;
class PopulateMonsters;
class PopulateTraps;
class InventoryIndexes;
class Query;
class ActorDataHandler;
class ScrollNameHandler;
class PotionNameHandler;
class Bash;
class Close;
class Look;
class AutoDescribeActor;
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
class Gods;
class Credits;
class PropDataHandler;
class SdlWrapper;
class Hide;

class Player;

class Engine {
public:
  Engine(bool* quitToMainMenu = NULL) : isCheatVisionEnabled_(false),
    quitToMainMenu_(quitToMainMenu) {
  }
  ~Engine() {}

  void initSdl();
  void cleanupSdl();
  void initConfig();
  void initRenderer();
  void cleanupRenderer();
  void initGame();
  void cleanupGame();
  void initAudio();
  void cleanupAudio();

  void sleep(const Uint32 DURATION) const;

  PlayerVisualMemory* playerVisualMemory;
  Renderer* renderer;
  ItemDataHandler* itemDataHandler;
  Map* map;
  Log* log;
  Phrases* phrases;
  Fov* fov;
  MapPatterns* mapPatterns;
  SndEmitter* sndEmitter;
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
  GameTime* gameTime;
  MainMenu* mainMenu;
  ItemFactory* itemFactory;
  Bot* bot;
  FeatureFactory* featureFactory;
  Art* art;
  Marker* marker;
  Thrower* thrower;
  Reload* reload;
  Gore* gore;
  Manual* manual;
  LineCalc* lineCalc;
  BresenhamLine* bresenhamLine;
  PopulateItems* populateItems;
  PopulateMonsters* populateMonsters;
  PopulateTraps* populateTraps;
  InventoryIndexes* inventoryIndexes;
  Query* query;
  ActorDataHandler* actorDataHandler;
  ScrollNameHandler* scrollNameHandler;
  PotionNameHandler* potionNameHandler;
  Bash* bash;
  Close* close;
  Look* look;
  AutoDescribeActor* autoDescribeActor;
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
  Gods* gods;
  Credits* credits;
  PropDataHandler* propDataHandler;
  SdlWrapper* sdlWrapper;
  Hide* hide;

  Player* player;

  bool isCheatVisionEnabled_;
  bool quitToMainMenu_;
};

#endif

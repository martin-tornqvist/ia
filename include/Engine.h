#ifndef ENGINE_H
#define ENGINE_H

#include "DebugMode.h" //Must be included before <assert.h>!
#include <assert.h>

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

#ifdef NDEBUG
#define trace if (1) ; else cerr
#define traceVerbose if (1) ; else cerr
#else
#define trace if (TRACE_LVL < 1) ; else cerr
#define traceVerbose if (TRACE_LVL < 2) ; else cerr
#endif

class PlayerVisualMemory;
class ItemDataHandler;
class Map;
class Log;
class Phrases;
class Fov;
class MapPatterns;
class FeatureDataHandler;
class DungeonMaster;
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
class Query;
class ActorDataHandler;
class ScrollNameHandler;
class PotionNameHandler;
class Bash;
class Close;
class Look;
class AutoDescribeActor;
class AbilityRoll;
class SpellHandler;
class Postmortem;
class HighScore;
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
  void initGame();
  void cleanupGame();

  PlayerVisualMemory* playerVisualMemory;
  ItemDataHandler* itemDataHandler;
  Map* map;
  Log* log;
  Phrases* phrases;
  Fov* fov;
  MapPatterns* mapPatterns;
  FeatureDataHandler* featureDataHandler;
  DungeonMaster* dungeonMaster;
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
  Query* query;
  ActorDataHandler* actorDataHandler;
  ScrollNameHandler* scrollNameHandler;
  PotionNameHandler* potionNameHandler;
  Bash* bash;
  Close* close;
  Look* look;
  AutoDescribeActor* autoDescribeActor;
  AbilityRoll* abilityRoll;
  SpellHandler* spellHandler;
  Postmortem* postmortem;
  HighScore* highScore;
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
  Hide* hide;

  Player* player;

  bool isCheatVisionEnabled_;
  bool quitToMainMenu_;
};

#endif

#include <cassert>

#include "Engine.h"
#include "MainMenu.h"
#include "Renderer.h"
#include "InventoryHandler.h"
#include "PlayerBonuses.h"
#include "PlayerCreateCharacter.h"
#include "ActorPlayer.h"
#include "MapGen.h"
#include "PopulateMonsters.h"
#include "DungeonClimb.h"
#include "Bot.h"
#include "Popup.h"
#include "Input.h"
#include "CharacterLines.h"
#include "Log.h"
#include "Query.h"
#include "Highscore.h"
#include "Postmortem.h"
#include "DungeonMaster.h"
#include "DebugModeStatPrinter.h"
#include "Audio.h"

#undef main
int main(int argc, char* argv[]) {
  trace << "main()..." << endl;

  bool quitToMainMenu = false;

  Engine* const engine = new Engine(&quitToMainMenu);
  engine->initSdl();
  engine->initConfig();
  engine->initRenderer();

  engine->renderer->clearScreen();
  engine->renderer->drawTextCentered(
    "Loading...", panel_screen, Pos(MAP_X_CELLS_HALF, 12),
    clrNosferatuSepiaDrk);
  engine->renderer->updateScreen();

  engine->initAudio();

  bool quitGame = false;
  while(quitGame == false) {
    engine->initGame();

    if(argc > 1) {
      const string arg1 = argv[1];
      if(arg1 == "-b") {
        engine->config->isBotPlaying = true;
      }
    }

    if(IS_DEBUG_MODE) {
      engine->debugModeStatPrinter->run();
    }

    int introMusChannel = -1;
    const GameEntry_t gameEntryType = engine->mainMenu->run(
                                        quitGame, introMusChannel);

    if(quitGame == false) {
      quitToMainMenu = false;

      if(gameEntryType == gameEntry_new) {
        if(engine->config->isBotPlaying) {
          engine->playerBonHandler->setAllBonsToPicked();
          engine->bot->init();
        }
        engine->playerCreateCharacter->run();
        engine->player->actorSpecific_spawnStartItems();

        engine->gameTime->insertActorInLoop(engine->player);

        if(engine->config->isIntroLevelSkipped == false) {
          //If intro level is used, build forest.
          engine->renderer->coverPanel(panel_screen);
          engine->renderer->updateScreen();
          MapGenIntroForest(engine).run();
        } else {
          //Else build first dungeon level
          engine->dungeonClimb->travelDown();
        }
        engine->dungeonMaster->setTimeStartedToNow();
        const TimeData& t = engine->dungeonMaster->getTimeStarted();
        trace << "Game started on: " << t.getTimeStr(time_minute, true) << endl;
      }

      engine->audio->fadeOutChannel(introMusChannel);

      engine->player->updateFov();
      engine->renderer->drawMapAndInterface();

      if(gameEntryType == gameEntry_new) {
        if(engine->config->isIntroLevelSkipped == 0) {
          string introMessage = "I stand on a cobbled forest path, ahead lies a shunned and decrepit old church building. ";
          introMessage += "From years of investigation and discreet inquiries, I know this to be the access point to the abhorred ";
          introMessage += "\"Cult of Starry Wisdom\". ";
          introMessage += "I will enter these sprawling catacombs and rob them of treasures and knowledge. ";
          introMessage += "The ultimate prize is an artifact of non-human origin called \"The shining Trapezohedron\" ";
          introMessage += "- a window to all secrets of the universe.";
          engine->popup->showMessage(introMessage, true, "The story so far...");
        }
      }

      /*
       * ========== M A I N   L O O P ==========
       */
      while(quitToMainMenu == false) {
        //------------------------------------------------ ACT
        if(engine->gameTime->getLoopSize() != 0) {
          if(engine->gameTime->getCurrentActor() == engine->player) {

            if(engine->config->isBotPlaying) {
              engine->bot->act();
            } else {
              engine->renderer->drawMapAndInterface();
              engine->input->handleMapModeInputUntilFound();
            }

          } else if(engine->player->deadState == actorDeadState_alive) {
            engine->gameTime->getCurrentActor()->newTurn();
          } else {
            engine->gameTime->endTurnOfCurrentActor();
          }
        }

        //If player has died, run postmortem, then return to main menu
        if(engine->player->deadState != actorDeadState_alive) {
          dynamic_cast<Player*>(engine->player)->waitTurnsLeft = -1;
//          engine->log->clearLog();
          engine->log->addMsg("=== I AM DEAD === (press any key to view postmortem information)", clrMessageBad);
          engine->renderer->drawMapAndInterface();
          engine->query->waitForKeyPress();
          engine->highScore->gameOver(false);
          engine->postmortem->run(&quitGame);
          quitToMainMenu = true;
        }
      }
    }
    engine->cleanupGame();
  }
  engine->cleanupAudio();
  engine->cleanupRenderer();
  engine->cleanupConfig();
  engine->cleanupSdl();

  delete engine;

  trace << "main() [DONE]" << endl;
  return 0;
}


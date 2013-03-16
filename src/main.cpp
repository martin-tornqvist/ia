#include <cassert>

#include "Engine.h"
#include "MainMenu.h"
#include "Render.h"
#include "InventoryHandler.h"
#include "PlayerBonuses.h"
#include "PlayerCreateCharacter.h"
#include "ActorPlayer.h"
#include "PlayerAllocBonus.h"
#include "MapBuild.h"
#include "PopulateMonsters.h"
#include "DungeonClimb.h"
#include "Bot.h"
#include "Popup.h"
#include "Input.h"
#include "Interface.h"
#include "Log.h"
#include "Query.h"
#include "Highscore.h"
#include "Postmortem.h"
#include "DungeonMaster.h"
#include "DebugModeStatPrinter.h"


//FILE * ctt = fopen("CON", "w" );
#undef main
int main(int argc, char* argv[]) {
  tracer << "main()..." << endl;

  bool quitToMainMenu = false;

  Engine* const engine = new Engine(&quitToMainMenu);
  engine->initConfigAndRenderer();
  engine->initAudio();

  bool quitGame = false;
  while(quitGame == false) {
    engine->initGame();

    if(argc > 1) {
      const string arg1 = argv[1];
      if(arg1 == "-b") {
        engine->config->BOT_PLAYING = true;
      }
    }

    if(IS_DEBUG_MODE) {
      engine->debugModeStatPrinter->run();
    }

    const GameEntry_t ENTRY_TYPE = engine->mainMenu->run(quitGame);

    if(quitGame == false) {
      quitToMainMenu = false;

      if(ENTRY_TYPE == gameEntry_new) {
        if(engine->config->BOT_PLAYING) {
          engine->playerBonusHandler->setAllBonusesToPicked();
          engine->bot->init();
        }
        engine->playerCreateCharacter->run();
        engine->player->actorSpecific_spawnStartItems();

        engine->gameTime->insertActorInLoop(engine->player);

        if(engine->config->SKIP_INTRO_LEVEL == false) {
          //If intro level is used, build forest.
          engine->renderer->coverRenderArea(renderArea_screen);
          engine->renderer->updateWindow();
          engine->mapBuild->buildForest();
        } else {
          //Else build first dungeon level
          engine->dungeonClimb->travelDown();
        }
        engine->dungeonMaster->setTimeStartedToNow();
        const TimeData& t = engine->dungeonMaster->getTimeStarted();
        tracer << "Game started on: " << t.getTimeStr(time_minute, true) << endl;
      }

      engine->player->FOVupdate();
      engine->renderer->drawMapAndInterface();

      if(ENTRY_TYPE == gameEntry_new) {
        if(engine->config->SKIP_INTRO_LEVEL == 0) {
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


            if(engine->config->BOT_PLAYING) {
              engine->bot->act();
            } else {
              engine->renderer->drawMapAndInterface();
              engine->input->handleMapModeInputUntilFound();
            }

          } else {
            //If not player turn, run AI
            if(engine->player->deadState == actorDeadState_alive) {
              engine->gameTime->getCurrentActor()->newTurn();
            } else {
              engine->gameTime->letNextAct();
            }
          }
        }

        //If player has died, run postmortem, then return to main menu
        if(engine->player->deadState != actorDeadState_alive) {
          dynamic_cast<Player*>(engine->player)->waitTurnsLeft = -1;
          engine->log->clearLog();
          engine->log->addMessage("=== I AM DEAD === (press any key to view postmortem information)", clrMessageBad);
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
  engine->cleanupConfigAndRenderer();
  engine->cleanupAudio();
  delete engine;
  tracer << "main() [DONE]" << endl;
  return 0;
}


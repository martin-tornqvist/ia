#include "engine.h"

#include "MainMenu.h"
#include "Renderer.h"
#include "InventoryHandler.h"
#include "PlayerBonuses.h"
#include "PlayerCreateCharacter.h"
#include "ActorPlayer.h"
#include "MapGen.h"
#include "PopulateMonsters.h"
#include "DungeonClimb.h"
#include "Popup.h"
#include "CharacterLines.h"
#include "Log.h"
#include "Query.h"
#include "Highscore.h"
#include "Postmortem.h"
#include "DungeonMaster.h"
#include "DebugModeStatPrinter.h"
#include "Audio.h"
#include "Bot.h"

#undef main
int main(int argc, char* argv[]) {
  try {
    trace << "main()..." << endl;

    bool quitToMainMenu = false;

    Engine* const eng = new Engine(&quitToMainMenu);
    eng->initSdl();
    eng->initConfig();
    eng->initRenderer();
    eng->initAudio();

    bool quitGame = false;
    while(quitGame == false) {
      eng->initGame();

      if(argc > 1) {
        const string arg1 = argv[1];
        if(arg1 == "-b") {
          eng->config->isBotPlaying = true;
        }
      }

      if(IS_DEBUG_MODE) {
        eng->debugModeStatPrinter->run();
      }

      int introMusChannel = -1;
      const GameEntry_t gameEntryType = eng->mainMenu->run(
                                          quitGame, introMusChannel);

      if(quitGame == false) {
        quitToMainMenu = false;

        if(gameEntryType == gameEntry_new) {
          if(eng->config->isBotPlaying) {
            eng->playerBonHandler->setAllTraitsToPicked();
            eng->bot->init();
          }
          eng->playerCreateCharacter->createCharacter();
          eng->player->actorSpecific_spawnStartItems();

          eng->gameTime->insertActorInLoop(eng->player);

          if(eng->config->isIntroLevelSkipped == false) {
            //If intro level is used, build forest.
            eng->renderer->coverPanel(panel_screen);
            eng->renderer->updateScreen();
            MapGenIntroForest(*eng).run();
          } else {
            //Else build first dungeon level
            eng->dungeonClimb->travelDown();
          }
          eng->dungeonMaster->setTimeStartedToNow();
          const TimeData& t = eng->dungeonMaster->getTimeStarted();
          trace << "Game started on: " << t.getTimeStr(time_minute, true) << endl;
        }

        eng->audio->fadeOutChannel(introMusChannel);

        eng->player->updateFov();
        eng->renderer->drawMapAndInterface();

        if(gameEntryType == gameEntry_new) {
          if(eng->config->isIntroLevelSkipped == 0) {
            string introMessage = "I stand on a cobbled forest path, ahead lies a shunned and decrepit old church building. ";
            introMessage += "From years of investigation and discreet inquiries, I know this to be the access point to the abhorred ";
            introMessage += "\"Cult of Starry Wisdom\". ";
            introMessage += "I will enter these sprawling catacombs and rob them of treasures and knowledge. ";
            introMessage += "The ultimate prize is an artifact of non-human origin called \"The shining Trapezohedron\" ";
            introMessage += "- a window to all secrets of the universe.";
            eng->popup->showMessage(introMessage, true, "The story so far...");
          }
        }

        //========== M A I N   L O O P ==========
        while(quitToMainMenu == false) {
          if(eng->player->deadState == actorDeadState_alive) {

            Actor* const actor = eng->gameTime->getCurrentActor();

            actor->updateColor();

            if(actor->getPropHandler()->allowAct()) {
              actor->onActorTurn();
            } else {
              if(actor == eng->player) {
                eng->renderer->drawMapAndInterface();
                eng->sleep(DELAY_PLAYER_UNABLE_TO_ACT);
              }
              eng->gameTime->endTurnOfCurrentActor();
            }
          } else {
            //Player is dead, run postmortem, then return to main menu
            dynamic_cast<Player*>(eng->player)->waitTurnsLeft = -1;
            eng->log->addMsg(
              "=== I AM DEAD === (press any key to view postmortem information)",
              clrMessageBad);
            eng->renderer->drawMapAndInterface();
            eng->query->waitForKeyPress();
            eng->highScore->gameOver(false);
            eng->postmortem->run(&quitGame);
            quitToMainMenu = true;
          }
        }
      }
      eng->cleanupGame();
    }
    eng->cleanupAudio();
    eng->cleanupRenderer();
    eng->cleanupConfig();
    eng->cleanupSdl();

    delete eng;

    trace << "main() [DONE]" << endl;

  } catch(...) {
    return 1;
  }
  return 0;
}


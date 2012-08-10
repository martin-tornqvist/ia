#include <cassert>

#include "SDL/SDL.h"

#include "Declares.h"
#include "Engine.h"
#include "MainMenu.h"
#include "Render.h"
#include "InventoryHandler.h"
#include "PlayerBonuses.h"
#include "PlayerCreateCharacter.h"
#include "ActorPlayer.h"
#include "PlayerAllocBonus.h"
#include "MapBuild.h"
#include "Populate.h"
#include "DungeonClimb.h"
#include "Bot.h"
#include "Popup.h"
#include "Input.h"
#include "Interface.h"
#include "Log.h"
#include "Query.h"
#include "Highscore.h"
#include "Postmortem.h"

//FILE * ctt = fopen("CON", "w" );
#undef main
int main(int argc, char* argv[]) {
	bool quitToMainMenu = false;

	Engine* const engine = new Engine(&quitToMainMenu);
	engine->initRenderer();
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

		const GameEntry_t ENTRY_TYPE = engine->mainMenu->run(&quitGame);

		if(quitGame == false) {
			quitToMainMenu = false;

			engine->renderer->clearRenderArea(renderArea_screen);
			engine->inventoryHandler->initPlayerSlotButtons();

			if(ENTRY_TYPE == gameEntry_new) {
				if(engine->config->BOT_PLAYING == true) {
					engine->playerBonusHandler->setAllBonusesToMax();
				}
//				engine->playerEnterName->run();
				engine->playerCreateCharacter->run();
				engine->player->actorSpecific_spawnStartItems();
				engine->playerAllocBonus->run();

				engine->gameTime->insertActorInLoop(engine->player);

				if(engine->config->SKIP_INTRO_LEVEL == false) {
					//If intro level is used, build forest.
					engine->renderer->clearRenderArea(renderArea_screen);
					engine->renderer->flip();
					engine->mapBuild->buildForest();
					engine->populate->populate();
				} else {
					//Else build first dungeon level
					engine->dungeonClimb->travelDown();
				}

				engine->bot->init();
			}

			engine->player->FOVupdate();
			//engine->gameTime->letNextAct();
			engine->renderer->clearAreaPixel(0, 0, engine->config->SCREEN_WIDTH, engine->config->SCREEN_HEIGHT);
			engine->renderer->drawMapAndInterface();
			//engine->interfaceRenderer->drawInterface();

			if(ENTRY_TYPE == gameEntry_new) {
				if(engine->config->SKIP_INTRO_LEVEL == 0) {
					string introMessage = "You stand on a cobbled forest path, a shunned and abandoned church building lies ahead. ";
					introMessage += "From years of investigation and discreet inquiries, you know this to be the access point to the abhorred ";
					introMessage += "\"Cult of Starry Wisdom\". Your plan is to infiltrate these sprawling catacombs and rob them of treasure and ";
					introMessage += "knowledge. The ultimate prize is an artifact of non-human origin called \"The shining Trapezohedron\" ";
					introMessage += "- a window to all secrets of the universe.";
					engine->popup->showMessage(introMessage);
				}
			}

			/*
			 * ========== M A I N   L O O P ==========
			 */
			while(quitToMainMenu == false) {
				//------------------------------------------------ ACT
				if(engine->gameTime->getLoopSize() != 0) {
					if(engine->gameTime->getCurrentActor() == engine->player) {

						engine->input->read();

						if(engine->config->BOT_PLAYING == true) {
							engine->bot->act();
						}

//						engine->interfaceRenderer->drawInterface();

						SDL_Delay(1);

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
					engine->log->addMessage("===YOU DIE=== (press any key to view postmortem information)", clrMessageBad);
					engine->renderer->flip();
					engine->query->waitForKeyPress();
					engine->highScore->gameOver(false);
					engine->postmortem->run(&quitGame);
					quitToMainMenu = true;
				}

				//engine->audio->updateSystem();
			}
		}
		engine->cleanupGame();
	}
	engine->cleanupRenderer();
	engine->cleanupAudio();
	delete engine;
	return 0;
}


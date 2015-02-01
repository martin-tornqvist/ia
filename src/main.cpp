#include "Init.h"

#include <SDL.h>

#include "SdlWrapper.h"
#include "Config.h"
#include "Render.h"
#include "MainMenu.h"
#include "PlayerBon.h"
#include "Bot.h"
#include "CreateCharacter.h"
#include "ActorPlayer.h"
#include "MapGen.h"
#include "MapTravel.h"
#include "DungeonMaster.h"
#include "Popup.h"
#include "Log.h"
#include "Query.h"
#include "Highscore.h"
#include "Postmortem.h"
#include "Map.h"
#include "Utils.h"

using namespace std;

#ifdef _WIN32
#undef main
#endif
int main(int argc, char* argv[])
{
    TRACE_FUNC_BEGIN;

    (void)argc;
    (void)argv;

    Init::initIO();
    Init::initGame();

    bool quitGame = false;
    while (!quitGame)
    {
        Init::initSession();

        int introMusChan = -1;
        const GameEntryMode gameEntryType = MainMenu::run(quitGame, introMusChan);

        if (!quitGame)
        {
            Init::quitToMainMenu = false;

            if (gameEntryType == GameEntryMode::newGame)
            {
                if (Config::isBotPlaying())
                {
                    PlayerBon::setAllTraitsToPicked();
                }
                CreateCharacter::createCharacter();
                Map::player->mkStartItems();

                if (Config::isIntroLvlSkipped())
                {
                    //Build first dungeon level
                    MapTravel::goToNxt();
                }
                else
                {
                    //Build forest.
                    Render::coverPanel(Panel::screen);
                    Render::updateScreen();
                    MapGen::mkIntroLvl();
                }
                DungeonMaster::setTimeStartedToNow();
                const TimeData& t = DungeonMaster::getStartTime();
                TRACE << "Game started on: " << t.getTimeStr(TimeType::minute, true)
                      << endl;
            }

            Audio::fadeOutChannel(introMusChan);

            Map::player->updateFov();
            Render::drawMapAndInterface();

            if (gameEntryType == GameEntryMode::newGame)
            {
                if (Config::isIntroLvlSkipped() == 0)
                {
                    const string msg =
                        "I stand on a cobbled forest path, ahead lies a shunned decrepit "
                        "old church building. From years of investigation and discreet "
                        "inquiries, I know this to be the access point to the abhorred "
                        "\"Cult of Starry Wisdom\". I will enter these sprawling catacombs "
                        "and rob them of treasures and knowledge. The ultimate prize is an "
                        "artifact of non-human origin called \"The shining Trapezohedron\" "
                        "- a window to all secrets of the universe.";
                    Popup::showMsg(msg, true, "The story so far...");
                }
            }

            //========== M A I N   L O O P ==========
            while (!Init::quitToMainMenu)
            {
                if (Map::player->isAlive())
                {
                    Actor* const actor = GameTime::getCurActor();

                    //Properties running on the actor's turn are not immediately applied
                    //on the actor, but instead placed in a buffer. This is to ensure
                    //that e.g. a property set to last one turn actually covers one turn
                    //(and not applied after the actor acts, and ends before the actor's
                    //next turn)
                    //The contents of the buffer are moved to the applied properties here
                    actor->getPropHandler().applyActorTurnPropBuffer();

                    actor->updateClr();

                    const bool ALLOW_ACT  = actor->getPropHandler().allowAct();
                    const bool IS_GIBBED  = actor->getState() == ActorState::destroyed;

                    if (ALLOW_ACT && !IS_GIBBED)
                    {
                        actor->onActorTurn();
                    }
                    else //Actor cannot act
                    {
                        if (actor->isPlayer())
                        {
                            SdlWrapper::sleep(DELAY_PLAYER_UNABLE_TO_ACT);
                        }
                        GameTime::tick();
                    }
                }
                else //Player is dead
                {
                    //Run postmortem, then return to main menu
                    static_cast<Player*>(Map::player)->waitTurnsLeft = -1;
                    Audio::play(SfxId::death);
                    Log::addMsg("I am dead...", clrMsgBad, false, true);
                    Log::clearLog();
                    HighScore::onGameOver(false);
                    Postmortem::run(&quitGame);
                    Init::quitToMainMenu = true;
                }
            }
        }
        Init::cleanupSession();
    }
    Init::cleanupGame();
    Init::cleanupIO();

    TRACE_FUNC_END;

    return 0;
}


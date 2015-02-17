#include "DungeonMaster.h"

#include <vector>

#include "Highscore.h"
#include "TextFormat.h"
#include "Render.h"
#include "Query.h"
#include "ActorPlayer.h"
#include "CharacterLines.h"
#include "Log.h"
#include "SdlWrapper.h"
#include "Map.h"
#include "Utils.h"
#include "CreateCharacter.h"
#include "ActorMon.h"

using namespace std;

namespace DungeonMaster
{

namespace
{

int       xpForLvl_[PLAYER_MAX_CLVL + 1];
int       clvl_  = 0;
int       xp_    = 0;
TimeData  timeStarted_;

void playerGainLvl()
{
    if (Map::player->isAlive())
    {
        clvl_++;

        Log::addMsg("Welcome to level " + toStr(clvl_) + "!", clrGreen, false, true);

        CreateCharacter::pickNewTrait(false);

        Map::player->restoreHp(999, false);
        Map::player->changeMaxHp(HP_PER_LVL, true);
        Map::player->restoreSpi(999, false);
        Map::player->changeMaxSpi(SPI_PER_LVL, true);
        Map::player->restoreShock(999, false);
    }
}

void initXpArray()
{
    xpForLvl_[0] = 0;
    xpForLvl_[1] = 0;
    for (int lvl = 2; lvl <= PLAYER_MAX_CLVL; lvl++)
    {
        xpForLvl_[lvl] = xpForLvl_[lvl - 1] + (100 * lvl);
    }
}

} //namespace

void init()
{
    clvl_ = 1;
    xp_   = 0;
    initXpArray();
}

void storeToSaveLines(vector<string>& lines)
{
    lines.push_back(toStr(clvl_));
    lines.push_back(toStr(xp_));
    lines.push_back(toStr(timeStarted_.year_));
    lines.push_back(toStr(timeStarted_.month_));
    lines.push_back(toStr(timeStarted_.day_));
    lines.push_back(toStr(timeStarted_.hour_));
    lines.push_back(toStr(timeStarted_.minute_));
    lines.push_back(toStr(timeStarted_.second_));
}

void setupFromSaveLines(vector<string>& lines)
{
    clvl_ = toInt(lines.front());
    lines.erase(begin(lines));
    xp_ = toInt(lines.front());
    lines.erase(begin(lines));
    timeStarted_.year_ = toInt(lines.front());
    lines.erase(begin(lines));
    timeStarted_.month_ = toInt(lines.front());
    lines.erase(begin(lines));
    timeStarted_.day_ = toInt(lines.front());
    lines.erase(begin(lines));
    timeStarted_.hour_ = toInt(lines.front());
    lines.erase(begin(lines));
    timeStarted_.minute_ = toInt(lines.front());
    lines.erase(begin(lines));
    timeStarted_.second_ = toInt(lines.front());
    lines.erase(begin(lines));
}

int         getCLvl()       {return clvl_;}
int         getXp()         {return xp_;}
TimeData    getStartTime()  {return timeStarted_;}

int getMonTotXpWorth(const ActorDataT& d)
{
    //K regulates player XP rate, higher -> more XP per monster
    const double K              = 0.45;

    const double HP             = d.hp;

    const double SPEED          = double(d.speed);
    const double SPEED_MAX      = double(ActorSpeed::END);
    const double SPEED_FACTOR   = 1.0 + ((SPEED / SPEED_MAX) * 0.50);

    const double SHOCK          = double(d.monShockLvl);
    const double SHOCK_MAX      = double(MonShockLvl::END);
    const double SHOCK_FACTOR   = 1.0 + ((SHOCK / SHOCK_MAX) * 0.75);

    const double UNIQUE_FACTOR  = d.isUnique ? 2.0 : 1.0;

    return ceil(K * HP * SPEED_FACTOR * SHOCK_FACTOR * UNIQUE_FACTOR);
}

void playerGainXp(const int XP_GAINED)
{
    if (Map::player->isAlive())
    {
        for (int i = 0; i < XP_GAINED; ++i)
        {
            xp_++;
            if (clvl_ < PLAYER_MAX_CLVL)
            {
                if (xp_ >= xpForLvl_[clvl_ + 1])
                {
                    playerGainLvl();
                }
            }
        }
    }
}

int getXpToNextLvl()
{
    if (clvl_ == PLAYER_MAX_CLVL) {return -1;}
    return xpForLvl_[clvl_ + 1] - xp_;
}

void playerLoseXpPercent(const int PERCENT)
{
    xp_ = (xp_ * (100 - PERCENT)) / 100;
}

void winGame()
{
    HighScore::onGameOver(true);

    Render::coverPanel(Panel::screen);
    Render::updateScreen();

    const vector<string> winMsg =
    {
        "As I touch the crystal, there is a jolt of electricity. A surreal glow "
        "suddenly illuminates the area. I feel as if I have stirred something. I notice "
        "a figure observing me from the edge of the light. It approaches me. There is "
        "no doubt in my mind concerning the nature of this entity; it is the Faceless "
        "God who dwells in the depths of the earth, it is the Crawling Chaos - "
        "Nyarlathotep! I panic for a moment. Why is it I find myself here, stumbling "
        "around in darkness? But I soon sense that this is all part of a plan, that I "
        "have proven myself worthy for something. The being beckons me to gaze into the "
        "stone.",

        "In the radiance I see visions beyond eternity, visions of unreal "
        "reality, visions of the brightest light of day and the darkest night of "
        "madness. There is only onward now, I demand to attain everything! So I make a "
        "pact with the Fiend.",

        "I now harness the shadows that stride from world to world to sow death and "
        "madness. The destinies of all things on earth, living and dead, are mine."
    };

    const int X0            = 6;
    const int MAX_W         = MAP_W - (X0 * 2);

    const int LINE_DELAY    = 40;

    int y = 2;

    for (const string& sectionMsg : winMsg)
    {
        vector<string> sectionLines;
        TextFormat::lineToLines(sectionMsg, MAX_W, sectionLines);

        for (const string& line : sectionLines)
        {
            Render::drawText(line, Panel::screen, Pos(X0, y), clrWhite, clrBlack);

            Render::updateScreen();
            SdlWrapper::sleep(LINE_DELAY);
            ++y;
        }
        ++y;
    }
    ++y;

    const string CMD_LABEL =
        "[space/esc/enter] to record high score and return to main menu";

    Render::drawTextCentered(CMD_LABEL, Panel::screen, Pos(MAP_W_HALF, y), clrMenuMedium,
                             clrBlack, true);

    Render::updateScreen();

    Query::waitForConfirm();
}

void onMonKilled(Actor& actor)
{
    ActorDataT& d = actor.getData();

    d.nrKills += 1;

    if (d.hp >= 3 && Map::player->obsessions[int(Obsession::sadism)])
    {
        Map::player->shock_ = max(0.0, Map::player->shock_ - 3.0);
    }

    Mon* const  mon = static_cast<Mon*>(&actor);

    if (!Map::player->isLeaderOf(mon))
    {
        const int MON_XP_TOT    = getMonTotXpWorth(d);
        const int XP_GAINED     = mon->hasGivenXpForSpotting_ ?
                                  max(1, MON_XP_TOT / 2) : MON_XP_TOT;
        playerGainXp(XP_GAINED);
    }
}

void onMonSeen(Actor& actor)
{
    Mon* const mon = static_cast<Mon*>(&actor);

    if (!mon->hasGivenXpForSpotting_)
    {
        mon->hasGivenXpForSpotting_ = true;
        playerGainXp(getMonTotXpWorth(mon->getData()) / 2);
    }
}

void setTimeStartedToNow()
{
    timeStarted_ = Utils::getCurTime();
}

} //DungeonMaster

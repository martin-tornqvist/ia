#include "Highscore.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include "Init.h"
#include "Converters.h"
#include "Highscore.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Popup.h"
#include "Input.h"
#include "Render.h"
#include "Utils.h"

using namespace std;

HighScoreEntry::HighScoreEntry(std::string dateAndTime, std::string name, int xp,
                               int lvl, int dlvl, int insanity, bool didWin, Bg bg) :
    dateAndTime_  (dateAndTime),
    name_         (name),
    xp_           (xp),
    lvl_          (lvl),
    dlvl_         (dlvl),
    ins_          (insanity),
    isWin_        (didWin),
    bg_           (bg) {}

int HighScoreEntry::getScore() const
{
    const double DLVL_DB      = double(dlvl_);
    const double DLVL_LAST_DB = double(DLVL_LAST);
    const double XP_DB        = double(xp_);

    const double FACTOR_XP    = 1.0 + XP_DB + (isWin_ ? (XP_DB / 5.0) : 0.0);
    const double FACTOR_DLVL  = 1.0 + (DLVL_DB / DLVL_LAST_DB);

    return int(FACTOR_XP * FACTOR_DLVL);
}

namespace HighScore
{

namespace
{

const int X_POS_DATE    = 0;
const int X_POS_NAME    = X_POS_DATE  + 19;
const int X_POS_LVL     = X_POS_NAME  + PLAYER_NAME_MAX_LEN + 2;
const int X_POS_DLVL    = X_POS_LVL   + 7;
const int X_POS_INS     = X_POS_DLVL  + 7;
const int X_POS_WIN     = X_POS_INS   + 10;
const int X_POS_SCORE   = X_POS_WIN   + 5;

void sortEntries(vector<HighScoreEntry>& entries)
{
    auto cmp = [](const HighScoreEntry & e1, const HighScoreEntry & e2)
    {
        return e1.getScore() > e2.getScore();
    };

    sort(entries.begin(), entries.end(), cmp);
}

void writeFile(vector<HighScoreEntry>& entries)
{
    ofstream file;
    file.open("data/highscores", ios::trunc);

    for (const auto entry : entries)
    {
        const string WIN_STR = entry.isWin() ? "W" : "0";

        file << WIN_STR                 << endl;
        file << entry.getDateAndTime()  << endl;
        file << entry.getName()         << endl;
        file << entry.getXp()           << endl;
        file << entry.getLvl()          << endl;
        file << entry.getDlvl()         << endl;
        file << entry.getInsanity()     << endl;
        file << int(entry.getBg())      << endl;
    }
}

void readFile(vector<HighScoreEntry>& entries)
{
    ifstream file;
    file.open("data/highscores");

    if (file.is_open())
    {
        string line = "";

        while (getline(file, line))
        {
            bool isWin                = line[0] == 'W';
            getline(file, line);
            const string dateAndTime  = line;
            getline(file, line);
            const string name         = line;
            getline(file, line);
            const int XP              = toInt(line);
            getline(file, line);
            const int LVL             = toInt(line);
            getline(file, line);
            const int DLVL            = toInt(line);
            getline(file, line);
            const int INS             = toInt(line);
            getline(file, line);
            Bg bg                     = Bg(toInt(line));
            entries.push_back(HighScoreEntry(dateAndTime, name, XP, LVL, DLVL, INS, isWin, bg));
        }
        file.close();
    }
}

void draw(const vector<HighScoreEntry>& entries, const int TOP_ELEMENT)
{
    TRACE_FUNC_BEGIN;

    Render::clearScreen();

    const Panel panel   = Panel::screen;
    const int   X_LABEL = 3;

    const string decorationLine(MAP_W, '-');

    Render::drawText(decorationLine, panel, Pos(0, 0), clrGray);

    Render::drawText(" Displaying High Scores ", panel, Pos(X_LABEL, 0), clrGray);

    Render::drawText(decorationLine, panel, Pos(0, SCREEN_H - 1), clrGray);

    Render::drawText(infoScrCmdInfo, panel, Pos(X_LABEL, SCREEN_H - 1), clrGray);

    int yPos = 1;

    Render::drawText("Ended",       panel, Pos(X_POS_DATE,    yPos), clrWhite);
    Render::drawText("Name",        panel, Pos(X_POS_NAME,    yPos), clrWhite);
    Render::drawText("Level",       panel, Pos(X_POS_LVL,     yPos), clrWhite);
    Render::drawText("Depth",       panel, Pos(X_POS_DLVL,    yPos), clrWhite);
    Render::drawText("Insanity",    panel, Pos(X_POS_INS,     yPos), clrWhite);
    Render::drawText("Win",         panel, Pos(X_POS_WIN,     yPos), clrWhite);
    Render::drawText("Score",       panel, Pos(X_POS_SCORE,   yPos), clrWhite);

    yPos++;

    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 3;

    for (
        int i = TOP_ELEMENT;
        i < int(entries.size()) && (i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR;
        i++)
    {
        const auto entry = entries[i];

        const string dateAndTime  = entry.getDateAndTime();
        const string name         = entry.getName();
        const string lvl          = toStr(entry.getLvl());
        const string dlvl         = toStr(entry.getDlvl());
        const string ins          = toStr(entry.getInsanity());
        const string win          = entry.isWin() ? "Yes" : "No";
        const string score        = toStr(entry.getScore());

        const Clr& clr = clrMenuHighlight;
        Render::drawText(dateAndTime, panel, Pos(X_POS_DATE,    yPos), clr);
        Render::drawText(name,        panel, Pos(X_POS_NAME,    yPos), clr);
        Render::drawText(lvl,         panel, Pos(X_POS_LVL,     yPos), clr);
        Render::drawText(dlvl,        panel, Pos(X_POS_DLVL,    yPos), clr);
        Render::drawText(ins + "%",   panel, Pos(X_POS_INS,     yPos), clr);
        Render::drawText(win,         panel, Pos(X_POS_WIN,     yPos), clr);
        Render::drawText(score,       panel, Pos(X_POS_SCORE,   yPos), clr);
        yPos++;
    }

    Render::updateScreen();

    TRACE_FUNC_END;
}

} //namespace

void runHighScoreScreen()
{
    vector<HighScoreEntry> entries;
    readFile(entries);

    if (entries.empty())
    {
        Popup::showMsg("No High Score entries found.", false);
        return;
    }

    sortEntries(entries);

    int topNr = 0;
    draw(entries, topNr);

    const int LINE_JUMP           = 3;
    const int NR_LINES_TOT        = entries.size();
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 3;

    //Read keys
    while (true)
    {
        draw(entries, topNr);

        const KeyData& d = Input::getInput();

        if (d.key == '2' || d.sdlKey == SDLK_DOWN || d.key == 'j')
        {
            topNr += LINE_JUMP;
            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                topNr = 0;
            }
            else
            {
                topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
            }
        }
        if (d.key == '8' || d.sdlKey == SDLK_UP || d.key == 'k')
        {
            topNr = max(0, topNr - LINE_JUMP);
        }
        if (d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE)
        {
            break;
        }
    }
}

void onGameOver(const bool IS_WIN)
{
    vector<HighScoreEntry> entries = getEntriesSorted();

    HighScoreEntry curPlayer(
        Utils::getCurTime().getTimeStr(TimeType::minute, true),
        Map::player->getNameA(),
        DungeonMaster::getXp(),
        DungeonMaster::getCLvl(),
        Map::dlvl,
        Map::player->getInsanity(),
        IS_WIN,
        PlayerBon::getBg());

    entries.push_back(curPlayer);

    sortEntries(entries);

    writeFile(entries);
}

vector<HighScoreEntry> getEntriesSorted()
{
    vector<HighScoreEntry> entries;
    readFile(entries);
    if (!entries.empty())
    {
        sortEntries(entries);
    }
    return entries;
}

} //HighScore

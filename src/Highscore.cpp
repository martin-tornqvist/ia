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

using namespace std;

namespace HighScore
{

namespace
{

const int X_POS_DATE      = 1;
const int X_POS_NAME      = X_POS_DATE + 19;
const int X_POS_SCORE     = X_POS_NAME + 14;
const int X_POS_LVL       = X_POS_SCORE + 12;
const int X_POS_DLVL      = X_POS_LVL + 8;
const int X_POS_INSANITY  = X_POS_DLVL + 8;
const int X_POS_RANK      = X_POS_INSANITY + 10;

bool isEntryHigher(const HighScoreEntry& cur,
                   const HighScoreEntry& other)
{
  return other.getScore() < cur.getScore();
}

void sortEntries(vector<HighScoreEntry>& entries)
{
  sort(entries.begin(), entries.end(), isEntryHigher);
}

void writeFile(vector<HighScoreEntry>& entries)
{
  ofstream file;
  file.open("data/highscores", ios::trunc);

  for(unsigned int i = 0; i < entries.size(); ++i)
  {
    const HighScoreEntry& entry = entries.at(i);

    const string VICTORY_STR = entry.isVictoryGame() ? "V" : "D";
    file << VICTORY_STR << endl;
    file << entry.getDateAndTime() << endl;
    file << entry.getName() << endl;
    file << entry.getXp() << endl;
    file << entry.getLvl() << endl;
    file << entry.getDlvl() << endl;
    file << entry.getInsanity() << endl;
  }
}

void readFile(vector<HighScoreEntry>& entries)
{
  ifstream file;
  file.open("data/highscores");

  if(file.is_open())
  {
    string line = "";

    while(getline(file, line))
    {
      bool isVictory = line.at(0) == 'V';
      getline(file, line);
      const string dateAndTime = line;
      getline(file, line);
      const string name = line;
      getline(file, line);
      const int XP = toInt(line);
      getline(file, line);
      const int LVL = toInt(line);
      getline(file, line);
      const int DLVL = toInt(line);
      getline(file, line);
      const int INSANITY = toInt(line);
      entries.push_back(
        HighScoreEntry(dateAndTime, name, XP, LVL, DLVL, INSANITY, isVictory));
    }
    file.close();
  }
}

void draw(const vector<HighScoreEntry>& entries, const int TOP_ELEMENT)
{
  TRACE_FUNC_BEGIN;

  Render::clearScreen();

  const int X_LABEL = 3;

  const string decorationLine(MAP_W, '-');

  Render::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  Render::drawText(" Displaying High Scores ", Panel::screen, Pos(X_LABEL, 0), clrGray);

  Render::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1), clrGray);

  Render::drawText(infoScrCmdInfo, Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);

  int yPos = 1;

  Render::drawText(
    "Ended",    Panel::screen, Pos(X_POS_DATE,     yPos), clrGray);
  Render::drawText(
    "Name",     Panel::screen, Pos(X_POS_NAME,     yPos), clrGray);
  Render::drawText(
    "Score",    Panel::screen, Pos(X_POS_SCORE,    yPos), clrGray);
  Render::drawText(
    "Level",    Panel::screen, Pos(X_POS_LVL,      yPos), clrGray);
  Render::drawText(
    "Depth",    Panel::screen, Pos(X_POS_DLVL,     yPos), clrGray);
  Render::drawText(
    "Insanity", Panel::screen, Pos(X_POS_INSANITY, yPos), clrGray);

  yPos++;

  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 3;

  for(
    int i = TOP_ELEMENT;
    i < int(entries.size()) && (i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR;
    i++)
  {
    const string dateAndTime  = entries.at(i).getDateAndTime();
    const string name         = entries.at(i).getName();
    const string score        = toStr(entries.at(i).getScore());
    const string lvl          = toStr(entries.at(i).getLvl());
    const string dlvl         = toStr(entries.at(i).getDlvl());
    const string ins          = toStr(entries.at(i).getInsanity());

    const Clr& clr = clrNosfTeal;
    Render::drawText(
      dateAndTime, Panel::screen, Pos(X_POS_DATE,      yPos), clr);
    Render::drawText(
      name,        Panel::screen, Pos(X_POS_NAME,      yPos), clr);
    Render::drawText(
      score,       Panel::screen, Pos(X_POS_SCORE,     yPos), clr);
    Render::drawText(
      lvl,         Panel::screen, Pos(X_POS_LVL,       yPos), clr);
    Render::drawText(
      dlvl,        Panel::screen, Pos(X_POS_DLVL,      yPos), clr);
    Render::drawText(
      ins + "%",   Panel::screen, Pos(X_POS_INSANITY,  yPos), clr);
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

  if(entries.empty())
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
  while(true)
  {
    draw(entries, topNr);

    const KeyData& d = Input::readKeysUntilFound();

    if(d.key == '2' || d.sdlKey == SDLK_DOWN || d.key == 'j')
    {
      topNr += LINE_JUMP;
      if(NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
      {
        topNr = 0;
      }
      else
      {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    }
    if(d.key == '8' || d.sdlKey == SDLK_UP || d.key == 'k')
    {
      topNr = max(0, topNr - LINE_JUMP);
    }
    if(d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE)
    {
      break;
    }
  }
}

void onGameOver(const bool IS_VICTORY)
{
  vector<HighScoreEntry> entries = getEntriesSorted();

  HighScoreEntry curPlayer(
    Utils::getCurTime().getTimeStr(time_minute, true),
    Map::player->getNameA(),
    DungeonMaster::getXp(),
    DungeonMaster::getCLvl(),
    Map::dlvl,
    Map::player->getInsanity(),
    IS_VICTORY);

  entries.push_back(curPlayer);

  sortEntries(entries);

  writeFile(entries);
}

vector<HighScoreEntry> getEntriesSorted()
{
  vector<HighScoreEntry> entries;
  readFile(entries);
  if(!entries.empty())
  {
    sortEntries(entries);
  }
  return entries;
}

} //HighScore

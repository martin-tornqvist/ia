#include "Highscore.h"

#include <algorithm>
#include <iostream>
#include <fstream>

#include "Converters.h"
#include "Engine.h"
#include "Highscore.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Popup.h"
#include "Input.h"
#include "Renderer.h"

const int X_POS_DATE      = 1;
const int X_POS_NAME      = X_POS_DATE + 19;
const int X_POS_SCORE     = X_POS_NAME + 14;
const int X_POS_LVL       = X_POS_SCORE + 12;
const int X_POS_DLVL      = X_POS_LVL + 8;
const int X_POS_INSANITY  = X_POS_DLVL + 8;
const int X_POS_RANK      = X_POS_INSANITY + 10;

bool HighScore::isEntryHigher(const HighScoreEntry& current,
                              const HighScoreEntry& other) {
  return other.getScore() < current.getScore();
}

vector<HighScoreEntry> HighScore::getEntriesSorted() {
  vector<HighScoreEntry> entries;
  readFile(entries);
  if(entries.empty() == false) {
    sortEntries(entries);
  }
  return entries;
}

void HighScore::gameOver(const bool IS_VICTORY) {
  vector<HighScoreEntry> entries = getEntriesSorted();

  HighScoreEntry currentPlayer(
    Utils::getCurrentTime().getTimeStr(time_minute, true),
    eng.player->getNameA(),
    eng.dungeonMaster->getXp(),
    eng.dungeonMaster->getCLvl(),
    eng.map->getDlvl(),
    eng.player->getInsanity(),
    IS_VICTORY);

  entries.push_back(currentPlayer);

  sortEntries(entries);

  writeFile(entries);
}

void HighScore::renderHighScoreScreen(const vector<HighScoreEntry>& entries,
                                      const int TOP_ELEMENT) const {
  trace << "HighScore::renderHighScoreScreen()..." << endl;

  Renderer::clearScreen();

  const int X_LABEL = 3;

  const string decorationLine(MAP_W, '-');

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

  Renderer::drawText(" 2/8, down/up to navigate | space/esc to exit ",
                         Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrGray);

  Renderer::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                         clrGray);

  Renderer::drawText(" Displaying High Scores ", Panel::screen,
                         Pos(X_LABEL, 0), clrGray);

  int yPos = 1;

  Renderer::drawText(
    "Ended",    Panel::screen, Pos(X_POS_DATE,     yPos), clrGray);
  Renderer::drawText(
    "Name",     Panel::screen, Pos(X_POS_NAME,     yPos), clrGray);
  Renderer::drawText(
    "Score",    Panel::screen, Pos(X_POS_SCORE,    yPos), clrGray);
  Renderer::drawText(
    "Level",    Panel::screen, Pos(X_POS_LVL,      yPos), clrGray);
  Renderer::drawText(
    "Depth",    Panel::screen, Pos(X_POS_DLVL,     yPos), clrGray);
  Renderer::drawText(
    "Insanity", Panel::screen, Pos(X_POS_INSANITY, yPos), clrGray);

  yPos++;

  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 3;

  for(
    int i = TOP_ELEMENT;
    i < int(entries.size()) && (i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR;
    i++) {
    const string dateAndTime  = entries.at(i).getDateAndTime();
    const string name         = entries.at(i).getName();
    const string score        = toString(entries.at(i).getScore());
    const string lvl          = toString(entries.at(i).getLvl());
    const string dlvl         = toString(entries.at(i).getDlvl());
    const string ins          = toString(entries.at(i).getInsanity());

    const SDL_Color& clr = clrNosfTeal;
    Renderer::drawText(
      dateAndTime, Panel::screen, Pos(X_POS_DATE,      yPos), clr);
    Renderer::drawText(
      name,        Panel::screen, Pos(X_POS_NAME,      yPos), clr);
    Renderer::drawText(
      score,       Panel::screen, Pos(X_POS_SCORE,     yPos), clr);
    Renderer::drawText(
      lvl,         Panel::screen, Pos(X_POS_LVL,       yPos), clr);
    Renderer::drawText(
      dlvl,        Panel::screen, Pos(X_POS_DLVL,      yPos), clr);
    Renderer::drawText(
      ins + "%",   Panel::screen, Pos(X_POS_INSANITY,  yPos), clr);
    yPos++;
  }

  Renderer::updateScreen();

  trace << "HighScore::renderHighScoreScreen() [DONE]" << endl;
}

void HighScore::runHighScoreScreen() {
  vector<HighScoreEntry> entries;
  readFile(entries);

  if(entries.empty()) {
    eng.popup->showMsg("No High Score entries found.", false);
    return;
  }

  sortEntries(entries);

  int topNr = 0;
  renderHighScoreScreen(entries, topNr);

  const int LINE_JUMP           = 3;
  const int NR_LINES_TOT        = entries.size();
  const int MAX_NR_LINES_ON_SCR = SCREEN_H - 3;

  //Read keys
  while(true) {
    renderHighScoreScreen(entries, topNr);

    const KeyboardReadRetData& d = Input::readKeysUntilFound(eng);

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topNr += LINE_JUMP;
      if(NR_LINES_TOT <= MAX_NR_LINES_ON_SCR) {
        topNr = 0;
      } else {
        topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
      }
    }
    if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topNr = max(0, topNr - LINE_JUMP);
    }
    if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      break;
    }
  }
}

void HighScore::sortEntries(vector<HighScoreEntry>& entries) {
  sort(entries.begin(), entries.end(), HighScore::isEntryHigher);
}

void HighScore::writeFile(vector<HighScoreEntry>& entries) {
  ofstream file;
  file.open("data/highscores", ios::trunc);

  for(unsigned int i = 0; i < entries.size(); i++) {
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

void HighScore::readFile(vector<HighScoreEntry>& entries) {
  ifstream file;
  file.open("data/highscores");

  if(file.is_open()) {
    string line = "";

    while(getline(file, line)) {
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

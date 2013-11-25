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
    eng->basicUtils->getCurrentTime().getTimeStr(time_minute, true),
    eng->player->getNameA(),
    eng->dungeonMaster->getXp(),
    eng->dungeonMaster->getCLvl(),
    eng->map->getDLVL(),
    eng->player->getInsanity(),
    IS_VICTORY);

  entries.push_back(currentPlayer);

  sortEntries(entries);

  writeFile(entries);
}

void HighScore::renderHighScoreScreen(const vector<HighScoreEntry>& entries,
                                      const int TOP_ELEMENT) const {
  trace << "HighScore::renderHighScoreScreen()..." << endl;

  eng->renderer->clearScreen();

  if(entries.empty()) {
    const int X0 = 1;
    const int Y0 = 4;
    eng->renderer->drawText(
      "No High Score entries found | space/esc to exit ",
      panel_screen, Pos(X0, Y0), clrWhite);
  } else {
    const string decorationLine(MAP_X_CELLS - 2, '-');

    eng->renderer->drawText(
      decorationLine, panel_character, Pos(0, 1), clrWhite);
    eng->renderer->drawText(
      " 2/8, down/up to navigate | space/esc to exit ",
      panel_character, Pos(3, 1), clrWhite);

    int yPos = 1;

    eng->renderer->drawText(
      decorationLine, panel_screen, Pos(1, yPos), clrWhite);
    eng->renderer->drawText(
      " Displaying High Scores ", panel_screen, Pos(3, yPos), clrWhite);

    yPos++;

    eng->renderer->drawText(
      "Ended", panel_screen, Pos(X_POS_DATE, yPos), clrGray);
    eng->renderer->drawText(
      "Name", panel_screen, Pos(X_POS_NAME, yPos), clrGray);
    eng->renderer->drawText(
      "Score", panel_screen, Pos(X_POS_SCORE, yPos), clrGray);
    eng->renderer->drawText(
      "Level", panel_screen, Pos(X_POS_LVL, yPos), clrGray);
    eng->renderer->drawText(
      "Depth", panel_screen, Pos(X_POS_DLVL, yPos), clrGray);
    eng->renderer->drawText(
      "Insanity", panel_screen, Pos(X_POS_INSANITY, yPos), clrGray);

    yPos++;

    for(
      int i = TOP_ELEMENT;
      i < int(entries.size()) && (i - TOP_ELEMENT) < MAP_Y_CELLS;
      i++) {
      const string dateAndTime = entries.at(i).getDateAndTime();
      trace << "HighScore: dateAndTime: " << dateAndTime << endl;
      const string name = entries.at(i).getName();
      trace << "HighScore: name: " << name << endl;
      const string score = toString(entries.at(i).getScore());
      trace << "HighScore: score: " << score << endl;
      const string lvl = toString(entries.at(i).getLvl());
      trace << "HighScore: lvl: " << lvl << endl;
      const string dlvl = toString(entries.at(i).getDlvl());
      trace << "HighScore: dlvl: " << dlvl << endl;
      const string ins = toString(entries.at(i).getInsanity());
      trace << "HighScore: ins: " << ins << endl;

      const SDL_Color clr = clrNosferatuSepia;
      eng->renderer->drawText(
        dateAndTime, panel_screen, Pos(X_POS_DATE, yPos), clr);
      eng->renderer->drawText(
        name, panel_screen, Pos(X_POS_NAME, yPos), clr);
      eng->renderer->drawText(
        score, panel_screen, Pos(X_POS_SCORE, yPos), clr);
      eng->renderer->drawText(
        lvl, panel_screen, Pos(X_POS_LVL, yPos), clr);
      eng->renderer->drawText(
        dlvl, panel_screen, Pos(X_POS_DLVL, yPos), clr);
      eng->renderer->drawText(
        ins + "%", panel_screen, Pos(X_POS_INSANITY, yPos), clr);
      yPos++;
    }
  }

  eng->renderer->updateScreen();

  trace << "HighScore::renderHighScoreScreen() [DONE]" << endl;
}

void HighScore::runHighScoreScreen() {
  vector<HighScoreEntry> entries;
  readFile(entries);

  if(entries.empty()) {
    eng->popup->showMessage("No High Score entries found.", false);
    return;
  }

  sortEntries(entries);

  int topElement = 0;
  renderHighScoreScreen(entries, topElement);

  //Read keys
  bool done = false;
  while(done == false) {
    const KeyboardReadReturnData& d = eng->input->readKeysUntilFound();

    if(d.key_ == '2' || d.sdlKey_ == SDLK_DOWN) {
      topElement =
        min(topElement + int(MAP_Y_CELLS / 5),
            int(entries.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      renderHighScoreScreen(entries, topElement);
    }
    if(d.key_ == '8' || d.sdlKey_ == SDLK_UP) {
      topElement =
        min(topElement - int(MAP_Y_CELLS / 5),
            int(entries.size()) - int(MAP_Y_CELLS));
      topElement = max(0, topElement);
      renderHighScoreScreen(entries, topElement);
    }
    if(d.sdlKey_ == SDLK_SPACE || d.sdlKey_ == SDLK_ESCAPE) {
      done = true;
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
